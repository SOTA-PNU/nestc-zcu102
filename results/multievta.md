# multievta 결과

- 시각: 2026-11-03 09:25 UTC
- 커밋: fd05743c
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41610446765

단일 코어 기준값은 results/latest.md 의 장당 시간과 비교한다.

### resnet18-2core — 통과

```
   코어 1  mxnet_exported_resnet18_1.cpp  (마스크 2)
-- CMA 현황 --
CmaTotal:         262144 kB
CmaFree:          253332 kB
   실행 중 최저 여유 163268 kB
   사용량 90064 kB  (코어당 약 45032 kB)
   총 5530.67ms / 100장
   장당 55.3ms (벽시계 기준)
   처리량 18.08 img/s
```

### resnet18-4core — 통과

```
   코어 1  mxnet_exported_resnet18_1.cpp  (마스크 2)
   코어 2  mxnet_exported_resnet18_2.cpp  (마스크 4)
   코어 3  mxnet_exported_resnet18_3.cpp  (마스크 8)
-- CMA 현황 --
CmaTotal:         262144 kB
CmaFree:          253332 kB
   실행 중 최저 여유 73632 kB
   사용량 179700 kB  (코어당 약 44925 kB)
   총 4157.06ms / 100장
   장당 41.6ms (벽시계 기준)
   처리량 24.06 img/s
```

