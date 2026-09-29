# 모델별 동작 상태와 원인 분석

2026-09-22 조사.

## 요약

세 모델 중 ResNet-18만 정상 동작한다. 나머지 둘은 **입력 전처리 설정이
모델과 맞지 않아** 실패하며, 둘 다 `main.cpp` 수정만으로 고칠 수 있다.

| 모델 | 출처 | 값 범위 | 채널 순서 | 결과 |
|------|------|---------|----------|------|
| ResNet-18 | MXNet | 0~255 | BGR | 정상 |
| ResNet-50 | Caffe2 | 0~1 | RGB | 입력 무관 고정 출력 |
| VGG-19 | Caffe2 | 0~1 | BGR | 균등분포 |

## 실측 결과

### ResNet-18 — 정상

```
dog_207.png    → 207, confidence 0.978, 110.8 ms
zebra_340.png  → 340, confidence 0.970, 109.0 ms
cat_285.png    → 281, confidence 0.478, 179.9 ms
```

cat이 281(tabby cat)로 나오지만 285(Egyptian cat)와 같은 고양이 품종군이고
confidence가 낮다. 양자화 모델이 헷갈린 정상 범위의 근사다.

### ResNet-50 — 입력과 무관하게 고정 출력

```
dog_207.png    → 619, confidence 0.083585, 482.6 ms
zebra_340.png  → 619, confidence 0.083585, 480.8 ms
cat_285.png    → 619, confidence 0.083585, 481.9 ms
```

**세 이미지의 결과와 confidence가 소수점까지 동일하다.** 출력이 입력에
의존하지 않는다는 뜻이다.

추론 시간 482 ms는 ResNet-18(110 ms)의 4.4배로, 연산량 비율을 고려하면
타당한 수치다. **연산 경로(VTA)는 정상이고 입력만 망가진 상태.**

### VGG-19 — 균등분포

```
dog_207.png    → 4, confidence 0.001002, 11126.5 ms
```

confidence 0.001은 1000개 클래스에 확률이 고르게 퍼졌다는 뜻으로,
네트워크가 아무것도 판별하지 못하는 상태다.

추론 시간 11초는 ResNet-18의 100배로, 연산량 비(약 11배)를 훨씬 넘는다.
VGG-19의 FC 레이어(fc6만 1억 파라미터)가 VTA에 맞지 않아 상당 부분이
ARM CPU로 폴백된 것으로 보인다.

## 원인 — 입력 전처리 불일치

`main.cpp`의 `loadImagesAndPreprocess()` 안에서 입력 범위를 정한다.

```cpp
float scale = ((range.second - range.first) / 255.0);
float bias  = range.first;
imageT[...] = float(ptr[...]) * scale + bias;
```

### 값 범위

```cpp
// mxnet_exported_resnet18BundleMain.cpp:181-184  (정상)
//std::pair<float, float> range = std::make_pair(0., 1.0);
std::pair<float, float> range = std::make_pair(0., 255.0);

// caffe2_resnet50Main.cpp:175  (고장)
std::pair<float, float> range = std::make_pair(0., 1.0);

// vgg19caffe2BundleMain.cpp:181-184  (고장)
std::pair<float, float> range = std::make_pair(0., 1.0);
//std::pair<float, float> range = std::make_pair(0., 255.0);
```

ResNet-18에는 주석이 남아 있다. 이전 작업자가 같은 문제를 겪고
`0~1`을 주석 처리한 뒤 `0~255`로 바꾼 흔적이다. VGG-19는 정반대로
`0~255` 쪽이 주석 처리되어 있다.

양자화 보정 프로파일(`*_calib_*.yaml`)은 특정 입력 범위를 전제로
각 레이어의 값 분포를 기록한다. 0~255 기준으로 보정된 모델에 0~1 값을
넣으면 **모든 값이 양자화 최하단에 몰려 같은 정수로 뭉개진다.**
입력이 상수가 되므로 출력도 상수가 된다 — ResNet-50의 증상 그대로다.

### 채널 순서

