# ZCU102 VTA / ETRI NEST-C 구축 경과 및 로드맵

작성 2026-10-07 · donghun (PNU-SOTA) · 대상 ZCU102 + PYNQ + ETRI NEST-C

---

## 1. 요약

ETRI의 NEST-C 컴파일러로 ZCU102 보드의 VTA NPU에 신경망을 올려 추론하는 **전 과정을 자체적으로 수행할 수 있는 환경을 구축**하였다. 모델 임포트, 양자화 보정 프로파일 생성, 번들 생성, 보드 실행까지 외부 배포물에 의존하지 않고 직접 수행한다.

검증은 ResNet-18로 수행하였으며, 자체 생성한 번들이 ETRI 배포 번들과 **확신도 소수점 여섯 자리까지 동일한 결과**를 산출함을 확인하였다.

구축 과정에서 NEST-C 상류 코드의 결함 **세 건**을 발견하였다. 이 중 한 건은 원인 규명과 수정을 완료하였고, 나머지 두 건은 원인이 특정된 상태이다.

---

## 2. 현재 상태

### 완료

보드 환경을 복원하고 원본 SD카드를 md5 검증과 함께 전량 백업한 뒤 복제본으로 작업 체계를 전환하였다. 원본은 보존 중이다.

ResNet-18과 ResNet-50이 VTA에서 정상 동작한다. ResNet-50은 상류 결함을 수정한 뒤에야 동작하였다.

PC(WSL + Docker)에서 번들을 생성하고 보드에서 실행하는 분리 구조를 확립하였다. 보드 단독 빌드 대비 소요 시간이 4시간에서 5분으로 단축되었다.

상류 저장소를 식별하고 하드웨어 호환성을 확인하였다. 보드의 FPGA 비트스트림이 2024년 상류 코드와 설정값이 완전히 일치하여 그대로 사용 가능하다.

### 진행 중

SqueezeNet 1.1 이식을 시도하여 float 기준 동작과 보정 프로파일 생성까지 완료하였으나, 번들 생성 단계에서 백엔드 미지원 명령으로 중단되었다. 원인은 특정되었다(5.3절).

### 미착수

트랜스포머 계열 모델 이식, VTA ISA 확장 설계, 업스트림 결함 보고.

---

## 3. 전체 플로우

PyTorch로 학습한 모델이 FPGA 위의 VTA에서 추론되기까지는 아홉 단계를 거친다. 각 단계에서 무엇이 결정되고 무엇이 되돌릴 수 없게 되는지를 이해하는 것이 중요하다.

```
[PC / Docker]
  PyTorch 모델
      │  torch.onnx.export(opset_version=9)
      ▼
  ONNX 그래프 (float32, NCHW)
      │  ① ONNXModelLoader
      ▼
  Glow 고수준 그래프 (Node)
      │  ② 그래프 최적화 — BN 융합, 상수 폴딩
      ▼
  최적화된 float 그래프 ──③ 프로파일링(VTAInterpreter)──▶ calib.yaml
      │                                                      │
      │  ④ 양자화 변환  ◀─────────────────────────────────────┘
      ▼
  int8 양자화 그래프
      │  ⑤ 백엔드 분할 — VTA / CPU 판정
      ▼
  분할된 그래프
      │  ⑥ 저수준 IR → VTA 명령 생성 (VTASave.cpp)
      ▼
  ⑦ 번들 (.cpp / .h / .weights.bin / VTARuntime.h)
      │
      │  scp
      ▼
[ZCU102 보드]
  ⑧ g++ 컴파일 + libVTABundle.a + libvta_runtime.a
      ▼
  실행 파일
      │  ⑨ 실행 — DMA, 명령 큐, GEMM
      ▼
  FPGA 위의 VTA
```

### 3.1 모델 준비 — PyTorch에서 ONNX로

`torch.onnx.export` 로 내보내되 **`opset_version` 을 9 이하로 지정**해야 한다. NEST-C의 ONNX 임포터는 Glow 기반이며 비교적 오래된 연산 집합을 전제한다. 최신 PyTorch의 기본 opset(17 이상)으로 내보내면 임포트 단계에서 실패한다.

이 단계에서 결정되는 것이 하나 더 있다. **전처리를 그래프 안에 넣을지 밖에 둘지**이다. ETRI가 배포하는 ResNet-18은 평균 차감과 정규화가 그래프에 포함되어 있어 원본 픽셀을 그대로 넣으면 된다. 반면 ONNX Model Zoo의 모델들은 전처리가 없어 호출하는 쪽에서 맞춰야 한다. 전처리가 틀리면 분류가 어긋나는데, 그 원인이 파이프라인인지 전처리인지 구분하기 어려우므로 **양자화 이전에 float으로 먼저 검증**하는 것이 중요하다.

### 3.2 ① ONNX 임포트

`lib/Importer/ONNXModelLoader.cpp` 가 ONNX 연산을 Glow의 내부 노드로 변환한다. 이 파일은 ETRI가 Glow 원본에서 포크하여 직접 유지하고 있으며, 지원 연산을 계속 추가해 왔다.

**모델이 사용하는 연산 중 하나라도 이 파일이 모르면 여기서 실패한다.** 현재 `Gather`, `LayerNormalization`, `ReduceMean`, `Sqrt` 가 없어 트랜스포머 계열이 막혀 있다. 반대로 `Erf` 는 추가되어 있어 정확한 GELU 표현은 가능하다.

### 3.3 ② 그래프 최적화 (float 단계)

양자화 이전에 수행되는 구조 변환이다. 가장 중요한 것이 **BatchNorm 융합**으로, 추론 시 BN은 채널별 선형 변환에 불과하므로 앞선 convolution의 가중치와 편향에 흡수시킬 수 있다. 이 때문에 ONNX에 있던 BN 노드가 컴파일 후에는 사라진다. 상수 폴딩, 불필요한 전치 제거, 죽은 노드 제거도 함께 이루어진다.

이 단계가 양자화보다 먼저 와야 하는 이유가 있다. BN이 융합되지 않은 채로 양자화하면 BN 출력에 별도의 스케일이 생겨 정밀도가 두 번 떨어진다.

### 3.4 ③ 프로파일링 — 양자화 보정 데이터 수집

**이 단계가 파이프라인 전체에서 가장 오해하기 쉬운 지점이다.**

int8 양자화는 실수 범위를 −128~127의 정수 눈금에 대응시키는 작업이다. 그러려면 각 텐서의 값 범위를 알아야 하는데, **가중치는 학습이 끝나 고정되어 있으므로 읽으면 되지만 활성값은 입력에 따라 변하므로 모델만 봐서는 알 수 없다.**

그래서 대표 입력 몇 장을 float으로 추론시키며 각 층 출력의 최솟값, 최댓값, 히스토그램을 수집한다. 이 결과가 `calib.yaml` 이고, 이를 보정(calibration) 프로파일이라 한다. NEST-C에서는 `image-classifier` 를 `VTAInterpreter` 백엔드로 돌리며 `-dump-profile` 로 생성한다.

프로파일 품질이 최종 정확도를 좌우한다. 입력 영상이 적거나 대표성이 없으면 범위가 실제와 어긋난다. 또한 현재 구현은 단순 최솟값·최댓값을 쓰므로 **극단적 이상치 하나가 범위 전체를 끌고 가** 나머지 값들이 몇 개 눈금 안으로 뭉개질 수 있다. 이를 개선하는 퍼센타일 클리핑이 로드맵 4단계의 과제이다.

