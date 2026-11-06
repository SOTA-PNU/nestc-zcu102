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
`concurrency` 는 저장소마다 따로라 서로를 기다리지 않는다.

서버 잠금(`scripts/board-lock.sh`, `~/.zcu102-board.lock`)을 만들어 nest-compiler 의
test-board 에도 넣었다가 **2026-11-06 에 nest-compiler 쪽에서는 뺐다**(PR #6, 1056a59).
공식 저장소에 개인 저장소의 장치를 넣지 않기로 했기 때문이다. 이 저장소 workflow 에는
남아 있지만 이제 이 저장소 job 끼리만 순서를 맞춘다(concurrency 와 같은 일).

그래서 두 저장소가 동시에 보드를 열 수 있다. 겹치면 둘 다 CMA 부족이나 VTA 오류로
실패하고 CMA 가 새어 재부팅이 필요할 수 있다. 이 저장소에서 수동 실행하기 전에
nest-compiler Actions 에 test-board 가 돌고 있지 않은지 본다.

트리를 저장소마다 따로 둔 것은 한쪽이 다른 커밋으로 checkout 해도 다른 쪽이
흔들리지 않게 하려는 것이다.

## 고치는 순서: 포크 → 여기서 시험 → nest-compiler (2026-11-06~)

```
udonghun/nest-compiler (포크) 브랜치에 푸시
        │
        ▼
nestc-zcu102 model-test / multievta  ── 수동 실행, 입력 ref=<브랜치 또는 커밋>
        │  서버 ~/nestc-upstream 를 그 커밋으로 맞추고 ninja 증분 빌드
        │  보드 ~/nest-compiler-2024 도 같은 커밋(vtalib 포함)으로 맞추고 make
        ▼
성과가 있으면 → SOTA-PNU/nest-compiler 에 PR → 그쪽 build.yml (test-board 포함)
통과만 했거나 실패하면 → 포크에 남겨 둔다. nest-compiler 에는 올리지 않는다.
```

- **올리는 기준은 '통과'가 아니라 '성과'다.** 둘 다 만족해야 한다.
  1. 정확도가 깨지지 않는다 — ResNet-18/50 의 281/207/340 확신도가 기준값과 같거나 낫다.
  2. 하나라도 나아진다 — 장당 시간, 멀티코어 처리량, CMA 사용량(최저 여유), 정확도.
  기준값은 `ref` 를 비운 실행(= `ci/upstream.ref`)의 결과다. 2026-11-06 기준:
  R18 91.5 ms · R50 418~420 ms · R18 4코어 24.00 · R50 3코어 5.59 · R50 4코어 6.71 img/s.
  PR 본문에 같은 표로 전후를 적는다. 측정 흔들림(±1~2%) 안의 차이는 성과로 치지 않는다.
- `scripts/sync-upstream.sh` 는 포크에서 먼저 찾고 없으면 원본에서 찾는다. 브랜치
  이름을 받으면 서버가 맞춘 커밋(40자)을 `UPSTREAM_REF` 로 보드에 넘겨 둘이 같은
  커밋을 쓴다. 결과 기록(`results/latest.md`, `results/multievta.md`)에 `상류:` 줄이 남는다.
- `ref` 를 비우면 예전처럼 `ci/upstream.ref` 커밋이다. 포크 시험 뒤에는 트리가 그
  커밋에 남아 있으므로, 다음 기본 실행이 자동으로 `ci/upstream.ref` 로 되돌린다.
- 포크는 원본 `main` 을 따라가지 않는다. 원본이 앞서가면 포크 화면의 **Sync fork** 를 누르거나
  브랜치를 원본 `main` 위로 rebase 한다.
- vta/vtalib 은 서브모듈(SOTA-PNU/vta_lib)이다. vtalib 을 고칠 일이 생기면 그것도 포크해서
  포크 브랜치의 `.gitmodules` 를 잠시 그쪽으로 돌린다(아직 해 본 적 없다).

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
