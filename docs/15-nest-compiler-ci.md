# 15. SOTA-PNU/nest-compiler 로 상류 교체와 보드 CI (2026-11-05 ~ 11-06)

[SOTA-PNU/nest-compiler](https://github.com/SOTA-PNU/nest-compiler) 는 권용인 님이
ETRI NEST-C 를 옮겨 와 고치고 있는 연구실 저장소다(vta/vtalib 은
[SOTA-PNU/vta_lib](https://github.com/SOTA-PNU/vta_lib)).

## 지금 구조 (2026-11-06)

| | SOTA-PNU/nest-compiler | SOTA-PNU/nestc-zcu102 (이 저장소) |
|---|---|---|
| 성격 | 연구실 공식 컴파일러 | 개인 공부·연구용 |
| 보드 CI | `build.yml` → `test-board` job | model-test, multievta, board-test, debug-dump, ... |
| 러너 | `sota-aip-zcu102` (`~/actions-runner-nc`, label `zcu102`) | `sota-aip` (`~/actions-runner`) |
| 서버 트리 | `~/nestc-sota` (커밋마다 바꿈) | `~/nestc-upstream` (`ci/upstream.ref` 고정) |
| 보드 트리 | `~/nest-compiler-sota` | `~/nest-compiler-2024` |
| 보드 키 | 서버 `~/.ssh/zcu102_key` | 시크릿 `BOARD_SSH_KEY` |

두 러너 모두 연구실 서버(sota-AIP, ehdgns)에 있고 보드는 한 대다. GitHub 의
`concurrency` 는 저장소마다 따로라 서로를 기다리지 않는다. 그래서 보드를 쓰는 모든
job 이 처음에 `scripts/board-lock.sh acquire`, 끝에 `release`(always)를 부른다.
잠금은 서버의 `~/.zcu102-board.lock` 디렉터리다. 300분 넘은 잠금은 버린다.
급하면 서버에서 `rm -rf ~/.zcu102-board.lock`.

트리를 저장소마다 따로 둔 것은 한쪽이 다른 커밋으로 checkout 해도 다른 쪽이
흔들리지 않게 하려는 것이다.

## 이 저장소의 상류 교체 (2026-11-06)

- 서버 `~/nestc-upstream`: ETRI `3951ea4` → nest-compiler `214c04b`.
  원래 커밋은 로컬 태그 `etri-base`, 로컬 수정은 `~/backups/nestc-upstream-etri-local-20261106.diff`.
- 보드 `~/nest-compiler-2024`: ETRI `7326f0d` → `214c04b`. 원래 커밋은 태그 `etri-base`.
- 상류 커밋은 `ci/upstream.ref` 한 줄이다. workflow 가 `scripts/sync-upstream.sh` 로
  서버·보드 트리를 그 커밋에 맞춘다(같으면 아무것도 안 한다). 바뀌면 패치가 지워지므로
  그 뒤에 패치를 다시 얹는다.

### 패치

| 패치 | ETRI 때 | nest-compiler 에서 |
|---|---|---|
| `patch_concat.py` | 적용 | **뺐다** — InsertTensor/Concat 지원이 이미 들어 있다(겹치면 충돌) |
| `patch_requant_round.py` | 실험용 | 쓰지 않는다 — `-vta-round-requant` 옵션으로 들어 있다 |
| `patch_tiling.py` (서버) | 적용 | 적용 |
| `patch_transpose.py` (보드) | 적용 | 적용 (model-test/multievta 보드 단계에도 넣었다) |
| `patch_relu_rescale.py` (보드) | 적용 | 적용 (aarch64 쪽은 "기준 구현과 다름" 으로 건너뜀, generic 만) |
| `patch_vta_max_xfer.py` (보드) | 적용 | 적용 |

## 검증 (2026-11-05, 실행 41969965672)

nest-compiler main `214c04b`(vta_lib `58c601b`)를 보드에서 돌린 결과, ETRI 원본과
같았다.

| | ETRI 7326f0d | nest-compiler 214c04b |
|---|---|---|
| ResNet-18 (281/207/340) | 0.478113 / 0.977876 / 0.969942, 91.5 ms | 같음, 91.6 ms |
| ResNet-50 (281/207/340) | 419 ms | 같음(확신도도), 418 ms |
| ResNet-18 4코어 | 23.68 img/s | 23.55 img/s |
| ResNet-50 3코어 | 5.60 img/s | 5.54 img/s |

이 검증을 하던 `nest-compiler.yml`(repository_dispatch 로 nest-compiler 커밋을 받아
돌리던 workflow)은 지웠다. 같은 일을 nest-compiler 의 `test-board` 가 직접 하고,
같은 트리(`~/nestc-sota`, `~/nest-compiler-sota`)를 써서 둘이 겹치면 서로의 트리를
바꾼다. 결과 기록 `results/nest-compiler.md` 는 남겨 둔다.
