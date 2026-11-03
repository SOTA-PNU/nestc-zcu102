#!/usr/bin/env bash
# 멀티 EVTA 용 보드 런타임(build_aarch64_multi)을 만들거나 다시 짓는다. 보드에서 돌린다.
#
#   scripts/board-setup-multi.sh [/home/xilinx/nest-compiler-2024]
#
# build_aarch64 와 같은 cmake 옵션에 NESTC_EVTA_MULTI=ON 만 바꾼다. 상류
# vta/vtalib/CMakeLists.txt 가 그때 버퍼를 15/18/17 로 잡는데, 이것이 4코어
# 비트스트림(zcu102_4_1x16_i8w8a32_16_15_18_17)의 구성이다. 단일 구성
# (16/19/18)의 라이브러리로 4코어 비트스트림을 돌리지 않는다.
#
# 보드가 쓰는 것은 vta/vtalib 의 libVTABundle.a 와 libvta_runtime.a 뿐이라
# 그 디렉터리만 짓는다. 처음에도 1분 남짓이다. 이미 있으면 cmake 를 건너뛰고
# make 만 돈다(증분).
set -euo pipefail

SRC=${1:-/home/xilinx/nest-compiler-2024}
OLD=$SRC/build_aarch64
NEW=$SRC/build_aarch64_multi

if [ ! -f "$NEW/CMakeCache.txt" ]; then
  [ -f "$OLD/CMakeCache.txt" ] || { echo "기준 빌드가 없다: $OLD" >&2; exit 1; }
  DEFS=()
  while IFS= read -r line; do
    DEFS+=("-D$line")
  done < <(grep -E '^[A-Za-z_][A-Za-z0-9_]*:(BOOL|STRING|FILEPATH|PATH|UNINITIALIZED)=' "$OLD/CMakeCache.txt" \
    | grep -vE '^(CMAKE_HOME_DIRECTORY|CMAKE_CACHEFILE_DIR|NESTC_EVTA_MULTI):' \
    | grep -vE '_BINARY_DIR:|_SOURCE_DIR:')
  mkdir -p "$NEW"
  echo "== cmake  (build_aarch64 옵션 ${#DEFS[@]}개 + NESTC_EVTA_MULTI=ON)"
  (cd "$NEW" && cmake -G "Unix Makefiles" "${DEFS[@]}" -DNESTC_EVTA_MULTI:BOOL=ON "$SRC" > cmake.log 2>&1) \
    || { tail -30 "$NEW/cmake.log"; exit 1; }
fi

grep -q '^NESTC_EVTA_MULTI:BOOL=ON' "$NEW/CMakeCache.txt" || {
  echo "$NEW 가 MULTI=ON 이 아니다" >&2; exit 1; }

echo "== vtalib 빌드"
make -C "$NEW/vta/vtalib" -j2 2>&1 | tail -5
ls -l "$NEW/vta/vtalib/libVTABundle.a" "$NEW/vta/vtalib/libvta_runtime.a"
echo "== 버퍼 구성"
grep -ho 'VTA_LOG_\(INP\|WGT\|ACC\)_BUFF_SIZE=[0-9]*' "$NEW"/vta/vtalib/CMakeFiles/*/flags.make | sort -u
