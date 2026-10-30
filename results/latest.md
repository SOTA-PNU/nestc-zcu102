# model-test 결과

- 시각: 2026-10-30 07:34 UTC
- 커밋: 4e76b5fb
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41020744248
- 대상: resnet18 squeezenet 

### resnet18 — 통과

```
   OK   cat_285  281  (0.533373, 1478.64ms)
   OK   dog_207  207  (0.980803, 1478.57ms)
   OK   zebra_340  340  (0.963957, 1479.24ms)
```

### squeezenet — 실패 (알려진 실패)

```
   FAIL cat_285  기대 281  실제 905  (0.058724, 645.683ms)
   FAIL dog_207  기대 205  실제 904  (0.208223, 645.159ms)
   FAIL zebra_340  기대 340  실제 533  (0.094922, 646.086ms)
```

