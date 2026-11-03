# multievta 결과

- 시각: 2026-11-03 08:42 UTC
- 커밋: ff6257a2
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41605026990

단일 코어 기준값은 results/latest.md 의 장당 시간과 비교한다.

### resnet18-2core — 통과

```
   코어 1  mxnet_exported_resnet18_1.cpp  (마스크 2)
-- CMA 현황 --
CmaTotal:         262144 kB
CmaFree:          253328 kB
   총 8881.53ms / 100장
   장당 88.8ms (벽시계 기준)
   처리량 11.26 img/s
```

### resnet18-4core — 통과

```
   코어 1  mxnet_exported_resnet18_1.cpp  (마스크 2)
   코어 2  mxnet_exported_resnet18_2.cpp  (마스크 4)
   코어 3  mxnet_exported_resnet18_3.cpp  (마스크 8)
-- CMA 현황 --
CmaTotal:         262144 kB
CmaFree:          253332 kB
   총 9385.95ms / 100장
   장당 93.9ms (벽시계 기준)
   처리량 10.65 img/s
```

