# model-test 결과

- 시각: 2026-11-04 09:41 UTC
- 커밋: 535ab032
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41809028759
- 대상: resnet18  + resnet18-4core(4코어)
- 보드 빌드: build_aarch64

### resnet18 — 통과

```
   OK   cat_285  281  (0.478113, 91.738ms)
   OK   dog_207  207  (0.977876, 91.555ms)
   OK   zebra_340  340  (0.969942, 91.568ms)
```

### resnet18-4core — 통과

```
   실행 중 최저 여유 74056 kB
   사용량 179184 kB  (코어당 약 44796 kB)
   총 4132.75ms / 100장
   장당 41.3ms (벽시계 기준)
   처리량 24.20 img/s
```

