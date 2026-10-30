#!/usr/bin/env bash
# 멀티 EVTA 번들을 보드에서 빌드하고 돌려 처리량을 잰다.
#
# gen-bundle.sh 를 MULTI_EVTA=1 로 돌려 만든 디렉터리를 받는다.
# 그 안에는 번들이 네 벌 들어 있다 — <base>.cpp 와 <base>_1..3.cpp.
# 각각 EVTA 코어 0~3 에 묶여 있고, Main.cpp 가 네 스레드로 동시에 돌린다.
#
#   scripts/board-multi-run.sh \
#     --bundle ~/bundles/resnet18-multi \
#     --src ~/nest-compiler-2024 \
#     --build ~/nest-compiler-2024/build_board
#
# 단일 코어와 비교하려면 같은 모델을 board-build-run.sh 로도 돌려
# 한 장당 시간을 재 두면 된다.
set -euo pipefail

BUNDLE=; SRC=; BUILD=
while [ $# -gt 0 ]; do
  case "$1" in
    --bundle) BUNDLE=$2; shift 2 ;;
    --src)    SRC=$2;    shift 2 ;;
    --build)  BUILD=$2;  shift 2 ;;
    *) echo "모르는 인자: $1" >&2; exit 2 ;;
  esac
done
[ -n "$BUNDLE" ] && [ -n "$SRC" ] && [ -n "$BUILD" ] || { sed -n '2,13p' "$0" >&2; exit 2; }

BUNDLE=$(cd "$BUNDLE" && pwd)
SRC=$(cd "$SRC" && pwd)
BUILD=$(cd "$BUILD" && pwd)

# shellcheck disable=SC1091
. "$BUNDLE/bundle.conf"
: "${MODEL_NAME:?bundle.conf 에 MODEL_NAME 이 없다}"

NETS=()
while IFS= read -r f; do NETS+=("$f"); done < <(ls "$BUNDLE"/*.cpp | grep -v '/Main\.cpp$' | sort)
CORES=${#NETS[@]}
[ "$CORES" -ge 1 ] && [ "$CORES" -le 4 ] || {
  echo "번들 소스가 1~4개가 아니다 ($CORES개). MULTI_EVTA=1 로 생성했는지 확인한다." >&2
  printf '   %s\n' "${NETS[@]}" >&2
  exit 1; }
echo "== 멀티 EVTA 번들 $MODEL_NAME  (코어 $CORES개)"
for n in "${NETS[@]}"; do echo "   $(basename "$n")"; done

findone() {
  local f
  f=$(find "$BUILD" -name "$1" -type f 2>/dev/null | head -1)
  [ -n "$f" ] || { echo "찾지 못함: $1 (아래에서) $BUILD" >&2; return 1; }
  echo "$f"
}
LIB_BUNDLE=$(findone libVTABundle.a)
LIB_RT=$(findone libvta_runtime.a)

ACL=()
for l in libarm_compute-static.a libarm_compute_core-static.a libarm_compute_graph-static.a; do
  p=$(find "$SRC" "$BUILD" -name "$l" -type f 2>/dev/null | head -1) || true
  [ -n "${p:-}" ] && ACL+=("$p")
done

V=$SRC/vta/vtalib
INC=(
  -I"$BUNDLE"
  -I"$V/include"
  -I"$V/include/zcu102"
  -I"$V/include/zcu102/vta"
  -I"$V/include/Bundle"
  -I"$SRC/vta/3rdparty/dlpack/include"
  -I"$SRC/vta/3rdparty/dmlc-core/include"
)
CXXFLAGS=(-Wall -Wno-psabi -O3 -DNDEBUG -march=native -std=c++14)

echo "== 컴파일 (번들 네 벌이라 단일보다 오래 걸린다)"
cd "$BUNDLE"
OBJS=()
c++ "${CXXFLAGS[@]}" "${INC[@]}" -c Main.cpp -o main.o
OBJS+=(main.o)
k=0
for n in "${NETS[@]}"; do
  c++ "${CXXFLAGS[@]}" "${INC[@]}" -c "$n" -o "net$k.o"
  OBJS+=("net$k.o")
  k=$((k+1))
done

echo "== 링크"
c++ "${CXXFLAGS[@]}" "${OBJS[@]}" -o "${MODEL_NAME}_multi" \
  "$LIB_BUNDLE" -lpng "$LIB_RT" \
  "${ACL[@]}" \
  /usr/lib/llvm-8.0/lib/libLLVMSupport.a -lz -lrt -ldl -ltinfo -lm \
  /usr/lib/llvm-8.0/lib/libLLVMDemangle.a -lpthread -lglog \
  -lgflags -lpthread -lgomp -lcma

echo "== 실행 (네 코어 동시)"
# 네 스레드가 합쳐 100장을 처리할 때까지 돌고 전체 벽시계 시간을 찍는다.
# 중단하면 FPGA 가 정리되지 않으니 끝까지 둔다.
IMGDIR=$SRC/glow/tests/images/imagenet
IMG=${MULTI_IMAGE:-cat_285}
[ -f "$IMGDIR/$IMG.png" ] || { echo "이미지 없음 $IMGDIR/$IMG.png" >&2; exit 1; }

# FPGA 는 연속 물리 메모리(CMA)를 쓴다. 코어마다 가중치·활성 버퍼를 따로
# 잡으므로 네 배가 필요하다. 모자라면 dat_queue.h 의
# "Check failed: fpga_buff_ != nullptr" 로 죽는다. 먼저 재 둔다.
echo "-- CMA 현황 --"
grep -i cma /proc/meminfo || echo "(meminfo 에 CMA 항목 없음)"
[ -r /sys/class/xlnk/xlnk/xlnk_bufpool_size ] && cat /sys/class/xlnk/xlnk/xlnk_bufpool_size || true
echo "-- 번들당 요구량 --"
for h in "$BUNDLE"/*.h; do
  case "$(basename "$h")" in VTARuntime.h) continue ;; esac
  printf '   %-40s ' "$(basename "$h")"
  grep -h "Total data size" "$h" | head -1 | tr -d '\n'
  echo
done
ls -l "$BUNDLE"/*.weights.bin | awk '{printf "   %-40s %s bytes\n", $NF, $5}'

out=$(sudo -E env PATH="$PATH" "./${MODEL_NAME}_multi" "$IMGDIR/$IMG.png" 2>&1 || true)
echo "$out" | tail -8

total=$(echo "$out" | sed -n 's/^Inference time: *//p' | tail -1 | tr -d 'ms')
wrong=$(echo "$out" | grep -c "should be" || true)

if [ -z "$total" ]; then
  echo "== 실패  시간 줄을 찾지 못했다"
  exit 1
fi
if [ "$wrong" != "0" ]; then
  echo "== 실패  틀린 추론 $wrong 건"
  exit 1
fi

# 상류 틀은 네 스레드가 공유 카운터로 100 회를 나눠 처리한다.
N=${MULTI_ITERS:-100}
per=$(awk -v t="$total" -v n="$N" 'BEGIN{printf "%.1f", t/n}')
tput=$(awk -v t="$total" -v n="$N" 'BEGIN{printf "%.2f", n*1000/t}')
echo "== 전부 통과"
echo "   총 ${total}ms / ${N}장"
echo "   장당 ${per}ms (벽시계 기준)"
echo "   처리량 ${tput} img/s"
