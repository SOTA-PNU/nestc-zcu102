#!/usr/bin/env python3
"""
VTA 백엔드에 Concat 지원을 추가한다.

Glow 는 Concat 을 AllocActivation + TouchInst + InsertTensorInst 로 로우어링한다.
VTA 백엔드는 인터프리터 경로에만 구현이 있고(VTANodes.cpp:1406), 번들(AOT)
경로인 VTASave.cpp 에는 없다. 그래서 보정 프로파일은 만들어지는데 번들 생성에서
깨진다. 게다가 default 분기가 llvm_unreachable 이라 Release 빌드에서는 정의되지
않은 동작이 되어 원인이 가려진다.

이 스크립트는 네 가지를 한다.

  1. vta/vtalib/include/Bundle/VTABundle.h  insert_tensor 선언 추가
  2. vta/vtalib/lib/Bundle/CPUBundle_{generic,aarch64,x86}.cpp  구현 추가
  3. lib/Backends/VTA/VTASave.cpp  saveInsertTensorInst 와 디스패치 두 분기 추가
  4. lib/Backends/VTA/VTASave.cpp  default 분기를 llvm_unreachable 에서
     이름을 출력하고 종료하는 것으로 교체

멱등하다. 이미 적용되어 있으면 아무 일도 하지 않는다.
어느 한 파일에서 기준점을 찾지 못하면 아무것도 바꾸지 않고 멈춘다.

사용법:
    python3 patch_concat.py <nest-compiler 루트>
"""
import os
import shutil
import sys
import time

MARKER = "nestc_insert_tensor_patch"

# ── 1. 헤더 선언 ──────────────────────────────────────────────────────
H_ANCHOR = "int transpose(int8_t *input, int8_t *output,"

H_ADD = """/* %s
 * Concat 로우어링이 만드는 InsertTensorInst 를 위한 런타임.
 * 차원 수가 가변이라 transpose 처럼 개별 인자로 풀지 않고 배열로 받는다.
 * offsetDim 은 Glow 시그니처와 맞추기 위한 것이며 사용하지 않는다.
 */
int insert_tensor(int8_t *tensor, int8_t *slice, const dim_t *offset,
                  const dim_t *tensorDim, const dim_t *sliceDim,
                  dim_t numDimsTensor, dim_t numDimsSlice,
                  dim_t offsetDim, dim_t count, dim_t axis);

""" % MARKER

# ── 2. 구현 ───────────────────────────────────────────────────────────
# Glow 원본(libjit.cpp:155)은 차원 수마다 루프를 펼쳐 6~1차원을 따로 쓴다.
# 같은 일을 자리올림 카운터로 하면 짧아지고 차원 수에 무관해진다.
IMPL = """/* %s */
static inline dim_t nestc_flat_index(const dim_t *dims, dim_t n,
                                     const dim_t *coord) {
  dim_t idx = 0;
  dim_t i;
  for (i = 0; i < n; i++) {
    idx = idx * dims[i] + coord[i];
  }
  return idx;
}

int insert_tensor(int8_t *tensor, int8_t *slice, const dim_t *offset,
                  const dim_t *tensorDim, const dim_t *sliceDim,
                  dim_t numDimsTensor, dim_t numDimsSlice,
                  dim_t offsetDim, dim_t count, dim_t axis) {
  dim_t S[6];
  dim_t C[6];
  dim_t total = 1;
  dim_t c, e, i;
  int k;

  (void)offsetDim;

  /* Glow 가 좌표 버퍼를 6으로 잡아 두었다. 그 이상은 지원하지 않는다. */
  if (numDimsSlice > 6 || numDimsTensor != numDimsSlice) {
    return -1;
  }
  if (axis >= numDimsSlice) {
    return -1;
  }

  for (i = 0; i < numDimsSlice; i++) {
    total *= sliceDim[i];
  }
  if (total == 0 || count == 0) {
    return 0;
  }

  for (c = 0; c < count; c++) {
    /* count 는 같은 slice 를 axis 방향으로 여러 번 이어 붙일 때 쓰인다. */
    const dim_t countAxisOffset = c * sliceDim[axis];

    for (i = 0; i < numDimsSlice; i++) {
      S[i] = 0;
    }

    for (e = 0; e < total; e++) {
      for (i = 0; i < numDimsSlice; i++) {
        C[i] = S[i] + offset[i] + ((i == axis) ? countAxisOffset : 0);
      }
      tensor[nestc_flat_index(tensorDim, numDimsTensor, C)] =
          slice[nestc_flat_index(sliceDim, numDimsSlice, S)];

      /* 자리올림. 마지막 차원부터 증가시킨다. */
      for (k = (int)numDimsSlice - 1; k >= 0; k--) {
        if (++S[k] < sliceDim[k]) {
          break;
        }
        S[k] = 0;
      }
    }
  }
  return 0;
}

""" % MARKER