```cpp
// ResNet-18, VGG-19 — BGR (뒤집음)
imageT[getXYZ(imageDims, row_n, col_n, 2)] = float(ptr[0]) * scale + bias;  // R → ch2
imageT[getXYZ(imageDims, row_n, col_n, 1)] = float(ptr[1]) * scale + bias;  // G → ch1
imageT[getXYZ(imageDims, row_n, col_n, 0)] = float(ptr[2]) * scale + bias;  // B → ch0

// ResNet-50 — RGB (그대로)
imageT[getXYZ(imageDims, row_n, col_n, 0)] = float(ptr[0]) * scale + bias;
imageT[getXYZ(imageDims, row_n, col_n, 1)] = float(ptr[1]) * scale + bias;
imageT[getXYZ(imageDims, row_n, col_n, 2)] = float(ptr[2]) * scale + bias;
```

Caffe2 계열 모델은 관례상 **BGR, 0~255**를 기대한다.

## 수정 방안

### VGG-19 — 한 줄

`vta/bundles/Vgg19Test/vgg19caffe2BundleMain.cpp` 181~184행의 주석을
뒤바꾼다. 채널 순서는 이미 BGR로 맞다.

```cpp
//std::pair<float, float> range = std::make_pair(0., 1.0);
std::pair<float, float> range = std::make_pair(0., 255.0);
```

### ResNet-50 — 두 군데

`vta/bundles/Resnet50Test/caffe2_resnet50Main.cpp`

1. 175행: `make_pair(0., 1.0)` → `make_pair(0., 255.0)`
2. 145~153행: 채널 배치를 BGR로 (ResNet-18 쪽 코드를 그대로 가져오면 된다)

### 중요 — 호스트 빌드가 필요 없다

`main.cpp` 는 **보드에서 컴파일되는 파일**이다. 번들(`resnet50.cpp`,
`weights.bin`)은 손대지 않으므로 `model-compiler` 도, LLVM 8 호스트
환경도 필요 없다. 수정 후 `make <타겟>` 만 다시 돌리면 2~3분이면 끝난다.

```bash
cd /home/xilinx/nest-compiler/exec1
make vtaCaffe2Resnet50Bundle
```

## 별개 문제 — VGG-19 보정 파일 이름 불일치

`vta/bundles/Vgg19Test/CMakeLists.txt` 에 버그가 있다. 받을 때와 쓸 때의
파일명이 다르다.

```cmake
# 6행 — vggprofile.yaml 을 받아서 vgg19-caffe2-9.yaml 로 저장
wget .../vggprofile.yaml -O ${CMAKE_CURRENT_BINARY_DIR}/vgg19-caffe2-9.yaml

# 33행 — 그런데 vggprofile.yaml 을 읽으려 함 (존재하지 않음)
-load-profile=${CMAKE_CURRENT_BINARY_DIR}/vggprofile.yaml
```

ResNet-50 은 같은 자리가 일치한다.

```cmake
wget .../resnet50_calib_2.yaml -O ${...}/resnet50_calib_2.yaml
-load-profile=${...}/resnet50_calib_2.yaml
```

이 버그는 `NESTC_USE_VTASIM=ON` (로컬 번들 생성) 경로에만 영향을 준다.
보드에서 실행한 번들은 다운로드 경로로 받은 것이므로, 범위 수정만으로
VGG-19 가 살아날 수도 있다. 안 되면 보정 프로파일부터 다시 만들어야 한다.

## 번들 출처

| 모델 | 번들 출처 | 신뢰도 |
|------|----------|--------|
| ResNet-18 | `github.com/purity2583/ds` (이전 작업자) | 동작 확인됨 |
| ResNet-50 | ETRI GitLab `nestc-data` 공식 | 공식 |
| VGG-19 | `github.com/purity2583/dd` (이전 작업자) | 검증 안 됨 |

ResNet-18 의 다운로드 주소는 커밋되지 않은 로컬 수정으로 바뀌어 있다
(`patches/Resnet18Test-CMakeLists.patch` 참고). 원래는 ETRI GitLab 을
가리켰다.

## 다음 과제

- ResNet-50, VGG-19 전처리 수정 후 재빌드·검증
- 세 모델의 추론 시간·정확도 비교 (아키텍처별 VTA 적합성)
- VGG-19 가 11초 걸리는 이유 분석 — 어느 레이어가 CPU 로 폴백되는지
  (`partition_profile_*` 번들 활용)

---

# 추가 조사 (2026-09-22 저녁)

## VGG-19 — 입력 범위 수정으로는 해결되지 않음

`vgg19caffe2BundleMain.cpp` 181~184행의 주석을 뒤바꿔 `0~255` 를 적용하고
재빌드했으나 결과가 전혀 달라지지 않았다.

