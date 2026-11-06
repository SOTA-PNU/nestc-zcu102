# CPU 단독 벤치마크 결과

- 시각: 2026-11-06 04:07 UTC
- 커밋: 01eb9cab
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/42159094932
- 보드 CPU: ARM Cortex-A53 x4 (ZCU102 PS), VTA 미사용

```
================ resnet18 ================
---------- resnet18 fp32 ----------
RESULT ../cat_285.raw 281 11.412699
RESULT ../dog_207.raw 207 12.708207
RESULT ../zebra_340.raw 340 12.142599
TOTAL_MS 75148.6 images=40 threads=1
LATENCY_MS 1878.7
THROUGHPUT 0.53 threads=1
TOTAL_MS 43235.5 images=80 threads=4
THROUGHPUT 1.85 threads=4
---------- resnet18 int8 ----------
RESULT ../cat_285.raw 644 5.625000
RESULT ../dog_207.raw 644 5.625000
RESULT ../zebra_340.raw 644 5.625000
TOTAL_MS 129515.4 images=40 threads=1
LATENCY_MS 3237.9
THROUGHPUT 0.31 threads=1
TOTAL_MS 64847.1 images=80 threads=4
THROUGHPUT 1.23 threads=4
================ resnet50 ================
---------- resnet50 fp32 ----------
RESULT ../cat_285.raw 285 0.582314
RESULT ../dog_207.raw 207 0.961597
RESULT ../zebra_340.raw 340 0.990179
TOTAL_MS 54916.9 images=12 threads=1
LATENCY_MS 4576.4
THROUGHPUT 0.22 threads=1
TOTAL_MS 31333.1 images=24 threads=4
THROUGHPUT 0.77 threads=4
---------- resnet50 int8 ----------
RESULT ../cat_285.raw 281 0.476562
RESULT ../dog_207.raw 207 0.968750
RESULT ../zebra_340.raw 340 0.992188
TOTAL_MS 92148.2 images=12 threads=1
LATENCY_MS 7679.0
THROUGHPUT 0.13 threads=1
TOTAL_MS 46239.7 images=24 threads=4
THROUGHPUT 0.52 threads=4
== 끝
```
