# model-test 결과

- 시각: 2026-11-03 09:17 UTC
- 커밋: e515d874
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41609535149
- 대상: resnet18 squeezenet  + resnet18-4core(4코어)
- 보드 빌드: build_aarch64

### resnet18 — 통과

```
   OK   cat_285  281  (0.478113, 91.658ms)
   OK   dog_207  207  (0.977876, 91.389ms)
   OK   zebra_340  340  (0.969942, 91.359ms)
```

### squeezenet — 실패 (알려진 실패)

```
   FAIL cat_285  기대 281  실제 905  (0.072325, 172.989ms)
   FAIL dog_207  기대 205  실제 904  (0.222572, 172.567ms)
   FAIL zebra_340  기대 340  실제 533  (0.095507, 172.823ms)
```

### resnet18-4core — 실패

```
== 중단  VTA 를 쓰는 프로세스가 있다. 비트스트림을 바꾸지 않는다.
```

