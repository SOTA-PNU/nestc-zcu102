# model-test 결과

- 시각: 2026-11-02 07:59 UTC
- 커밋: 2478a1b1
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41409699672
- 대상: resnet18 sq-cut-fire5 squeezenet 
- 보드 빌드: build_aarch64

### resnet18 — 통과

```
   OK   cat_285  281  (0.478113, 92.111ms)
   OK   dog_207  207  (0.977876, 92.794ms)
   OK   zebra_340  340  (0.969942, 92.925ms)
```

### sq-cut-fire5 — 실패 (알려진 실패)

```
(결과 줄 없음)
```

### squeezenet — 실패 (알려진 실패)

```
   FAIL cat_285  기대 281  실제 905  (0.072325, 174.441ms)
   FAIL dog_207  기대 205  실제 904  (0.222572, 175.278ms)
   FAIL zebra_340  기대 340  실제 533  (0.095507, 175.519ms)
```

