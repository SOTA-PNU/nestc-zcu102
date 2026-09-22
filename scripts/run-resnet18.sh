#!/bin/bash
# ZCU102 VTA에서 ResNet-18 추론 실행
#
# 사용법:
#   sudo ./run-resnet18.sh [이미지경로]
#
# 인자를 주지 않으면 dog_207.png 로 실행한다.
# /dev/xlnk 가 root 전용이므로 sudo 가 필요하다.

set -euo pipefail

NESTC=/home/xilinx/nest-compiler
BUNDLE=$NESTC/origin/vta/bundles/Resnet18Test
IMAGE=${1:-$NESTC/glow/tests/images/imagenet/dog_207.png}

if [ "$EUID" -ne 0 ]; then
  echo "오류: sudo 로 실행해야 한다 (/dev/xlnk 가 root 전용)" >&2
  exit 1
fi

if [ ! -e /dev/xlnk ]; then
  echo "오류: /dev/xlnk 가 없다. VTA 드라이버를 확인할 것" >&2
  exit 1
fi

if [ ! -f "$IMAGE" ]; then
  echo "오류: 이미지를 찾을 수 없다: $IMAGE" >&2
  echo "사용 가능한 이미지:" >&2
  ls "$NESTC/glow/tests/images/imagenet/" >&2
  exit 1
fi

cd "$BUNDLE"
exec ./vtaMxnetResnet18Bundle "$IMAGE"
