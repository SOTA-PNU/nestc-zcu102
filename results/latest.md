# model-test 결과

- 시각: 2026-10-30 13:55 UTC
- 커밋: 8b609e59
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41067230081
- 대상: resnet18 squeezenet 
- 보드 빌드: build_board

### resnet18 — 통과

```
   OK   cat_285  281  (0.533373, 1475.2ms)
   OK   dog_207  207  (0.980803, 1475.12ms)
   OK   zebra_340  340  (0.963957, 1475.65ms)
```

### squeezenet — 실패 (알려진 실패)

```
   FAIL cat_285  기대 281  실제 905  (0.058724, 647.388ms)
   FAIL dog_207  기대 205  실제 904  (0.208223, 646.072ms)
   FAIL zebra_340  기대 340  실제 533  (0.094922, 646.014ms)
```

