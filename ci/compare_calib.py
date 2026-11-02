#!/usr/bin/env python3
"""보드가 실제로 낸 int8 값을, 그 층이 내야 할 값과 맞춰 본다.

어떻게 기준값을 얻는가.

번들 소스(생성된 .cpp)에는 층마다 양자화 스케일이 상수로 박혀 있다.

    nonvtaconvolution(입력, 1.0/32.0, 0, 가중치, 1.0/512.0, 0,
                      바이어스, 1.0/16384.0, 0, 출력, 1.0/32.0, 0, ...)

즉 출력 텐서는 `int8 값 = 실수 값 × 32` 로 저장된다. 한편 보정
프로파일(calib.yaml)에는 같은 층의 실수 범위를 인터프리터가 잰 값이 있다.
둘을 합치면 그 층이 내야 할 int8 최대값이 바로 나온다.

    기대 int8 최대 = 보정 최대 × (1/스케일),   127 에서 자른다

보드 덤프의 실제 최대값이 이것과 크게 다르면 그 층에서 값이 어긋난 것이다.
스케일을 추측하지 않는다는 것이 요점이다 — 번들에 적혀 있는 것을 읽는다.

쓰는 법:
    compare_calib.py calib.yaml 덤프디렉터리 번들.cpp

검증:
    맞는 모델(resnet18)에 먼저 돌린다. 거기서도 층이 줄줄이 어긋나면
    이 도구를 믿으면 안 된다.

읽는 법:
    포화(기대 127 이상)인 층은 비교에서 뺀다. int8 벽에 막혀 최대값이
    눌리기 때문에 어긋난 것처럼 보인다.
"""
import math
import os
import re
import struct
import sys

NUM = r"[0-9]+(?:\.[0-9]+)?"


def parse_calib(path):
    out, name = {}, None
    with open(path, encoding="utf-8", errors="replace") as f:
        for line in f:
            m = re.match(r"\s*-?\s*NodeOutputName:\s*'?([^'\s]+)'?", line)
            if m:
                name = m.group(1)
                if name.endswith(":0"):
                    name = name[:-2]
                out.setdefault(name, {})
                continue
            if name is None:
                continue
            m = re.match(r"\s*(Min|Max):\s*(-?[0-9.eE+-]+)", line)
            if m:
                out[name][m.group(1).lower()] = float(m.group(2))
    return {k: v for k, v in out.items() if "min" in v and "max" in v}


def parse_scales(path):
    """번들 소스에서 버퍼 이름마다 양자화 스케일의 역수를 읽는다.

    같은 버퍼가 뒤 층의 입력으로 다시 나오면 같은 스케일이 또 적힌다.
    어긋나면 알려 준다 — 그건 이 파서가 틀렸다는 뜻이다.
    """
    src = open(path, encoding="utf-8", errors="replace").read()
    inv = {}
    conflict = set()
    for m in re.finditer(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\)?\s*,\s*1\.0/(" + NUM + r")", src):
        name, val = m.group(1), float(m.group(2))
        if name in inv and abs(inv[name] - val) > 1e-9:
            conflict.add(name)
        inv.setdefault(name, val)
    # 스케일을 바꾸지 않는 연산은 입력의 것을 물려받는다.
    for _ in range(4):
        for m in re.finditer(
                r"\b(?:maxpool|avgpool|transpose_\w+)\(\s*([^,()]*\(?\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)?[^,]*),\s*"
                r"([^,()]*\(?\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)?[^,]*),", src):
            a, b = m.group(2), m.group(4)
            if a in inv and b not in inv:
                inv[b] = inv[a]
            elif b in inv and a not in inv:
                inv[a] = inv[b]
    return inv, conflict


def parse_dumps(path):
    rows = []
    for fn in sorted(os.listdir(path)):
        m = re.match(r"dump_(\d+)_(.+)\.bin$", fn)
        if not m:
            continue
        with open(os.path.join(path, fn), "rb") as f:
            buf = f.read()
        if not buf:
            continue
        vals = struct.unpack("%db" % len(buf), buf)
        rows.append((int(m.group(1)), m.group(2), len(vals), min(vals), max(vals)))
    seen, uniq = set(), []
    for r in rows:                      # 사진이 여러 장이면 같은 층이 반복된다
        if r[0] in seen:
            continue
        seen.add(r[0])
        uniq.append(r)
    return uniq


