# nest-compiler 보드 검증 결과

- 시각: 2026-11-05 04:53 UTC
- nest-compiler: `214c04b1b59e` (ref `main`) https://github.com/SOTA-PNU/nest-compiler/commit/214c04b1b59e3dce0dad055f71f950a2fbf40eb0
- vta_lib: `58c601be8853` https://github.com/SOTA-PNU/vta_lib/commit/58c601be8853ac874b511a6dec440db8a08ea9e6
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41969965672
- 얹은 패치: 서버 patch_tiling / 보드 patch_transpose, patch_relu_rescale, patch_vta_max_xfer
- 비교 기준(ETRI 원본 7326f0d): results/latest.md, results/multievta.md

### resnet18 — 통과

```
   OK   cat_285  281  (0.478113, 91.666ms)
   OK   dog_207  207  (0.977876, 91.756ms)
   OK   zebra_340  340  (0.969942, 91.402ms)
```

### resnet50 — 통과

```
   OK   cat_285  281  (0.001961, 417.819ms)
   OK   dog_207  207  (0.002554, 418.361ms)
   OK   zebra_340  340  (0.002706, 418.127ms)
```

### resnet18-4core — 통과

```
   실행 중 최저 여유 74112 kB
   사용량 179220 kB  (코어당 약 44805 kB)
   총 4245.54ms / 100장
   장당 42.5ms (벽시계 기준)
   처리량 23.55 img/s
```

### resnet50-3core — 통과

```
   실행 중 최저 여유 76804 kB
   사용량 176524 kB  (코어당 약 58841 kB)
   총 18065ms / 100장
   장당 180.7ms (벽시계 기준)
   처리량 5.54 img/s
```