### 3.5 ④ 양자화 변환

프로파일의 범위로부터 텐서별 스케일을 계산하고, 그래프에 `Quantize`, `Dequantize`, `RescaleQuantized` 노드를 삽입한다. convolution과 완전연결층의 가중치는 이 시점에 int8로 변환된다.

NEST-C는 VTA를 위해 `symmetric_with_power2_scale` 스킴을 사용한다. **대칭(symmetric)** 은 영점을 0으로 고정한다는 뜻이고, **2의 거듭제곱 스케일(power2)** 은 스케일 값이 2ⁿ 형태로 제한된다는 뜻이다.

이 제약은 하드웨어에서 온다. VTA의 ALU에는 부동소수점 곱셈기가 없고 시프트 연산만 있다. 스케일이 2의 거듭제곱이면 재양자화가 비트 시프트 한 번으로 끝난다. 대신 표현할 수 있는 스케일이 듬성듬성해져 정밀도 손실이 생기고, 텐서 단위 단일 스케일이라 채널마다 분포가 크게 다른 경우에 특히 불리하다. 트랜스포머 활성값의 이상치 문제가 여기서 발생한다.

**양자화 스케일은 이 시점에 번들 코드 안에 상수로 박힌다. 런타임에 바꿀 수 없다.** 따라서 양자화 품질을 바꾸려면 프로파일을 고쳐 번들을 다시 만드는 수밖에 없다.

### 3.6 ⑤ 백엔드 분할 — 무엇이 VTA로 가고 무엇이 CPU로 가는가

VTA는 범용 프로세서가 아니다. 실질적으로 두 가지 일만 한다.

**GEMM 유닛**은 16×16 int8 시스톨릭 어레이로 행렬곱을 수행한다. convolution은 im2col 방식으로 행렬곱에 환원되고, 완전연결층은 그 자체가 행렬곱이므로 둘 다 여기로 간다. 모델 연산량의 90% 이상이 이 두 가지이므로 가속 효과가 크다.

**ALU**는 원소별 연산을 수행하는데 가능한 것이 add, max, min, shift 네 가지뿐이다. convolution 직후의 ReLU는 max(x, 0)이므로 GEMM 명령의 플래그로 융합되어 같은 파이프라인에서 처리된다.

나머지는 전부 CPU로 떨어진다. max pooling은 슬라이딩 윈도우 주소 계산이 GEMM과 ALU 어느 패턴에도 맞지 않는다. softmax는 지수 함수가 필요하다. 잔차 경로의 독립적인 ReLU는 융합 대상이 아니다. 레이아웃 변환(transpose)과 Concat도 CPU 몫이다.

CNN에서는 이 CPU 몫이 전체 연산량의 1/1000 수준이라 무시할 만하다. 그러나 **트랜스포머에서는 LayerNorm과 softmax의 비중이 커서 상황이 달라진다.** 이것이 로드맵 5단계 ISA 확장의 출발점이다.

CPU 폴백 구현에는 두 가지가 있다. `NESTC_EVTA_RUN_WITH_GENERIC_BUNDLE` 옵션이 이를 선택하는데, generic은 이식성을 위한 순수 C++ 참조 구현이고 aarch64는 NEON SIMD와 ARM Compute Library를 쓴다. **기본값이 generic이며 두 구현의 성능 차가 13.6배이다**(6.1절).

VTA와 CPU의 경계마다 데이터 레이아웃 변환이 삽입된다. VTA는 채널을 16개씩 묶은 타일 레이아웃을 쓰고 CPU 코드는 NHWC를 쓰기 때문이다. 이 변환 자체도 CPU 비용이다.

### 3.7 ⑥ 명령 생성

분할된 그래프가 Glow의 저수준 IR(Instruction)로 낮춰지고, `lib/Backends/VTA/VTASave.cpp` 가 명령 종류별로 코드를 생성한다. VTA에 배치된 명령은 VTA ISA 명령 시퀀스로, CPU에 배치된 명령은 `libVTABundle.a` 의 함수 호출로 변환된다.

**이 디스패치에 없는 명령이 하나라도 나오면 컴파일이 중단된다.** SqueezeNet이 막힌 지점이 여기이며, Concat 구현에 쓰이는 `TouchInst` 가 처리되지 않는다(5.3절).

### 3.8 ⑦ 번들

출력은 네 개의 파일이다. `<model>.cpp` 가 VTA 명령 시퀀스와 CPU 함수 호출을 담은 C++ 소스이고, `<model>.h` 가 그 인터페이스, `<model>.weights.bin` 이 int8로 변환된 가중치, `VTARuntime.h` 가 VTA 하드웨어 제어 래퍼이다.

이것이 AOT(ahead-of-time) 컴파일이라는 말의 의미이다. 모든 결정이 이 시점에 끝나므로 **보드에는 컴파일러가 필요 없다.** 번들을 컴파일할 g++와 런타임 라이브러리만 있으면 된다. 반대로 텐서 형상이 전부 상수로 고정되므로, 길이가 변하는 입력을 다루는 자기회귀 생성 같은 작업은 그대로는 표현할 수 없다.

### 3.9 ⑧ 보드 컴파일

번들 `.cpp` 와 호출부 `main.cpp` 를 컴파일하여 두 라이브러리와 링크한다. `libVTABundle.a` 가 CPU 폴백 연산들의 구현을 담고 있고, `libvta_runtime.a` 가 FPGA 드라이버 계층이다. ARM Compute Library도 aarch64 구현을 쓸 때 함께 링크된다.

### 3.10 ⑨ 실행

실행 시 가중치 바이너리를 DRAM에 올리고, VTA의 LOAD 모듈이 DMA로 필요한 타일을 온칩 SRAM(입력 64KB, 가중치 512KB, 누산 256KB)으로 가져온다.

VTA는 네 개의 모듈이 명령 큐로 연결된 구조이다. **FETCH**가 DRAM에서 명령을 읽어 세 큐로 분배하고, **LOAD**가 데이터를 SRAM으로, **COMPUTE**가 GEMM과 ALU 연산을, **STORE**가 결과를 DRAM으로 내보낸다. 각 모듈이 독립적으로 동작하며 의존성 토큰으로 동기화되므로, **한 타일을 계산하는 동안 다음 타일을 미리 적재**하는 파이프라인이 가능하다. 이것이 DMA 지연을 숨기는 핵심 기법이다.

누산은 int32로 이루어진다. int8 × int8 곱의 합이 int8 범위를 넘기 때문이다. 결과를 다시 int8로 내보낼 때 재양자화가 일어나는데, 여기서 ④에서 계산한 2의 거듭제곱 스케일이 **시프트 연산 한 번**으로 적용된다.

CPU 폴백 구간에서는 호스트 ARM 코어가 `libVTABundle.a` 의 함수를 호출하고, 끝나면 다시 VTA로 제어가 넘어간다.

### 3.11 발견한 결함의 위치

세 결함이 서로 다른 단계에 있다는 점이 흥미롭다.

