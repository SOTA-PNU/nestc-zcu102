# 15. SOTA-PNU/nest-compiler 로 상류 교체와 보드 CI (2026-11-05 ~ 11-06)

[SOTA-PNU/nest-compiler](https://github.com/SOTA-PNU/nest-compiler) 는 권용인 님이
ETRI NEST-C 를 옮겨 와 고치고 있는 연구실 저장소다(vta/vtalib 은
[SOTA-PNU/vta_lib](https://github.com/SOTA-PNU/vta_lib)).

## 지금 구조 (2026-11-06 저녁)

보드 CI 는 둘이다. 둘 다 `build.yml` 의 `test-board` job 이고 연구실 서버(sota-AIP, ehdgns)의
러너에서 돈다.

| | SOTA-PNU/nest-compiler | udonghun/nest-compiler (포크) |
|---|---|---|
| 성격 | 연구실 공식 컴파일러 | 개인 실험장 |
| 언제 도나 | main 푸시, 같은 저장소 브랜치의 PR | 아무 브랜치 푸시, PR |
| build(컴파일·단위 테스트) | compute-nestc 에서 돈 뒤 test-board | 건너뜀(포크엔 그 러너가 없다) |
| 러너 | `sota-aip-zcu102` (`~/actions-runner-nc`) | `sota-aip-fork` (`~/actions-runner-fork`) |
| 서버 트리 / 보드 트리 | `~/nestc-sota` / `~/nest-compiler-sota` | 같은 트리를 쓴다 |
| 진행 중 실행 취소 | PR 브랜치 새 푸시 때 취소된다(주의) | 취소하지 않는다 |

- 두 test-board 모두 이 저장소(nestc-zcu102) main 을 checkout 해서 스크립트·모델·패치를 쓴다.
  **이 저장소 main 에 푸시하면 두 CI 에 바로 반영된다.**
- 이 저장소의 workflow 8개는 2026-11-06 에 GitHub 에서 비활성화했다(파일은 남김,
  Actions 화면에서 다시 켤 수 있다). 러너 `sota-aip`(`~/actions-runner`)도 내렸다.
- 두 러너는 서로를 기다리지 않는다(concurrency 는 저장소마다 따로, 보드 잠금은 쓰지 않기로 함).
  동시에 돌면 보드에서 겹쳐 둘 다 실패하고 CMA 가 새어 재부팅이 필요할 수 있다.
  서버 트리도 같은 것을 쓰므로 동시에 돌면 서로의 checkout 을 바꾼다.

## 고치는 순서: 포크 → 공식 저장소

```
포크 udonghun/nest-compiler 에 브랜치 푸시
        │  포크 Actions: test-board (서버 ninja 증분 빌드 → 보드 make → ResNet-18/50, 멀티코어)
        ▼
성과가 있으면 → 고친 커밋만 SOTA-PNU/nest-compiler main 위로 옮겨(cherry-pick)
               그 저장소에 브랜치로 푸시 → PR → 공식 build + test-board 가 한 번 더 확인
통과만 했거나 실패하면 → 포크에 남겨 둔다.
```

- 포크에서 공식 저장소로 PR 을 보내지 않는다. 공식 build.yml 은 포크에서 온 PR 에는
  self-hosted job 을 돌리지 않으므로(보안 조건) 아무 확인 없이 올라가게 된다.
- 포크 main 은 공식 main + PR #6(test-board) + 포크 전용 커밋 `da06632` 이다.
  `da06632` 은 공식 저장소로 옮기지 않는다.
- **올리는 기준은 '통과'가 아니라 '성과'다.** 둘 다 만족해야 한다.
  1. 정확도가 깨지지 않는다 — ResNet-18/50 의 281/207/340 확신도가 기준값과 같거나 낫다.
  2. 하나라도 나아진다 — 장당 시간, 멀티코어 처리량, CMA 사용량(최저 여유), 정확도.
  기준값은 2026-11-06 nest-compiler `214c04b` 의 결과다:
  R18 91.5 ms · R50 418~420 ms · R18 4코어 24.00 · R50 3코어 5.59 · R50 4코어 6.71 img/s.
  PR 본문에 같은 표로 전후를 적는다. 측정 흔들림(±1~2%) 안의 차이는 성과로 치지 않는다.
- 포크는 원본 `main` 을 따라가지 않는다. 원본이 앞서가면 포크 화면의 **Sync fork** 를 누르거나
  브랜치를 포크 main 위로 rebase 한다.
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
