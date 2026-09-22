# ResNet-18 추론 실행

## 실행

```bash
cd /home/xilinx/nest-compiler/origin/vta/bundles/Resnet18Test
sudo ./vtaMxnetResnet18Bundle \
  /home/xilinx/nest-compiler/glow/tests/images/imagenet/dog_207.png
```

출력 예:

```
Loaded weights of size: 12003168 from the file mxnet_exported_resnet18.weights.bin
Loaded image: .../imagenet/dog_207.png
Loaded images size in bytes is: 602112
Result: 207
Confidence: 0.977876
Inference time: 110.817ms
```

## sudo가 필요한 이유

VTA 런타임은 FPGA와 DMA로 데이터를 주고받기 위해 `/dev/xlnk` 를 연다.
이 디바이스는 root 전용이다.

```
crw------- 1 root root 244, 0 /dev/xlnk
```

일반 사용자로 실행하면 즉시 이렇게 끝난다.

```
Failed to open /dev/xlnk
```

## 입력 이미지는 필수 인자다

인자 없이 실행하면 가중치는 읽지만 이미지가 0개라 세그폴트가 난다.

```
Loaded weights of size: 12003168 ...
Loaded images size in bytes is: 0
Segmentation fault
```

`Loaded images size in bytes is: 602112` 가 정상값이다.
224 x 224 x 3 x 4 바이트(float32 RGB) = 602112.

## 사용 가능한 테스트 이미지

`glow/tests/images/imagenet/` 에 세 개가 있다.

| 파일 | ImageNet 클래스 |
|------|----------------|
| cat_285.png | 285 (Egyptian cat) |
| dog_207.png | 207 (golden retriever) |
| zebra_340.png | 340 (zebra) |

## 실행 인자를 모를 때

각 번들 디렉토리의 `CTestTestfile.cmake` 에 원래 등록된 실행 명령이 그대로 남아 있다.

```bash
cat CTestTestfile.cmake
```

```cmake
add_test(vtaMxnetResnet18Bundle "sh" "-c"
  "/home/xilinx/nest-compiler/origin/vta/bundles/Resnet18Test/vtaMxnetResnet18Bundle
   /home/xilinx/nest-compiler/glow/tests/images/imagenet/cat_285.png")
set_tests_properties(vtaMxnetResnet18Bundle PROPERTIES LABELS "NESTC;ZCU102")
```

`LABELS "NESTC;ZCU102"` 로 이 테스트가 ZCU102 실기용임을 알 수 있다.
다른 번들도 같은 방법으로 실행법을 확인할 수 있다.

## 번들 구성

```
origin/vta/bundles/Resnet18Test/
├── vtaMxnetResnet18Bundle              실행 파일 (3.8 MB)
├── mxnet_exported_resnet18.cpp         생성된 번들 소스 (40 KB)
├── mxnet_exported_resnet18.h
├── mxnet_exported_resnet18.weights.bin 가중치 (12 MB)
└── VTARuntime.h
```

가중치가 **12,003,168 바이트**인데, ResNet-18의 파라미터 수는 11,689,512개다.

| 가정 | 예상 크기 | 실제와 비교 |
|------|----------|-----------|
| float32 (4 B) | 46,758,048 B | 4배 차이, 아님 |
| **int8 (1 B)** | **11,689,512 B** | 31만 B 차이 |

int8 가정과 거의 일치한다. 남는 31만 바이트는 bias(int32)와 레이어별
양자화 스케일·오프셋으로 설명된다. VTA가 int8 가속기라는 점과도 맞는다.

## ResNet-18 번들 위치

번들이 생성된 디렉토리가 여러 곳 있다. 이전 작업자의 실험 흔적으로 보인다.

| 경로 | 비고 |
|------|------|
| `origin/vta/bundles/Resnet18Test` | baseline. 검증에 사용 |
| `pruning/vta/bundles/Resnet18Test` | 프루닝 실험으로 추정 |
| `exe/vta/bundles/Resnet18Test` | |
| `exec_exec/vta/bundles/Resnet18Test` | |

소스는 `vta/bundles/Resnet18Test/` 에 있다
(`mxnet_exported_resnet18BundleMain.cpp`, `VTAConvolutionTune.txt`,
`VTASkipQuantizeNodes.txt`).

## 결과 해석

`cat_285.png` 가 281로 나오는 것은 오동작이 아니다.

- 281 = tabby cat
- 285 = Egyptian cat

같은 고양이 품종군이고 confidence가 0.478로 낮다. 모델이 헷갈린 경우이며,
int8 양자화 모델에서 흔하다. dog와 zebra는 정확히 맞고 confidence도 0.97 이상이다.

## 성능

첫 실행 약 180 ms, 이후 약 110 ms. 첫 회에는 VTA 드라이버 초기화와
캐시 워밍업이 포함된다. 벤치마크할 때는 첫 회를 버리는 편이 낫다.

NEST-C는 그래프 파티셔닝을 하므로 전체가 VTA에서 도는 것은 아니다.
conv 등 일부 연산만 VTA로 가고 나머지는 ARM CPU(Arm Compute Library)가 처리한다.
