# model-test 결과

- 시각: 2026-10-30 23:24 UTC
- 커밋: c4a70703
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41144238880
- 대상: resnet18 squeezenet 
- 보드 빌드: build_aarch64

### resnet18 — 통과

```
   OK   cat_285  281  (0.478113, 92.575ms)
   OK   dog_207  207  (0.977876, 91.612ms)
   OK   zebra_340  340  (0.969942, 92.231ms)
```

### squeezenet — 실패 (알려진 실패)

```
   FAIL cat_285  기대 281  실제 905  (0.072325, 175.15ms)
   FAIL dog_207  기대 205  실제 904  (0.222572, 174.672ms)
   FAIL zebra_340  기대 340  실제 533  (0.095507, 174.144ms)
```