| 결함 | 단계 | 영향 |
|---|---|---|
| transpose 미구현 | ⑨ 실행 (CPU 폴백 구현) | 컴파일은 되고 실행도 되지만 **결과가 틀림** |
| TouchInst 미처리 | ⑥ 명령 생성 | Concat 모델 **컴파일 불가** |
| 기본 분기 UB | ⑥ 명령 생성 | 진단 메시지 소실, **원인 파악 불가** |

첫 번째가 가장 위험하다. 오류 없이 조용히 틀린 답을 내기 때문이다. 입력을 바꿔도 결과가 변하지 않는다는 점을 알아차리지 못했다면 계속 모르고 지나갔을 것이다.

---

## 4. 구축한 시스템

### 4.1 구성

작업을 두 축으로 분리하였다.

**PC (WSL Ubuntu 22.04, 12코어, 13GB)** 가 컴파일을 담당한다. ETRI가 공개한 Docker 이미지 `leejaymin/nestc-ssh:latest`(Ubuntu 20.04 + clang 8.0.1 + LLVM 8)를 사용하여 검증된 빌드 환경을 재현한다. 3절의 ①~⑦이 여기서 이루어진다.

**ZCU102 보드**가 실행을 담당한다. ⑧과 ⑨를 수행한다.

이 분리는 보드의 자원 제약에서 비롯되었다. 보드는 DDR 4GB 중 대부분을 VTA용 연속 메모리(CMA)로 예약하여 리눅스가 사용할 수 있는 메모리가 1.5GB에 불과하다. 단일 작업 빌드만 가능하며, 컴파일러 전체 빌드는 4시간이 소요되고 그마저 실패하였다. 반면 ⑦에서 설명한 대로 **보드가 실제로 필요로 하는 것은 컴파일러가 아니라 런타임뿐이다.**

### 4.2 번들 생성 명령

**1단계, 보정 프로파일 생성.**

```
image-classifier <images> -m=<model>.onnx -model-input-name=<input>
  -backend=VTAInterpreter -image-layout=<NCHW|NHWC> -image-mode=0to1
  -use-imagenet-normalization
  -dump-profile=<calib>.yaml
  -quantization-schema=symmetric_with_power2_scale
```

**2단계, 번들 생성.**

```
model-compiler -g -model=<model>.onnx -backend=VTA
  -emit-bundle=<절대경로> -bundle-api=dynamic
  -model-input=<input>,float,<shape>
  -load-profile=<calib>.yaml
  -quantization-schema=symmetric_with_power2_scale
```

### 4.3 보드 실행

생성된 번들을 전송하고 상류 VTA 런타임과 링크하여 실행 파일을 만든다. CMake 빌드 시스템을 거치지 않고 기존 빌드의 컴파일 인자를 추출해 직접 컴파일하는 방식을 사용한다. 기존 작업 트리를 변경하지 않기 위함이다.

주의 사항으로, 상류 번들은 상류 런타임을 요구한다. 2024년 번들이 호출하는 `VTAUopBufferReset()` 이 보드의 2021년 런타임에는 존재하지 않아 상류 `vta_lib` 을 보드에서 별도로 빌드하였다. 링크 시 **상류 헤더 경로를 기존 트리보다 앞에 두어야** 한다.

### 4.4 환경 제약

보드 실행 중 Ctrl+C로 중단하면 FPGA와 커널 메모리 할당기가 정리되지 않아 재부팅 전까지 보드를 사용할 수 없다. SSH 연결이 자주 끊기므로 장시간 작업은 tmux 내부에서 수행한다.

ETRI가 데이터 배포에 사용하던 AWS S3 버킷 `nestc-data-pub` 이 삭제되어 공식 설치 문서의 상당 부분이 현재 동작하지 않는다. 모델 파일은 GitLab의 구 경로에서 받아야 하며, `NESTC_USE_PRECOMPILED_EVTA_LIBRARY` 옵션은 사용할 수 없다. 해당 옵션을 끄면 서브모듈 소스에서 직접 빌드하여 정상 동작한다.

---

## 5. 발견한 상류 결함

### 5.1 transpose 미구현 (수정 완료)

`vta/vtalib/lib/Bundle/CPUBundle_aarch64.cpp` 의 범용 축 치환 함수가 미구현 상태이다.

```c
int transpose(int8_t *input, int8_t *output, ...) {
//TODO re-implement
  return -1;
}
```

출력 버퍼에 아무것도 기록하지 않으므로 초기화되지 않은 메모리가 후속 연산으로 전달된다. ResNet-50은 평균 풀링과 완전연결층 사이에서 이 함수를 호출하므로, **입력 영상과 무관하게 고정된 결과**(클래스 619, 확신도 0.083585)를 산출하였다. ResNet-18은 VTA 타일 레이아웃 전용 변환 함수만 사용하여 영향을 받지 않았고, 이 때문에 결함이 드러나지 않았다.

동일 기능이 generic 구현에 존재하므로 이를 이식하여 수정하였다. 수정 과정에서 switch 문을 배열 인덱싱으로 대체하여 미초기화 경고를 제거하고 축 번호 범위 검사를 추가하였다. 수정 후 ResNet-50이 정상 분류한다.

**2024년 7월 기준 최신 상류 코드에도 동일한 상태로 존재한다.**

### 5.2 미지원 명령 처리의 정의되지 않은 동작

`lib/Backends/VTA/VTASave.cpp` 의 명령 디스패치 기본 분기가 다음과 같이 구현되어 있다.

```cpp
default:
  std::string msg = I.getKindName();
  msg.append(" is an unhandled instruction");
  llvm_unreachable(msg.c_str());
```

`llvm_unreachable` 은 `NDEBUG` 빌드에서 `__builtin_unreachable()` 로 치환된다. 이는 컴파일러에게 해당 지점에 도달하지 않음을 선언하는 것이므로, 실제로 도달하면 정의되지 않은 동작이 된다. 결과적으로 Release 빌드에서는 의도한 진단 메시지 대신 `std::length_error: basic_string::append` 와 스택 트레이스만 출력되어 원인 파악이 불가능하다.

`llvm::errs()` 출력 후 `std::exit(1)` 로 대체하면 해결된다.

### 5.3 Concat 계열 모델 미지원

위 5.2의 결함 때문에 가려져 있었으나, 명령 디스패치 루프에 디버그 출력을 삽입하여 원인을 특정하였다. **`TouchInst` 명령에 대한 처리가 존재하지 않는다.**

Glow는 Concat 연산을 "출력 버퍼 할당 → `touch` 로 미초기화 사용 표시 → 각 입력을 `InsertTensor` 로 기록"의 순서로 낮춘다. VTA 백엔드에 `TouchInst` 케이스가 없어 기본 분기로 떨어지며, 따라서 **Concat을 사용하는 모든 모델이 컴파일 불가**하다.

ResNet 계열은 잔차 연결에 Add를 사용하므로 영향이 없다. SqueezeNet은 Fire 모듈마다 Concat을 사용하여(총 8개) 이 제약에 걸렸다. Inception, DenseNet 등 분기-병합 구조를 가진 모델 전반이 동일하게 영향을 받는다.

`touch` 는 실제 연산을 수반하지 않는 표시용 명령이므로 빈 케이스 추가만으로 해결될 가능성이 있다. 다만 후속 `InsertTensorInst` 의 지원 여부를 함께 확인해야 하며, 미지원이라면 Concat 출력 버퍼의 메모리 배치까지 구현해야 한다.

