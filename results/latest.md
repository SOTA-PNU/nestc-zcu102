# model-test 결과

- 시각: 2026-11-04 10:22 UTC
- 커밋: 6b5ad8c3
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41813475989
- 대상: resnet18 resnet50  + resnet18-4core(4코어)
- 보드 빌드: build_aarch64

### resnet18 — 통과

```
   OK   cat_285  281  (0.478113, 91.452ms)
   OK   dog_207  207  (0.977876, 91.596ms)
   OK   zebra_340  340  (0.969942, 91.818ms)
```

### resnet50 — 통과

```
   OK   cat_285  281  (0.001961, 420.54ms)
   OK   dog_207  207  (0.002554, 418.744ms)
   OK   zebra_340  340  (0.002706, 419.787ms)
```

### resnet18-4core — 통과

```
   실행 중 최저 여유 73740 kB
   사용량 179468 kB  (코어당 약 44867 kB)
   총 4222.21ms / 100장
   장당 42.2ms (벽시계 기준)
   처리량 23.68 img/s
```

