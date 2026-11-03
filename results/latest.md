# model-test 결과

- 시각: 2026-11-03 13:37 UTC
- 커밋: 590cf7a7
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41645141780
- 대상: resnet18 squeezenet  + resnet18-4core(4코어)
- 보드 빌드: build_aarch64

### resnet18 — 통과

```
   OK   cat_285  281  (0.478113, 91.261ms)
   OK   dog_207  207  (0.977876, 91.246ms)
   OK   zebra_340  340  (0.969942, 91.218ms)
```

### squeezenet — 실패 (알려진 실패)

```
   FAIL cat_285  기대 281  실제 905  (0.072325, 173.092ms)
   FAIL dog_207  기대 205  실제 904  (0.222572, 173.562ms)
   FAIL zebra_340  기대 340  실제 533  (0.095507, 172.371ms)
```

### resnet18-4core — 통과

```
   실행 중 최저 여유 74584 kB
   사용량 178744 kB  (코어당 약 44686 kB)
   총 4231.68ms / 100장
   장당 42.3ms (벽시계 기준)
   처리량 23.63 img/s
```

