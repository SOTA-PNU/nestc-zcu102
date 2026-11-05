# 15. SOTA-PNU/nest-compiler 보드 검증 CI (2026-11-05)

[SOTA-PNU/nest-compiler](https://github.com/SOTA-PNU/nest-compiler) 는 권용인 님이
ETRI NEST-C 를 옮겨 와 고치고 있는 저장소다(vta/vtalib 은
[SOTA-PNU/vta_lib](https://github.com/SOTA-PNU/vta_lib)). 그쪽 CI(build.yml)는
컴파일러 빌드까지만 하고 보드 테스트를 이 저장소에 맡겼다. 그 보드 테스트가 이것이다.

## 흐름

```
nest-compiler  push / PR
   └ board-test.yml (GitHub 호스티드)  ── repository_dispatch(ref=커밋) ──┐
                                                                         ▼
nestc-zcu102   nest-compiler.yml (연구실 서버 러너, 보드 대기열 zcu102-board)
   1. 서버 ~/nestc-sota 를 그 커밋으로 → patch_tiling → ninja (증분, 약 2분)
   2. 보드 ~/nest-compiler-sota 를 그 커밋으로 → 패치 3개 → vtalib 빌드
   3. ResNet-18 / ResNet-50 1코어 (정답 3장)
   4. ResNet-18 4코어 / ResNet-50 3코어 (정답 + oops 0 + 처리량)
   5. results/nest-compiler.md 커밋
   └ 판정 ──────────────────────────────────── board-test 가 기다렸다가 ✓/✗ 로 보여 줌
```

기존 model-test / multievta 는 ETRI 원본(7326f0d) 기준 그대로 둔다. 두 결과를 나란히
비교할 수 있다(`results/latest.md`, `results/multievta.md` 대 `results/nest-compiler.md`).

## 패치

nest-compiler 에 이미 같은 것이 들어간 우리 패치는 뺐다.

| 패치 | nest-compiler 에서 | 이 CI 에서 |
|---|---|---|
| `patch_concat` | InsertTensor/Concat 지원이 들어감(4b19030, vta_lib 8e64c7d). 같이 넣으면 `saveInsertTensorInst` 중복 정의로 빌드 실패 | **안 씀** |
| `patch_requant_round` | 같은 원리의 `-vta-round-requant` 가 들어감(VTARequantRounding) | 안 씀 |
| `patch_tiling` | 다른 문제(튜닝 표에 없는 층) | 서버에 얹음 |
| `patch_transpose` | aarch64 `transpose()` 여전히 미구현 | 보드에 얹음 |
| `patch_relu_rescale` | 그대로 | 보드에 얹음 |
| `patch_vta_max_xfer` | 그대로 (멀티코어 CMA) | 보드에 얹음 |

남은 넷을 nest-compiler / vta_lib 에 PR 로 올리면 이 목록이 비고, 순수한 상류를 검증하게 된다.

## 트리 준비 (한 번만, 2026-11-05 에 해 둠)

- 서버: `cp -a ~/nestc-upstream ~/nestc-sota` 후 `sota` 원격(nest-compiler) 으로 checkout,
  vtalib 서브모듈은 SOTA-PNU/vta_lib. 빌드 캐시가 같은 `/root/nestc` 경로라 증분으로 된다.
  `git fetch` 는 `--no-recurse-submodules` 로 한다(옛 GitLab 주소로 서브모듈을 받으려다 실패).
- 보드: `~/nest-compiler-2024` 를 `build_*` 빼고 복사해 `~/nest-compiler-sota`,
  `build_aarch64` / `build_aarch64_multi` 는 기존 CMakeCache 옵션으로 새로 cmake.

## nest-compiler 쪽 설정

`.github/workflows/board-test.yml` 과 시크릿 `BOARD_CI_TOKEN` 이 필요하다. 토큰은
fine-grained, Resource owner SOTA-PNU, 저장소 nestc-zcu102 만, 권한 Actions: Read and write.
nest-compiler → Settings → Secrets and variables → Actions 에 넣는다. 포크 PR 은 건너뛴다.

수동 실행: nestc-zcu102 → Actions → nest-compiler → Run workflow (ref 에 브랜치·커밋).
매일 03:40 KST 에 main 이 바뀌었으면 스스로 돈다.
