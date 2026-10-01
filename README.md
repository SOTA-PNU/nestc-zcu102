# nestc

Xilinx ZCU102 보드의 FPGA에 올라간 **VTA(NPU)** 에서 CNN 추론을 실행하는
환경의 설정·실행·디버깅 기록.

컴파일러는 ETRI의 [NEST-C](https://github.com/etri/nest-compiler) —
Meta의 Glow를 기반으로 NPU용 코드를 생성하는 딥러닝 컴파일러다.

```
ONNX 모델
   ↓  NEST-C (model-compiler)
C++ 번들 (.cpp + weights.bin)      ← int8 양자화, VTA 명령으로 변환
   ↓  gcc (보드에서)
실행 파일
   ↓  ZCU102
VTA(FPGA) + ARM CPU 분산 추론
```

## 현재 상태

| 모델 | 파라미터 | 결과 | 추론 시간 |
|------|---------|------|----------|
| ResNet-18 | 11.7M | **정상** (207 / 340 / 281) | 110 ms |
| ResNet-50 | 25.6M | **정상** (207 / 340) — 패치 필요 | 480 ms |
| VGG-19 | 143.7M | 미해결 (번들 재생성 필요) | 10,495 ms |

ResNet-18 실측 (2026-09-21):

```
dog_207.png    → 207  confidence 0.978   110.8 ms
zebra_340.png  → 340  confidence 0.970   109.0 ms
cat_285.png    → 281  confidence 0.478   179.9 ms
```

cat이 281(tabby cat)로 나오지만 285(Egyptian cat)와 같은 품종군이고
confidence가 낮다. int8 양자화 모델이 헷갈린 정상 범위의 근사다.
첫 실행 180 ms는 VTA 드라이버 초기화와 캐시 워밍업이 포함된 값이다.

ResNet-50 은 `CPUBundle_aarch64.cpp` 의 **`transpose()` 가 미구현**
(`//TODO re-implement; return -1`)이어서 입력과 무관한 고정 출력을 냈다.
`patches/fix-aarch64-transpose.sh` 로 고치면 정상 동작하며 1,987 ms → 480 ms
로 4.1배 빨라진다. VGG-19 는 아직 미해결이다.
→ [docs/05-model-debugging.md](docs/05-model-debugging.md)

### ResNet-50 을 쓰려면

```bash
bash patches/fix-aarch64-transpose.sh          # transpose() 구현 이식
cd <빌드디렉토리>
cmake . -DNESTC_EVTA_RUN_WITH_GENERIC_BUNDLE=OFF \
        -DLLVM_DIR=/usr/lib/llvm-8.0/lib/cmake/llvm
make vtaCaffe2Resnet50Bundle
```

## 빠른 시작

보드를 부팅하고 SSH로 접속한 뒤:

```bash
cd /home/xilinx/nest-compiler/origin/vta/bundles/Resnet18Test
sudo ./vtaMxnetResnet18Bundle \
  /home/xilinx/nest-compiler/glow/tests/images/imagenet/dog_207.png
```

`sudo` 는 필수다. VTA 런타임이 FPGA와 DMA하기 위해 여는 `/dev/xlnk` 가
root 전용이라, 일반 사용자로 실행하면 `Failed to open /dev/xlnk` 로 끝난다.

입력 이미지도 필수 인자다. 없으면 `Loaded images size in bytes is: 0` 후
세그폴트가 난다.

부팅 절차는 [docs/01-boot-setup.md](docs/01-boot-setup.md),
실행 상세는 [docs/02-run-resnet18.md](docs/02-run-resnet18.md).

## 번들 구조 — 이 프로젝트의 핵심

각 모델의 `CMakeLists.txt` 는 두 갈래로 나뉜다.

```cmake
if(NESTC_USE_VTASIM)
    # model-compiler 로 ONNX → 번들 생성 (Glow + LLVM >= 7.0 필요)
else()
    # wget 으로 이미 생성된 번들을 다운로드
endif()
```

보드는 **다운로드 경로**를 쓴다. 받아온 `.cpp` 를 `*Main.cpp` 와 함께
gcc로 컴파일할 뿐이므로 **LLVM이 필요 없다.**

번들 빌드는 보드에서 문제없이 된다 — ResNet-50을 실제로 빌드해 확인했다.

새 모델을 직접 컴파일하려면 `model-compiler` 가 필요하고, 이쪽은
LLVM >= 7.0 을 요구한다. `apt` 저장소는 LLVM 6.0 이 최대지만
**보드에 LLVM 8.0.1 이 별도로 설치되어 있다** (`/usr/lib/llvm-8.0/`,
clang 포함). cmake 에 다음을 주면 된다.

```
-DLLVM_DIR=/usr/lib/llvm-8.0/lib/cmake/llvm
```

즉 호스트 PC 없이 보드에서 모델을 직접 컴파일할 수 있다.

```bash
model-compiler -g \
  -model=<model>.onnx \
  -backend=VTA \
  -emit-bundle=<출력디렉토리> \
  -bundle-api=dynamic \
  -model-input-name=<입력텐서명>,float,[1,3,224,224] \
  -load-profile=<calib>.yaml \
  -quantization-schema=symmetric_with_power2_scale \
  -keep-original-precision-for-nodes=SoftMax
```

`symmetric_with_power2_scale` 은 VTA가 2의 거듭제곱 스케일만 다루기 때문이고,
SoftMax는 float으로 남긴다.

## 환경

| 항목 | 값 |
|------|-----|
| 보드 | Xilinx ZCU102 Rev1.0 (XCZU9EG) |
| OS | PYNQ Linux (Ubuntu 18.04) |
| 커널 | 4.14.0-xilinx-v2018.3 aarch64 |
| RAM | 1.5 GB (+1 GB swap) |
| SD | 128 GB 카드, 64 GB 파티션 (rootfs 59 GiB, 15 GB 사용) |
| LLVM | 6.0 (apt) + **8.0.1** (`/usr/lib/llvm-8.0/`, clang 포함) |
| 빌드도구 | cmake 3.10.2, ninja, gcc 7.3.0 |

## 문서

| 문서 | 내용 |
|------|------|
| [01-boot-setup.md](docs/01-boot-setup.md) | 부트 스위치, UART 포트 매핑, 부팅 흐름, SSH |
| [02-run-resnet18.md](docs/02-run-resnet18.md) | 추론 실행, sudo가 필요한 이유, 결과 해석 |
| [03-build-notes.md](docs/03-build-notes.md) | 빌드 옵션, LLVM 제약, cmake 함정들 |
| [04-backup.md](docs/04-backup.md) | SD카드 백업 절차와 복원 |
| [05-model-debugging.md](docs/05-model-debugging.md) | 모델별 실패 원인 분석과 수정안 |

## 저장소 구성

```
.
├── docs/         설정·실행·빌드·백업·디버깅 문서
├── patches/      nest-compiler의 커밋 안 된 수정분과 참고 소스
├── scripts/      실행 스크립트
└── backups/      SD카드 백업 (.gitignore — 로컬 전용)
```

`backups/` 는 저장소에 올리지 않는다. 용량이 14 GB로 GitHub 제한을 넘고,
`system-rest.tar` 에는 SSH 호스트 키와 `/etc/shadow` 등 인증정보가 들어 있다.

## 주의사항

### 커밋되지 않은 수정이 있다

`nest-compiler` 작업 트리에 이전 작업자의 수정 2건이 커밋 없이 남아 있다.

```
vta/bundles/Resnet18Test/CMakeLists.txt            (106줄)
vta/bundles/Resnet18PartitionTest/CMakeLists.txt   (76줄)
```

`git checkout` 으로 되돌리면 복구할 수 없다. `patches/` 에 백업해 두었다.
저장소가 shallow clone 이라 `git log` 에는 커밋 하나(`ccdbfef 2021-10-16`)
밖에 없다.

`vta/bundles/Vgg19Test/` 는 아예 git에 추적되지 않는다. nest-compiler
원본에 없고 이전 작업자가 추가한 디렉토리다.

### cmake 템플릿이 덮어쓴다

`cmake/glow/CMakeLists.txt` 가 템플릿이라, cmake를 실행할 때마다
`glow/CMakeLists.txt` 를 덮어쓴다. 한쪽만 고치면 다음 실행에서 되돌아간다.

### 생성기가 Ninja다

`build/` 의 생성기는 Ninja이므로 `make` 가 통하지 않는다
(`No rule to make target`). `exec1/`, `realexec/` 등에는 Makefile이 있어
`make` 가 된다. 헷갈리기 쉬우니 `CMAKE_GENERATOR` 를 확인할 것.

```bash
grep CMAKE_GENERATOR: build/CMakeCache.txt
```

## 주의 — 실행 중 Ctrl+C 금지

VTA 프로그램을 강제 종료하면 `destroyVTARuntime()` 이 호출되지 않아 FPGA 와
xlnk 드라이버가 정리되지 않는다. 이후 모든 실행이 멈추며 **재부팅해야만
풀린다.** 증상은 프로그램이 끝나지 않고 `time` 의 `sys` 시간이 비정상적으로
큰 것(실측 25분 중 sys 20분)이며, dmesg 에는 아무 에러도 남지 않는다.

## 다음 과제

- VGG-19 해결 — `transpose` 패치 적용 후 aarch64 로 재시도,
  안 되면 `model-compiler` 로 번들 재생성 (보정 프로파일 파일명 버그 수정)
- ETRI 업스트림에 `transpose()` 미구현 버그 보고
- `NESTC_EVTA_MULTI` 로 멀티 코어 EVTA 활용 (번들 재생성 필요)
- `partition_profile_*` 번들로 VTA/CPU 분담 비율 측정

## 참고

- [etri/nest-compiler](https://github.com/etri/nest-compiler) — NEST-C 본체
- [etri/nest-data](https://github.com/etri/nest-data) — ONNX 모델, VTA 라이브러리, 비트스트림
- [NEST-C: A deep learning compiler framework for heterogeneous computing systems with AI accelerators](https://onlinelibrary.wiley.com/doi/10.4218/etrij.2024-0139) — ETRI Journal, 2024
- [Apache TVM VTA](https://tvm.apache.org/docs/topic/vta/index.html)
