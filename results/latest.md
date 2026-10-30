# model-test 결과

- 시각: 2026-10-30 07:37 UTC
- 커밋: 2516aa16
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41021143725
- 대상: resnet18 squeezenet 

### resnet18 — 통과

```
   OK   cat_285  281  (0.533373, 1477.82ms)
   OK   dog_207  207  (0.980803, 1478.75ms)
   OK   zebra_340  340  (0.963957, 1480.78ms)
```

### squeezenet — 실패 (알려진 실패)

```
   FAIL cat_285  기대 281  실제 905  (0.058724, 645.119ms)
   FAIL dog_207  기대 205  실제 904  (0.208223, 644.73ms)
   FAIL zebra_340  기대 340  실제 533  (0.094922, 646.452ms)
```