### 5.4 기타 관찰

`-emit-bundle` 에 상대 경로를 전달하면 진단 없이 비정상 종료한다. 4차원 및 6차원 외의 전치는 지원되지 않으며 오류 메시지에 오타(`dimenstion`)가 있고 해당 노드를 특정해 주지 않는다. 공식 설치 문서가 삭제된 S3 버킷을 안내한다.

---

## 6. 측정 결과

### 6.1 추론 성능

| 모델 | generic 구현 | aarch64 구현 |
|---|---|---|
| ResNet-18 | 1,492 ms | 110 ms |
| ResNet-50 | 1,987 ms | 480 ms |

`NESTC_EVTA_RUN_WITH_GENERIC_BUNDLE` 빌드 옵션이 CPU 폴백 구현 전체를 교체한다(3.6절). 기본값은 generic이다.

동일 모델(ResNet-18)에 대한 두 구현의 차이가 **13.6배**이다. 따라서 ResNet-50이 ResNet-18보다 18배 느려 보였던 현상은 13.6배가 구현 차이에서 기인하며 모델 규모의 기여는 1.33배에 불과하다. **서로 다른 구현으로 측정한 수치는 비교 대상이 될 수 없다.**

### 6.2 자체 생성 번들 검증

ResNet-18에 대해 보정 프로파일부터 번들까지 자체 생성하고 보드에서 실행한 결과이다.

| 입력 | 결과 클래스 | 확신도 | 추론 시간 |
|---|---|---|---|
| cat_285.png | 281 | 0.478113 | 122.1 ms |
| dog_207.png | 207 | 0.977876 | 109.0 ms |
| zebra_340.png | 340 | 0.969942 | 112.6 ms |

세 확신도 모두 ETRI 배포 번들의 값과 완전히 일치한다. 입력에 따라 결과가 변화하므로, 5.1절의 결함 상태에서 관찰된 고정 출력 현상과 명확히 구분된다.

가중치 파일 크기는 12,003,168바이트로 ResNet-18의 파라미터 수(약 1,170만)와 int8 양자화 가정에 정확히 부합한다.

### 6.3 SqueezeNet float 기준값

번들 생성이 중단되었으므로 보드 측정값은 없으나, 양자화 이전 기준값을 확보하였다. 향후 int8 결과는 이 값과 비교해야 한다.

| 입력 | 1위 | 확신도 | 2위 |
|---|---|---|---|
| cat_285.png | 281 | 0.4366 | 285 (0.3250) |
| dog_207.png | 205 | 0.3526 | 207 (0.3489) |
| zebra_340.png | 340 | 0.9996 | 292 (0.0001) |

개 영상에서 1·2위가 뒤바뀌었으나 두 클래스(flat-coated retriever, golden retriever)가 유사 품종이고 격차가 0.0037에 불과하다. 파라미터 1.2M 규모 모델의 한계이며 전처리 오류가 아니다. 다만 양자화 후 순위가 뒤집힐 가능성이 높아, 합격 판정 기준으로는 부적절하고 **양자화 민감도 관찰 사례로 활용**하는 것이 적절하다.

### 6.4 하드웨어 구성

| 항목 | 값 |
|---|---|
| GEMM 어레이 | 16 × 16 |
| 입력/가중치 정밀도 | int8 |
| 누산 정밀도 | int32 |
| 배치 | 1 |
| 입력/가중치/누산 버퍼 | 64KB / 512KB / 256KB |
| 코어 수 | 4 (현재 단일 코어 구성으로 사용) |
| 리눅스 가용 메모리 | 1.5 GB |

보드의 실제 빌드 설정과 2024년 상류 코드의 설정값이 전 항목 일치하여, 5년 전 생성된 비트스트림을 그대로 사용한다.

---

## 7. 로드맵

### 1단계 — Concat 지원 (1~2주)

VTA 백엔드에 `TouchInst` 처리를 추가하고 `InsertTensorInst` 지원 여부를 확인한다. 동시에 5.2절의 기본 분기를 진단 가능한 형태로 수정한다.

완료 시 SqueezeNet, Inception, DenseNet 등 분기-병합 구조 모델 전반이 열린다. 검증은 SqueezeNet으로 수행하며, 6.3절의 float 기준값 대비 양자화 손실을 측정한다.

산출물은 동작하는 SqueezeNet 번들과 양자화 손실 수치, 그리고 상류에 제출할 패치이다.

### 2단계 — 객체 검출 (2~3주)

Tiny YOLOv2를 이식한다. 구성 연산(convolution, max pooling, leaky ReLU, batch normalization)이 모두 지원 범위 내에 있으며 opset 8 공개 모델이 존재한다. 비최대 억제는 모델 외부 후처리로 구현한다.

분류에서 검출로 과제 영역을 확장하는 의미가 있고, 결과를 화면에 시각적으로 제시할 수 있어 시연에 적합하다. 보드의 VNC 환경은 설정이 완료되어 있다.

### 3단계 — 연산자 배치 분석 (2주)

3.6절의 분할 결과를 모델별로 정량화한다. 각 연산이 VTA와 CPU 중 어디에 배치되는지, 각각의 소요 시간이 얼마인지를 측정한다. 컴파일러의 DAG 덤프와 프로파일링 기능을 활용한다.

이 데이터가 이후 모든 개선 작업의 판단 근거가 된다. 특히 depthwise separable convolution 계열(MobileNet, EfficientNet)이 16×16 GEMM 어레이에서 구조적으로 불리하다는 가설을 실측으로 확인할 수 있다. 채널 그룹마다 독립 계산하므로 한 번에 처리하는 채널이 1개뿐이어서 어레이 활용률이 1/16 이하로 떨어진다는 것이 가설이다.

### 4단계 — 트랜스포머 이식 (4~6주)

현재 확인된 제약은 네 가지이다. ONNX 임포터에 `Gather`, `LayerNormalization`, `ReduceMean`, `Sqrt` 가 없다(3.2절). 어텐션의 Q·Kᵀ는 두 피연산자가 모두 런타임 계산값인 행렬곱인데 백엔드 지원 여부가 미확인이다(3.6절). AOT 컴파일이라 가변 길이 KV 캐시를 표현할 수 없다(3.8절). 2의 거듭제곱 텐서 단위 양자화가 트랜스포머 활성값의 이상치를 감당하지 못한다(3.5절).

이에 대한 대응 방안을 수립하였다. 임베딩 룩업은 원-핫 벡터와의 행렬곱으로 대체하여 `Gather` 없이 표현한다. 지원되지 않는 LayerNorm 등은 번들을 층 단위로 분할하고 번들 사이에서 호스트 코드로 계산한다. 어차피 CPU로 떨어질 연산이므로 실행 주체는 동일하다. KV 캐시는 최대 문맥 길이로 고정하고 마스킹으로 처리하여 정적 형상을 유지한다. 양자화는 보정 프로파일의 퍼센타일 클리핑과 SmoothQuant 방식의 오프라인 스케일 재배치로 개선한다.

목표는 BERT-Tiny 규모의 인코더 모델을 고정 시퀀스 길이로 동작시키고, float 대비 정확도 손실을 정량화하는 것이다.

### 5단계 — ISA 확장 제안 (3~4주)