IMPL_ANCHOR = "int transpose(int8_t *input, int8_t *output,"

# ── 3. VTASave.cpp 생성 함수 ──────────────────────────────────────────
SAVE_ANCHOR = "void saveReluInst(const glow::ReluInst *Inst, std::string *bundle,"

SAVE_FUNC = """/* %s */
void saveInsertTensorInst(const glow::InsertTensorInst *Inst,
                          std::string *bundle, VTASaveContext *ctx) {
  auto src = Inst->getSrc();
  auto dest = Inst->getDest();

  auto srcWeight = static_cast<WeightVar *>(src);
  if (srcWeight->getMutability() == glow::WeightVar::MutabilityKind::Mutable) {
    addSymbolEntryGenBundle(srcWeight, bundle, ctx);
  }
  auto destWeight = static_cast<WeightVar *>(dest);
  if (destWeight->getMutability() == glow::WeightVar::MutabilityKind::Mutable) {
    addSymbolEntryGenBundle(destWeight, bundle, ctx);
  }

  auto destDims = dest->dims();
  auto srcDims = src->dims();
  auto offsets = Inst->getOffsets();

  // 차원과 오프셋은 컴파일 시점 상수다. 리터럴 배열로 박는다.
  // 한 번들에 여러 개가 생기므로 이름이 겹치지 않게 일련번호를 붙인다.
  static int nestcInsertTensorSeq = 0;
  std::string tag = "_it" + std::to_string(nestcInsertTensorSeq++);

  auto emitArray = [&](const char *suffix, llvm::ArrayRef<glow::dim_t> vals) {
    bundle->append("  const dim_t ");
    bundle->append(tag);
    bundle->append(suffix);
    bundle->append("[] = {");
    for (size_t i = 0; i < vals.size(); i++) {
      if (i) {
        bundle->append(", ");
      }
      bundle->append(std::to_string(vals[i]));
    }
    bundle->append("};\\n");
  };

  emitArray("_dd", destDims);
  emitArray("_sd", srcDims);
  emitArray("_of", offsets);

  auto emitName = [&](glow::Value *v, WeightVar *w) {
    if (w->getMutability() == glow::WeightVar::MutabilityKind::Mutable) {
      auto ste = addSymbolEntry(w, ctx);
      bundle->append(ste.name);
    } else {
      bundle->append(v->getName());
    }
  };

  bundle->append("  insert_tensor(");
  emitName(dest, destWeight);
  bundle->append(", ");
  emitName(src, srcWeight);
  bundle->append(", ");
  bundle->append(tag + "_of, ");
  bundle->append(tag + "_dd, ");
  bundle->append(tag + "_sd, ");
  bundle->append(std::to_string(destDims.size()) + ", ");
  bundle->append(std::to_string(srcDims.size()) + ", ");
  bundle->append(std::to_string(offsets.size()) + ", ");
  bundle->append(std::to_string(Inst->getCount()) + ", ");
  bundle->append(std::to_string(Inst->getAxis()));
  bundle->append(");\\n");
}

""" % MARKER

# ── 3-2. 디스패치 + 4. default 분기 ───────────────────────────────────
DISPATCH_OLD = """    default:
      std::string msg = I.getKindName();
      msg.append(" is an unhandled instruction");

      llvm_unreachable(msg.c_str());
    }"""

