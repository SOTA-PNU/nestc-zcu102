# 빌드 메모

## 결론부터

번들 빌드는 보드에서 문제없이 된다. `model-compiler` 를 포함한 전체 빌드도
**보드에 설치된 LLVM 8.0.1 을 지정하면 가능하다.**

```
-DLLVM_DIR=/usr/lib/llvm-8.0/lib/cmake/llvm
```

> **2026-09-22 정정.** 이 문서는 당초 "보드에서 빌드 불가"로 결론냈으나
> 이는 틀렸다. `apt-cache search llvm` 이 6.0 까지만 보여주고
> `llvm-config --version` 이 6.0.0 을 반환해서 그렇게 판단했는데,
> `/usr/lib/llvm-8.0/` 에 clang 까지 포함된 완전한 LLVM 8 이 별도로
> 설치되어 있다. 아래 "LLVM 버전 제약" 절은 apt 저장소에 한정된 이야기다.

```bash
$ /usr/lib/llvm-8.0/bin/llvm-config --version
8.0.1
```

## LLVM 버전 제약 (apt 저장소 한정)

공식 문서(`docs/nestc/install.md`)의 요구사항은 **LLVM >= 7.0** 이고,
권장 환경은 Ubuntu 20.04 + llvm-8 이다.

이 보드는 Ubuntu 18.04 arm64이고, apt 저장소에 있는 LLVM은 다음이 전부다.

```
llvm-3.7  llvm-3.9  llvm-4.0  llvm-5.0  llvm-6.0
```

**6.0이 최대치라 요구사항을 만족할 수 없다.** 소스 빌드는 RAM 1.5 GB로는
현실성이 없다(LLVM 링크 단계만으로 수 GB를 쓴다).

### 우회는 권하지 않는다

`glow/CMakeLists.txt:176` 의 버전 검사를 `SEND_ERROR` 에서 `WARNING` 으로
낮추면 cmake는 통과한다.

```cmake
find_package(LLVM CONFIG)
if(NOT LLVM_FOUND OR LLVM_VERSION VERSION_LESS 7.0)
  message(SEND_ERROR "LLVM >= 7.0 is required to build Glow")
endif()
```

하지만 지원되지 않는 조합이므로 컴파일 단계에서 터질 가능성이 높다.
검증되지 않은 바이너리를 만드느니 호스트에서 정석대로 빌드하는 편이 낫다.

## 함정 — cmake/glow 템플릿이 덮어쓴다

`cmake/glow/CMakeLists.txt` 가 템플릿이고, cmake를 실행할 때마다
`glow/CMakeLists.txt` 를 덮어쓴다.

**한쪽만 수정하면 다음 cmake 실행에서 되돌아간다.** glow 쪽을 고쳤는데
같은 에러가 계속 난다면 이것을 의심할 것. 두 파일을 함께 고쳐야 한다.

## 함정 — 생성기가 Ninja다

`build/` 의 `CMAKE_GENERATOR` 는 **Ninja** 로 설정되어 있다.
`Makefile` 이 없으므로 `make` 는 다음과 같이 실패한다.

```
make: *** No rule to make target 'Resnet18Test'.  Stop.
```

`ninja` 를 써야 한다. 다만 `exec1/`, `realexec/` 등 일부 디렉토리에는
Makefile이 있어 혼동하기 쉽다.

```bash
grep CMAKE_GENERATOR: build/CMakeCache.txt
```

## 공식 빌드 절차 (호스트 PC)

출처: `docs/nestc/install.md`, `docs/nestc/evta.md`

### 의존성 (Ubuntu 20.04)

```bash
sudo apt-get install clang clang-8 cmake graphviz libpng-dev \
    libprotobuf-dev llvm-8 llvm-8-dev ninja-build protobuf-compiler wget \
    opencl-headers libgoogle-glog-dev libboost-all-dev \
    libdouble-conversion-dev libevent-dev libssl-dev libgflags-dev \
    libjemalloc-dev libpthread-stubs0-dev python python3-pip
pip3 install numpy decorator attrs pytest onnx scipy
pip install --upgrade protobuf
```

### 소스와 서브모듈

```bash
git clone https://github.com/etri/nest-compiler.git
cd nest-compiler
git submodule update --init --recursive glow
git submodule update --init --recursive tvm
```

`fmt` 라이브러리는 특정 커밋으로 별도 빌드가 필요하다.

```bash
git clone https://github.com/fmtlib/fmt
cd fmt && git reset --hard efe3694f150a1f307d014e68cd88350067769b19
mkdir build && cd build && cmake .. && make && sudo make install
```

### cmake

```bash
export TVM_HOME=<nest-compiler>/tvm
export PYTHONPATH=<nest-compiler>/tvm/python
export TVM_LIBRARY_PATH=<build>/tvm

cmake -G Ninja <nest-compiler> \
  -DCMAKE_BUILD_TYPE=Release \
  -DNESTC_WITH_EVTA=ON \
  -DNESTC_EVTA_BUNDLE_TEST=ON \
  -DGLOW_WITH_BUNDLES=ON \
  -DNESTC_USE_PRECOMPILED_EVTA_LIBRARY=ON
```

LLVM이 표준 경로에 없으면 `-DLLVM_DIR=/usr/lib/llvm-8/lib/cmake/llvm` 를 추가한다.

## ZCU102 전용 옵션

| 옵션 | 값 | 비고 |
|------|-----|------|
| `NESTC_EVTA_RUN_ON_ZCU102` | ON | 실기 실행 |
| `NESTC_USE_VTASIM` | OFF | 시뮬레이터 비활성화 |
| `NESTC_EVTA_RUN_WITH_GENERIC_BUNDLE` | OFF | **ResNet-18** |
| `NESTC_EVTA_RUN_WITH_GENERIC_BUNDLE` | ON | **ResNet-50** |

기본값으로 cmake를 돌리면 `Build VTA runtime with target: sim` 이 찍힌다.
실기용으로는 `NESTC_USE_VTASIM=OFF` 가 필요하다.

## 빌드 타겟

| 모델 | 타겟 |
|------|------|
| ResNet-18 | `vtaMxnetResnet18Bundle` |
| ResNet-50 | `vtaCaffe2Resnet50Bundle` |

`Resnet18Test` 같은 디렉토리 이름이 아니라 위 타겟 이름을 써야 한다.

## 건드리면 안 되는 것

`git diff` 에 커밋되지 않은 수정이 2건 잡힌다.

```
vta/bundles/Resnet18PartitionTest/CMakeLists.txt |  76 +++---
vta/bundles/Resnet18Test/CMakeLists.txt          | 106 +++++----
```

이전 작업자가 ZCU102에서 ResNet-18을 돌리려고 작업한 내용으로 보인다.
**`git checkout` 으로 되돌리면 복구할 수 없다.** `patches/` 에 백업해 두었다.

`glow/` 는 git 서브모듈이라 상위 저장소의 `git checkout glow/...` 가 듣지 않는다.
서브모듈 안으로 들어가서 처리해야 한다.
