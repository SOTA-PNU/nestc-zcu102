#!/usr/bin/env python3
"""
독립 ReLU 의 재양자화에서 int8 포화를 복원한다.

Glow 는 ReLU 를 가능하면 컨볼루션에 합친다(doRelu). 합쳐지지 않은 ReLU —
예를 들어 Concat 뒤에 오는 것 — 는 번들에서 `relu(in, out, size, inScale,
outScale)` 로 따로 호출된다. 그 구현이 이렇다.

    output[i] = input[i] > 0 ? input[i] : 0;      // int8_t
    output[i] *= (int)(inputScale/outputScale);   // int8_t 에 그대로 대입

두 가지가 틀렸다.

  1. 포화가 없다. int8_t 값 100 에 2 를 곱하면 200 이 되어 -56 으로 뒤집힌다.
     63 을 넘는 모든 활성값이 음수가 된다.
  2. 배율을 int 로 자른다. 배율이 0.5 면 (int)0.5 == 0 이라 전부 0 이 된다.

같은 연산을 VTAInterpreter 는 이렇게 한다(VTANodes.cpp:1210
fwdVTAReluInstQuantizedImpl). 이쪽이 정답을 낸다.

    scale = outScale / inScale
    scale < 1 이면 왼쪽 시프트, 아니면 오른쪽 시프트
    int32 로 계산한 뒤 MAX(val, 0) 과 clip<int32,int8>

이 패치는 번들 구현을 인터프리터와 같은 식으로 바꾼다. 2의 거듭제곱 스케일
(symmetric_with_power2_scale)을 쓰므로 시프트로 정확히 떨어진다.

ResNet 계열은 ReLU 가 전부 컨볼루션에 합쳐져 이 경로를 타지 않는다. 그래서
지금까지 드러나지 않았다. SqueezeNet 은 Fire 모듈마다 Concat 뒤 ReLU 가 있고
스케일이 0.5 -> 0.25 라 배율이 2 가 되어 정확히 이 버그를 밟는다.

멱등하다. 기준 문자열을 찾지 못한 파일은 건너뛰고, 하나도 고치지 못하면 실패한다.

사용법:
    python3 patch_relu_rescale.py <nest-compiler 루트>
"""
import os
import shutil
import sys
import time

MARKER = "nestc_relu_rescale_patch"

OLD = """  for(int i = 0; i < size; i ++){
    if(input[i]>0)
      output[i]=input[i];
    else
      output[i]=0;
    output[i] *= (int)(inputScale/outputScale);
  }
"""

NEW = """  /* %s
   * VTAInterpreter(VTANodes.cpp:1210 fwdVTAReluInstQuantizedImpl)와 같은 식으로
   * 계산한다. int32 로 올려 시프트한 뒤 0..127 로 포화시킨다.
   * 원래 구현은 int8_t 에 바로 곱해 넘침을 그대로 두었고, 배율을 int 로 잘라
   * 1 미만이면 0 이 되었다.
   */
  {
    float nestcScale = (inputScale != 0.0f) ? (outputScale / inputScale) : 1.0f;
    float nestcTemp = 1.0f;
    int nestcShift = 0;
    int nestcLeft = 0;
    unsigned_t i;
    if (nestcScale < 1.0f) {
      nestcLeft = 1;
      while (nestcTemp > nestcScale) {
        nestcTemp /= 2;
        nestcShift++;
      }
    } else {
      while (nestcTemp < nestcScale) {
        nestcTemp *= 2;
        nestcShift++;
      }
    }
    for (i = 0; i < size; i++) {
      int32_t val = input[i];
      val = nestcLeft ? (val << nestcShift) : (val >> nestcShift);
      if (val < 0) {
        val = 0;
      }
      if (val > 127) {
        val = 127;
      }
      output[i] = (int8_t)val;
    }
  }
""" % MARKER

CANDIDATES = [
    "vta/vtalib/lib/Bundle/CPUBundle_generic.cpp",
    "vta/vtalib/lib/Bundle/CPUBundle_aarch64.cpp",
    "vta/vtalib/lib/Bundle/CPUBundle_x86.cpp",
]

# 보드가 실제로 쓰는 경로. 여기만은 반드시 고쳐야 한다.
REQUIRED = "vta/vtalib/lib/Bundle/CPUBundle_generic.cpp"


def main():
    if len(sys.argv) != 2:
        print(__doc__)
        return 2
    root = sys.argv[1]

    planned, skipped, missing = [], [], []
    for rel in CANDIDATES:
        path = os.path.join(root, rel)
        if not os.path.exists(path):
            missing.append(rel)
            continue
        src = open(path, encoding="utf-8").read()
        if MARKER in src:
            skipped.append(rel)
            continue
        if OLD not in src:
            # 구현이 다르다. 조용히 넘기지 않고 알린다.
            print("경고: 기준 구현과 다름, 건너뜀 — %s" % rel, file=sys.stderr)
            missing.append(rel)
            continue
        planned.append((path, rel, src.replace(OLD, NEW, 1)))

    if REQUIRED in missing:
        print("실패: %s 를 고치지 못했다. 보드가 쓰는 경로다." % REQUIRED,
              file=sys.stderr)
        return 1

    if not planned:
        print("전부 이미 패치되어 있다.")
        return 0

    stamp = time.strftime("%Y%m%d-%H%M%S")
    for path, rel, out in planned:
        shutil.copy2(path, "%s.relubak.%s" % (path, stamp))
        with open(path, "w", encoding="utf-8") as f:
            f.write(out)
        print("패치 %s" % rel)
    for rel in skipped:
        print("건너뜀 %s (이미 패치됨)" % rel)

    print("\n다음 — 보드에서 vtalib 을 다시 빌드한다:")
    print("  cd <빌드 디렉터리> && make -j1 VTABundle")
    return 0


if __name__ == "__main__":
    sys.exit(main())
