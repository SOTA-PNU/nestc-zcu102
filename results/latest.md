# model-test 결과

- 시각: 2026-11-02 07:55 UTC
- 커밋: 8a643a77
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41409287457
- 대상: resnet18 squeezenet 
- 보드 빌드: build_aarch64

### resnet18 — 통과

```
   OK   cat_285  281  (0.478113, 91.935ms)
   OK   dog_207  207  (0.977876, 92.257ms)
   OK   zebra_340  340  (0.969942, 93.249ms)
```

### squeezenet — 실패 (알려진 실패)

```
   FAIL cat_285  기대 281  실제 905  (0.072325, 175.428ms)
   FAIL dog_207  기대 205  실제 904  (0.222572, 175.306ms)
   FAIL zebra_340  기대 340  실제 533  (0.095507, 175.773ms)
```

