#!/usr/bin/env python3
"""
vtalib 의 aarch64 CPU 폴백에서 transpose() 가 미구현 스텁으로 남아 있는 문제를 고친다.

원본:
    int transpose(int8_t *input, int8_t *output, ...) {
    //TODO re-implement
      return -1;
    }

출력 버퍼에 아무것도 쓰지 않으므로 초기화되지 않은 메모리가 후속 연산으로 흘러간다.
ResNet-50 은 avgpool 과 FC 사이에서 이 함수를 호출하기 때문에,
aarch64 경로로 돌리면 입력 영상과 무관하게 항상 같은 결과(클래스 619)를 낸다.
ResNet-18 은 VTA 타일 레이아웃 전용 변환만 쓰므로 영향을 받지 않아
지금까지 드러나지 않았다.

generic 구현에 동일 기능이 있어 그것을 이식한다.
switch 문을 배열 인덱싱으로 바꿔 미초기화 경고를 없애고,
축 번호 범위 검사를 추가했다.

멱등하다. 이미 패치되어 있으면 아무 일도 하지 않는다.

사용법:
    python3 ci/patch_transpose.py vta/vtalib/lib/Bundle/CPUBundle_aarch64.cpp
"""
import shutil
import sys
import time

STUB = """int transpose(int8_t *input, int8_t *output, dim_t inDim0, dim_t inDim1, dim_t inDim2, dim_t inDim3,
              dim_t outDim0, dim_t outDim1, dim_t outDim2, dim_t outDim3,
              unsigned_t shf0, unsigned_t shf1, unsigned_t shf2, unsigned_t shf3) {
//TODO re-implement
  return -1;
}"""

IMPL = """static inline int8_t *at8dim_nestc_patch(int8_t *input, dim_t dim0, dim_t dim1,
                                         dim_t dim2, dim_t dim3, dim_t loc0,
                                         dim_t loc1, dim_t loc2, dim_t loc3) {
  (void)dim0;
  dim_t index = loc0 * dim1 * dim2 * dim3 + loc1 * dim2 * dim3 + loc2 * dim3 + loc3;
  return (input + index);
}

int transpose(int8_t *input, int8_t *output, dim_t inDim0, dim_t inDim1, dim_t inDim2, dim_t inDim3,
              dim_t outDim0, dim_t outDim1, dim_t outDim2, dim_t outDim3,
              unsigned_t shf0, unsigned_t shf1, unsigned_t shf2, unsigned_t shf3) {
  dim_t i0 = 0, j0 = 0, k0 = 0, l0 = 0;
  dim_t *axis[4] = {&i0, &j0, &k0, &l0};
  if (shf0 > 3 || shf1 > 3 || shf2 > 3 || shf3 > 3) {
    return -1;
  }
  dim_t *i1 = axis[shf0];
  dim_t *j1 = axis[shf1];
  dim_t *k1 = axis[shf2];
  dim_t *l1 = axis[shf3];
  for (i0 = 0; i0 < inDim0; i0++) {
    for (j0 = 0; j0 < inDim1; j0++) {
      for (k0 = 0; k0 < inDim2; k0++) {
        for (l0 = 0; l0 < inDim3; l0++) {
          int8_t *dst = at8dim_nestc_patch(output, outDim0, outDim1, outDim2,
                                           outDim3, *i1, *j1, *k1, *l1);
          int8_t *src = at8dim_nestc_patch(input, inDim0, inDim1, inDim2,
                                           inDim3, i0, j0, k0, l0);
          *dst = *src;
        }
      }
    }
  }
  return 0;
}"""

MARKER = "at8dim_nestc_patch"


def main() -> int:
    if len(sys.argv) != 2:
        print(__doc__)
        return 2
    path = sys.argv[1]

    try:
        src = open(path, encoding="utf-8").read()
    except OSError as e:
        print(f"열 수 없음: {e}", file=sys.stderr)
        return 1

    if MARKER in src:
        print("이미 패치됨 — 건너뜀")
        return 0

    if STUB not in src:
        print("스텁을 찾지 못했다. 상류가 이미 고쳤거나 코드가 바뀌었다.", file=sys.stderr)
        print("수동 확인 필요:", path, file=sys.stderr)
        return 1

    bak = f"{path}.bak.{time.strftime('%Y%m%d-%H%M%S')}"
    shutil.copy2(path, bak)
    with open(path, "w", encoding="utf-8") as f:
        f.write(src.replace(STUB, IMPL))
    print(f"패치 완료 — 백업 {bak}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
