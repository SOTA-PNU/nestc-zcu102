// CPU 단독 벤치마크용 main.
//
// 같은 NEST-C(Glow) 컴파일러의 CPU 백엔드로 만든 aarch64 번들(.o)을 보드의
// ARM Cortex-A53 에서 돌린다. VTA 는 쓰지 않는다. VTA 번들과 같은 모델·같은
// 입력으로 지연 시간과 처리량을 재어 비교하는 것이 목적이다.
//
// 입력은 서버에서 미리 전처리해 둔 float32 원시 파일(.raw)이다. 레이아웃·채널
// 순서·값 범위는 그 모델의 VTA main 과 똑같이 맞춘다(scripts/bench-cpu.sh).
//
// 빌드(보드):
//   g++ -O2 -std=c++11 -no-pie -DBUNDLE_HDR='"m.h"' -DENTRY=m -DCONFIG=m_config
//       -DIN_SYM='"data"' -DOUT_SYM='"out"' cpu_bench_main.cpp m.o -lpthread
//
// 실행:
//   ./bench <weights.bin> <반복> <스레드> <in1.raw> [in2.raw ...]
//
// 출력:
//   RESULT <파일> <argmax> <확신도>
//   LATENCY_MS <1스레드 장당 평균>            (스레드 1일 때)
//   THROUGHPUT <img/s> threads=<n>            (스레드 n 일 때)

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

#include BUNDLE_HDR

extern "C" int ENTRY(uint8_t *constantWeight, uint8_t *mutableWeight,
                     uint8_t *activations);

static const SymbolTableEntry *findSym(const char *name) {
  for (unsigned i = 0; i < CONFIG.numSymbols; i++)
    if (!strcmp(CONFIG.symbolTable[i].name, name))
      return &CONFIG.symbolTable[i];
  fprintf(stderr, "심볼 없음: %s\n", name);
  for (unsigned i = 0; i < CONFIG.numSymbols; i++)
    fprintf(stderr, "  있는 것: %s\n", CONFIG.symbolTable[i].name);
  exit(2);
}

static uint8_t *alignedAlloc(uint64_t size) {
  void *p = nullptr;
  uint64_t a = CONFIG.alignment < sizeof(void *) ? sizeof(void *) : CONFIG.alignment;
  if (posix_memalign(&p, a, size ? size : a)) {
    fprintf(stderr, "메모리 할당 실패 %llu\n", (unsigned long long)size);
    exit(2);
  }
  memset(p, 0, size);
  return (uint8_t *)p;
}

static std::vector<float> readRaw(const char *path, uint64_t n) {
  std::vector<float> v(n);
  FILE *f = fopen(path, "rb");
  if (!f || fread(v.data(), sizeof(float), n, f) != n) {
    fprintf(stderr, "입력 파일을 읽지 못했다(원소 %llu개 필요): %s\n",
            (unsigned long long)n, path);
    exit(2);
  }
  fclose(f);
  return v;
}

struct Ctx {
  uint8_t *mut;
  uint8_t *act;
};

int main(int argc, char **argv) {
  if (argc < 5) {
    fprintf(stderr, "사용: %s <weights.bin> <반복> <스레드> <in.raw>...\n", argv[0]);
    return 1;
  }
  const char *wpath = argv[1];
  int iters = atoi(argv[2]);
  int threads = atoi(argv[3]);
  std::vector<std::string> inputs(argv + 4, argv + argc);

  // 상수 가중치는 모든 스레드가 공유한다(읽기 전용).
  uint8_t *cw = alignedAlloc(CONFIG.constantWeightVarsMemSize);
  {
    FILE *f = fopen(wpath, "rb");
    if (!f || fread(cw, 1, CONFIG.constantWeightVarsMemSize, f) !=
                  CONFIG.constantWeightVarsMemSize) {
      fprintf(stderr, "가중치를 읽지 못했다: %s\n", wpath);
      return 2;
    }
    fclose(f);
  }

  const SymbolTableEntry *in = findSym(IN_SYM);
  const SymbolTableEntry *out = findSym(OUT_SYM);

  std::vector<std::vector<float>> imgs;
  for (auto &p : inputs) imgs.push_back(readRaw(p.c_str(), in->size));

  // 스레드마다 입출력·중간값 버퍼를 따로 둔다. 번들은 재진입 가능하다.
  std::vector<Ctx> ctx(threads);
  for (auto &c : ctx) {
    c.mut = alignedAlloc(CONFIG.mutableWeightVarsMemSize);
    c.act = alignedAlloc(CONFIG.activationsMemSize);
  }

  // 1) 정답 확인: 이미지마다 한 번씩
  for (size_t i = 0; i < imgs.size(); i++) {
    memcpy(ctx[0].mut + in->offset, imgs[i].data(), in->size * sizeof(float));
    int rc = ENTRY(cw, ctx[0].mut, ctx[0].act);
    if (rc) { fprintf(stderr, "번들 오류 %d\n", rc); return 3; }
    const float *o = (const float *)(ctx[0].mut + out->offset);
    uint64_t best = 0;
    for (uint64_t k = 1; k < out->size; k++) if (o[k] > o[best]) best = k;
    printf("RESULT %s %llu %f\n", inputs[i].c_str(), (unsigned long long)best, o[best]);
  }

  // 2) 시간 측정: 전체 iters 장을 스레드들이 나눠 처리한다(이미지는 돌아가며).
  //    VTA 멀티코어 측정(board-multi-run.sh)과 같은 방식이다.
  for (int t = 0; t < threads; t++)  // 데우기
    memcpy(ctx[t].mut + in->offset, imgs[0].data(), in->size * sizeof(float)),
        ENTRY(cw, ctx[t].mut, ctx[t].act);

  std::atomic<int> next(0);
  auto worker = [&](int t) {
    for (;;) {
      int k = next.fetch_add(1);
      if (k >= iters) break;
      const auto &img = imgs[k % imgs.size()];
      memcpy(ctx[t].mut + in->offset, img.data(), in->size * sizeof(float));
      ENTRY(cw, ctx[t].mut, ctx[t].act);
    }
  };
  auto t0 = std::chrono::steady_clock::now();
  std::vector<std::thread> pool;
  for (int t = 0; t < threads; t++) pool.emplace_back(worker, t);
  for (auto &th : pool) th.join();
  double ms = std::chrono::duration<double, std::milli>(
                  std::chrono::steady_clock::now() - t0).count();

  printf("TOTAL_MS %.1f images=%d threads=%d\n", ms, iters, threads);
  if (threads == 1) printf("LATENCY_MS %.1f\n", ms / iters);
  printf("THROUGHPUT %.2f threads=%d\n", iters * 1000.0 / ms, threads);
  return 0;
}
