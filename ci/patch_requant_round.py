#!/usr/bin/env python3
"""
VTA 컨볼루션의 재양자화를 버림에서 반올림으로 바꾼다. (환경변수로 켠다)

## 왜

SqueezeNet 을 보드에서 층별로 떠서 보정 기록과 대조하니, 보정이 예상한 크기
대비 실측 비율이 층을 거치며 단조롭게 줄었다.

    0.78 -> 0.91 -> 0.79 -> 0.72 -> 0.75 -> 0.64 -> 0.36 -> 0.28

ResNet-18 로 같은 대조를 하면 1 근처에서 흩어질 뿐 추세가 없다. 즉 특정 층이
깨진 것이 아니라 SqueezeNet 에서만 매 층 조금씩 크기가 샌다.

VTA 의 재양자화는 누산기를 오른쪽으로 `shift` 비트 미는 것이다. 산술 우시프트는
버림이므로 매번 평균 0.5 LSB 아래로 치우친다. 숫자가 맞는다 — SqueezeNet 의
VTA 컨볼루션은 24개이고, 층당 유지율 r 에 대해 r^24 = 0.28 이면 r = 0.948,
즉 층당 5.2% 손실이다. 그 층들의 전형적인 양자값이 10~20 이므로 0.5 LSB 는
2.5~5% 에 해당한다.

ResNet 이 버티는 이유도 설명된다. 채널이 64 이상이라 누산 항이 많아 상대
편향이 작고, 잔차 덧셈이 크기를 되살린다. SqueezeNet 의 squeeze 층은 출력이
16 채널인 1x1 이라 그 완충이 없다.

## 어떻게

버림 시프트를 반올림으로 만드는 고전적인 수법은 시프트 전에 1 << (shift-1) 을
더하는 것이다. VTA 에서 누산기에 더해지는 값은 바이어스이므로, 바이어스에 그
값을 더해 두면 하드웨어를 바꾸지 않고 같은 효과를 낸다.

바이어스 버퍼를 제자리에서 고치고 VTASynchronize 뒤에 원복한다. 동기화 뒤이므로
DMA 와 겹치지 않는다. 임시 버퍼를 만들지 않아 CMA 를 더 쓰지 않는다.

## 기본값은 꺼짐

`NESTC_REQUANT_ROUND` 환경변수가 있을 때만 동작한다. 패치를 적용해도 그것을
켜지 않으면 동작이 전혀 바뀌지 않는다. 검증되지 않은 런타임 변경으로 보드를
멈추는 일을 피하려는 것이다. A/B 는 재빌드 없이 환경변수만으로 한다.

    sudo -E env NESTC_REQUANT_ROUND=1 PATH=$PATH ./resnet18 <이미지>

## 확인할 것

ResNet 기준값(281/207/340, aarch64 확신도 0.478113)이 유지되는지 반드시 본다.
반올림이 평균적으로 더 정확하므로 깨질 이유는 없지만, 기준이 바뀌면 기록한다.

사용법:
    python3 patch_requant_round.py <nest-compiler 루트>
"""
import os
import re
import shutil
import sys
import time

MARKER = "nestc_requant_round"
REL = "vta/vtalib/lib/Bundle/VTABundle.cpp"

ADD = """  /* %s
   * 버림 시프트를 반올림으로 만든다. 누산기에 더해지는 값이 바이어스이므로
   * 시프트 전에 1 << (shift-1) 을 더하는 것과 같다.
   * NESTC_REQUANT_ROUND 가 설정되어 있을 때만 동작한다.
   */
  int32_t nestcRoundAdd = 0;
  if (doBias && shift > 0 && getenv("NESTC_REQUANT_ROUND") != NULL) {
    int32_t *nestcBias = (int32_t *)VTABufferGetVirtAddr(vta_bias_buf);
    int nestcI;
    nestcRoundAdd = (int32_t)1 << (shift - 1);
    for (nestcI = 0; nestcI < KN; nestcI++) {
      nestcBias[nestcI] += nestcRoundAdd;
    }
  }

""" % MARKER

RESTORE = """
  /* %s: 원복. VTASynchronize 뒤이므로 DMA 와 겹치지 않는다. */
  if (nestcRoundAdd) {
    int32_t *nestcBias = (int32_t *)VTABufferGetVirtAddr(vta_bias_buf);
    int nestcI;
    for (nestcI = 0; nestcI < KN; nestcI++) {
      nestcBias[nestcI] -= nestcRoundAdd;
    }
  }
""" % MARKER

ANCHOR_BIAS = "  void *vta_bias_buf = bias;\n"
ANCHOR_SYNC = "  VTASynchronize(vtaCmdH, 1 << 31);\n"
FUNC_RE = re.compile(r"^int convolution_wo_tr\(int8_t", re.M)


def main():
    if len(sys.argv) != 2:
        print(__doc__)
        return 2
    path = os.path.join(sys.argv[1], REL)
    try:
        src = open(path, encoding="utf-8").read()
    except OSError as e:
        print("열 수 없음: %s" % e, file=sys.stderr)
        return 1
    if MARKER in src:
        print("이미 패치되어 있다.")
        return 0

    m = FUNC_RE.search(src)
    if not m:
        print("convolution_wo_tr 를 찾지 못했다.", file=sys.stderr)
        return 1
    start = m.start()
    end = src.find("\n}\n", start)
    if end < 0:
        print("함수 끝을 찾지 못했다.", file=sys.stderr)
        return 1
    body = src[start:end]

    if body.count(ANCHOR_BIAS) != 1:
        print("바이어스 기준점이 하나가 아니다 (%d)" % body.count(ANCHOR_BIAS),
              file=sys.stderr)
        return 1
    n_sync = body.count(ANCHOR_SYNC)
    if n_sync < 1:
        print("VTASynchronize 를 찾지 못했다.", file=sys.stderr)
        return 1

    new = body.replace(ANCHOR_BIAS, ANCHOR_BIAS + ADD, 1)
    # 마지막 동기화 뒤에 원복을 넣는다.
    i = new.rfind(ANCHOR_SYNC)
    new = new[:i + len(ANCHOR_SYNC)] + RESTORE + new[i + len(ANCHOR_SYNC):]

    out = src[:start] + new + src[end:]
    if "#include <stdlib.h>" not in out and "#include <cstdlib>" not in out:
        j = out.index("\n", out.index("#include"))
        out = out[:j + 1] + "#include <stdlib.h>  /* %s: getenv */\n" % MARKER + out[j + 1:]

    shutil.copy2(path, "%s.roundbak.%s" % (path, time.strftime("%Y%m%d-%H%M%S")))
    open(path, "w", encoding="utf-8").write(out)
    print("패치 %s  (동기화 지점 %d개 중 마지막에 원복)" % (REL, n_sync))
    print("\n보드에서 vtalib 을 다시 빌드한다:")
    print("  cd <빌드 디렉터리> && make -j1 VTABundle")
    print("켜는 법: 실행할 때 NESTC_REQUANT_ROUND=1")
    return 0


if __name__ == "__main__":
    sys.exit(main())
