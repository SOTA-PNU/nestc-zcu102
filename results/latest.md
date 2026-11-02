# model-test 결과

- 시각: 2026-11-02 08:47 UTC
- 커밋: 2e2d4895
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41415385783
- 대상: resnet18 sq-cut-fire2 sq-cut-fire5 squeezenet 
- 보드 빌드: build_aarch64

### resnet18 — 통과

```
   OK   cat_285  281  (0.478113, 91.471ms)
   OK   dog_207  207  (0.977876, 91.664ms)
   OK   zebra_340  340  (0.969942, 91.146ms)
```

### sq-cut-fire2 — 실패 (알려진 실패)

```
   FAIL cat_285  Result 줄이 없다
```

### sq-cut-fire5 — 실패 (알려진 실패)

```
   FAIL cat_285  Result 줄이 없다
```

### squeezenet — 실패 (알려진 실패)

```
   FAIL cat_285  기대 281  실제 905  (0.072325, 173.653ms)
   FAIL dog_207  기대 205  실제 904  (0.222572, 173.884ms)
   FAIL zebra_340  기대 340  실제 533  (0.095507, 173.183ms)
```

