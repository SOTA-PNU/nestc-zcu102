#!/usr/bin/env python3
"""
멀티 EVTA main 의 스레드 수를 줄인다.

상류 틀(vta/multievta_bundles/MultiEVTAResnet18Test/multi_threads_main.cpp)은
스레드 네 개가 고정이다. 코어를 네 개 다 쓰면 FPGA 연속 메모리(CMA)가 모자랄
수 있는데, 그때 두 개로 줄여 보면 "CMA 가 모자란 것"과 "멀티코어 자체가 안 되는
것"을 가를 수 있다. 수치를 1 → 2 → 4 로 늘려 가며 확장성도 볼 수 있다.

하는 일은 지우는 것뿐이다. N 번째 이상의
  - void *thread_mainK(...) { ... }   함수 정의
  - ThreadArg thread_argK; ... 버퍼 초기화 묶음
  - pthread_create(&thread[K], ...)   줄
  - pthread_join(thread[K], ...)      줄
을 없앤다. 버퍼 초기화까지 지워야 의미가 있다 — 그것이 CMA 를 잡는 자리다.

사용법:
    python3 make_multi_main.py <입력 main.cpp> <출력 main.cpp> <코어 수 1..4>
"""
import re
import sys


def drop_function(text, name):
    """`void *<name>(` 로 시작해 열 0 의 `}` 로 끝나는 정의를 지운다."""
    start = text.find("void *%s(" % name)
    if start < 0:
        return text, False
    end = text.find("\n}\n", start)
    if end < 0:
        return text, False
    return text[:start] + text[end + 3:], True


def drop_threadarg_block(text, idx):
    """`ThreadArg thread_argK;` 부터 그 묶음의 마지막 대입까지 지운다."""
    suffix = "" if idx == 0 else str(idx)
    head = "ThreadArg thread_arg%s;" % suffix
    start = text.find(head)
    if start < 0:
        return text, False
    tail = "thread_arg%s.activationsAddr" % suffix
    at = text.find(tail, start)
    if at < 0:
        return text, False
    end = text.find(";", at)
    if end < 0:
        return text, False
    return text[:start] + text[end + 2:], True


def main():
    if len(sys.argv) != 4:
        print(__doc__)
        return 2
    src, dst, n = sys.argv[1], sys.argv[2], int(sys.argv[3])
    if not 1 <= n <= 4:
        print("코어 수는 1..4", file=sys.stderr)
        return 2

    t = open(src, encoding="utf-8").read()
    if n == 4:
        open(dst, "w", encoding="utf-8").write(t)
        print("코어 4개 — 틀을 그대로 쓴다")
        return 0

    removed = []
    for k in range(n, 4):
        name = "thread_main" if k == 0 else "thread_main%d" % k
        t, ok = drop_function(t, name)
        removed.append("%s:%s" % (name, "지움" if ok else "없음"))

        t, ok = drop_threadarg_block(t, k)
        removed.append("thread_arg%d:%s" % (k, "지움" if ok else "없음"))

        t = re.sub(r"[ \t]*pthread_create\(&thread\[%d\][^\n]*\n" % k, "", t)
        t = re.sub(r"[ \t]*pthread_join\(thread\[%d\][^\n]*\n" % k, "", t)
        # 그 코어의 번들을 아예 만들지 않으므로 헤더도 지운다.
        t = re.sub(r'#include "[A-Za-z0-9_]+_%d\.h"\n' % k, "", t)

    # 구조 검사. 중괄호가 맞지 않으면 쓰지 않는다.
    if t.count("{") != t.count("}"):
        print("중괄호 불균형 — 변환 실패", file=sys.stderr)
        return 1
    left = len(re.findall(r"pthread_create\(", t))
    if left != n:
        print("pthread_create 가 %d 개 남았다 (기대 %d)" % (left, n), file=sys.stderr)
        return 1
    if len(re.findall(r"pthread_join\(", t)) != n:
        print("pthread_join 개수가 맞지 않는다", file=sys.stderr)
        return 1
    for k in range(n, 4):
        if re.search(r"[A-Za-z0-9_]+_%d[._(]" % k, t):
            print("코어 %d 참조가 남아 있다" % k, file=sys.stderr)
            return 1

    open(dst, "w", encoding="utf-8").write(t)
    print("코어 %d개로 축소: %s" % (n, ", ".join(removed)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