3·4단계의 측정 결과를 근거로 VTA 명령어 집합의 부족한 지점을 정리하고 확장을 제안한다.

현재 식별된 후보는 네 가지이다. **비선형 함수 룩업 테이블 유닛** — softmax의 지수, LayerNorm의 역제곱근, GELU를 구간별 선형 근사로 처리한다. **축 방향 축약 명령** — 레인 간 가산 트리를 추가하여 평균·분산·최댓값·합을 가속기 내부에서 계산한다. 앞의 둘을 합치면 LayerNorm과 softmax가 통째로 VTA 안에서 끝난다. **채널별 스케일 곱셈** — 재양자화 단계에 작은 곱셈기 배열을 추가하여 2의 거듭제곱 제약을 해소한다. **DMA 단계 레이아웃 변환** — 주소 생성기에 스트라이드 서술자를 넣어 메모리 이동 중 축을 바꾼다.

검증은 저장소에 포함된 VTA 인터프리터에 명령을 추가하는 방식으로 수행한다. FPGA 재합성은 Vivado 합성과 타이밍 클로저에 수 시간이 걸려 반복 실험에 부적합하다. 최종적으로 가장 구현 난이도가 낮은 한 가지만 RTL로 내려 면적과 동작 주파수를 측정하면, 나머지 추정치에도 신뢰를 부여할 수 있다.

---

## 8. 위험 요소

**양자화 품질의 하한.** 2의 거듭제곱 텐서 단위 스케일은 시프트 연산만 보유한 하드웨어에서 비롯된 제약이다. 소프트웨어 기법으로 개선할 수 있는 범위에 한계가 있으며, 트랜스포머에서는 정확도가 실용 수준에 미치지 못할 가능성이 실재한다. 다만 그 경우에도 한계를 정량적으로 제시하는 것이 결과가 된다.

**상류 코드의 성숙도.** 현재까지 세 건의 결함을 발견하였으며, 이는 VTA 백엔드가 특정 모델군(ResNet 계열) 위주로 검증되었음을 시사한다. 새로운 구조의 모델을 시도할 때마다 유사한 문제를 만날 가능성이 있으므로 일정에 여유를 두어야 한다.

**보드 자원 제약.** 가용 메모리 1.5GB는 보드 상의 작업 범위를 제한한다. 현재의 PC-보드 분리 구조로 대부분 해소되었으나, 런타임 라이브러리를 보드에서 재빌드해야 하는 경우가 남아 있다.

---

## 9. 참고 사항

**저장소.** 현행 개발 원본은 `gitlab.com/ones-ai/nest-compiler`(최종 2024-07-09)이며 `gitlab.com/ones-ai/vta_lib` 을 서브모듈로 포함한다. 보드에 설치되어 있던 것은 2021-10-16 시점의 스냅샷이었다. GitHub `etri/nest-compiler` 는 공개 미러로 추정되나 동기화 여부는 미확인이다.

**연락처.** 과제 책임자 `yongin.kwon@etri.re.kr`. 결함 보고와 패치 제출 경로이다.

**관련 문서.** 보드 설정은 `01-boot-setup.md`, 실행 절차는 `02-run-resnet18.md`, 빌드 관련 사항은 `03-build-notes.md`, 백업은 `04-backup.md`, ResNet-50 디버깅 전 과정은 `05-model-debugging.md` 를 참조한다.

---

## 10. 상류 재현과 CI 구축 (2026-10-07 추가)

4절까지는 ETRI 배포 번들과 자체 생성 번들을 보드에서 실행하는 데 초점이 있었다. 이후 작업에서는 **상류 저장소 전체를 보드에 재현하고, 그것을 자동으로 검증하는 체계**를 세웠다. 교수님의 "GitLab에 있던 원래 서버를 이 보드에 복구"라는 요구에 대한 결과물이다.

### 10.1 재현 결과

GitLab `ones-ai/nest-compiler` 2024-07-09 상류를 보드에 클론하여 빌드하고, ZCU102 라벨이 붙은 테스트 전체를 실행하였다.

```
100% tests passed, 0 tests failed out of 51
Total Test time (real) =  12.40 sec
```

5년 전 FPGA 비트스트림과 2024년 상류 소스가 그대로 호환됨을 51개 테스트 전수로 확인한 것이다. 4절의 "설정값이 일치한다"는 서류상 확인을 실측으로 대체하였다.

### 10.2 두 CPU 폴백 경로의 비교

NEST-C는 VTA가 처리하지 못하는 연산을 CPU로 넘긴다. 이 CPU 구현이 두 가지이며 `NESTC_EVTA_RUN_WITH_GENERIC_BUNDLE` 로 전환된다. ON이면 이식성 우선의 generic C++ 참조 구현, OFF이면 aarch64 NEON과 ARM Compute Library를 쓰는 최적화 구현이다. 상류 CI는 이 중 generic만 돌렸고 aarch64 경로는 한 번도 검증된 적이 없다.

두 경로를 같은 트리에서 별도 빌드 디렉터리로 나란히 측정하였다.

| | generic | aarch64 |
|---|---|---|
| ZCU102 테스트 수 | 51 | 50 |
| 통과 | 51 | 48 |
| ctest 총 시간 | 12.40초 | 6.45초 |
| ResNet-50 추론 | 1989.5ms | **487.4ms** |

ResNet-50 세 장의 분류 결과는 두 경로가 일치한다.

| 입력 | generic | aarch64 |
|---|---|---|
| cat_285.png | 281 (0.665328) | 281 (0.658912) |
| dog_207.png | 207 (0.583738) | 207 (0.685082) |
| zebra_340.png | 340 (0.991000) | 340 (0.991693) |

**aarch64 경로가 4.08배 빠르다.** 확신도의 소수점 차이는 재양자화 반올림 방식의 차이에서 오며 10.4절에서 다룬다.

### 10.3 상류는 aarch64에서 ResNet-50을 의도적으로 제외하였다

`vta/bundles/Resnet50Test/CMakeLists.txt:84`

```cmake
if(NESTC_EVTA_RUN_WITH_GENERIC_BUNDLE)
    add_nestc_test(ZCU102 NAME vtaCaffe2Resnet50Bundle ...)
else()
    add_nestc_test(NAME vtaCaffe2Resnet50Bundle ...)
endif()
```

aarch64 경로에서는 ZCU102 라벨을 떼어 보드 테스트 집합에서 제외한다. 라벨이 없으면 `zcu102` 집계 타깃의 의존에서도 빠지므로 **바이너리 자체가 빌드되지 않는다.** aarch64의 테스트 수가 51이 아니라 50인 이유가 이것이다.

ResNet-50은 avgpool과 FC 사이에서 `transpose()` 를 호출하는데, 5.1절에서 다룬 대로 상류의 aarch64 구현은 그 함수가 미구현 스텁이다. 즉 상류는 이 모델이 aarch64에서 동작하지 않음을 알고 있었고, 수정 대신 테스트에서 제외하는 쪽을 택하였다. 5.1절에서 제기한 "이 결함이 CI에서 한 번도 밟히지 않았다"는 추정의 직접 증거이다.

패치를 적용한 뒤 해당 타깃을 수동으로 빌드하여 실행한 결과가 10.2절의 표이다. 세 장이 각각 다른, 그리고 올바른 클래스를 산출한다. 패치 이전이라면 입력과 무관하게 동일한 클래스가 나온다.