```
수정 전: Result 4, Confidence 0.001002, 11126.5 ms
수정 후: Result 4, Confidence 0.001002, 10688.6 ms
```

수정이 반영되지 않은 것이 아니다. 확인한 내용:

- `vgg19caffe2BundleMain.cpp.o` 타임스탬프(10:36)가 소스 수정(10:34)보다 이후
- `link.txt` 가 `vgg19caffe2BundleMain.cpp.o` 를 링크 대상으로 사용

따라서 **번들 자체의 양자화가 망가진 상태**로 봐야 한다. 보정 프로파일
없이 생성된 모델은 각 레이어의 스케일이 이미 엉터리로 굳어져 있어,
실행 시점의 입력 전처리를 아무리 맞춰도 살아나지 않는다.

앞서 지적한 `Vgg19Test/CMakeLists.txt` 의 보정 파일명 불일치
(`vggprofile.yaml` 로 받아 `vgg19-caffe2-9.yaml` 로 저장 후 전자를 읽음)가
실제 원인일 가능성이 높다.

**VGG-19 를 고치려면 번들을 다시 생성해야 한다.**

## 보드에 LLVM 8.0.1 이 설치되어 있다

`link.txt` 에서 발견:

```
/usr/lib/llvm-8.0/lib/libLLVMSupport.a
/usr/lib/llvm-8.0/lib/libLLVMDemangle.a
```

확인:

```bash
$ /usr/lib/llvm-8.0/bin/llvm-config --version
8.0.1
$ ls /usr/lib/llvm-8.0/bin/ | head
bugpoint  c-index-test  clang  clang++  clang-8  ...
```

**`03-build-notes.md` 의 "보드에서 빌드 불가" 결론은 틀렸다.**
`apt-cache search llvm` 이 6.0 까지만 보여주고 `llvm-config --version` 이
6.0.0 을 반환해서 그렇게 판단했으나, `/usr/lib/llvm-8.0/` 에 clang 까지
포함된 완전한 LLVM 8 이 별도로 설치되어 있다.

cmake 에 다음을 주면 요구사항(LLVM >= 7.0)을 만족한다.

```
-DLLVM_DIR=/usr/lib/llvm-8.0/lib/cmake/llvm
```

이것이 열어주는 것:

- `model-compiler` 를 보드에서 빌드 가능
- ONNX 모델을 직접 번들로 변환 가능 (`NESTC_USE_VTASIM=ON` 경로)
- VGG-19 번들 재생성으로 수정 가능
- 임의의 모델을 보드에서 바로 컴파일 가능 — **호스트 PC 불필요**

단 RAM 1.5GB 에서 Glow 전체 빌드가 통과할지는 별개 문제다. `-j1` 로
천천히 돌리고 swap 을 늘려야 할 수 있다.

## ResNet-50 — 입력 범위 수정으로도 해결되지 않음

`caffe2_resnet50Main.cpp:175` 을 `make_pair(0., 255.0)` 으로 바꾸고
재빌드했으나 결과가 동일했다.

```
수정 전: dog 619 / zebra 619, 모두 confidence 0.083585
수정 후: dog 619 / zebra 619, 모두 confidence 0.083585
```

빌드 반영 확인:

- `caffe2_resnet50Main.cpp.o` 타임스탬프(10:41)가 소스 수정 이후
- `link.txt` 가 해당 오브젝트를 링크 대상으로 사용

VGG-19 와 같은 결론이다. **실행 시점의 입력 전처리 수정으로는 두 모델 모두
고칠 수 없다.** 번들 생성 단계의 양자화가 문제이므로 번들을 다시 만들어야 한다.

채널 순서(BGR) 수정은 아직 시도하지 않았다. 다만 범위 수정에도 출력이
소수점까지 동일했다는 점은 입력이 네트워크에 전혀 반영되지 않고 있음을
시사하므로, 채널 순서만으로 달라질 가능성은 낮아 보인다.

## 다음 작업 순서 제안

1. `-DLLVM_DIR=/usr/lib/llvm-8.0/lib/cmake/llvm` 로 cmake 재설정 후
   `model-compiler` 빌드 (RAM 1.5GB 이므로 `-j1`, swap 확보 필요)
2. `NESTC_USE_VTASIM=ON` 경로로 ResNet-50 번들을 직접 생성해 비교
   — ETRI 배포 번들이 이 하드웨어/런타임 조합과 맞지 않을 가능성 점검
