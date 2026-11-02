#!/usr/bin/env python3
"""VTA 명령 큐의 크기를 낮춰 네 코어가 CMA 에 들어가게 한다.

왜 필요한가.

비트스트림에는 EVTA 코어가 넷 있는데(VTA_DEVICE_NUM=4) 네 개를 동시에
띄우면 FPGA 연속 메모리(CMA)가 모자라 죽는다. 2026-11-02 에 실행 중
CmaFree 를 3초마다 재서 코어당 사용량을 실측했더니 **77,616 kB** 였다.
그런데 번들이 요구하는 것은 가중치 12,003,168 바이트와 활성 606,112
바이트, 합쳐 12.6MB 뿐이다. 65MB 가 설명되지 않았다.

출처는 이것이다.

    vta/vtalib/include/zcu102/vta/driver.h:45
        #define VTA_MAX_XFER (1<<25)              // 32MB

    vta/vtalib/include/zcu102/cmd_queue.h:579,581
        UopQueue <VTA_MAX_XFER, ...> uop_queue_;
        InsnQueue<VTA_MAX_XFER, ...> insn_queue_;

컨텍스트마다 큐가 둘이니 32MB × 2 = 64MB 가 고정으로 잡힌다. 거기에
12.6MB 를 더하면 76.6MB — 실측과 맞는다.

    1<<25 (32MB)   코어당 76.6MB   ×4 = 306MB   CMA 256MB 에 안 들어감
    1<<24 (16MB)   코어당 44.6MB   ×4 = 178MB   들어감
    1<<23 ( 8MB)   코어당 28.6MB   ×4 = 114MB   여유 있게 들어감

상류 TVM 의 VTA 는 이 값이 1<<22(4MB)다. ETRI 가 8배로 올려 두었다.
우리가 1<<23 으로 낮춰도 상류 기본값의 두 배다.

위험과 그 처리.

큐가 모자라면 `CHECK(buff_size <= kMaxBytes)` 에 걸려 죽는다
(dat_queue.h:165, 572, 589). 조용히 틀린 답을 내는 것이 아니라 그 자리에서
멈추므로, 너무 작게 잡았는지는 바로 드러난다. 그래도 한 번에 끝까지
내리지 말고 1<<24 부터 확인하는 편이 낫다.

    NESTC_VTA_MAX_XFER_LOG=24 python3 patch_vta_max_xfer.py <상류>

기본값은 23 이다. 멱등하다 — 이미 바뀌어 있으면 아무것도 하지 않는다.
되돌리려면 25 를 주면 된다.

이 패치는 **보드에서** 적용하고 vtalib 를 다시 빌드해야 한다. 컴파일러
쪽은 영향이 없다.
"""
import os
import re
import sys

REL = "vta/vtalib/include/zcu102/vta/driver.h"
PAT = re.compile(r"(#define\s+VTA_MAX_XFER\s*\()\s*1\s*<<\s*(\d+)\s*(\))")


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    root = sys.argv[1]
    want = int(os.environ.get("NESTC_VTA_MAX_XFER_LOG", "23"))
    if not 20 <= want <= 25:
        sys.exit("NESTC_VTA_MAX_XFER_LOG 는 20..25 사이여야 한다: %d" % want)

    path = os.path.join(root, REL)
    if not os.path.isfile(path):
        sys.exit("찾지 못했다: %s" % path)
    src = open(path, encoding="utf-8", errors="replace").read()

    m = PAT.search(src)
    if not m:
        sys.exit("VTA_MAX_XFER 정의를 찾지 못했다. 상류가 바뀌었는지 본다: %s"
                 % path)
    cur = int(m.group(2))
    if cur == want:
        print("   이미 1<<%d 다. 건드리지 않는다." % want)
        return

    new = src[:m.start()] + "%s1<<%d%s" % (m.group(1), want, m.group(3)) \
        + src[m.end():]
    open(path, "w", encoding="utf-8").write(new)
    mb = (1 << want) >> 20
    print("   VTA_MAX_XFER  1<<%d -> 1<<%d  (%dMB, 코어당 큐 둘이니 %dMB)"
          % (cur, want, mb, mb * 2))
    print("   보드에서 vtalib 를 다시 빌드해야 적용된다.")


if __name__ == "__main__":
    main()
