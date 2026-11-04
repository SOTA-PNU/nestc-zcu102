# model-test 결과

- 시각: 2026-11-04 10:13 UTC
- 커밋: 53cc808d
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41812447185
- 대상: resnet18 resnet50  + resnet18-4core(4코어)
- 보드 빌드: build_aarch64

### resnet18 — 통과

```
   OK   cat_285  281  (0.478113, 91.654ms)
   OK   dog_207  207  (0.977876, 91.644ms)
   OK   zebra_340  340  (0.969942, 91.608ms)
```

### resnet50 — 실패

```
   OK   cat_285  281  (0.001794, 419.484ms)
   FAIL dog_207  기대 207  실제 170  (0.001504, 419.934ms)
   OK   zebra_340  340  (0.002698, 418.7ms)
```

### resnet18-4core — 통과

```
   실행 중 최저 여유 73544 kB
   사용량 179788 kB  (코어당 약 44947 kB)
   총 4200.76ms / 100장
   장당 42.0ms (벽시계 기준)
   처리량 23.81 img/s
```