3. VGG-19: 보정 파일명 버그를 고치고 올바른 프로파일로 번들 재생성
4. 여전히 안 되면 `partition_profile_*` 번들로 어느 레이어에서 값이
   무너지는지 추적

## 미해결 의문

`gpu_0_data_0` 의 크기가 150528 로 출력되는데, 이는 224x224x3 의 원소 수이고
바이트로는 602112(float32)다. main 이 복사하는 바이트 수와 일치하므로
메모리 레이아웃 자체는 맞는 것으로 보인다. 그럼에도 입력이 출력에 전혀
영향을 주지 않는 이유는 아직 설명되지 않았다.

---

# 조사 결과 (2026-09-22 저녁, 웹 조사)

보드 실험이 아니라 저장소·문서 조사 결과다. **확인된 것과 추론을 구분해 적는다.**

## 확인됨 — 공식 문서가 LLVM 8 경로를 명시한다

`docs/nestc/evta.md` 의 ZCU102 빌드 예시에 다음이 들어 있다.

```
-DLLVM_DIR=/usr/lib/llvm-8.0/lib/cmake/llvm
-DNESTC_USE_VTASIM=OFF
-DNESTC_USE_PRECOMPILED_BUNDLE=ON
-DNESTC_WITH_EVTA=ON
-DCMAKE_BUILD_TYPE=Release
```

보드에 LLVM 8.0.1 이 설치된 것은 우연이 아니라 **문서가 요구하는 구성**이다.
`03-build-notes.md` 의 "빌드 불가" 결론이 틀렸음이 이것으로 확정된다.

## 확인됨 — ResNet-18 과 ResNet-50 은 빌드 조건이 다르다

| | ResNet-18 | ResNet-50 |
|---|---|---|
| `NESTC_EVTA_RUN_WITH_GENERIC_BUNDLE` | OFF | **ON** |
| 비트스트림 | **호환 비트스트림 필요** | 언급 없음 |

evta.md 에서 "호환 비트스트림을 먼저 다운로드하라"는 지시는 **ResNet-18 절에만**
있고, 지정된 파일은 `zcu102_1x16_i8w8a32_16_16_19_18.bit` 다.

우리는 ResNet-50 을 `GENERIC_BUNDLE=OFF` 인 `exec1/` 에서 빌드했다.
**빌드 조건이 문서와 다르다.**

## 확인됨 — nest-data 의 비트스트림 목록

`https://github.com/etri/nest-data/tree/master/bitstreams`

```
ultra96_1x16_i8w8a32_15_15_18_17.bit
ultra96_1x16_i8w8a32_16_15_18_17.bit
zcu102_1x16_i8w8a32_15_15_18_17.bit
zcu102_1x16_i8w8a32_16_15_18_17.bit
zcu102_1x16_i8w8a32_16_16_19_18.bit    ← evta.md 가 ResNet-18 용으로 지정
zcu102_1x32_i8w8a32_15_15_18_17.bit
zcu102_2_1x16_i8w8a32_15_15_18_17.bit
zcu102_2_1x16_i8w8a32_16_15_18_17.bit
zcu102_4_1x16_i8w8a32_16_15_18_17.bit
```

**VTA 설정이 여러 종류라는 것이 핵심이다.** 명명 규칙(추론):
`1x16` = LOG_BATCH 0 x LOG_BLOCK 4, `i8w8a32` = 입력/가중치 8bit + 누산기 32bit,
뒤 네 숫자 = LOG_UOP / LOG_INP / LOG_WGT / LOG_ACC 버퍼 크기.

번들은 생성 시점의 VTA 설정에 맞춰 만들어지므로, **FPGA 에 올라간 비트스트림과
설정이 다르면 연산 결과가 무의미해진다.** 우리 BOOT.BIN(2020-06) 에 어떤 설정이
들어 있는지는 확인되지 않았다.

## 주의 — 내가 한 ResNet-50 수정이 틀렸을 수 있다

조사에 따르면 상위 저장소의 ResNet-50 은 `range = (0., 1.0)` + BGR 이 **정상**이고,
양자화 보정도 `0to1` 기준으로 되어 있다는 지적이 나왔다. 사실이면
`255.0` 으로 바꾼 것은 **개악**이다.

