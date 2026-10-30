# 독립 ReLU 재양자화의 int8 넘침 (2026-10-30)

## 증상

SqueezeNet 을 보드에서 돌리면 세 장 모두 엉뚱한 라벨이 나온다(905 / 904 / 533).
같은 모델을 같은 보정 프로파일로 VTAInterpreter 에서 돌리면 281 / 207 / 340 으로
전부 맞는다. 연산 정의와 양자화는 맞는데 보드만 틀리다는 뜻이다.

## 원인

Glow 는 ReLU 를 가능하면 컨볼루션에 합친다(`doRelu`). 합쳐지지 않은 ReLU —
Concat 뒤에 오는 것 — 는 번들에서 따로 호출된다.

```c
relu(입력, 출력, 크기, inScale, outScale)
```

`vta/vtalib/lib/Bundle/CPUBundle_generic.cpp` 의 구현:

```c
for(int i = 0; i < size; i ++){
  if(input[i]>0) output[i]=input[i]; else output[i]=0;
  output[i] *= (int)(inputScale/outputScale);
}
```

두 가지가 틀렸다.

**포화가 없다.** `output[i]` 는 `int8_t` 다. 값 100 에 배율 2 를 곱하면 200 이
되어 -56 으로 뒤집힌다. 63 을 넘는 모든 활성값이 음수가 된다.

**배율을 int 로 자른다.** 배율이 0.5 면 `(int)0.5 == 0` 이라 출력이 전부 0 이 된다.

VTAInterpreter 는 같은 연산을 이렇게 한다
(`lib/Backends/VTA/VTANodes.cpp:1210` `fwdVTAReluInstQuantizedImpl`).

```cpp
int32_t val = inW.raw(i);
val = val << shift;                                              // int32 로 계산
outW.raw(i) = quantization::clip<int32_t, int8_t>(MAX(val, 0));  // 포화
```

스케일이 2의 거듭제곱(`symmetric_with_power2_scale`)이므로 시프트로 정확히
떨어지고, int32 로 올려 계산한 뒤 clip 한다. 이쪽이 정답을 낸다.

## 왜 지금까지 드러나지 않았나

ResNet-18 과 ResNet-50 은 ReLU 가 전부 컨볼루션에 합쳐져 이 함수를 타지 않는다.
상류의 `vtaReluTestBundle` 은 통과하는데, 그 테스트는 입력과 출력 스케일이 같아
배율이 1 이라 넘칠 일이 없다.

SqueezeNet 은 Fire 모듈마다 expand1x1 과 expand3x3 을 Concat 한 뒤 ReLU 가 온다.
두 Conv 가 모두 스케일 0.5 로 쓰고 ReLU 가 0.25 로 재양자화하므로 배율이 정확히
2 다. Fire 모듈 여덟 개가 전부 이 경로를 밟는다.

## 고친 방식 — `ci/patch_relu_rescale.py`

인터프리터와 같은 식으로 바꾼다. 배율을 `outScale / inScale` 로 잡고, 1 미만이면
왼쪽 시프트, 아니면 오른쪽 시프트한 뒤 int32 에서 0..127 로 포화시킨다.

이 패치는 **보드 쪽 런타임(vtalib)** 을 고친다. 컴파일러 패치가 아니므로

- PC 컨테이너: 할 일 없음
- 보드: `make -j1 VTABundle`

`model-test` 워크플로에 「보드 런타임 패치 적용 및 재빌드」 단계로 들어가 있어
사람이 보드에 들어갈 필요는 없다.

## 추적 경로 (기록)

가설을 하나씩 지운 순서다. 다음에 비슷한 증상을 만나면 같은 순서로 좁히면 된다.

1. Concat 구현 — numpy 대조로 알고리즘 일치, 생성된 차원·오프셋·축 정확. 배제
2. 양자화 — 같은 프로파일로 float Interpreter 가 정답. 배제
3. 보드 번들이 옛 것 — 파일 시각과 전처리 상수 확인. 배제
4. 입력 레이아웃 — 번들이 NCHW 로 받아 내부에서 전치, Main.cpp 와 일치. 배제
5. VTA 연산 정의 — VTAInterpreter 가 281/207/340 으로 정답. 배제
   (이걸 돌리려면 `VTAInterpreterFunction.cpp` 에 `TouchInst` 분기를 먼저
   넣어야 했다. 그 전에는 시작하자마자 죽어 로그가 104 바이트였다.)
6. 컨볼루션 타일링 — 폭 방향 타일링이 상류에서 한 번도 쓰인 적 없어 의심했으나,
   기본값을 고쳐 타일을 바꿔도 결과가 **비트 단위로 동일**했다. 배제
   (그래도 고칠 가치가 있어 남겼다 — `vta-tiling-fallback.md`)
7. CPU 폴백 연산 — `maxpool` 은 정상, `relu` 에서 발견

5번과 6번이 결정적이었다. 인터프리터가 맞고 타일링을 바꿔도 안 변한다면 남는
것은 하드웨어가 아니라 CPU 쪽 폴백 연산뿐이다.
