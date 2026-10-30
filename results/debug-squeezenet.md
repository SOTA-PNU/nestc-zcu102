# debug-dump 결과 — squeezenet

- 시각: 2026-10-30 22:45 UTC
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41140134832

```
--- 분석 ---
파일(실행 순서)                                           원소    최소    최대       평균     0%    음수%    포화%  소견
dump_000_squeezenet0_conv0_fwd__1_quantize_r    150528   -68    84    -2.25    0.9   44.4    0.0  
dump_001_squeezenet0_conv0_fwd__9_res.bin       150528   -68    84    -2.25    0.9   44.4    0.0  
dump_002_squeezenet0_conv0_fwd__2_res.bin       788544     0    39     2.33   51.8    0.0    0.0  
dump_003_squeezenet0_pool0_fwd__1_res.bin       193600     0    39     6.77   10.1    0.0    0.0  
dump_004_squeezenet0_conv1_fwd__2_res.bin        48400     0   118    20.19   20.0    0.0    0.0  
dump_005_squeezenet0_conv2_fwd__2_res.bin       193600   -67    33    -2.77    4.7   64.9    0.0  음수 65%
dump_006_squeezenet0_conv3_fwd__2_res.bin       193600   -58    46    -5.68    4.5   63.7    0.0  음수 64%
dump_007_squeezenet0_concat0__3.bin             387200   -67    46    -4.23    4.6   64.3    0.0  음수 64%
dump_008_squeezenet0_relu2_fwd__2_res.bin       387200     0    92     4.18   68.9    0.0    0.0  
dump_009_squeezenet0_conv4_fwd__2_res.bin        48400     0    70    14.85    7.2    0.0    0.0  
dump_010_squeezenet0_conv5_fwd__7_res.bin       193600   -49    67     4.97    3.3   33.4    0.0  
dump_011_squeezenet0_conv6_fwd__2_res.bin       193600   -87    82    -6.97    2.6   70.8    0.0  음수 71%
dump_012_squeezenet0_concat1__3.bin             387200     0    82     5.35   55.0    0.0    0.0  
dump_013_squeezenet0_pool1_fwd__1_res.bin        93312     0    82    10.58   21.1    0.0    0.0  
dump_014_squeezenet0_conv7_fwd__2_res.bin        23328     0    87    19.66    6.9    0.0    0.0  
dump_015_squeezenet0_conv8_fwd__7_res.bin        93312   -30    38     0.69    6.8   46.8    0.0  음수 47%
dump_016_squeezenet0_conv9_fwd__2_res.bin        93312   -70    49    -2.94    4.8   54.4    0.0  음수 54%
dump_017_squeezenet0_concat2__3.bin             186624   -70    49    -1.13    5.8   50.6    0.0  음수 51%
dump_018_squeezenet0_relu8_fwd__2_res.bin       186624     0    98     5.41   56.4    0.0    0.0  
dump_019_squeezenet0_conv10_fwd__2_res.bin       23328     0    71    14.62    9.3    0.0    0.0  
dump_020_squeezenet0_conv11_fwd__7_res.bin       93312   -43    71     0.63    3.9   49.8    0.0  음수 50%
dump_021_squeezenet0_conv12_fwd__2_res.bin       93312  -105    98   -11.51    1.8   78.0    0.0  음수 78%
dump_022_squeezenet0_concat3__3.bin             186624     0    98     3.66   66.8    0.0    0.0  
dump_023_squeezenet0_pool2_fwd__1_res.bin        43264     0    98     6.33   46.8    0.0    0.0  
dump_024_squeezenet0_conv13_fwd__2_res.bin        8112     0    72    13.10   16.7    0.0    0.0  
dump_025_squeezenet0_conv14_fwd__7_res.bin       32448   -40    35    -1.38    5.9   54.6    0.0  음수 55%
dump_026_squeezenet0_conv15_fwd__2_res.bin       32448  -128    63    -9.68    2.8   69.5    0.3  음수 69%
dump_027_squeezenet0_concat4__3.bin              64896     0    63     2.44   66.3    0.0    0.0  
dump_028_squeezenet0_conv16_fwd__2_res.bin        8112     0    93    11.58   20.2    0.0    0.0  
dump_029_squeezenet0_conv17_fwd__7_res.bin       32448   -30    33    -1.56    5.8   54.9    0.0  음수 55%
dump_030_squeezenet0_conv18_fwd__2_res.bin       32448   -81    61    -7.98    2.5   67.1    0.0  음수 67%
dump_031_squeezenet0_concat5__3.bin              64896     0    61     2.78   65.1    0.0    0.0  
dump_032_squeezenet0_conv19_fwd__2_res.bin       10816     0   100     7.75   37.6    0.0    0.0  
dump_033_squeezenet0_conv20_fwd__7_res.bin       43264   -38    29    -4.35    5.5   70.7    0.0  음수 71%
dump_034_squeezenet0_conv21_fwd__2_res.bin       43264  -105    38   -16.89    1.8   86.0    0.0  음수 86%
dump_035_squeezenet0_concat6__3.bin              86528     0    38     0.97   82.0    0.0    0.0  
dump_036_squeezenet0_conv22_fwd__2_res.bin       10816     0    47     3.36   42.3    0.0    0.0  
dump_037_squeezenet0_conv23_fwd__7_res.bin       43264   -16     8    -2.66    8.8   86.1    0.0  음수 86%
dump_038_squeezenet0_conv24_fwd__2_res.bin       43264   -20     9    -5.57    3.0   94.0    0.0  음수 94%
dump_039_squeezenet0_concat7__3.bin              86528   -20     9    -4.11    5.9   90.1    0.0  음수 90%
dump_040_squeezenet0_relu23_fwd__2_res.bin       86528     0    18     0.13   96.0    0.0    0.0  
dump_041_squeezenet0_conv25_fwd__2_res.bin      169000     0    31     1.47   50.8    0.0    0.0  
dump_042_squeezenet0_pool3_fwd__1_res.bin         1000     0    28     5.90    2.0    0.0    0.0  
dump_043_squeezenet0_conv0_fwd__1_quantize_r    150528   -68    84    -2.25    0.9   44.4    0.0  
dump_044_squeezenet0_conv0_fwd__9_res.bin       150528   -68    84    -2.25    0.9   44.4    0.0  
dump_045_squeezenet0_conv0_fwd__2_res.bin       788544     0    39     2.33   51.8    0.0    0.0  
dump_046_squeezenet0_pool0_fwd__1_res.bin       193600     0    39     6.77   10.1    0.0    0.0  
dump_047_squeezenet0_conv1_fwd__2_res.bin        48400     0   118    20.19   20.0    0.0    0.0  
dump_048_squeezenet0_conv2_fwd__2_res.bin       193600   -67    33    -2.77    4.7   64.9    0.0  음수 65%
dump_049_squeezenet0_conv3_fwd__2_res.bin       193600   -58    46    -5.68    4.5   63.7    0.0  음수 64%
dump_050_squeezenet0_concat0__3.bin             387200   -67    46    -4.23    4.6   64.3    0.0  음수 64%
dump_051_squeezenet0_relu2_fwd__2_res.bin       387200     0    92     4.18   68.9    0.0    0.0  
dump_052_squeezenet0_conv4_fwd__2_res.bin        48400     0    70    14.85    7.2    0.0    0.0  
dump_053_squeezenet0_conv5_fwd__7_res.bin       193600   -49    67     4.97    3.3   33.4    0.0  
dump_054_squeezenet0_conv6_fwd__2_res.bin       193600   -87    82    -6.97    2.6   70.8    0.0  음수 71%
dump_055_squeezenet0_concat1__3.bin             387200     0    82     5.35   55.0    0.0    0.0  
dump_056_squeezenet0_pool1_fwd__1_res.bin        93312     0    82    10.58   21.1    0.0    0.0  
dump_057_squeezenet0_conv7_fwd__2_res.bin        23328     0    87    19.66    6.9    0.0    0.0  
dump_058_squeezenet0_conv8_fwd__7_res.bin        93312   -30    38     0.69    6.8   46.8    0.0  음수 47%
dump_059_squeezenet0_conv9_fwd__2_res.bin        93312   -70    49    -2.94    4.8   54.4    0.0  음수 54%
dump_060_squeezenet0_concat2__3.bin             186624   -70    49    -1.13    5.8   50.6    0.0  음수 51%
dump_061_squeezenet0_relu8_fwd__2_res.bin       186624     0    98     5.41   56.4    0.0    0.0  
dump_062_squeezenet0_conv10_fwd__2_res.bin       23328     0    71    14.62    9.3    0.0    0.0  
dump_063_squeezenet0_conv11_fwd__7_res.bin       93312   -43    71     0.63    3.9   49.8    0.0  음수 50%
dump_064_squeezenet0_conv12_fwd__2_res.bin       93312  -105    98   -11.51    1.8   78.0    0.0  음수 78%
dump_065_squeezenet0_concat3__3.bin             186624     0    98     3.66   66.8    0.0    0.0  
dump_066_squeezenet0_pool2_fwd__1_res.bin        43264     0    98     6.33   46.8    0.0    0.0  
dump_067_squeezenet0_conv13_fwd__2_res.bin        8112     0    72    13.10   16.7    0.0    0.0  
dump_068_squeezenet0_conv14_fwd__7_res.bin       32448   -40    35    -1.38    5.9   54.6    0.0  음수 55%
dump_069_squeezenet0_conv15_fwd__2_res.bin       32448  -128    63    -9.68    2.8   69.5    0.3  음수 69%
dump_070_squeezenet0_concat4__3.bin              64896     0    63     2.44   66.3    0.0    0.0  
dump_071_squeezenet0_conv16_fwd__2_res.bin        8112     0    93    11.58   20.2    0.0    0.0  
dump_072_squeezenet0_conv17_fwd__7_res.bin       32448   -30    33    -1.56    5.8   54.9    0.0  음수 55%
dump_073_squeezenet0_conv18_fwd__2_res.bin       32448   -81    61    -7.98    2.5   67.1    0.0  음수 67%
dump_074_squeezenet0_concat5__3.bin              64896     0    61     2.78   65.1    0.0    0.0  
dump_075_squeezenet0_conv19_fwd__2_res.bin       10816     0   100     7.75   37.6    0.0    0.0  
dump_076_squeezenet0_conv20_fwd__7_res.bin       43264   -38    29    -4.35    5.5   70.7    0.0  음수 71%
dump_077_squeezenet0_conv21_fwd__2_res.bin       43264  -105    38   -16.89    1.8   86.0    0.0  음수 86%
dump_078_squeezenet0_concat6__3.bin              86528     0    38     0.97   82.0    0.0    0.0  
dump_079_squeezenet0_conv22_fwd__2_res.bin       10816     0    47     3.36   42.3    0.0    0.0  
dump_080_squeezenet0_conv23_fwd__7_res.bin       43264   -16     8    -2.66    8.8   86.1    0.0  음수 86%
dump_081_squeezenet0_conv24_fwd__2_res.bin       43264   -20     9    -5.57    3.0   94.0    0.0  음수 94%
dump_082_squeezenet0_concat7__3.bin              86528   -20     9    -4.11    5.9   90.1    0.0  음수 90%
dump_083_squeezenet0_relu23_fwd__2_res.bin       86528     0    18     0.13   96.0    0.0    0.0  
dump_084_squeezenet0_conv25_fwd__2_res.bin      169000     0    31     1.47   50.8    0.0    0.0  
dump_085_squeezenet0_pool3_fwd__1_res.bin         1000     0    28     5.90    2.0    0.0    0.0  

처음 이상해지는 지점: dump_005_squeezenet0_conv2_fwd__2_res.bin  (음수 65%)
그 층의 연산 하나만 보면 된다. 그 앞은 정상이라는 뜻이다.
```