다만 이는 GitHub 최신 main 기준이고 **보드의 체크아웃은 2021년 버전**이라
다를 수 있다. 직접 확인하지 못했으므로 단정하지 않는다.

**내일 먼저 할 일: 원복하고 원래 상태의 출력을 다시 확인할 것.**

```bash
cd /home/xilinx/nest-compiler/vta/bundles/Resnet50Test
cp caffe2_resnet50Main.cpp.bak caffe2_resnet50Main.cpp
grep -n "make_pair" caffe2_resnet50Main.cpp     # (0., 1.0) 확인
```

## 다른 가설 — 심볼 테이블 프리픽스 오매칭

`main.cpp` 의 `getWeightVar()` 가 `strncmp` 프리픽스 비교로 첫 일치 심볼을
반환한다. Release 빌드(`-DNDEBUG`)에서는 관련 assert 가 꺼지므로, 입력 데이터가
엉뚱한 심볼 위치에 복사될 수 있다. 그러면 네트워크는 입력을 무시하고
상수를 출력한다 — ResNet-50 의 증상과 일치한다.

ResNet-50 main 은 이미 `printWeightVars()` 로 심볼 테이블을 출력한다.

```
config.numSymbols = 2
gpu_0_data_0     offset 0        size 150528
gpu_0_softmax_1  offset 602112   size 1000
```

`150528` 이 원소 수인지 바이트인지 확인이 필요하다. 바이트라면 main 이 복사하는
602112 바이트와 4배 차이가 나며 **입력이 출력 영역을 덮어쓰게 된다.**

## 확인 불가

- ETRI 배포 ResNet-50 번들이 어떤 VTA 설정으로 만들어졌는지 (S3 접근 차단)
- ResNet-50 이 ZCU102 에서 동작했다는 기록 (GitHub Issues, 논문 모두 접근 차단)
- VTA 런타임이 하드웨어 설정을 읽어오는 API 존재 여부
  (상위 TVM 의 `pynq_driver.cc` 에는 없음. EVTA 가 추가했는지는 불명)

## 내일 확인할 것 — 우선순위

1. **ResNet-50 전처리 원복** 후 재측정 (내 수정이 개악이었는지 판정)

2. **번들과 하드웨어의 VTA 설정 비교** — 가장 유력한 가설
   ```bash
   cd /home/xilinx/nest-compiler
   grep -rE "VTA_LOG_(BATCH|BLOCK|UOP|INP|WGT|ACC)" \
     vta/vtalib/include/zcu102/vta/hw_spec_const.h
   md5sum origin/vta/bundles/Resnet18Test/VTARuntime.h \
          exec1/vta/bundles/Resnet50Test/VTARuntime.h \
          eee/vta/bundles/Vgg19Test/VTARuntime.h
   ```
   `VTARuntime.h` 가 서로 다르면 번들마다 다른 VTA 설정을 전제한 것이다.

3. **비트스트림 신원 확인**
   ```bash
   strings /path/to/BOOT.BIN | grep -iE "zcu102_|i8w8a32|1x16"
   ```

4. **심볼 테이블 해석** — `150528` 의 단위 확인, 입출력 영역 겹침 여부

5. 문서대로 `GENERIC_BUNDLE=ON` 으로 ResNet-50 재빌드
   (지금은 OFF 인 `exec1/` 에서 빌드했다)


---

# 해결 (2026-09-29)

## ResNet-50 — 해결됨

원인은 **CPU 폴백 구현체**였다. 입력 전처리도 레이아웃도 아니었다.

```
dog_207.png    → Result 207   Confidence 0.583738   1984 ms
zebra_340.png  → Result 340   Confidence 0.991000   1990 ms
```

### 원인

`vta/vtalib/CMakeLists.txt:112`

```cmake
if(NOT NESTC_EVTA_RUN_WITH_GENERIC_BUNDLE)
    add_definitions(-DVTA_RUN_ON_AARCH64)
    add_library(VTABundle lib/Bundle/VTABundle.cpp lib/Bundle/CPUBundle_aarch64.cpp)
    INCLUDE_DIRECTORIES(/usr/lib/llvm-8.0/include)
else()
    add_library(VTABundle lib/Bundle/VTABundle.cpp lib/Bundle/CPUBundle_generic.cpp)
endif()
```

VTA가 처리하지 못하는 연산은 CPU로 떨어지는데, 이 옵션이 **CPU 구현체를 통째로 교체한다.**

