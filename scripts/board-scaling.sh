#!/usr/bin/env bash
# 보드에서 돈다. 멀티 EVTA 를 1~4 스레드로 돌려 확장성을 재고, 연산별 시간
# (ci/nt_instrument.py 로 계측한 복사본)과 CPU/메모리 간섭 실험을 찍는다.
# 미리 보드 ~/ 에 ci/nt_instrument.py, ci/make_multi_main.py,
# scripts/board-*.sh 를 올려 둔다. 약 5분. 끝나면 single 로 되돌린다.
# 결과 해석은 results/scaling-20261103.md.
set -u
export PATH=$HOME/bin:$PATH
SRC=$HOME/nest-compiler-2024
BUILD=$SRC/build_aarch64_multi
B=$HOME/bundles/multi-resnet18-4core
S=$HOME/scaling
IMG=$SRC/glow/tests/images/imagenet/cat_285.png
oops() { sudo dmesg | grep -c 'Internal error: Oops' || true; }
O0=$(oops)

~/board-bitstream.sh multi | tail -2 || exit 1
trap '~/board-bitstream.sh single | tail -1' EXIT

rm -rf "$S"; mkdir -p "$S"
for N in 1 2 3 4; do
  D=$S/n$N; mkdir -p "$D"
  cp "$B"/*.cpp "$B"/*.h "$B"/*.bin "$B"/bundle.conf "$D"/
  for k in 1 2 3; do [ "$k" -ge "$N" ] && rm -f "$D"/mxnet_exported_resnet18_$k.*; done
  if [ "$N" != 4 ]; then python3 ~/make_multi_main.py "$D/Main.cpp" "$D/Main.cpp.tmp" "$N" && mv "$D/Main.cpp.tmp" "$D/Main.cpp"; fi
  for f in "$D"/mxnet_exported_resnet18*.cpp; do
    t=$(basename "$f" .cpp | sed 's/mxnet_exported_resnet18/c/; s/^c$/c_0/')
    python3 ~/nt_instrument.py "$f" "$t" | sed 's|^.*/||'
  done
  echo "=== N=$N 빌드와 첫 실행 (board-multi-run)"
  ~/board-multi-run.sh --bundle "$D" --src "$SRC" --build "$BUILD" 2>&1 | grep -E "^== (전부|실패|중단)|처리량|총 " || { echo "board-multi-run 실패"; exit 1; }
  [ "$(oops)" = "$O0" ] || { echo "oops 발생 — 중단"; exit 1; }
done

run() {  # $1 = 디렉터리, $2 = 라벨
  echo "=== $2"
  ( cd "$1" && sudo -E env PATH="$PATH" timeout -s KILL 300 ./resnet18-4core_multi "$IMG" 2>&1 ) \
    | grep -E "^NT |Inference time|should be"
  [ "$(oops)" = "$O0" ] || { echo "oops 발생 — 중단"; exit 1; }
}
for N in 1 2 3 4; do for r in 1 2; do run "$S/n$N" "N=$N 반복$r"; done; done

# 간섭 실험: VTA 스레드 1개 + 다른 CPU 3개에 부하
echo "=== 간섭: CPU 연산 부하 3개 (메모리 거의 안 씀)"
for i in 1 2 3; do timeout 60 sh -c 'while :; do :; done' & done
sleep 1; run "$S/n1" "N=1 + CPU부하3"; pkill -f 'while :; do :; done' || true; wait 2>/dev/null
echo "=== 간섭: 메모리 복사 부하 3개 (64MB memcpy 반복)"
for i in 1 2 3; do timeout 60 python3 -c '
import numpy as np
a=np.ones(64<<20,np.uint8); b=np.empty_like(a)
while True: np.copyto(b,a)' & done
sleep 3; run "$S/n1" "N=1 + 메모리부하3"; pkill -f 'np.copyto' || true; wait 2>/dev/null
echo "=== 끝  oops 전 $O0 후 $(oops)  EXT4 오류 $(dmesg | grep -c 'EXT4-fs error')  CmaFree $(grep CmaFree /proc/meminfo | awk '{print $2}')"
