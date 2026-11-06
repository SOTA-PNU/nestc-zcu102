#!/bin/bash
# 상류 트리를 주어진 커밋(또는 브랜치)에 맞춘다.
#
#   bash scripts/sync-upstream.sh <트리> <커밋|브랜치> [board]
#
# 찾는 순서: 포크 udonghun/nest-compiler -> 원본 SOTA-PNU/nest-compiler.
# 2026-11-06 부터 고칠 것은 포크에서 만들어 여기서 보드로 시험하고, 통과한 것만
# nest-compiler 에 PR 로 올린다(docs/15). 그래서 포크의 브랜치도 받는다.
#
# 이미 그 커밋이면 아무것도 하지 않는다. 다를 때만 받아서 바꾸고 서브모듈을 맞춘다.
# 바꾸면 우리 패치(ci/patch_*.py)가 지워지므로 이 스크립트 다음에 패치를 다시 얹는다.
# board 를 주면 보드 트리로 보고 vta/vtalib 서브모듈만 맞춘다(보드엔 나머지가 필요 없다).
# 마지막 줄에 맞춘 커밋(40자)을 "SHA=<커밋>" 으로 찍는다. 브랜치를 준 경우
# 보드에는 이 값을 넘겨 서버와 같은 커밋을 쓰게 한다.
set -euo pipefail
DIR=$1; REF=$2; MODE=${3:-server}
cd "$DIR"
if [ "$(git rev-parse HEAD)" = "$REF" ]; then
  echo "상류 그대로: $(git log --oneline -1)"
  echo "SHA=$REF"
  exit 0
fi
echo "상류 바꿈: $(git log --oneline -1) -> $REF"
git remote get-url fork >/dev/null 2>&1 || git remote add fork https://github.com/udonghun/nest-compiler.git
git remote get-url sota >/dev/null 2>&1 || git remote add sota https://github.com/SOTA-PNU/nest-compiler.git
# 보드는 재부팅 직후 DNS 가 늦게 올라온다(2026-11-05). 그래서 몇 번 다시 시도한다.
got=
for t in 1 2 3 4 5; do
  for r in fork sota; do
    if git fetch -q --no-recurse-submodules "$r" "$REF" 2>/dev/null; then got=$r; break 2; fi
  done
  [ "$t" = 5 ] && { echo "포크에도 원본에도 $REF 가 없다(또는 네트워크 실패)" >&2; exit 1; }
  sleep 30
done
SHA=$(git rev-parse FETCH_HEAD)
echo "받은 곳: $got  ($REF -> ${SHA:0:12})"
git checkout -q -f --detach "$SHA"
git submodule sync -q
if [ "$MODE" = board ]; then
  git submodule update -q --init --force vta/vtalib
else
  git submodule update -q --init --force
fi
echo "상류: $(git log --oneline -1)"
echo "vtalib: $(git -C vta/vtalib log --oneline -1)"
echo "SHA=$SHA"
