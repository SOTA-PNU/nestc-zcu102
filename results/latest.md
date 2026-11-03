# model-test 결과

- 시각: 2026-11-03 13:52 UTC
- 커밋: 4e839c08
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41648017944
- 대상: resnet18  + resnet18-4core(4코어)
- 보드 빌드: build_aarch64

### resnet18 — 통과

```
   OK   cat_285  281  (0.478113, 91.38ms)
   OK   dog_207  207  (0.977876, 91.539ms)
   OK   zebra_340  340  (0.969942, 91.175ms)
```

### resnet18-4core — 통과

```
   실행 중 최저 여유 74472 kB
   사용량 178860 kB  (코어당 약 44715 kB)
   총 4302.31ms / 100장
   장당 43.0ms (벽시계 기준)
   처리량 23.24 img/s
```

