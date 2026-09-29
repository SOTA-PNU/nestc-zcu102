#!/bin/bash
# CPUBundle_aarch64.cpp 의 미구현 transpose() 를 generic 구현으로 채운다.
#
# 문제: aarch64 버전의 transpose() 가 "//TODO re-implement / return -1" 스텁이라
#       출력 버퍼에 아무것도 쓰지 않는다. ResNet-50 은 이 함수를 1회 호출하므로
#       그 뒤 FC/softmax 가 초기화되지 않은 메모리를 읽어 입력과 무관한
#       고정 출력(619, confidence 0.083585)을 낸다.
#       ResNet-18 은 transpose() 를 호출하지 않아 영향이 없다.
#
# 사용법:  bash fix-aarch64-transpose.sh
set -euo pipefail

DIR=/home/xilinx/nest-compiler/vta/vtalib/lib/Bundle
SRC=$DIR/CPUBundle_aarch64.cpp

cd "$DIR"

if grep -q "at8dim_nestc_patch" "$SRC"; then
  echo "이미 패치가 적용되어 있습니다."
  exit 0
fi

cp "$SRC" "$SRC.bak.$(date +%Y%m%d_%H%M%S)"
echo "백업: $SRC.bak.*"

python3 - << 'PY'
import re

path = '/home/xilinx/nest-compiler/vta/vtalib/lib/Bundle/CPUBundle_aarch64.cpp'
src = open(path, encoding='utf-8', errors='surrogateescape').read()

stub = """int transpose(int8_t *input, int8_t *output, dim_t inDim0, dim_t inDim1, dim_t inDim2, dim_t inDim3,
              dim_t outDim0, dim_t outDim1, dim_t outDim2, dim_t outDim3,
              unsigned_t shf0, unsigned_t shf1, unsigned_t shf2, unsigned_t shf3) {
//TODO re-implement
  return -1;
}"""

impl = """// [NESTC PATCH] CPUBundle_generic.cpp 의 구현을 이식.
// 원본은 "//TODO re-implement; return -1" 스텁이라 출력에 아무것도 쓰지 않았고,
// transpose() 를 사용하는 ResNet-50 이 입력과 무관한 고정 출력을 냈다.
static inline int8_t *at8dim_nestc_patch(int8_t *input, dim_t dim0, dim_t dim1,
                                         dim_t dim2, dim_t dim3, dim_t loc0,
                                         dim_t loc1, dim_t loc2, dim_t loc3) {
  dim_t index = loc0 * dim1 * dim2 * dim3 + loc1 * dim2 * dim3 + loc2 * dim3 + loc3;
  return (input + index);
}

int transpose(int8_t *input, int8_t *output, dim_t inDim0, dim_t inDim1, dim_t inDim2, dim_t inDim3,
              dim_t outDim0, dim_t outDim1, dim_t outDim2, dim_t outDim3,
              unsigned_t shf0, unsigned_t shf1, unsigned_t shf2, unsigned_t shf3) {
  dim_t i0 = 0, j0 = 0, k0 = 0, l0 = 0;
  dim_t *i1 = &i0, *j1 = &j0, *k1 = &k0, *l1 = &l0;

  dim_t *axis[4] = {&i0, &j0, &k0, &l0};
  if (shf0 > 3 || shf1 > 3 || shf2 > 3 || shf3 > 3) {
    return -1;
  }
  i1 = axis[shf0];
  j1 = axis[shf1];
  k1 = axis[shf2];
  l1 = axis[shf3];

  for (i0 = 0; i0 < inDim0; i0++)
    for (j0 = 0; j0 < inDim1; j0++)
      for (k0 = 0; k0 < inDim2; k0++)
        for (l0 = 0; l0 < inDim3; l0++) {
          int8_t *dst = at8dim_nestc_patch(output, outDim0, outDim1, outDim2,
                                           outDim3, *i1, *j1, *k1, *l1);
          int8_t *src = at8dim_nestc_patch(input, inDim0, inDim1, inDim2,
                                           inDim3, i0, j0, k0, l0);
          *dst = *src;
        }
  return 0;
}"""

if stub not in src:
    raise SystemExit("스텁을 찾지 못했습니다. 이미 수정됐거나 소스가 다릅니다.")

src = src.replace(stub, impl, 1)
open(path, 'w', encoding='utf-8', errors='surrogateescape').write(src)
print("transpose() 구현 삽입 완료")
PY

echo
echo "=== 확인 ==="
grep -n "NESTC PATCH\|^int transpose" "$SRC"
