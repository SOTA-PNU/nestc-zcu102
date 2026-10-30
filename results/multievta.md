# multievta 결과

- 시각: 2026-10-30 11:19 UTC
- 커밋: 45a5d5be
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41047066012

단일 코어 기준값은 results/latest.md 의 장당 시간과 비교한다.

### resnet18-2core — 실패

```
   코어 1  mxnet_exported_resnet18_1.cpp  (마스크 2)
-- CMA 현황 --
CmaTotal:         262144 kB
CmaFree:           17392 kB
```

### resnet18-4core — 실패

```
   코어 1  mxnet_exported_resnet18_1.cpp  (마스크 2)
   코어 2  mxnet_exported_resnet18_2.cpp  (마스크 4)
   코어 3  mxnet_exported_resnet18_3.cpp  (마스크 8)
-- CMA 현황 --
CmaTotal:         262144 kB
CmaFree:           17392 kB
```