**상류가 포기한 모델을 복구한 것이 이 작업의 결과이다.**

### 10.4 aarch64 실패 2건의 성격

**`vtaFCTestBundle_compare` — 테스트 기준의 문제이다.** 출력 1000바이트 중 48바이트가 골든과 다르나 모두 정확히 1씩, 일관되게 aarch64 쪽이 크다. int32 누산을 int8로 재양자화할 때 generic의 절삭과 NEON의 반올림이 경계값에서 갈리는, 양자화 구현에서 정상 범위의 차이다. 문제는 `*_compare` 테스트가 `cmp` 로 바이트 단위 완전 일치를 요구한다는 점이다. 골든이 generic으로 생성된 이상 수치 경로가 다른 aarch64는 구조적으로 통과할 수 없다. 허용 오차 비교나 top-1 일치로 바꾸어야 한다.

**`vtaVGG16Cifar10Bundle` — 실제 결함이다.** `[ERROR] invalid layout` 을 출력하고 airplane(0)을 2로 분류한다(확신도 0.907). 원인은 `vta/vtalib/lib/Bundle/CPUBundle_aarch64.cpp:4051` 의 전제 조건이다.

```c
if(inputDim1 != kernelDim0 || inputDim1%16 != 0 || kernelDim1%4 != 0) {
    printf("[ERROR] invalid layout\n");
    return -1;
}
```

NEON 벡터화를 위해 입력 차원이 16의 배수, 출력 차원이 4의 배수여야 한다. VGG16-CIFAR10의 최종 FC는 출력이 10 클래스라 조건을 만족하지 못한다. 함수는 아무 계산도 하지 않고 `-1` 을 반환하는데 **호출부가 반환값을 검사하지 않아** 초기화되지 않은 출력 버퍼로 분류가 진행된다.

5.1절의 `transpose()` 와 동일한 패턴이다. 실패를 반환하지만 아무도 확인하지 않는다. 5.2절의 `llvm_unreachable` 과 함께 보면, **이 코드베이스는 오류 경로를 신뢰할 수 없다**는 일관된 성질을 가진다. 새 모델을 시도할 때 "틀린 답이 조용히 나오는" 상황을 항상 의심해야 한다는 뜻이다.

### 10.5 CI

상류 `.gitlab-ci.yml` 의 `test-board` 스테이지는 cmake 설정까지만 하고 `sudo make check_zcu102` 가 주석 처리되어 있다. 보드 테스트가 실제로 실행된 적이 없다는 의미이며, 5.1절과 10.4절의 결함이 장기간 남아 있던 이유이다.

이를 GitHub Actions로 이식하면서 실행까지 포함하도록 복원하였다.

```
.github/workflows/build.yml        컴파일러 빌드 (GitHub 호스티드 러너)
.github/workflows/board-test.yml   ZCU102 보드 테스트 (self-hosted 러너)
ci/patch_transpose.py              상류 transpose 결함 패치 (멱등)
```

원본과 달라진 점이 셋이다. 첫째, 보드 테스트를 실제로 실행한다. 둘째, 삭제된 S3를 GitLab 미러로 우회하는 shim을 사용한다(11.2절). 셋째, generic과 aarch64 두 경로를 모두 돌리고 aarch64의 알려진 실패 2건만 허용하며, 그 외 실패가 나오면 즉시 실패 처리한다. 더하여 상류가 제외한 ResNet-50을 aarch64에서 직접 빌드·실행하여 세 장의 분류 결과를 검증하는 단계를 두었다. transpose 패치가 깨지면 이 단계가 잡는다.

**러너는 보드가 아니라 PC에 둔다.** 보드는 Ubuntu 18.04(glibc 2.27)이고, GitHub Actions의 자바스크립트 액션은 Node 20을 요구하는데 Node 20은 glibc 2.28 이상을 필요로 한다. 가용 메모리도 615MB뿐이라 러너 프로세스가 빌드와 충돌한다. PC(WSL x64)의 러너가 SSH로 보드에 명령만 보내는 구조로 두 문제를 모두 회피하였다.

---

## 11. 인수인계 — 처음부터 따라 하는 절차

이 절만 따라 하면 새로 합류한 사람이 동일한 환경을 재현할 수 있다. 각 단계는 앞 단계가 끝난 것을 전제한다.

### 11.0 터미널 구분

작업 중 세 종류의 창을 오가게 된다. 프롬프트로 구분한다.

| 프롬프트 | 어디 | 용도 |
|---|---|---|
| `xilinx@pynq:~$` | ZCU102 보드 | 빌드와 실행 |
| `ehdgns@...:~$` | PC의 WSL | 번들 생성, CI 러너 |
| `PS C:\...>` | PowerShell | git 작업 |

**명령을 치기 전에 프롬프트를 확인하는 습관**을 들이는 편이 좋다. 창을 혼동해 보드 명령을 WSL에서 실행하는 실수가 반복되었다.

### 11.1 보드 접속

보드는 사설망(`192.168.1.40`) 뒤에 있고 외부에서는 포워딩을 통해 접근한다.

```sh
ssh xilinx@sota.pusan.ac.kr -p 25022
```

긴 작업은 반드시 tmux 안에서 한다. SSH가 끊겨도 살아남는다.

```sh
tmux new -s work          # 새 세션
tmux attach -t work       # 다시 붙기
# Ctrl+B 다음 D 로 빠져나오기
```

**VTA 프로그램 실행 중에는 Ctrl+C를 누르지 않는다.** FPGA와 커널 메모리 할당기(xlnk)가 정리되지 않아 재부팅 전까지 보드를 사용할 수 없다. 빌드(컴파일) 중단은 안전하다.

### 11.2 aws shim 설치 (보드)

ETRI가 쓰던 S3 버킷 `nestc-data-pub` 은 삭제되어 `NoSuchBucket` 을 반환한다. 동일한 내용 198개 파일이 GitLab `yongin.kwon/nestc-data` 에 남아 있고 경로가 1:1로 대응하므로, `aws` 명령을 흉내 내는 shim을 두어 CMakeLists를 수정하지 않고 통과시킨다.

```sh
mkdir -p ~/bin
cat > ~/bin/aws <<'EOF'
#!/bin/sh
BASE=https://gitlab.com/yongin.kwon/nestc-data/-/raw/master
if [ "$1" = "s3" ] && [ "$2" = "cp" ]; then
  SRC=$3; DST=$4
  case "$SRC" in
    s3://nestc-data-pub/*) P=${SRC#s3://nestc-data-pub/} ;;
    s3://nestc-pub/*)      P=${SRC#s3://nestc-pub/} ;;
    *) echo "aws-shim: 지원하지 않는 경로 $SRC" >&2; exit 1 ;;
  esac
  mkdir -p "$(dirname "$DST")" 2>/dev/null
  curl -fL "$BASE/$P" -o "$DST" || { echo "aws-shim: 실패 $P" >&2; exit 1; }
  exit 0
fi
exit 0
EOF
chmod +x ~/bin/aws
export PATH=$HOME/bin:$PATH
```

`export PATH` 는 보드에서 빌드하는 모든 셸에서 필요하다. `~/.bashrc` 에 넣어두면 편하다.