DISPATCH_NEW = """    /* %s */
    case Kinded::Kind::InsertTensorInstKind: {
      auto I2 = llvm::cast<InsertTensorInst>(&I);
      saveInsertTensorInst(I2, &bundle, &ctx);
      break;
    }
    // TouchInst 는 버퍼가 초기화되지 않았음을 알리는 표시일 뿐이다.
    // Glow 의 다른 백엔드도 전부 그냥 넘긴다.
    case Kinded::Kind::TouchInstKind:
      break;
    default: {
      // 원래는 llvm_unreachable 이었다. Release 빌드에서 NDEBUG 가 켜지면
      // 그 매크로는 최적화 힌트로만 남아 정의되지 않은 동작이 된다.
      // 미지원 명령을 만나도 조용히 이상하게 동작하므로 이름을 찍고 멈춘다.
      llvm::errs() << "VTA backend: unhandled instruction "
                   << I.getKindName() << "\\n";
      std::exit(1);
    }
    }""" % MARKER


def patch(path, pieces):
    """pieces: [(기준문자열, 넣을내용, 앞에넣을지)] 또는 [(옛것, 새것, None)]"""
    try:
        src = open(path, encoding="utf-8").read()
    except OSError as e:
        return None, f"열 수 없음: {e}"

    if MARKER in src:
        return None, "이미 패치됨"

    out = src
    for anchor, add, before in pieces:
        if before is None:
            if anchor not in out:
                return None, f"기준점 없음: {anchor[:50]}..."
            out = out.replace(anchor, add, 1)
        else:
            if anchor not in out:
                return None, f"기준점 없음: {anchor[:50]}..."
            out = out.replace(anchor, add + anchor, 1)
    return out, None


def main():
    if len(sys.argv) != 2:
        print(__doc__)
        return 2
    root = sys.argv[1]

    targets = [
        (os.path.join(root, "vta/vtalib/include/Bundle/VTABundle.h"),
         [(H_ANCHOR, H_ADD, True)]),
        (os.path.join(root, "vta/vtalib/lib/Bundle/CPUBundle_generic.cpp"),
         [(IMPL_ANCHOR, IMPL, True)]),
        (os.path.join(root, "vta/vtalib/lib/Bundle/CPUBundle_aarch64.cpp"),
         [(IMPL_ANCHOR, IMPL, True)]),
        (os.path.join(root, "vta/vtalib/lib/Bundle/CPUBundle_x86.cpp"),
         [(IMPL_ANCHOR, IMPL, True)]),
        (os.path.join(root, "lib/Backends/VTA/VTASave.cpp"),
         [(SAVE_ANCHOR, SAVE_FUNC, True),
          (DISPATCH_OLD, DISPATCH_NEW, None)]),
    ]

    # 먼저 전부 검사한다. 하나라도 실패하면 아무것도 바꾸지 않는다.
    planned = []
    skipped = []
    for path, pieces in targets:
        out, err = patch(path, pieces)
        if err == "이미 패치됨":
            skipped.append(path)
            continue
        if err:
            print(f"실패 {path}\n  {err}", file=sys.stderr)
            print("아무것도 바꾸지 않았다.", file=sys.stderr)
            return 1
        planned.append((path, out))

    if not planned:
        print("전부 이미 패치되어 있다.")
        return 0

    stamp = time.strftime("%Y%m%d-%H%M%S")
    for path, out in planned:
        bak = f"{path}.bak.{stamp}"
        shutil.copy2(path, bak)
        with open(path, "w", encoding="utf-8") as f:
            f.write(out)
        print(f"패치 {path}  (백업 {os.path.basename(bak)})")
    for path in skipped:
        print(f"건너뜀 {path}  (이미 패치됨)")

    print("\n다음 — 컨테이너에서 컴파일러를 다시 빌드한다:")
    print("  cd /root/nestc/build && ninja model-compiler image-classifier")
    return 0


if __name__ == "__main__":
    sys.exit(main())