def rule_scale_inv(cmin, cmax):
    """번들이 스케일을 안 적는 층을 위한 규칙.

    VTA 합성곱(`convolution_wo_tr`)은 스케일을 `1.0/N` 상수로 적지 않고
    시프트 값으로 들고 있어서 소스에서 바로 읽을 수 없다. 하필 우리가
    의심하는 층들이 전부 이쪽이다.

    대신 규칙으로 메운다. ResNet-18 에서 번들이 적어 둔 스케일 18개를
    보정 범위와 맞춰 보니 예외 없이

        스케일의 역수 = 2^floor(log2(127 / 최대절대값))

    이었다. 읽은 값이 있으면 그것을 쓰고, 없을 때만 이 규칙을 쓴다.
    규칙으로 메운 줄은 표에 그렇다고 적는다 — 읽은 것과 추정한 것을
    섞어 놓고 구분하지 않으면 나중에 속는다.
    """
    ma = max(abs(cmin), abs(cmax))
    if ma <= 0:
        return None
    return 2.0 ** math.floor(math.log2(127.0 / ma))


def match_calib(name, calib):
    """끝의 __숫자 까지 같을 때만 맞은 것으로 친다.

    접두사만 맞추면 같은 층의 양자화 전/후(__1 과 __2)가 뒤섞여, 멀쩡한
    층이 어긋난 것처럼 보인다. 처음 판에서 실제로 그렇게 속았다.
    """
    base = re.sub(r"_res$", "", name)
    for cand in (base, name):
        if cand in calib:
            return cand
    m = re.search(r"__\d+$", base)
    if not m:
        return None
    hits = [k for k in calib if k.startswith(base) and k.endswith(m.group(0))]
    return hits[0] if len(hits) == 1 else None


def main():
    if len(sys.argv) < 4:
        sys.exit(__doc__)
    calib = parse_calib(sys.argv[1])
    rows = parse_dumps(sys.argv[2])
    inv, conflict = parse_scales(sys.argv[3])
    if not rows:
        sys.exit("덤프를 찾지 못했다: %s" % sys.argv[2])
    tol = float(os.environ.get("NESTC_TOL", "0.25"))   # 25% 까지는 봐준다

    print("보정 %d개, 스케일 %d개, 덤프 %d층, 허용 오차 %.0f%%"
          % (len(calib), len(inv), len(rows), tol * 100))
    if conflict:
        print("(경고) 스케일이 두 값으로 읽힌 버퍼: %s"
              % ", ".join(sorted(conflict)[:6]))
    print()
    hdr = ("%-52s %7s %7s %9s %8s  %s"
           % ("층(실행 순서)", "보드최대", "기대최대", "보정최대", "1/스케일", "소견"))
    print(hdr)
    print("-" * len(hdr))

    first_bad = None
    skipped = 0           # 비교하지 못한 층
    skipped_before = 0    # 그중 first_bad 앞쪽
    for idx, name, n, bmin, bmax in rows:
        key = match_calib(name, calib)
        scale_inv = inv.get(name) or inv.get(re.sub(r"_res$", "", name))
        guessed = False
        if scale_inv is None and key is not None:
            scale_inv = rule_scale_inv(calib[key]["min"], calib[key]["max"])
            guessed = scale_inv is not None
        if key is None or scale_inv is None:
            skipped += 1
            if first_bad is None:
                skipped_before += 1
            print("%-52s %7d %7s %9s %8s  %s"
                  % (name, bmax, "-", "-",
                     "%g" % scale_inv if scale_inv else "-",
                     "보정에 없음" if key is None else "스케일 못 읽음"))
            continue
        cmax = calib[key]["max"]
        want = cmax * scale_inv
        note = ["규칙 추정"] if guessed else []
        if want >= 127:
            note.append("포화 — 비교 제외")
            skipped += 1
            if first_bad is None:
                skipped_before += 1
        else:
            err = (bmax - want) / want if want > 0 else None
            if err is None:
                note.append("비교 불가")
            elif abs(err) > tol:
                note.append("어긋남 %+.0f%%" % (err * 100))
                if first_bad is None:
                    first_bad = (idx, name, bmax, want, err)
            else:
                note.append("%+.0f%%" % (err * 100))
        print("%-52s %7d %7.1f %9.3f %8g  %s"
              % (name, bmax, want, cmax, scale_inv, " ".join(note)))

    print()
    if first_bad:
        idx, name, bmax, want, err = first_bad
        print("처음 어긋나는 층: #%d %s — 보드 %d, 기대 %.1f (%+.0f%%)"
              % (idx, name, bmax, want, err * 100))
        if skipped_before:
            print("다만 그 앞에서 %d개 층을 비교하지 못하고 건너뛰었다."
                  % skipped_before)
            print("따라서 이것이 '처음' 이라고 단정할 수 없다. 건너뛴 층부터 본다.")
        else:
            print("그 층의 연산 하나만 보면 된다. 그 앞은 맞는다는 뜻이다.")
    elif skipped:
        print("비교한 층은 전부 허용 오차 안이다. 다만 %d개 층을 비교하지"
              " 못했으므로 '문제 없음' 이라고 할 수는 없다." % skipped)
    else:
        print("모든 층이 허용 오차 안이다. 크기 문제는 아니다.")


if __name__ == "__main__":
    main()
