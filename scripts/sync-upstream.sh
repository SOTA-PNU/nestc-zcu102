#!/bin/bash
# 상류 트리를 ci/upstream.ref 의 커밋(SOTA-PNU/nest-compiler)에 맞춘다.
#
#   bash scripts/sync-upstream.sh <트리> <커밋> [board]
#
# 이미 그 커밋이면 아무것도 하지 않는다. 다를 때만 받아서 바꾸고 서브모듈을 맞춘다.
# 바꾸면 우리 패치(ci/patch_*.py)가 지워지므로 이 스크립트 다음에 패치를 다시 얹는다.
# board 를 주면 보드 트리로 보고 vta/vtalib 서브모듈만 맞춘다(보드엔 나머지가 필요 없다).
#
# 상류를 올리려면 ci/upstream.ref 의 커밋만 바꿔 푸시한다.
set -euo pipefail
DIR=$1; REF=$2; MODE=${3:-server}
URL=https://github.com/SOTA-PNU/nest-compiler.git
cd "$DIR"
if [ "$(git rev-parse HEAD)" = "$REF" ]; then
  echo "상류 그대로: $(git log --oneline -1)"
  exit 0
fi
echo "상류 바꿈: $(git log --oneline -1) -> $REF"
git remote get-url sota >/dev/null 2>&1 || git remote add sota "$URL"
# 보드는 재부팅 직후 DNS 가 늦게 올라온다(2026-11-05).
for t in 1 2 3 4 5; do
  git fetch -q --no-recurse-submodules sota "$REF" && break
  [ "$t" = 5 ] && exit 1
  sleep 30
done
git checkout -q -f --detach "$REF"
git submodule sync -q
if [ "$MODE" = board ]; then
  git submodule update -q --init --force vta/vtalib
else
  git submodule update -q --init --force
fi
echo "상류: $(git log --oneline -1)"
echo "vtalib: $(git -C vta/vtalib log --oneline -1)"
