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