- `OFF` → `CPUBundle_aarch64.cpp` (ARM 최적화). ResNet-18 은 이걸로 정상
- `ON` → `CPUBundle_generic.cpp` (범용 참조 구현). ResNet-50 은 이게 필요

`docs/nestc/evta.md` 가 ResNet-18 은 `OFF`, ResNet-50 은 `ON` 으로 모델마다 다르게
지정한 이유가 이것이다. 우리는 `OFF` 인 `exec1/` 에서 빌드해서 틀린 조합이었다.

### 해결 절차

```bash
cd /home/xilinx/nest-compiler/exec1
cmake . -DNESTC_EVTA_RUN_WITH_GENERIC_BUNDLE=ON         -DLLVM_DIR=/usr/lib/llvm-8.0/lib/cmake/llvm
make vtaCaffe2Resnet50Bundle
```

기존 빌드 디렉토리를 재구성하면 바뀐 부분만 다시 빌드하므로 몇 분이면 끝난다.
`LLVM_DIR` 을 명시해야 LLVM 6.0 검사에 걸리지 않는다.

빌드 로그에 `CPUBundle_generic.cpp.o` 가 보이면 제대로 교체된 것이다.

### 대가 — 18배 느려짐

| 모델 | CPU 구현 | 추론 시간 |
|------|---------|----------|
| ResNet-18 | aarch64 | 110 ms |
| ResNet-50 | generic | 1,987 ms |

연산량 차이는 2.3배(1.8 → 4.1 GFLOPs)뿐인데 시간은 18배다. `generic` 이 ARM
최적화 없는 참조 구현이라 CPU 폴백 구간이 크게 느려진다. **정확도와 속도를
맞바꾼 구성**이며, 이 자체가 비교 실험의 소재가 된다.

## VGG-19 — 미해결, 번들 재생성 필요

CPU 구현을 교체해도 해결되지 않았다.

```
aarch64 + 0~255 : Result 4, Confidence 0.001002, 11,127 ms
generic + 0~255 : Result 6, Confidence 0.001003, 18,168 ms
generic + 0~1   : Result 6, Confidence 0.001003, 18,177 ms
generic + 0~1, zebra : Result 6, Confidence 0.001003, 18,174 ms   ← 입력 무관
```

CPU 구현을 바꾸니 결과값이 4에서 6으로 변했다. 연산 경로는 실제로 달라졌다는
뜻이다. 그러나 confidence 는 여전히 0.001 = 1/1000 으로 균등분포이고,
**개와 얼룩말이 소수점까지 동일하다.** 입력이 결과에 반영되지 않는다.

### 소거된 원인

| 요인 | 결과 |
|------|------|
| 입력 범위 (0~1 / 0~255) | 영향 없음 |
| 채널 순서 | 이미 BGR (ResNet-18 과 동일) |
| NCHW/NHWC 레이아웃 | ResNet-50 에서 무관함이 확인됨 |
| CPU 구현 (aarch64 / generic) | 결과값만 변할 뿐 균등분포 유지 |
| VTA 설정 (`VTARuntime.h`) | ResNet-18 과 md5 동일 — 같은 설정 |

### 남은 원인 — 보정 프로파일 누락

`vta/bundles/Vgg19Test/CMakeLists.txt` 의 파일명 불일치가 유력하다.

```cmake
# 6행 — vggprofile.yaml 을 받아서 vgg19-caffe2-9.yaml 로 저장
wget .../vggprofile.yaml -O ${CMAKE_CURRENT_BINARY_DIR}/vgg19-caffe2-9.yaml

# 33행 — 존재하지 않는 vggprofile.yaml 을 읽으려 함
-load-profile=${CMAKE_CURRENT_BINARY_DIR}/vggprofile.yaml
```

보정 없이 양자화하면 레이어별 스케일이 엉터리로 굳어져 출력이 평평해진다.
실행 시점에는 고칠 수 없고 **번들을 다시 생성해야 한다.**

VGG-19 는 `git status` 에서 미추적(`??`)으로 나오는, 이전 작업자가 추가한
디렉토리다. ETRI 가 검증한 구성이 아니다.

## 운영상 주의 — Ctrl+C 금지

VTA 프로그램을 Ctrl+C 로 강제 종료하면 `destroyVTARuntime()` 이 호출되지 않아
FPGA 와 xlnk 드라이버가 정리되지 않는다. 이후 모든 실행이 멈추며,
**재부팅해야만 풀린다.**

