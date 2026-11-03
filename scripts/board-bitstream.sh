#!/usr/bin/env bash
# 보드 FPGA 에 올라갈 비트스트림을 고른다. 보드에서 돌린다.
#
#   scripts/board-bitstream.sh single   부팅 기본(rc.local)과 같은 단일 VTA
#   scripts/board-bitstream.sh multi    ETRI 공개 4코어 EVTA
#
# 2026-11-03 에 알았다. 부팅 때 rc.local 이 올리는 /root/bitstream/vta.bit
# (vta_wrapper, 2020/08/04)는 단일 VTA 다. 디바이스 트리에도 코어 0 만
# 있다. 그 위에서 코어 1~3 주소(0xA001xxxx~0xA003xxxx)를 건드리면 SError
# 로 스레드가 죽는다. 4코어는 ETRI nest-data 의
# zcu102_4_1x16_i8w8a32_16_15_18_17.bit 를 올려야 하고, 보드 런타임도
# NESTC_EVTA_MULTI=ON (버퍼 15/18/17) 으로 지은 것을 써야 한다
# (scripts/board-setup-multi.sh, build_aarch64_multi).
#
# 단일 구성의 기준값(ResNet-18 0.478113)은 단일 비트스트림에서 잰 것이다.
# 그래서 4코어를 쓴 뒤에는 반드시 single 로 되돌린다. 재부팅해도 rc.local 이
# single 을 다시 올린다.
#
# VTA 를 쓰는 프로세스가 있으면 바꾸지 않는다. 추론 중에 FPGA 를 다시 쓰면
# 보드가 멎는다.
set -euo pipefail

MODE=${1:-}
SINGLE=/root/bitstream/vta.bit
MULTI_DIR=/home/xilinx/bitstreams
MULTI_NAME=zcu102_4_1x16_i8w8a32_16_15_18_17.bit
MULTI=$MULTI_DIR/$MULTI_NAME
MULTI_SHA=7d550befe9039046372d98bd50ccdfe6078c06e9ea0978f9db682c03dc24056f
MULTI_URL=https://media.githubusercontent.com/media/etri/nest-data/master/bitstreams/$MULTI_NAME

case "$MODE" in
  single) BIT=$SINGLE ;;
  multi)  BIT=$MULTI ;;
  *) sed -n '2,6p' "$0" >&2; exit 2 ;;
esac

# VTA 런타임은 cma_alloc 으로 /dev/xlnk 를 연다. 명령줄로 찾으면 이 스크립트를
# 부른 ssh 셸 자신(인자에 /bundles/ 가 들어 있다)이 걸려 오판한다(2026-11-03).
if sudo fuser /dev/xlnk >/dev/null 2>&1; then
  echo "== 중단  /dev/xlnk 를 연 프로세스가 있다(VTA 사용 중). 비트스트림을 바꾸지 않는다." >&2
  sudo fuser -v /dev/xlnk >&2 || true
  exit 1
fi

if [ "$MODE" = multi ]; then
  if [ ! -f "$MULTI" ] || [ "$(sha256sum "$MULTI" | cut -d' ' -f1)" != "$MULTI_SHA" ]; then
    echo "== 4코어 비트스트림 받기"
    mkdir -p "$MULTI_DIR"
    curl -sSL -m 600 -o "$MULTI.part" "$MULTI_URL"
    [ "$(sha256sum "$MULTI.part" | cut -d' ' -f1)" = "$MULTI_SHA" ] || {
      echo "== 실패  해시가 맞지 않는다" >&2; rm -f "$MULTI.part"; exit 1; }
    mv "$MULTI.part" "$MULTI"
  fi
fi

echo "== 비트스트림 $MODE  ($BIT)"
OOPS_BEFORE=$(sudo dmesg | grep -c 'Internal error: Oops' || true)
# rc.local 의 bitwrite.py 와 같은 방법이다. pynq 가 종료할 때 내는
# mmap_file 경고는 무해하다.
sudo python3 -c "
import sys
from pynq import Bitstream
Bitstream(sys.argv[1]).download()
" "$BIT" 2>&1 | grep -v mmap_file || true
STATE=$(cat /sys/class/fpga_manager/fpga0/state)
echo "   fpga $STATE"
[ "$STATE" = operating ] || { echo "== 실패  FPGA 가 operating 이 아니다" >&2; exit 1; }

# 코어 0 은 어느 비트스트림에나 있다. multi 일 때만 코어 1~3 을 읽는다 —
# single 에서 읽으면 그 자체가 SError 를 낸다.
ADDRS="0xA0000000"
[ "$MODE" = multi ] && ADDRS="0xA0000000 0xA0010000 0xA0020000 0xA0030000"
for a in $ADDRS; do
  v=$(sudo python3 -c "
import mmap,os,struct,sys
a=int(sys.argv[1],16); fd=os.open('/dev/mem',os.O_RDONLY|os.O_SYNC)
m=mmap.mmap(fd,0x1000,mmap.MAP_SHARED,mmap.PROT_READ,offset=a)
print('%08x'%struct.unpack('<I',m[0:4])[0])
" "$a")
  echo "   $a  ctrl=$v"
done
OOPS_AFTER=$(sudo dmesg | grep -c 'Internal error: Oops' || true)
if [ "$OOPS_AFTER" -gt "$OOPS_BEFORE" ]; then
  echo "== 실패  코어 레지스터를 읽다가 커널 oops 가 났다" >&2
  exit 1
fi
echo "== 완료  $MODE"
