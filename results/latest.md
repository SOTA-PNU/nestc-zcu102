# model-test 결과

- 시각: 2026-11-06 08:17 UTC
- 커밋: 6487245e
- 상류: 214c04b1b59e
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/42186076852
- 대상: resnet18 resnet50  + resnet18-4core(4코어)
- 보드 빌드: build_aarch64

### resnet18 — 통과

```
   OK   cat_285  281  (0.478113, 91.504ms)
   OK   dog_207  207  (0.977876, 91.43ms)
   OK   zebra_340  340  (0.969942, 91.653ms)
```

### resnet50 — 통과

```
   OK   cat_285  281  (0.001961, 418.265ms)
   OK   dog_207  207  (0.002554, 421.327ms)
   OK   zebra_340  340  (0.002706, 418.642ms)
```

### resnet18-4core — 통과

```
   실행 중 최저 여유 74416 kB
   사용량 178916 kB  (코어당 약 44729 kB)
   총 4225.49ms / 100장
   장당 42.3ms (벽시계 기준)
   처리량 23.67 img/s
```

