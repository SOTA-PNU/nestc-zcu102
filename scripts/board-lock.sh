#!/bin/bash
# 보드 잠금 — 연구실 서버의 두 러너가 ZCU102 한 대를 번갈아 쓰게 한다.
#
#   bash scripts/board-lock.sh acquire   # 보드가 비면 잡는다(빌 때까지 기다린다)
#   bash scripts/board-lock.sh release   # 내가 잡은 잠금만 푼다
#
# 서버(sota-AIP, ehdgns 계정)에 러너가 둘 있다.
#   sota-aip         SOTA-PNU/nestc-zcu102  (model-test, multievta, ...)
#   sota-aip-zcu102  SOTA-PNU/nest-compiler (build.yml 의 test-board)
# GitHub 의 concurrency 그룹(zcu102-board)은 저장소마다 따로라 두 저장소의 작업이
# 서로를 기다리지 않는다. 그래서 두 러너가 공유하는 홈 디렉터리에 잠금을 둔다.
# mkdir 는 원자적이라 둘이 동시에 잡을 수 없다.
#
# 잠금을 잡은 job 이 러너째로 죽으면 풀리지 않는다. STALE_MIN(기본 300분)이
# 지난 잠금은 버려진 것으로 보고 지운다. 급하면 서버에서 손으로 지운다:
#   rm -rf ~/.zcu102-board.lock
set -eu
L=${BOARD_LOCK:-$HOME/.zcu102-board.lock}
STALE_MIN=${BOARD_LOCK_STALE_MIN:-300}
ME="${GITHUB_REPOSITORY:-local}#${GITHUB_RUN_ID:-$$}.${GITHUB_RUN_ATTEMPT:-1}"

owner() { cat "$L/owner" 2>/dev/null || echo "(주인 기록 없음)"; }

case "${1:-}" in
  acquire)
    n=0
    until mkdir "$L" 2>/dev/null; do
      if [ -n "$(find "$L" -maxdepth 0 -mmin +"$STALE_MIN" 2>/dev/null)" ]; then
        echo "::warning::${STALE_MIN}분 넘은 잠금을 버린다: $(owner)"
        rm -rf "$L"
        continue
      fi
      [ $((n % 10)) = 0 ] && echo "[$(date +%H:%M)] 보드 사용 중: $(owner) — 기다린다"
      n=$((n + 1))
      sleep 30
    done
    echo "$ME $(date '+%F %T')" > "$L/owner"
    echo "보드 잠금: $ME"
    ;;
  release)
    if [ -f "$L/owner" ] && [ "$(cut -d' ' -f1 "$L/owner")" = "$ME" ]; then
      rm -rf "$L"
      echo "보드 잠금 해제: $ME"
    else
      echo "내 잠금이 아니라 두었다: $(owner)"
    fi
    ;;
  *)
    echo "사용: $0 acquire|release" >&2
    exit 2
    ;;
esac
