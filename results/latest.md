# model-test 결과

- 시각: 2026-11-06 03:53 UTC
- 커밋: 01eb9cab
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/42159094844
- 대상: resnet18 resnet50  + resnet18-4core(4코어)
- 보드 빌드: build_aarch64

### resnet18 — 통과

```
   OK   cat_285  281  (0.478113, 91.739ms)
   OK   dog_207  207  (0.977876, 91.54ms)
   OK   zebra_340  340  (0.969942, 91.445ms)
```

### resnet50 — 통과

```
   OK   cat_285  281  (0.001961, 419.576ms)
   OK   dog_207  207  (0.002554, 418.152ms)
   OK   zebra_340  340  (0.002706, 418.159ms)
```

### resnet18-4core — 통과

```
   실행 중 최저 여유 37448 kB
   사용량 215364 kB  (코어당 약 53841 kB)
   총 4245.47ms / 100장
   장당 42.5ms (벽시계 기준)
   처리량 23.55 img/s
```

