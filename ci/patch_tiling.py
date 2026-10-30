#!/usr/bin/env python3
"""
튜닝 표에 없는 레이어의 기본 타일링을 고친다.

VTA 컨볼루션은 출력을 타일로 나눠 FPGA 에 올린다. 타일 크기는
`include/Backends/VTA/VTASchedules.h` 의 손튜닝 표에서 가져오는데, 그 표는
레이어 모양 열 개(입력 NHWC, 필터 NHWC, stride, pad)를 전부 정확히 일치시켜야
하고 들어 있는 항목은 전부 ResNet 모양이다. 어긋나면 `VTASave.cpp` 가
`", 1, 14, 14, "` 를 그대로 박는다.

표의 항목을 전부 계산해 보면 **tileWSize 가 언제나 출력 폭과 같다.** 즉 상류는
폭 방향으로 타일을 나눠 본 적이 한 번도 없다(nTileW 가 늘 1). 높이 방향은
나머지가 생기는 경우까지 쓰이고 잘 돈다.

그래서 기본값 14 는 출력 폭이 14 의 배수가 아닌 모델에서 검증된 적 없는 경로로
들어간다. SqueezeNet 의 Fire 모듈은 출력이 55×55 라 폭이 네 조각으로 나뉜다.

이 패치는 기본값을 이렇게 바꾼다.

  - 폭은 나누지 않는다 (tileW = 출력 폭). 표의 모든 항목과 같은 방식이다.
  - 높이만 나눈다. 14 에서 시작하되 타일 면적을 448(표의 최대치 8×56)로 묶어
    VTA 버퍼를 넘지 않게 한다.

모델에 무관한 규칙이라 SqueezeNet 전용 예외가 아니다. 타일 크기는 런타임
인자이므로 **보드의 vtalib 을 다시 빌드할 필요가 없다.** 컴파일러만 다시
빌드하고 번들을 새로 만들면 된다.

멱등하다. 기준점을 하나라도 찾지 못하면 아무것도 바꾸지 않는다.

사용법:
    python3 patch_tiling.py <nest-compiler 루트>
"""
import os
import re
import shutil
import sys
import time

MARKER = "nestc_tiling_fallback_patch"

HELPER_ANCHOR = "void saveFloatConvolutionInst(const glow::ConvolutionInst *Inst,"

HELPER = """/* %s
 * 튜닝 표(VTASchedules.h)에 없는 레이어의 기본 타일링.
 *
 * 상류는 ", 1, 14, 14, " 를 그대로 쓴다. 그런데 표의 항목을 전부 계산해 보면
 * tileWSize 는 언제나 출력 폭과 같다 — 폭 방향 타일링은 한 번도 실행된 적이
 * 없다. 출력 폭이 14 의 배수가 아닌 모델(예: SqueezeNet 의 55x55)에서는
 * 기본값 14 가 검증되지 않은 경로를 깨운다.
 *
 * 폭은 통째로 두고 높이만 나눈다. 타일 면적은 표의 최대치(8*56=448)로 묶어
 * VTA 입력 버퍼를 넘지 않게 한다.
 */
static std::string nestcFallbackTiling(uint64_t outH, uint64_t outW) {
  uint64_t tileW = outW ? outW : 1;
  uint64_t tileH = outH < 14 ? outH : 14;
  if (tileH == 0) {
    tileH = 1;
  }
  while (tileH > 1 && tileH * tileW > 448) {
    tileH--;
  }
  return ", 1, " + std::to_string(tileH) + ", " + std::to_string(tileW) + ", ";
}

""" % MARKER

# 기본값을 박는 줄. 셋 다 같은 모양이라 줄 번호가 아니라 문맥으로 가린다.
FALLBACK_RE = re.compile(r'^(\s*)bundle->append\(", 1, (\d+), (\d+), "\);\s*$')
ISTUNED_RE = re.compile(r'^\s*if\s*\(\s*!isTuned\s*\)\s*\{\s*$')
DIMS_RE = re.compile(r'bundle->append\(std::to_string\(dest->dims\(\)\[(\d+)\]\)\);')


def patch_save(path):
    try:
        src = open(path, encoding="utf-8").read()
    except OSError as e:
        return None, "열 수 없음: %s" % e

    if MARKER in src:
        return None, "이미 패치됨"

    if HELPER_ANCHOR not in src:
        return None, "기준점 없음: saveFloatConvolutionInst"

    lines = src.split("\n")
    hits = []

    for i, line in enumerate(lines):
        m = FALLBACK_RE.match(line)
        if not m:
            continue

        # 바로 앞의 빈 줄이 아닌 줄이 `if (!isTuned) {` 인 것만 고친다.
        # BNN 쪽에는 `if(bnn_with_scale)` 안에 같은 모양의 줄이 있는데,
        # 거기는 조건 분기가 따로 있으므로 건드리지 않는다.
        j = i - 1
        while j >= 0 and not lines[j].strip():
            j -= 1
        if j < 0 or not ISTUNED_RE.match(lines[j]):
            continue

        # out_h / out_w 가 몇 번째 차원인지는 함수마다 다르다
        # (NHWC 는 [1],[2], NCHW 는 [2],[3]). 이 줄 앞에서 가장 가까운
        # dest->dims()[k] 두 개를 그대로 쓴다.
        dims = []
        for k in range(i - 1, max(i - 80, -1), -1):
            found = DIMS_RE.findall(lines[k])
            if found:
                dims = found + dims
            if len(dims) >= 2:
                break
        if len(dims) < 2:
            return None, "출력 차원을 찾지 못함 (줄 %d)" % (i + 1)

        outH, outW = dims[-2], dims[-1]
        hits.append((i, m.group(1), outH, outW))

    if not hits:
        return None, "고칠 기본값 줄을 찾지 못함"

    for i, indent, outH, outW in hits:
        lines[i] = ("%sbundle->append(nestcFallbackTiling("
                    "dest->dims()[%s], dest->dims()[%s]));" % (indent, outH, outW))

    out = "\n".join(lines)
    out = out.replace(HELPER_ANCHOR, HELPER + HELPER_ANCHOR, 1)
    return out, None


def main():
    if len(sys.argv) != 2:
        print(__doc__)
        return 2
    root = sys.argv[1]
    path = os.path.join(root, "lib/Backends/VTA/VTASave.cpp")

    out, err = patch_save(path)
    if err == "이미 패치됨":
        print("이미 패치되어 있다.")
        return 0
    if err:
        print("실패 %s\n  %s" % (path, err), file=sys.stderr)
        print("아무것도 바꾸지 않았다.", file=sys.stderr)
        return 1

    bak = "%s.tilebak.%s" % (path, time.strftime("%Y%m%d-%H%M%S"))
    shutil.copy2(path, bak)
    with open(path, "w", encoding="utf-8") as f:
        f.write(out)
    print("패치 %s  (백업 %s)" % (path, os.path.basename(bak)))
    print("\n다음 — 컨테이너에서 컴파일러만 다시 빌드한다 (보드는 그대로 둔다):")
    print("  cd /root/nestc/build && ninja model-compiler")
    return 0


if __name__ == "__main__":
    sys.exit(main())
