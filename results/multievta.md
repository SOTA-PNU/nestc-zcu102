# multievta 결과

- 시각: 2026-11-04 10:42 UTC
- 커밋: c65d83a3
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41815522975

단일 코어 기준값은 results/latest.md 의 장당 시간과 비교한다.

### resnet18-2core — 통과

```
   코어 1  mxnet_exported_resnet18_1.cpp  (마스크 2)
-- CMA 현황 --
CmaTotal:         262144 kB
CmaFree:          253244 kB
   실행 중 최저 여유 163912 kB
   사용량 89332 kB  (코어당 약 44666 kB)
   총 5480.71ms / 100장
   장당 54.8ms (벽시계 기준)
   처리량 18.25 img/s
```

### resnet18-4core — 통과

```
   코어 1  mxnet_exported_resnet18_1.cpp  (마스크 2)
   코어 2  mxnet_exported_resnet18_2.cpp  (마스크 4)
   코어 3  mxnet_exported_resnet18_3.cpp  (마스크 8)
-- CMA 현황 --
CmaTotal:         262144 kB
CmaFree:          253292 kB
   실행 중 최저 여유 74192 kB
   사용량 179100 kB  (코어당 약 44775 kB)
   총 4130.47ms / 100장
   장당 41.3ms (벽시계 기준)
   처리량 24.21 img/s
```

### resnet50-3core — 통과

```
   코어 1  resnet50_1.cpp  (마스크 2)
   코어 2  resnet50_2.cpp  (마스크 4)
-- CMA 현황 --
CmaTotal:         262144 kB
CmaFree:          253300 kB
   실행 중 최저 여유 78716 kB
   사용량 174584 kB  (코어당 약 58194 kB)
   총 17869.8ms / 100장
   장당 178.7ms (벽시계 기준)
   처리량 5.60 img/s
```

### resnet50-4core — 통과

```
   코어 1  resnet50_1.cpp  (마스크 2)
   코어 2  resnet50_2.cpp  (마스크 4)
   코어 3  resnet50_3.cpp  (마스크 8)
-- CMA 현황 --
CmaTotal:         262144 kB
CmaFree:          253268 kB
   실행 중 최저 여유 19340 kB
   사용량 233928 kB  (코어당 약 58482 kB)
   총 15073ms / 100장
   장당 150.7ms (벽시계 기준)
   처리량 6.63 img/s
```

