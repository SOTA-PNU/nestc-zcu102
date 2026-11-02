# model-test 결과

- 시각: 2026-11-02 08:11 UTC
- 커밋: 3129211d
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41410939109
- 대상: resnet18 sq-cut-fire2 sq-cut-fire5 squeezenet 
- 보드 빌드: build_aarch64

### resnet18 — 통과

```
   OK   cat_285  281  (0.478113, 92.593ms)
   OK   dog_207  207  (0.977876, 92.642ms)
   OK   zebra_340  340  (0.969942, 91.889ms)
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
   FAIL cat_285  Result 줄이 없다
   FAIL dog_207  Result 줄이 없다
   FAIL zebra_340  Result 줄이 없다
```

