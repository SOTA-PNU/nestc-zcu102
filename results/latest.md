# model-test 결과

- 시각: 2026-11-04 10:05 UTC
- 커밋: 08d9a873
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41811657405
- 대상: resnet18 resnet50  + resnet18-4core(4코어)
- 보드 빌드: build_aarch64

### resnet18 — 통과

```
   OK   cat_285  281  (0.478113, 91.684ms)
   OK   dog_207  207  (0.977876, 91.689ms)
   OK   zebra_340  340  (0.969942, 91.797ms)
```

### resnet50 — 실패

```
   FAIL cat_285  Result 줄이 없다
   FAIL dog_207  Result 줄이 없다
   FAIL zebra_340  Result 줄이 없다
```

### resnet18-4core — 실패

```
== 중단  CMA 가 모자란다
```

