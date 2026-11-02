# multievta 결과

- 시각: 2026-11-02 08:09 UTC
- 커밋: 3129211d
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41410939087

단일 코어 기준값은 results/latest.md 의 장당 시간과 비교한다.

### resnet18-2core — 통과

```
   코어 1  mxnet_exported_resnet18_1.cpp  (마스크 2)
-- CMA 현황 --
CmaTotal:         262144 kB
CmaFree:          175024 kB
   총 9074.2ms / 100장
   장당 90.7ms (벽시계 기준)
   처리량 11.02 img/s
```

### resnet18-4core — 실패

```
   코어 1  mxnet_exported_resnet18_1.cpp  (마스크 2)
   코어 2  mxnet_exported_resnet18_2.cpp  (마스크 4)
   코어 3  mxnet_exported_resnet18_3.cpp  (마스크 8)
```

