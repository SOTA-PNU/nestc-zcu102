# model-test 결과

- 시각: 2026-11-03 08:33 UTC
- 커밋: c79a458b
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41602883251
- 대상: resnet18 squeezenet 
- 보드 빌드: build_aarch64

### resnet18 — 통과

```
   OK   cat_285  281  (0.478113, 92.072ms)
   OK   dog_207  207  (0.977876, 91.405ms)
   OK   zebra_340  340  (0.969942, 91.726ms)
```

### squeezenet — 실패 (알려진 실패)

```
   FAIL cat_285  기대 281  실제 905  (0.072325, 173.849ms)
   FAIL dog_207  기대 205  실제 904  (0.222572, 173.223ms)
   FAIL zebra_340  기대 340  실제 533  (0.095507, 173.119ms)
```

