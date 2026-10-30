# model-test 결과

- 시각: 2026-10-30 11:57 UTC
- 커밋: 0460cc51
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41051653672
- 대상: resnet18 squeezenet 

### resnet18 — 통과

```
   OK   cat_285  281  (0.533373, 1474.89ms)
   OK   dog_207  207  (0.980803, 1476.08ms)
   OK   zebra_340  340  (0.963957, 1476.08ms)
```

### squeezenet — 실패 (알려진 실패)

```
   FAIL cat_285  기대 281  실제 905  (0.058724, 647.095ms)
   FAIL dog_207  기대 205  실제 904  (0.208223, 644.405ms)
   FAIL zebra_340  기대 340  실제 533  (0.094922, 645.694ms)
```

