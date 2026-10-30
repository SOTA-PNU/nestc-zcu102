# VTA 컨볼루션 타일링 기본값 (2026-10-30)

## 무엇이 문제인가

VTA 컨볼루션은 출력을 타일로 쪼개 FPGA 에 올린다. 타일 크기는
`include/Backends/VTA/VTASchedules.h` 에서 가져오는데, 이것은 알고리즘이 아니라
**레이어 모양을 통째로 외워 둔 손튜닝 표**다.

```cpp
bool isMatch(inputN, inputH, inputW, inputC,
             filterN, filterH, filterW, filterC, stride, pad)
```

열 개가 전부 정확히 일치해야 한다. 표에 든 항목은 열한 개이고 전부 ResNet
모양(56/28/14/7, 채널 64/128/256/512)이다. 하나도 맞지 않으면 `VTASave.cpp` 가
`", 1, 14, 14, "` 를 그대로 박는다.

즉 **ResNet 계열이 아닌 모든 모델은 타일링이 기본값으로 떨어진다.**

## 왜 기본값이 위험한가

표의 열한 항목을 전부 계산해 보면 한 가지가 눈에 띈다.

| 입력 | 필터 | stride | 출력 | tileH | tileW | H 타일 수 | W 타일 수 |
|---|---|---|---|---|---|---|---|
| 56×56×64 | 64 3×3 | 1 | 56×56 | 8 | 56 | 7 | **1** |
| 56×56×64 | 128 1×1 | 2 | 28×28 | 7 | 28 | 4 | **1** |
| 56×56×64 | 128 3×3 | 2 | 28×28 | 8 | 28 | 4 (나머지 4) | **1** |
| 28×28×128 | 128 3×3 | 1 | 28×28 | 14 | 28 | 2 | **1** |
| 28×28×128 | 256 1×1 | 2 | 14×14 | 7 | 14 | 2 | **1** |
| … | | | | | | | **1** |

**`tileWSize` 가 언제나 출력 폭과 같다.** 상류는 폭 방향으로 타일을 나눠 본 적이
한 번도 없다. 높이 방향은 나머지가 생기는 경우(28 을 8 로 → 마지막 타일 4)까지
쓰이고 정상 동작한다.

그런데 기본값 `tileW = 14` 는 출력 폭이 14 의 배수가 아니면 폭을 여러 조각으로
나눈다. SqueezeNet 의 Fire 모듈은 출력이 55×55 라 폭이 **네 조각**으로 나뉜다.
ETRI 가 한 번도 실행해 본 적 없는 경로다.

## 고친 방식 — `ci/patch_tiling.py`

표에 없을 때의 기본값을 고정 상수에서 계산으로 바꾼다.

```cpp
static std::string nestcFallbackTiling(uint64_t outH, uint64_t outW) {
  uint64_t tileW = outW ? outW : 1;            // 폭은 나누지 않는다
  uint64_t tileH = outH < 14 ? outH : 14;
  if (tileH == 0) tileH = 1;
  while (tileH > 1 && tileH * tileW > 448) tileH--;   // 표의 최대 면적 8*56
  return ", 1, " + std::to_string(tileH) + ", " + std::to_string(tileW) + ", ";
}
```

표의 모든 항목과 같은 방식이고 모델에 무관하다. SqueezeNet 전용 예외가 아니다.

SqueezeNet 의 55×55 레이어는 `1, 8, 55` 가 된다. 55 = 6×8 + 7 이라 높이에만
나머지가 생기는데, 그 경로는 ResNet 이 이미 쓰고 있어 검증되어 있다.

`VTASave.cpp` 안에 기본값을 박는 자리가 네 군데다. 그중 `if (!isTuned)` 바로
아래 셋만 고치고, BNN 전용 분기(`bnn_with_scale`) 하나는 조건 분기가 따로 있어
건드리지 않는다. 함수마다 출력 차원의 인덱스가 다르므로(NHWC 는 `[1],[2]`,
NCHW 는 `[2],[3]`) 스크립트가 각 자리 앞의 `dest->dims()` 사용을 보고 맞춘다.

## 중요 — 보드는 다시 빌드하지 않아도 된다