증상: 프로그램이 끝나지 않고 `time` 의 `sys` 시간이 비정상적으로 크다
(실측 25분 중 sys 20분). dmesg 에는 아무 에러도 남지 않는다.

이 문제로 25분을 날리고, 그 상태에서 한 실험을 "패치 때문"으로 잘못 판단한 적이
있다. 끊었다면 다음 실험 전에 반드시 재부팅할 것.

## 복원 시 누락 — swap

백업에서 `--exclude=/var/swap` 으로 제외했기 때문에 복원한 카드에는 스왑이 없다
(`Swap: 0B`). 원본은 1GB 였다. RAM 1.5GB 환경에서는 다시 만들어주는 편이 안전하다.

```bash
sudo fallocate -l 1G /var/swap
sudo chmod 600 /var/swap
sudo mkswap /var/swap
sudo swapon /var/swap
```

## 현재 상태 종합

| 모델 | 결과 | Confidence | 시간 | CPU 구현 | 빌드 위치 |
|------|------|-----------|------|---------|----------|
| ResNet-18 | 207 / 340 / 281 | 0.978 / 0.970 / 0.478 | 110 ms | aarch64 | `origin/` |
| ResNet-50 | 207 / 340 | 0.584 / 0.991 | 1,987 ms | generic | `exec1/` |
| VGG-19 | 6 (고정) | 0.001 | 18,174 ms | generic | `eee/` |

**주의:** `exec1/` 과 `eee/` 는 이제 `GENERIC_BUNDLE=ON` 으로 재구성되어
`libVTABundle.a` 가 generic 구현으로 바뀌었다. 이 디렉토리들에서
ResNet-18 을 빌드하면 느려질 수 있다. ResNet-18 은 `origin/` 을 쓸 것.

## 다음 과제

1. VGG-19: `model-compiler` 빌드 후 올바른 보정 프로파일로 번들 재생성
2. ResNet-50 이 왜 `generic` 을 요구하는지 — `CPUBundle_aarch64.cpp` 의
   어느 연산이 잘못 동작하는지 추적 (ResNet-50 에만 쓰이는 연산일 것)
3. 세 모델 추론 시간 비교 시 CPU 구현체가 다르다는 점을 반드시 명시
4. `partition_profile_*` 번들로 VTA/CPU 분담 비율 측정

---

# 근본 원인 규명 및 성능 복구 (2026-09-29)

## 결론 — aarch64 의 `transpose()` 가 미구현이었다

`vta/vtalib/lib/Bundle/CPUBundle_aarch64.cpp:4582`

```cpp
int transpose(int8_t *input, int8_t *output, dim_t inDim0, dim_t inDim1, dim_t inDim2, dim_t inDim3,
              dim_t outDim0, dim_t outDim1, dim_t outDim2, dim_t outDim3,
              unsigned_t shf0, unsigned_t shf1, unsigned_t shf2, unsigned_t shf3) {
//TODO re-implement
  return -1;
}
```

**함수 본문이 `return -1` 한 줄이고 출력 버퍼에 아무것도 쓰지 않는다.**

ResNet-50 은 네트워크 끝(avgpool 뒤, FC 앞)에서 `transpose()` 를 1회 호출한다.
aarch64 빌드에서는 이 호출이 아무 일도 하지 않으므로 출력 버퍼에 초기화되지 않은
값이 남고, 이후 FC 와 softmax 가 그 값을 처리해 **입력과 무관한 고정 결과**
(619, confidence 0.083585)를 낸다.

ResNet-18 은 `transpose()` 를 호출하지 않는다(`transpose_nhwc2vtaio` /
`transpose_vtaio2nhwc` 만 사용). 그래서 aarch64 로도 정상 동작했다.

### 범인을 좁힌 방법

두 번들의 연산 호출 횟수를 비교했다.

| 연산 | ResNet-18 | ResNet-50 |
|------|-----------|-----------|
| `convolution_wo_tr` | 19 | 52 |
| `transpose_nhwc2vtaio` / `vtaio2nhwc` | 19 / 19 | 52 / 52 |
| `relu` | 8 | 16 |
| `quantize` `dequantize` `maxpool` `avgpool` | 각 1 | 각 1 |
| **`transpose`** | **0** | **1** |
| **`softmax`** | **0** | **1** |

