# multievta 결과

- 시각: 2026-11-06 08:13 UTC
- 커밋: 79b72e83
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/42185796710

단일 코어 기준값은 results/latest.md 의 장당 시간과 비교한다.

### resnet18-2core — 통과

```
   코어 1  mxnet_exported_resnet18_1.cpp  (마스크 2)
-- CMA 현황 --
CmaTotal:         262144 kB
CmaFree:          253316 kB
   실행 중 최저 여유 164020 kB
   사용량 89296 kB  (코어당 약 44648 kB)
   총 5588.28ms / 100장
   장당 55.9ms (벽시계 기준)
   처리량 17.89 img/s
```

### resnet18-4core — 통과

```
   코어 1  mxnet_exported_resnet18_1.cpp  (마스크 2)
   코어 2  mxnet_exported_resnet18_2.cpp  (마스크 4)
   코어 3  mxnet_exported_resnet18_3.cpp  (마스크 8)
-- CMA 현황 --
CmaTotal:         262144 kB
CmaFree:          253324 kB
   실행 중 최저 여유 74856 kB
   사용량 178468 kB  (코어당 약 44617 kB)
   총 4167.18ms / 100장
   장당 41.7ms (벽시계 기준)
   처리량 24.00 img/s
```

### resnet50-3core — 통과

```
   코어 1  resnet50_1.cpp  (마스크 2)
   코어 2  resnet50_2.cpp  (마스크 4)
-- CMA 현황 --
CmaTotal:         262144 kB
CmaFree:          253324 kB
   실행 중 최저 여유 77396 kB
   사용량 175928 kB  (코어당 약 58642 kB)
   총 17897.1ms / 100장
   장당 179.0ms (벽시계 기준)
   처리량 5.59 img/s
```

### resnet50-4core — 통과

```
   코어 1  resnet50_1.cpp  (마스크 2)
   코어 2  resnet50_2.cpp  (마스크 4)
   코어 3  resnet50_3.cpp  (마스크 8)
-- CMA 현황 --
CmaTotal:         262144 kB
CmaFree:          253196 kB
   실행 중 최저 여유 15668 kB
   사용량 237528 kB  (코어당 약 59382 kB)
   총 14903.7ms / 100장
   장당 149.0ms (벽시계 기준)
   처리량 6.71 img/s
```

