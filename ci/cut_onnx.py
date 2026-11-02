#!/usr/bin/env python3
"""
ONNX 를 중간 텐서에서 끊어 그 지점까지만 하는 모델을 만든다.

## 왜

SqueezeNet 이 보드에서 틀리는데, 지금까지 배제한 것이 열다섯 가지다.
남은 것은 **VTAInterpreter 가 거치지 않는 경로** — VTASave 가 내보낸 번들
코드와 VTA 하드웨어 실행 경로뿐이다. 그 둘을 더 좁히려면 어느 지점부터
갈라지는지 알아야 한다.

모델을 중간에서 끊으면 그 지점의 텐서가 모델의 출력이 된다. 같은 입력으로
인터프리터와 보드를 각각 끝까지 돌려 답(argmax)을 비교하면, 갈라지는 첫
지점을 이분 탐색으로 찾을 수 있다. 계측도 시뮬레이터도 필요 없고 기존
파이프라인을 그대로 쓴다.

## 쓰는 법

conf 에 `ONNX_CUT_AT=<텐서 이름>` 을 넣으면 gen-bundle.sh 가 이 스크립트를
먼저 돌려 잘린 모델을 만들고, 그것으로 보정과 번들 생성을 한다.

텐서 이름은 ONNX 그래프의 이름이다. 목록은 이렇게 본다.

    python3 cut_onnx.py --list <model.onnx>

사용법:
    python3 cut_onnx.py --list <입력.onnx>
    python3 cut_onnx.py <입력.onnx> <출력.onnx> <입력텐서> <자를텐서>
"""
import sys


def main():
    try:
        import onnx
        from onnx.utils import extract_model
    except ImportError:
        print("onnx 파이썬 패키지가 없다. 컨테이너 안에서 돌려야 한다.",
              file=sys.stderr)
        return 1

    if len(sys.argv) == 3 and sys.argv[1] == "--list":
        m = onnx.load(sys.argv[2])
        print("입력:")
        for i in m.graph.input:
            print("   ", i.name)
        print("노드 출력 (자를 수 있는 지점):")
        for n in m.graph.node:
            for o in n.output:
                print("   %-40s %s" % (o, n.op_type))
        return 0

    if len(sys.argv) != 5:
        print(__doc__)
        return 2

    src, dst, in_name, cut = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]
    m = onnx.load(src)
    names = {o for n in m.graph.node for o in n.output}
    if cut not in names:
        print("그런 텐서가 없다: %s" % cut, file=sys.stderr)
        print("고를 수 있는 지점:", file=sys.stderr)
        for n in m.graph.node:
            for o in n.output:
                print("   %-44s %s" % (o, n.op_type), file=sys.stderr)
        return 1

    extract_model(src, dst, [in_name], [cut])
    out = onnx.load(dst)
    print("잘랐다: %s -> %s" % (src, dst))
    print("   입력 %s, 출력 %s, 노드 %d개 (원본 %d개)"
          % (in_name, cut, len(out.graph.node), len(m.graph.node)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
