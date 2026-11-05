#!/usr/bin/env bash
# CPU 단독 벤치마크 — VTA 없이 보드 ARM 에서만 돌린 기준값을 잰다.
#
# 연구실 서버(CI 러너 기계)에서 저장소 루트를 현재 폴더로 두고 실행한다.
#   bash scripts/bench-cpu.sh [모델 conf ...]      (기본: models/resnet18.conf models/resnet50.conf)
#
# 모델마다 네 가지 번들을 만들어 보드에서 잰다.
#   fp32 / int8  x  1스레드(지연) / 4스레드(처리량)
# int8 은 VTA 번들과 같은 보정 이미지·같은 양자화 방식(symmetric_with_power2_scale)
# 이다. 컴파일러도 같은 NEST-C(Glow) 이고 백엔드만 CPU(LLVM, aarch64 Cortex-A53).
#
# 보드의 VTA·CMA 는 건드리지 않는다. CI 와 겹치면 시간만 흐트러지므로
# board-health(매시 17분) 를 비켜서 돌린다.
#
# 환경 변수: NESTC_UPSTREAM(기본 ~/nestc-upstream), DOCKER_IMAGE, ITERS_<모델>
set -euo pipefail

UP=${NESTC_UPSTREAM:-$HOME/nestc-upstream}
IMG=${DOCKER_IMAGE:-leejaymin/nestc-ssh:latest}
REPO=$(pwd)
WORK=$REPO/bench-out
BB=/home/xilinx/bench-cpu
IMAGES="cat_285 dog_207 zebra_340"
THREADS_MAX=4
[ $# -gt 0 ] && CONFS=("$@") || CONFS=(models/resnet18.conf models/resnet50.conf)

[ -f bench/cpu_bench_main.cpp ] || { echo "저장소 루트에서 실행한다" >&2; exit 2; }
mkdir -p "$WORK"
ssh board "mkdir -p $BB"
scp -q bench/cpu_bench_main.cpp board:$BB/

dock() {
  docker run --rm --user "$(id -u):$(id -g)" -v "$UP":/src -v "$WORK":/work -w /work "$IMG" "$@"
}

for CONF in "${CONFS[@]}"; do
  (
  # shellcheck disable=SC1090
  . "$CONF"
  : "${CHANNEL_ORDER:=RGB}"
  M=$MODEL_NAME
  O=$WORK/$M; rm -rf "$O"; mkdir -p "$O"
  echo "================ $M ================"

  # ONNX 위치(컨테이너 기준)
  case "$MODEL_ONNX" in
    upstream:*) ONNX=/src/${MODEL_ONNX#upstream:} ;;
    http*)      curl -fsSL "$MODEL_ONNX" -o "$O/model.onnx"; ONNX=/work/$M/model.onnx ;;
    *)          echo "모르는 ONNX 경로: $MODEL_ONNX" >&2; exit 2 ;;
  esac
  CALIB=/src/${CALIB_IMAGES#upstream:}
  # shellcheck disable=SC2206
  EXTRA=( ${CALIB_EXTRA:-} )

  # 1) int8 보정 프로파일 (VTA 와 같은 이미지, 같은 방식. CPU 계열이라 Interpreter)
  dock /src/build/glow/bin/image-classifier "$CALIB" -m="$ONNX" \
    -model-input-name="$INPUT_NAME" -backend=Interpreter \
    -image-layout="$IMAGE_LAYOUT" -image-mode="$IMAGE_MODE" "${EXTRA[@]}" \
    -dump-profile=/work/$M/calib.yaml \
    -quantization-schema=symmetric_with_power2_scale >/dev/null 2>&1
  [ -s "$O/calib.yaml" ] || { echo "보정 프로파일 실패" >&2; exit 1; }

  # 2) CPU 번들 두 가지
  for P in fp32 int8; do
    Q=(); [ $P = int8 ] && Q=(-load-profile=/work/$M/calib.yaml -quantization-schema=symmetric_with_power2_scale)
    dock /src/build/glow/bin/model-compiler -backend=CPU -model="$ONNX" \
      -model-input="$INPUT_NAME,float,$INPUT_SHAPE" -emit-bundle=/work/$M/$P \
      -bundle-api=dynamic -target=aarch64-linux-gnu -mcpu=cortex-a53 "${Q[@]}" \
      2>&1 | grep -vE "InitGoogleLogging|HostManager" || true
    ls "$O/$P"/*.o >/dev/null
  done

  # 3) 입력 전처리 — 그 모델의 VTA main 과 똑같이(레이아웃, 채널 순서, 값 범위)
  python3 - "$UP/glow/tests/images/imagenet" "$O" "$IMAGE_LAYOUT" "$IMAGE_MODE" "$CHANNEL_ORDER" $IMAGES <<'PY'
import sys, numpy as np
from PIL import Image
src, out, layout, mode, order, *names = sys.argv[1:]
for n in names:
    a = np.asarray(Image.open(f"{src}/{n}.png").convert("RGB"), dtype=np.float32)
    assert a.shape == (224, 224, 3), a.shape
    if mode == "0to1": a = a / 255.0
    elif mode != "0to255": sys.exit(f"모르는 IMAGE_MODE {mode}")
    if order.upper() == "BGR": a = a[:, :, ::-1]
    if layout.upper() == "NCHW": a = a.transpose(2, 0, 1)
    np.ascontiguousarray(a, dtype=np.float32).tofile(f"{out}/{n}.raw")
PY

  BASE=$(basename "$(ls "$O"/fp32/*.o)" .o)
  INSYM=$(echo "$INPUT_NAME" | sed 's/[^A-Za-z0-9_]/_/g')
  OUTSYM=$(echo "$OUTPUT_NAME" | sed 's/[^A-Za-z0-9_]/_/g')
  ITERS_VAR="ITERS_$M"; ITERS=${!ITERS_VAR:-}
  [ -n "$ITERS" ] || { case $M in resnet50) ITERS=12 ;; *) ITERS=40 ;; esac; }

  ssh board "rm -rf $BB/$M; mkdir -p $BB/$M"
  scp -q "$O"/*.raw board:$BB/$M/
  for P in fp32 int8; do
    ssh board "mkdir -p $BB/$M/$P"
    scp -q "$O/$P"/* board:$BB/$M/$P/
    echo "---------- $M $P ----------"
    ssh board "set -e; cd $BB/$M/$P
      g++ -O2 -std=c++11 -no-pie -I. \
        -DBUNDLE_HDR='\"$BASE.h\"' -DENTRY=$BASE -DCONFIG=${BASE}_config \
        -DIN_SYM='\"$INSYM\"' -DOUT_SYM='\"$OUTSYM\"' \
        $BB/cpu_bench_main.cpp $BASE.o -lpthread -o bench
      ins=\$(for n in $IMAGES; do printf '../%s.raw ' \$n; done)
      ./bench $BASE.weights.bin $ITERS 1 \$ins
      ./bench $BASE.weights.bin $((ITERS * 2)) $THREADS_MAX \$ins | grep -E 'TOTAL|THROUGHPUT'"
  done
  )
done
echo "== 끝"
