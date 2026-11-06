# multievta 결과

- 시각: 2026-11-06 05:14 UTC
- 커밋: a95534ad
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/42167062768

단일 코어 기준값은 results/latest.md 의 장당 시간과 비교한다.

### resnet18-2core — 통과

```
   코어 1  mxnet_exported_resnet18_1.cpp  (마스크 2)
-- CMA 현황 --
CmaTotal:         262144 kB
CmaFree:          162528 kB
   실행 중 최저 여유 34912 kB
   사용량 127492 kB  (코어당 약 63746 kB)
   총 5588.14ms / 100장
   장당 55.9ms (벽시계 기준)
   처리량 17.90 img/s
```

### resnet18-4core — 실패

```
   코어 1  mxnet_exported_resnet18_1.cpp  (마스크 2)
   코어 2  mxnet_exported_resnet18_2.cpp  (마스크 4)
   코어 3  mxnet_exported_resnet18_3.cpp  (마스크 8)
== 중단  CMA 가 모자란다
```

### resnet50-3core — 실패

```
   코어 1  resnet50_1.cpp  (마스크 2)
   코어 2  resnet50_2.cpp  (마스크 4)
== 중단  CMA 가 모자란다
```

### resnet50-4core — 실패

```
   코어 1  resnet50_1.cpp  (마스크 2)
   코어 2  resnet50_2.cpp  (마스크 4)
   코어 3  resnet50_3.cpp  (마스크 8)
== 중단  CMA 가 모자란다
```