개수만 다른 연산은 용의선상에서 제외된다. ResNet-18 이 conv 19개로 정상이라면
52개라고 갑자기 틀릴 이유가 없다. ResNet-50 에만 있는 두 연산 중 `softmax` 는
양쪽 구현이 모두 존재했고, `transpose` 만 스텁이었다.

```bash
grep -oE "\b(convolution_wo_tr|transpose[a-z_0-9]*|relu|softmax|maxpool|avgpool|quantize|dequantize)\s*\(" \
  <번들>.cpp | sort | uniq -c | sort -rn
```

## 수정과 결과

`CPUBundle_generic.cpp:493` 의 구현을 aarch64 쪽으로 이식했다
(`patches/fix-aarch64-transpose.sh`). generic 원본은 `i1~l1` 포인터를
switch 문으로 설정하면서 범위를 벗어난 `shf` 값에 대해 초기화되지 않는
경고가 있어, 배열 인덱싱과 범위 검사로 정리했다.

```
수정 전 (generic) : 207 / 340,  confidence 0.584 / 0.991,  1,987 ms
수정 후 (aarch64) : 207 / 340,  confidence 0.685 / 0.992,    480 ms
```

**정확도를 유지하면서 4.1배 빨라졌다.** confidence 도 소폭 올랐는데, 두 구현의
수치 처리가 미세하게 다르기 때문이다.

이는 ETRI 원본의 버그이며 업스트림 기여 대상이다.

## generic 과 aarch64 의 성능 차이

같은 ResNet-18 을 두 구현으로 측정했다.

```
aarch64 (origin/) :   110 ms,  confidence 0.977876
generic (exec1/)  : 1,492 ms,  confidence 0.980803
                    ─────────
                    13.6 배
```

이 측정으로 ResNet-50 이 느려 보였던 이유가 분해된다.

```
1,492 ms  ResNet-18 generic
1,987 ms  ResNet-50 generic
          ────────
          1.33 배  ← 순수 모델 크기 차이 (연산량 2.3배 대비 효율적)
```

즉 18배로 보였던 차이는 **13.6배가 generic 탓, 1.33배만 모델 탓**이었다.
서로 다른 CPU 구현으로 측정한 값을 비교하면 안 된다.

## 프로파일링은 불가

`NESTC_EVTA_PROFILE=ON` 은 컴파일되지 않는다.

```
pynq_driver.h:68:2: error: #error PERF_MON cannot be used for MULTI-VTA
```

이 보드의 VTA 구성이 멀티 코어라 하드웨어 성능 카운터를 쓸 수 없다.
역으로 **비트스트림에 EVTA 가 여러 개 올라가 있다는 방증**이기도 하다
(ResNet-18 번들의 `VTARuntime.h` 가 코어 4개용인 것과 일치).

측정이 필요하면 같은 모델을 다른 설정으로 돌려 비교하는 방식을 쓸 것.

## 최종 상태

| 모델 | CPU 구현 | 결과 | Confidence | 시간 | 빌드 위치 |
|------|---------|------|-----------|------|----------|
| ResNet-18 | aarch64 | 207 / 340 / 281 | 0.978 / 0.970 / 0.478 | 110 ms | `origin/` |
| ResNet-50 | aarch64 (패치) | 207 / 340 | 0.685 / 0.992 | 480 ms | `exec1/` |
| VGG-19 | generic | 6 (고정) | 0.001 | 18,174 ms | `eee/` |

ResNet-18 과 ResNet-50 이 같은 조건(aarch64)이 되어 공정한 비교가 가능하다.

## VGG-19 에 남은 가능성

`transpose` 수정 후 VGG-19 를 aarch64 로 다시 시도해볼 가치가 있다. 다만
VGG-19 는 generic 으로도 균등분포였으므로 원인이 다를 가능성이 높다
(보정 프로파일 누락). 확인 순서:

1. `eee/` 를 `GENERIC_BUNDLE=OFF` 로 재구성 후 재빌드·실행
2. 그래도 안 되면 `model-compiler` 로 번들 재생성

## 다음 과제

- VGG-19 재시도 (위 순서)
- `transpose()` NEON 최적화 — 480 ms 에서 더 줄일 여지는 크지 않아 보임
- ETRI 업스트림에 `transpose()` 미구현 버그 보고
- `NESTC_EVTA_MULTI` 로 멀티 코어 활용 (번들 재생성 필요)