`s3://nestc-pub/vta/bundles/ResConv*Test/` 만 GitLab 미러가 없어 ResConv 1~10 테스트는 이 경로로 복구되지 않는다. 현재 ZCU102 테스트 집합에는 포함되지 않으므로 영향이 없다.

### 11.3 상류 클론과 패치 (보드)

```sh
git clone --recursive https://gitlab.com/ones-ai/nest-compiler.git ~/nest-compiler-2024
```

1.1GB 정도이며 시간이 걸린다. 그다음 5.1절의 transpose 결함을 고친다. 저장소의 `ci/patch_transpose.py` 를 보드로 복사해 실행하면 된다. 멱등하므로 여러 번 돌려도 안전하다.

```sh
python3 patch_transpose.py ~/nest-compiler-2024/vta/vtalib/lib/Bundle/CPUBundle_aarch64.cpp
grep -c at8dim_nestc_patch ~/nest-compiler-2024/vta/vtalib/lib/Bundle/CPUBundle_aarch64.cpp
```

마지막 줄이 `3` 이면 적용된 것이다(함수 정의 1 + 호출 2).

### 11.4 빌드와 테스트 (보드)

generic 경로와 aarch64 경로는 CPU 폴백 구현 전체가 달라 오브젝트를 공유할 수 없다. **반드시 별도 디렉터리**에 둔다.

```sh
cat > ~/run-build.sh <<'EOF'
set -e
export PATH=$HOME/bin:$PATH
cd ~/nest-compiler-2024
mkdir -p build_board && cd build_board
cmake .. \
  -DNESTC_WITH_EVTA=ON \
  -DLLVM_DIR=/usr/lib/llvm-8.0/lib/cmake/llvm \
  -DCMAKE_BUILD_TYPE=Release \
  -DNESTC_USE_VTASIM=OFF \
  -DVTA_RESNET18_WITH_SKIPQUANT0=ON \
  -DNESTC_EVTA_RUN_ON_ZCU102=ON \
  -DNESTC_USE_PRECOMPILED_BUNDLE=ON \
  -DNESTC_EVTA_RUN_WITH_GENERIC_BUNDLE=ON
make -j1 zcu102 || true
echo DONE
EOF
chmod +x ~/run-build.sh
tmux new -d -s bld '~/run-build.sh > ~/bld.log 2>&1'
```

aarch64를 측정하려면 `build_board` 를 `build_aarch64` 로, 마지막 플래그를 `OFF` 로 바꾼 사본을 쓴다.

진행 확인은 이렇게 한다.

```sh
tail -3 ~/bld.log
grep -cE 'error:' ~/bld.log      # 실제 컴파일 에러 수
grep -c 'aws-shim' ~/bld.log     # shim 호출 수
```

보드에서 `-j1` 로 한 시간 가까이 걸린다. 끝나면 테스트를 실행한다.

```sh
tmux new -s chk
cd ~/nest-compiler-2024/build_board
sudo -E env PATH=$HOME/bin:$PATH ctest -L ZCU102 --output-on-failure 2>&1 | tee ~/check.log
```

### 11.5 흔히 빠지는 함정

**`make` 전체 빌드를 하지 않는다.** 아무 인자 없이 `make -j1` 을 돌리면 TVM 전체와 Glow 컴파일러 본체를 만들기 시작한다. 보드는 `NESTC_USE_PRECOMPILED_BUNDLE=ON` 이라 `model-compiler` 가 필요 없고, `check_zcu102` 는 `DEPENDS ${ZCU102_TEST_DEPENDS}` 로 테스트 바이너리에만 의존한다(`CMakeLists.txt:182`). 전체 빌드는 수 시간을 쓰고 LLVM 8.0의 RTTI 불일치에 부딪힐 뿐이다. `make zcu102` 만 하면 된다.

**`zcu102` 는 빌드 타깃이 아니라 실행 타깃이다.** 빌드 후 끝에 ctest를 실행한다. 비root로 돌리면 `/dev/xlnk` 와 VTA 레지스터에 접근하지 못해 전부 실패하고 make는 `Error 8`(ctest의 종료 코드)로 끝난다. 이것을 빌드 실패로 오인하기 쉽다. **make의 `Error N` 은 자식 프로세스의 종료 코드일 뿐 컴파일 에러가 아니다.** 실제 컴파일 에러는 `grep -cE 'error:'` 로 확인한다.

**`| tail -N` 로 빌드를 보지 않는다.** 파이프 버퍼링 때문에 출력이 멈춘 것처럼 보여 멀쩡한 빌드를 중단하게 된다. `tee` 로 로그 파일에 남기고 별도 창에서 `tail` 한다.

**동시에 두 개의 `make` 를 돌리지 않는다.** 가용 메모리 1.5GB에서 `cc1plus` 두 개가 뜨면 바로 한계에 닿는다. 중단한 빌드의 유령 프로세스가 남아 있는지 `pgrep -a cc1plus` 로 확인하는 습관이 필요하다.

**`NESTC_USE_PRECOMPILED_EVTA_LIBRARY=ON` 을 쓰지 않는다.** 공식 install.md에 있으나 S3가 죽어 404로 실패한다. 기본값 OFF면 서브모듈 소스에서 빌드하여 정상 동작한다. 공식 문서의 다른 부분도 같은 이유로 신뢰할 수 없다.

**`/home/xilinx/nest-compiler/exec1` 에서 빌드하지 않는다.** cmake 캐시에 `-fno-rtti` 가 오염되어 있다.

### 11.6 PC 측 번들 생성 환경 (WSL + Docker)

4.2절의 명령을 실행할 환경이다. **도커 이미지는 빌드 환경만 제공하며 컴파일러는 포함되어 있지 않다.** 호스트 디렉터리를 컨테이너에 마운트하고 그 안에서 상류를 빌드하면, 결과물은 컨테이너가 아니라 호스트에 남는다.

| 항목 | 값 |
|---|---|
| 호스트 작업 디렉터리 | `~/nestc-upstream` (WSL) |
| 컨테이너 안 경로 | `/root/nestc` |
| 빌드 결과물 | `~/nestc-upstream/build/glow/bin/{model-compiler,image-classifier}` |
| 컨테이너 이름 | `nestc` |

**번들 생성에는 `leejaymin/nestc-ssh:latest` 를 쓴다.** 6.2절의 검증이 이 환경에서 이루어졌다.

**CI 빌드에는 ETRI 공식 SDK `onesai1/nest-compiler-sdk:1.0.0` 을 쓴다.** 상류 `.gitlab-ci.yml` 이 지정하는 태그를 그대로 따랐다. 정의는 `gitlab.com/ones-ai/nest-compiler-sdk` 에 있다.

주의할 점이 둘 있다. 공식 SDK의 `1.0.0` 과 `latest` 는 **서로 다른 이미지다.** 다이제스트가 다르고 크기도 4.68GB와 6.26GB로 차이 난다. 그리고 `1.0.0` 에는 `llvm-config` 가 PATH에 없어 cmake가 LLVM을 자동으로 찾지 못할 수 있으므로 `-DLLVM_DIR=/usr/lib/llvm-8/lib/cmake/llvm` 을 명시하는 편이 안전하다. 공식 SDK로 번들을 생성해 본 적은 아직 없으므로, 환경을 통일하려면 6.2절의 기준값과 먼저 대조해야 한다.

