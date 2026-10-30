#!/usr/bin/env python3
"""
생성된 번들 소스에 층별 덤프 호출을 끼워 넣는다.

왜 이렇게 하는가. 상류 CMakeLists 에 주석으로 남아 있는
`-instrument-debug -instrument-debug-format=rawbin` 은 Glow 의 LLVM 백엔드
기능이라 VTA 저장 경로에서는 아무 명령도 삽입하지 않는다. 플래그는 받아들여
지지만 파일이 하나도 생기지 않는다(2026-10-30 확인). 그래서 컴파일러를
건드리는 대신 생성된 C++ 소스를 후처리한다.

번들은 중간 텐서마다 이런 쌍을 만든다.

    int8_t *squeezenet0_conv1_fwd__2_res = (int8_t *)malloc(193600);
    ...
    free(squeezenet0_conv1_fwd__2_res);

`free` 직전이 그 텐서의 최종값이 담긴 시점이다. 거기에 덤프 호출을 넣으면
실행 순서대로 파일이 쌓인다. 파일 이름에 일련번호가 붙으므로 정렬도 정확하다.

마지막 출력 텐서는 free 되지 않으므로(가변 가중치 영역에 있다) 빠진다.
그건 어차피 Result 줄로 보인다.

사용법:
    python3 inject_layer_dump.py <번들 디렉터리>
"""
import os
import re
import sys

MARKER = "nestc_layer_dump"

HELPER = """/* %s
 * 층별 덤프. 실행한 디렉터리에 dump_<순번>_<이름>.bin 으로 쌓인다.
 * ci/analyze_debug_dump.py 가 이것을 읽어 처음 망가지는 층을 짚는다.
 */
#include <stdio.h>
static int nestc_dump_seq = 0;
static void nestc_dump_layer(const char *name, const void *buf, size_t n) {
  char path[512];
  FILE *f;
  snprintf(path, sizeof(path), "dump_%%03d_%%s.bin", nestc_dump_seq++, name);
  f = fopen(path, "wb");
  if (!f) {
    return;
  }
  fwrite(buf, 1, n, f);
  fclose(f);
}

""" % MARKER

ALLOC_RE = re.compile(
    r'^\s*int8_t \*([A-Za-z0-9_]+) = \(int8_t \*\)malloc\((\d+)\);\s*$')
FREE_RE = re.compile(r'^(\s*)free\(([A-Za-z0-9_]+)\);\s*$')


def main():
    if len(sys.argv) != 2:
        print(__doc__)
        return 2
    d = sys.argv[1]

    nets = [f for f in os.listdir(d)
            if f.endswith(".cpp") and f != "Main.cpp"]
    if len(nets) != 1:
        print("번들 소스가 하나가 아니다: %s" % nets, file=sys.stderr)
        return 1
    path = os.path.join(d, nets[0])

    src = open(path, encoding="utf-8").read()
    if MARKER in src:
        print("이미 주입되어 있다.")
        return 0

    lines = src.split("\n")
    sizes = {}
    for ln in lines:
        m = ALLOC_RE.match(ln)
        if m:
            sizes[m.group(1)] = m.group(2)
    if not sizes:
        print("malloc 된 활성 버퍼를 찾지 못했다. 생성 코드 모양이 바뀌었다.",
              file=sys.stderr)
        return 1

    out = []
    injected = 0
    for ln in lines:
        m = FREE_RE.match(ln)
        if m and m.group(2) in sizes:
            indent, name = m.group(1), m.group(2)
            out.append('%snestc_dump_layer("%s", %s, %s);'
                       % (indent, name, name, sizes[name]))
            injected += 1
        out.append(ln)

    if injected == 0:
        print("free 지점을 찾지 못했다.", file=sys.stderr)
        return 1

    text = "\n".join(out)

    # 도우미를 첫 함수 앞에 넣는다. 번들은 extern "C" 블록을 쓰므로
    # 그 앞, 마지막 #include 다음에 둔다.
    idx = max((text.rfind("\n#include"), 0))
    if idx == 0:
        print("#include 를 찾지 못했다.", file=sys.stderr)
        return 1
    nl = text.index("\n", idx + 1)
    text = text[:nl + 1] + "\n" + HELPER + text[nl + 1:]

    open(path, "w", encoding="utf-8").write(text)
    print("덤프 주입 %s — 버퍼 %d개, 호출 %d곳"
          % (nets[0], len(sizes), injected))
    return 0


if __name__ == "__main__":
    sys.exit(main())
