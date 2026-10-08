#!/usr/bin/env bash
# 보드에서 번들을 실행 파일로 만들고 돌려, 기대 클래스와 대조한다.
#
# gen-bundle.sh 가 만든 디렉터리를 그대로 받는다.
#
#   scripts/board-build-run.sh \
#     --bundle ~/bundles/resnet18 \
#     --src ~/nest-compiler-2024 \
#     --build ~/nest-compiler-2024/build_board
#
# CMake 를 거치지 않고 직접 컴파일한다. 기존 작업 트리를 건드리지 않기
# 위함이며, 상류가 번들마다 CMakeLists 를 두는 구조라 새 모델은 어차피
# 등록되어 있지 않다.
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

NET=$(ls "$BUNDLE"/*.cpp | grep -v '/Main\.cpp$' | head -1)
[ -n "$NET" ] || { echo "번들 소스를 찾지 못했다" >&2; exit 1; }
echo "== 번들 $MODEL_NAME  소스 $(basename "$NET")"

# ── 라이브러리 탐색 ───────────────────────────────────────────────────
# 상류 빌드 트리에서 찾는다. 경로가 버전마다 달라 고정하지 않는다.
findone() {
  local f
  f=$(find "$BUILD" -name "$1" -type f 2>/dev/null | head -1)
  [ -n "$f" ] || { echo "찾지 못함: $1 (아래에서) $BUILD" >&2; return 1; }
  echo "$f"
}

LIB_BUNDLE=$(findone libVTABundle.a)
LIB_RT=$(findone libvta_runtime.a)
echo "   VTABundle   $LIB_BUNDLE"
echo "   vta_runtime $LIB_RT"

# ARM Compute Library 는 aarch64 번들에서만 필요하다.
ACL=()
for l in libarm_compute-static.a libarm_compute_core-static.a libarm_compute_graph-static.a; do
  p=$(find "$SRC" "$BUILD" -name "$l" -type f 2>/dev/null | head -1) || true
  [ -n "${p:-}" ] && ACL+=("$p")
done
[ ${#ACL[@]} -gt 0 ] && echo "   ACL         ${#ACL[@]}개"

# ── 인클루드 경로 ─────────────────────────────────────────────────────
# 상류 헤더가 보드의 구 트리보다 반드시 앞에 와야 한다.
# 2024년 번들이 호출하는 VTAUopBufferReset() 이 2021년 런타임 헤더에는
# 없어서, 순서가 뒤바뀌면 "was not declared" 로 깨진다.
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
for d in "${INC[@]}"; do
  p=${d#-I}
  [ -d "$p" ] || echo "   (경고) 없는 인클루드 경로 $p" >&2
done

CXXFLAGS=(-Wall -Wno-psabi -O3 -DNDEBUG -march=native -std=c++14)

echo "== 컴파일"
cd "$BUNDLE"
c++ "${CXXFLAGS[@]}" "${INC[@]}" -c Main.cpp -o main.o
c++ "${CXXFLAGS[@]}" "${INC[@]}" -c "$NET" -o net.o

echo "== 링크"
c++ "${CXXFLAGS[@]}" main.o net.o -o "$MODEL_NAME" \
  "$LIB_BUNDLE" -lpng "$LIB_RT" \
  "${ACL[@]}" \
  /usr/lib/llvm-8.0/lib/libLLVMSupport.a -lz -lrt -ldl -ltinfo -lm \
  /usr/lib/llvm-8.0/lib/libLLVMDemangle.a -lpthread -lglog \
  -lgflags -lpthread -lgomp -lcma

echo "== 실행"
# VTA 는 /dev/xlnk 를 열어야 하므로 root 권한이 필요하다.
# 실행 중 중단하면 FPGA 와 xlnk 가 정리되지 않아 재부팅 전까지 보드를 못 쓴다.
IMGDIR=$SRC/glow/tests/images/imagenet
FAIL=0
for pair in ${EXPECT:-}; do
  img=${pair%%:*}; want=${pair##*:}
  path="$IMGDIR/$img.png"
  [ -f "$path" ] || { echo "   이미지 없음 $path"; FAIL=1; continue; }

  # 종료 코드로 판정하지 않는다. 번들은 추론에 성공하고도 0 이 아닌 값을
  # 반환하는 경우가 있다(두 번째 실행부터 VTA 정리 단계에서 그런다).
  # 판정은 출력의 Result 줄로만 한다.
  out=$(sudo -E env PATH="$PATH" "./$MODEL_NAME" "$path" 2>&1 || true)

  got=$(echo "$out"  | sed -n 's/^Result: *//p'     | head -1)
  if [ -z "$got" ]; then
    echo "   FAIL $img  Result 줄이 없다"
    echo "$out" | tail -5
    FAIL=1
    continue
  fi
  conf=$(echo "$out" | sed -n 's/^Confidence: *//p' | head -1)
  ms=$(echo "$out"   | sed -n 's/^Inference time: *//p' | head -1)

  if [ "$got" = "$want" ]; then
    echo "   OK   $img  $got  ($conf, $ms)"
  else
    echo "   FAIL $img  기대 $want  실제 ${got:-없음}  ($conf, $ms)"
    FAIL=1
  fi
done

[ "$FAIL" = 0 ] && echo "== 전부 통과" || echo "== 실패 있음"
exit "$FAIL"
