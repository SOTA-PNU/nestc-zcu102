#!/usr/bin/env python3
"""
보드가 떨군 층별 중간 텐서를 훑어 처음 망가지는 지점을 찾는다.

`gen-bundle.sh` 를 INSTRUMENT_DEBUG=1 로 돌리면 번들에 층마다 텐서를 파일로
쓰는 코드가 들어간다(-instrument-debug -instrument-debug-format=rawbin).
보드에서 추론을 한 번 돌리면 작업 디렉터리에 .bin 이 잔뜩 생긴다.

이 스크립트는 그 파일들을 **쓰여진 순서대로**(mtime) 훑으며 분포를 본다.
양자화된 int8 텐서가 정상이라면 값이 어느 정도 퍼져 있어야 한다. 다음 중
하나라도 보이면 그 층에서 계산이 깨진 것이다.

  - 전부 0        앞단이 0 을 내보냈거나 재양자화 배율이 0 으로 잘렸다
  - 전부 같은 값   버퍼가 초기화되지 않았거나 덮어써지지 않았다
  - 포화 과다      스케일이 잘못돼 127/-128 에 몰렸다
  - 음수 과다      ReLU 뒤인데 음수가 많다면 넘침으로 뒤집힌 것이다

판정이 아니라 단서다. "여기부터 이상하다"를 찾으면 그 층의 연산 하나만
들여다보면 된다. 함수를 처음부터 하나씩 읽는 것보다 훨씬 빠르다.

사용법:
    python3 analyze_debug_dump.py <덤프 디렉터리> [calib.yaml]
"""
import os
import struct
import sys


def stats(path):
    with open(path, "rb") as f:
        b = f.read()
    n = len(b)
    if n == 0:
        return None
    vals = struct.unpack("%db" % n, b)
    mn = min(vals)
    mx = max(vals)
    total = float(n)
    zeros = sum(1 for v in vals if v == 0)
    neg = sum(1 for v in vals if v < 0)
    sat = sum(1 for v in vals if v == 127 or v == -128)
    mean = sum(vals) / total
    return {
        "n": n, "min": mn, "max": mx, "mean": mean,
        "zero%": 100.0 * zeros / total,
        "neg%": 100.0 * neg / total,
        "sat%": 100.0 * sat / total,
    }


def verdict(s):
    out = []
    if s["min"] == s["max"]:
        out.append("전부 %d" % s["min"])
    if s["zero%"] > 99.0:
        out.append("거의 전부 0")
    if s["sat%"] > 20.0:
        out.append("포화 %.0f%%" % s["sat%"])
    if s["neg%"] > 45.0:
        out.append("음수 %.0f%%" % s["neg%"])
    return ", ".join(out)


def load_profile(path):
    """calib.yaml 에서 이름별 (min, max) 만 거칠게 긁는다. 파서는 쓰지 않는다."""
    ranges = {}
    name = None
    mn = mx = None
    try:
        for line in open(path, encoding="utf-8", errors="replace"):
            t = line.strip()
            if t.startswith("- nodeOutputName:"):
                name = t.split(":", 1)[1].strip().strip('"\'')
                mn = mx = None
            elif name and t.startswith("min:"):
                mn = t.split(":", 1)[1].strip()
            elif name and t.startswith("max:"):
                mx = t.split(":", 1)[1].strip()
                ranges[name] = (mn, mx)
                name = None
    except OSError:
        return {}
    return ranges


def main():
    if len(sys.argv) not in (2, 3):
        print(__doc__)
        return 2
    d = sys.argv[1]
    prof = load_profile(sys.argv[2]) if len(sys.argv) == 3 else {}

    files = [os.path.join(d, f) for f in os.listdir(d) if f.endswith(".bin")]
    # 가중치 파일은 층 출력이 아니다.
    files = [f for f in files if not f.endswith(".weights.bin")]
    if not files:
        print("덤프 .bin 을 찾지 못했다: %s" % d, file=sys.stderr)
        return 1
    # 주입된 덤프는 dump_<순번>_<이름>.bin 이라 이름순이 곧 실행 순서다.
    # 그렇지 않으면 쓰여진 시각으로 정렬한다.
    import re as _re
    if all(_re.match(r"dump_\d+_", os.path.basename(f)) for f in files):
        files.sort(key=lambda f: int(os.path.basename(f).split("_")[1]))
    else:
        files.sort(key=lambda f: os.path.getmtime(f))

    print("%-44s %9s %5s %5s %8s %6s %6s %6s  %s" %
          ("파일(실행 순서)", "원소", "최소", "최대", "평균", "0%", "음수%", "포화%", "소견"))
    flagged = []
    for f in files:
        s = stats(f)
        if s is None:
            print("%-44s (빈 파일)" % os.path.basename(f)[:44])
            continue
        v = verdict(s)
        if v:
            flagged.append((os.path.basename(f), v))
        print("%-44s %9d %5d %5d %8.2f %6.1f %6.1f %6.1f  %s" %
              (os.path.basename(f)[:44], s["n"], s["min"], s["max"], s["mean"],
               s["zero%"], s["neg%"], s["sat%"], v))

    print()
    if flagged:
        print("처음 이상해지는 지점: %s  (%s)" % (flagged[0][0], flagged[0][1]))
        print("그 층의 연산 하나만 보면 된다. 그 앞은 정상이라는 뜻이다.")
    else:
        print("분포만으로는 이상한 층이 없다.")
        print("그러면 값이 조용히 틀린 경우다 — 인터프리터와 수치를 직접 비교해야 한다.")

    if prof:
        print("\ncalib.yaml 에 기록된 텐서 %d 개의 범위를 읽었다." % len(prof))
        print("이름이 겹치는 덤프가 있으면 보정 범위와 대조해 볼 수 있다.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