타일 크기는 `convolution_wo_tr` 의 **런타임 인자**다. 바뀌는 것은 생성되는 번들
소스의 숫자뿐이다. 그러므로

- PC 컨테이너: `ninja model-compiler` (증분, 1~2분)
- 보드: 번들만 다시 컴파일 (vtalib 재빌드 불필요)

`patch_concat.py` 와는 다르다. 그쪽은 `insert_tensor` 라는 새 함수를 vtalib 에
넣으므로 보드 빌드가 필요했다.

## 자동화

`model-test` 워크플로에 「컴파일러 패치 적용 및 재빌드」 단계를 넣었다.
`ci/**` 가 바뀌면 워크플로가 저절로 돌고, 패치는 멱등하므로 바뀐 것이 있을 때만
컴파일러를 다시 빌드한다. 실험 한 번이 "파일 고치고 푸시"가 된다.

## 상류에 보고할 것

`VTASchedules.h` 의 손튜닝 표는 ResNet 전용이며, 표에 없는 레이어의 기본값
`1, 14, 14` 는 폭 방향 타일링을 깨운다. 그 경로는 표의 어떤 항목도 밟지 않으므로
사실상 검증되지 않은 상태다. 최소한 기본값을 `tileW = 출력 폭` 으로 두기를 권한다.


## 왜 타일 크기를 바꿔도 결과가 같았나 (2026-10-30 밤)

패치를 넣고 보드에서 다시 돌렸더니 SqueezeNet 결과가 905/904/533 으로, 신뢰도
끝자리까지 **완전히 동일**했다. 타일 크기가 실제로 `1, 14, 14` 에서
`1, 8, 55` 로 바뀐 것은 생성 코드에서 확인했는데도 그랬다.

`VTABundle.cpp` 의 `conv_kernel` 을 보면 이유가 나온다.

```c
uint32_t ACC_MEM_ENTRY = 2048;
uint32_t nFilterinLoop = KN / nVirtualThread / 16;
nFilterinLoop = min(nFilterinLoop,
                    ACC_MEM_ENTRY / nVirtualThread / outTileH / outTileW);
for (; nFilterinLoop > 0; nFilterinLoop--)
  if ((KN/16) % (nFilterinLoop*nVirtualThread) == 0) break;
```

타일 크기는 이 `min` 의 둘째 항에만 들어간다. SqueezeNet 의 55×55 층에서

| 타일 | 둘째 항 | squeeze (KN=16) | expand (KN=64) |
|---|---|---|---|
| 14×14 (옛 기본값) | 2048/196 = 10 | min(1,10) = 1 | min(4,10) = 4 |
| 8×55 (새 기본값) | 2048/440 = 4 | min(1,4) = 1 | min(4,4) = 4 |

**둘 다 같은 값이 나온다.** 즉 발행되는 VTA 명령 구성이 동일하므로 결과가 같은
것이 당연하다. 패치가 무효였던 것이 아니라, 이 모델에서는 구분되지 않는 변경이다.

폭 방향 타일링이 상류에서 한 번도 쓰이지 않았다는 사실은 그대로이고, 다른
모델에서는 드러날 수 있으므로 패치는 유지한다.

## 덤으로 좁혀진 것 — `nVirtualThread = 1`

같은 식에서 `KN = 16` 이면 `KN / nVirtualThread / 16` 이 1 이 되어야 하므로
`nVirtualThread` 는 **1 일 수밖에 없다**. 2 로 두면 `16/2/16 = 0` 이 되고
바로 아래에서 0 으로 나눈다.

그런데 튜닝 표의 ResNet 항목 열한 개는 **전부 `nVirtualThread = 2`** 다.
즉 SqueezeNet 의 squeeze 층(출력 16채널)은 "출력 블록 하나, 가상 스레드 하나" 라는
ResNet 이 한 번도 밟지 않은 조합으로 들어간다. 폭 타일링과 같은 종류의
미검증 경로이고, 지금 가장 유력한 자리다.

확인하려면 층별 덤프(`debug-dump` 워크플로)로 squeeze 층의 출력부터 보면 된다.
거기가 처음 망가지는 지점이면 가설이 맞는 것이다.