보정 프로파일은 **받는 것이 아니라 생성하는 것**이다. `-dump-profile` 로 VTAInterpreter를 돌려 레이어별 Min/Max/Histogram을 수집한다. 이 사실을 몰라 이틀을 소모하였으므로 특히 강조해 둔다.

### 11.7 CI 설정

러너는 PC의 WSL에 둔다(10.5절). 저장소 Settings → Actions → Runners → New self-hosted runner에서 Linux / x64를 선택하고 안내대로 진행한 뒤 서비스로 등록한다.

```sh
sudo ./svc.sh install && sudo ./svc.sh start
```

보드 접속용 키를 만들고 공개키를 보드에 등록한다.

```sh
ssh-keygen -t ed25519 -f ~/.ssh/board_ci -N ""
cat ~/.ssh/board_ci.pub       # 이 한 줄을 보드의 ~/.ssh/authorized_keys 에 추가
ssh -i ~/.ssh/board_ci -p 25022 xilinx@sota.pusan.ac.kr 'echo OK'
```

**마지막 확인이 WSL에서 성공해야 한다.** WSL2는 자체 NAT를 쓰므로 보드의 사설 주소 `192.168.1.40` 에는 닿지 않는다. 반드시 포워딩 주소(`sota.pusan.ac.kr:25022`)를 사용한다. 이 점을 몰라 CI 첫 실행이 `Connection timed out` 으로 실패하였다.

저장소 시크릿 네 개를 등록한다.

| 이름 | 값 |
|---|---|
| `BOARD_HOST` | `sota.pusan.ac.kr` |
| `BOARD_PORT` | `25022` |
| `BOARD_USER` | `xilinx` |
| `BOARD_SSH_KEY` | `cat ~/.ssh/board_ci` 출력 전체 |

보드에서 sudo 비밀번호를 생략하도록 설정한다. 워크플로가 `sudo ctest` 를 비대화식으로 실행하기 때문이다.

```sh
echo 'xilinx ALL=(ALL) NOPASSWD: ALL' | sudo tee /etc/sudoers.d/xilinx-ci
sudo chmod 440 /etc/sudoers.d/xilinx-ci
sudo -n true && echo NOPASSWD-OK
```

Actions 탭에서 `board-test` 를 `Run workflow` 로 실행한다. 첫 실행은 두 경로를 처음부터 빌드하므로 두 시간가량 걸리고, 이후에는 증분이라 훨씬 짧다.

**토큰과 키는 터미널과 시크릿 입력란에만 입력한다.** 채팅이나 문서에 붙여넣지 않는다. 노출된 자격 증명은 즉시 폐기하고 재발급한다.

---

## 12. 현재 상태 갱신 (2026-10-08 기준)

**동작하는 것.** ResNet-18과 ResNet-50이 VTA에서 정상 추론한다. 상류 51개 ZCU102 테스트가 generic 경로에서 전수 통과한다. aarch64 경로는 48/50이며 실패 2건의 원인이 모두 규명되어 있다. 자체 번들 생성 파이프라인이 ETRI 배포 번들과 동일한 결과를 산출한다. CI가 두 경로를 자동 검증한다.

**푸시로 추론까지 자동화되었다(2026-10-08).** `models/<이름>.conf` 하나를 저장소에 푸시하면 CI가 번들을 생성해 보드로 보내고, 컴파일·추론·판정까지 수행한다. ResNet-18로 전 구간이 통과했다. 구성은 다음과 같다.

| 파일 | 역할 |
|---|---|
| `models/*.conf` | 모델 하나를 기술한다. ONNX 경로, 입력 이름·형상, 전처리, Main.cpp, 기대값 |
| `scripts/gen-bundle.sh` | 보정 프로파일 생성 → 번들 생성. 컨테이너 안에서 돈다 |
| `scripts/board-build-run.sh` | 보드에서 컴파일·링크·실행·판정 |
| `.github/workflows/model-test.yml` | 위 둘을 잇는다. `models/**` 푸시로 기동 |

설계에서 중요한 결정이 셋이다.

**번들 생성은 컨테이너 안에서 한다.** `model-compiler` 는 Ubuntu 20.04 컨테이너에서 빌드되어 `libprotobuf.so.17` 을 요구하는데, 러너가 사는 WSL은 22.04라 그 라이브러리가 없다. 워크플로가 `docker run` 으로 같은 이미지를 띄우고, 상류 트리를 `/src` 에 마운트한다. `/root` 아래에 두면 `--user` 로 권한을 낮춘 컨테이너가 접근하지 못한다.

**판정 기준은 "정답"이 아니라 "어제와 같은 값"이다.** `cat_285.png` 의 기대값은 285가 아니라 281이다. 285는 Egyptian cat, 281은 tabby cat이므로 모델은 틀렸고 확신도도 0.53으로 낮다. 그래도 281로 고정한다. 이 테스트가 보는 것은 모델의 정확도가 아니라 파이프라인의 재현성이기 때문이다. 285로 두면 영원히 실패해 아무 신호도 주지 못한다. 모델 정확도 평가는 ImageNet 검증셋을 쓰는 별개의 작업이다.

**보드는 한 대뿐이다.** `concurrency: zcu102-board` 로 실행이 겹치지 않게 하되, 앞선 실행을 취소하지 않고 기다리게 했다. VTA 실행 중 취소는 FPGA와 xlnk를 정리하지 못한 채 끝나므로 재부팅 전까지 보드를 쓸 수 없다. 같은 이유로 CI는 한 저장소에만 건다. `concurrency` 는 저장소 단위라 서로를 막아 주지 못한다.

자동 실행에서 확인된 미해결 사항이 둘 있다. 추론 시간이 1477ms로 ETRI 배포 번들의 110ms보다 13배 느린데, 이는 generic과 aarch64 폴백의 차이와 일치하므로 보드 빌드 디렉터리를 `build_aarch64` 로 바꾸면 해소될 것으로 보인다. 그리고 확신도가 0.533373으로 배포본의 0.478113과 다른데, `VTASkipQuantizeNodes.txt` 를 번들에 포함하지 않은 탓으로 추정된다.

**막혀 있는 것.** VTA 백엔드에 `TouchInst` 와 `InsertTensorInst` 가 구현되어 있지 않아 Concat을 처리하지 못한다(5.3절). SqueezeNet, Inception, DenseNet이 여기서 막힌다. 7절 로드맵의 1단계가 이것이다.

**아직 하지 않은 것.** 상류 결함 보고 세 건(`transpose()` 미구현, aarch64 FC의 반환값 미검사, `*_compare` 의 바이트 단위 비교). 수신처는 `yongin.kwon@etri.re.kr` 이다. 세 건 모두 재현 절차와 수정안이 준비되어 있다.

---

## 13. 저장소

```
.github/workflows/      CI 정의
ci/patch_transpose.py   상류 transpose 결함 패치
ci-templates/           CI 템플릿과 설명
docs/                   01~07 문서
patches/                상류 패치 모음
scripts/                보조 스크립트
backups/                설정 백업
```

개인 원본은 `github.com/udonghun/nestc`, 연구실 저장소는 `github.com/SOTA-PNU/nestc-zcu102` 이며 두 곳에 동일하게 푸시한다. CI는 후자에 등록되어 있다.
