# CPU 단독 벤치마크 결과

- 시각: 2026-11-05 01:58 UTC
- 커밋: 30a9dc7a
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41950655199
- 보드 CPU: ARM Cortex-A53 x4 (ZCU102 PS), VTA 미사용

```
================ resnet18 ================
---------- resnet18 fp32 ----------
RESULT ../cat_285.raw 281 11.412699
RESULT ../dog_207.raw 207 12.708207
RESULT ../zebra_340.raw 340 12.142599
TOTAL_MS 75175.4 images=40 threads=1
LATENCY_MS 1879.4
THROUGHPUT 0.53 threads=1
TOTAL_MS 43229.1 images=80 threads=4
THROUGHPUT 1.85 threads=4
---------- resnet18 int8 ----------
RESULT ../cat_285.raw 644 5.625000
RESULT ../dog_207.raw 644 5.625000
RESULT ../zebra_340.raw 644 5.625000
TOTAL_MS 129533.8 images=40 threads=1
LATENCY_MS 3238.3
THROUGHPUT 0.31 threads=1
TOTAL_MS 64847.6 images=80 threads=4
THROUGHPUT 1.23 threads=4
================ resnet50 ================
---------- resnet50 fp32 ----------
RESULT ../cat_285.raw 285 0.582314
RESULT ../dog_207.raw 207 0.961597
RESULT ../zebra_340.raw 340 0.990179
TOTAL_MS 54992.8 images=12 threads=1
LATENCY_MS 4582.7
THROUGHPUT 0.22 threads=1
TOTAL_MS 31299.1 images=24 threads=4
THROUGHPUT 0.77 threads=4
---------- resnet50 int8 ----------
RESULT ../cat_285.raw 281 0.476562
RESULT ../dog_207.raw 207 0.968750
RESULT ../zebra_340.raw 340 0.992188
TOTAL_MS 92144.6 images=12 threads=1
LATENCY_MS 7678.7
THROUGHPUT 0.13 threads=1
TOTAL_MS 46240.8 images=24 threads=4
THROUGHPUT 0.52 threads=4
== 끝
```
