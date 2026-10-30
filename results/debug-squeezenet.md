# debug-dump 결과 — squeezenet

- 시각: 2026-10-30 23:25 UTC
- 실행: https://github.com/SOTA-PNU/nestc-zcu102/actions/runs/41144301058

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

## 보정 범위 (calib.yaml)

덤프의 int8 최대값에 그 층의 스케일을 곱한 값이 아래 범위와
크게 다르면 그 층에서 크기가 어긋난 것이다.
```
---
GlowToolsVersion: '2026-10-07 (3951ea4) ()'
...
---
GraphPreLowerHash: 0x5C990577383E8635
...
---
- NodeOutputName:  'data:0'
  Min:             -2.1179039478302001953125
  Max:             2.6400001049041748046875
  Histogram:       [ 3141, 4009, 6343, 9244, 10423, 10833, 9971, 7609, 
                     6809, 82146 ]
- NodeOutputName:  'squeezenet0_concat0__2:0'
  Min:             -35.978801727294921875
  Max:             29.427059173583984375
  Histogram:       [ 18, 83, 777, 5910, 44622, 307630, 27421, 705, 26, 
                     8 ]
- NodeOutputName:  'squeezenet0_concat1__2:0'
  Min:             -58.33453369140625
  Max:             44.924282073974609375
  Histogram:       [ 44, 194, 832, 4570, 32148, 282702, 63316, 3042, 299, 
                     53 ]
- NodeOutputName:  'squeezenet0_concat2__2:0'
  Min:             -169.6217498779296875
  Max:             123.49857330322265625
  Histogram:       [ 18, 25, 200, 1815, 13608, 123434, 46203, 1192, 114, 
                     15 ]
- NodeOutputName:  'squeezenet0_concat3__2:0'
  Min:             -197.8346710205078125
  Max:             137.153839111328125
  Histogram:       [ 12, 93, 668, 3868, 21599, 95979, 58005, 5723, 626, 
                     51 ]
- NodeOutputName:  'squeezenet0_concat4__2:0'
  Min:             -431.69757080078125
  Max:             333.8541259765625
  Histogram:       [ 5, 33, 249, 1448, 9020, 43789, 9876, 440, 35, 1 ]
- NodeOutputName:  'squeezenet0_concat5__2:0'
  Min:             -354.268341064453125
  Max:             383.43292236328125
  Histogram:       [ 53, 391, 2344, 9449, 34746, 16432, 1343, 127, 8, 3 ]
- NodeOutputName:  'squeezenet0_concat6__2:0'
  Min:             -435.06292724609375
  Max:             417.680694580078125
  Histogram:       [ 13, 193, 1351, 8769, 42655, 31563, 1865, 100, 16, 
                     3 ]
- NodeOutputName:  'squeezenet0_concat7__2:0'
  Min:             -650.97674560546875
  Max:             261.357208251953125
  Histogram:       [ 4, 15, 76, 401, 2265, 13917, 55430, 14028, 362, 30 ]
- NodeOutputName:  'squeezenet0_conv0_bias:0'
  Min:             -1.6891486644744873046875
  Max:             1.44970715045928955078125
  Histogram:       [ 1, 0, 1, 7, 5, 31, 9, 8, 0, 2 ]
- NodeOutputName:  'squeezenet0_conv0_fwd__1:0'
  Min:             -2.1179039478302001953125
  Max:             2.6400001049041748046875
  Histogram:       [ 3141, 4009, 6343, 9244, 10423, 10833, 9971, 7609, 
                     6809, 82146 ]
- NodeOutputName:  'squeezenet0_conv0_fwd__2:0'
  Min:             -14.6664676666259765625
  Max:             18.968265533447265625
  Histogram:       [ 29, 166, 1152, 77209, 680270, 28913, 701, 74, 14, 
                     16 ]
- NodeOutputName:  'squeezenet0_conv0_weight__1:0'
  Min:             -0.849246501922607421875
  Max:             0.74099910259246826171875
  Histogram:       [ 5, 41, 78, 186, 373, 508, 313, 127, 72, 25 ]
- NodeOutputName:  'squeezenet0_conv10_bias:0'
  Min:             -0.2137905061244964599609375
  Max:             0.31318199634552001953125
  Histogram:       [ 2, 2, 2, 5, 3, 7, 5, 3, 2, 1 ]
- NodeOutputName:  'squeezenet0_conv10_fwd__1:0'
  Min:             -80.43129730224609375
  Max:             157.422698974609375
  Histogram:       [ 3, 76, 945, 8754, 9539, 3329, 580, 83, 15, 4 ]
- NodeOutputName:  'squeezenet0_conv10_fwd__1_bias:0'
  Min:             -80.43129730224609375
  Max:             157.422698974609375
  Histogram:       [ 3, 76, 945, 8754, 9539, 3329, 580, 83, 15, 4 ]
- NodeOutputName:  'squeezenet0_conv10_fwd__1_dot:0'
  Min:             -80.38822174072265625
  Max:             157.48675537109375
  Histogram:       [ 3, 75, 957, 8772, 9537, 3309, 573, 83, 15, 4 ]
- NodeOutputName:  'squeezenet0_conv10_fwd__2:0'
  Min:             -80.43129730224609375
  Max:             157.422698974609375
  Histogram:       [ 3, 76, 945, 8754, 9539, 3329, 580, 83, 15, 4 ]
- NodeOutputName:  'squeezenet0_conv10_fwd__2_ReshapeInput:0'
  Min:             0.0
  Max:             123.49857330322265625
  Histogram:       [ 165679, 16326, 3488, 751, 219, 92, 42, 17, 9, 1 ]
- NodeOutputName:  'squeezenet0_conv10_fwd__2_ReshapeOutput:0'
  Min:             -80.43129730224609375
  Max:             157.422698974609375
  Histogram:       [ 3, 76, 945, 8754, 9539, 3329, 580, 83, 15, 4 ]
- NodeOutputName:  'squeezenet0_conv10_fwd__2_ReshapeWeights:0'
  Min:             -0.496030330657958984375
  Max:             0.712246835231781005859375
  Histogram:       [ 21, 85, 616, 2674, 3361, 1157, 219, 43, 11, 5 ]
- NodeOutputName:  'squeezenet0_conv10_fwd__2_TransposeWeights:0'
  Min:             -0.496030330657958984375
  Max:             0.712246835231781005859375
  Histogram:       [ 21, 85, 616, 2674, 3361, 1157, 219, 43, 11, 5 ]
- NodeOutputName:  'squeezenet0_conv10_weight__1:0'
  Min:             -0.496030330657958984375
  Max:             0.712246835231781005859375
  Histogram:       [ 21, 85, 616, 2674, 3361, 1157, 219, 43, 11, 5 ]
- NodeOutputName:  'squeezenet0_conv11_bias:0'
  Min:             -0.067396223545074462890625
  Max:             0.115951128304004669189453125
  Histogram:       [ 5, 10, 16, 27, 30, 22, 11, 3, 1, 3 ]
- NodeOutputName:  'squeezenet0_conv11_fwd__1:0'
  Min:             -77.59148406982421875
  Max:             94.623260498046875
  Histogram:       [ 12, 186, 1877, 16272, 44973, 21584, 6467, 1479, 415, 
                     47 ]
- NodeOutputName:  'squeezenet0_conv11_fwd__1_bias:0'
  Min:             -77.59148406982421875
  Max:             94.623260498046875
  Histogram:       [ 12, 186, 1877, 16272, 44973, 21584, 6467, 1479, 415, 
                     47 ]
- NodeOutputName:  'squeezenet0_conv11_fwd__1_dot:0'
  Min:             -77.6210784912109375
  Max:             94.51004791259765625
  Histogram:       [ 12, 184, 1855, 16193, 44937, 21680, 6506, 1481, 418, 
                     46 ]
- NodeOutputName:  'squeezenet0_conv11_fwd__2:0'
  Min:             -77.59148406982421875
  Max:             94.623260498046875
  Histogram:       [ 12, 186, 1877, 16272, 44973, 21584, 6467, 1479, 415, 
                     47 ]
- NodeOutputName:  'squeezenet0_conv11_fwd__2_ReshapeInput:0'
  Min:             0.0
  Max:             157.422698974609375
  Histogram:       [ 10281, 6833, 3958, 1606, 458, 142, 31, 12, 5, 2 ]
- NodeOutputName:  'squeezenet0_conv11_fwd__2_ReshapeOutput:0'
  Min:             -77.59148406982421875
  Max:             94.623260498046875
  Histogram:       [ 12, 186, 1877, 16272, 44973, 21584, 6467, 1479, 415, 
                     47 ]
- NodeOutputName:  'squeezenet0_conv11_fwd__2_ReshapeWeights:0'
  Min:             -0.4409655034542083740234375
  Max:             0.8508021831512451171875
  Histogram:       [ 17, 176, 1090, 1820, 742, 187, 55, 8, 0, 1 ]
- NodeOutputName:  'squeezenet0_conv11_fwd__2_TransposeWeights:0'
  Min:             -0.4409655034542083740234375
  Max:             0.8508021831512451171875
  Histogram:       [ 17, 176, 1090, 1820, 742, 187, 55, 8, 0, 1 ]
- NodeOutputName:  'squeezenet0_conv11_weight__1:0'
  Min:             -0.4409655034542083740234375
  Max:             0.8508021831512451171875
  Histogram:       [ 17, 176, 1090, 1820, 742, 187, 55, 8, 0, 1 ]
- NodeOutputName:  'squeezenet0_conv12_bias:0'
  Min:             -0.0443533100187778472900390625
  Max:             0.041721858084201812744140625
  Histogram:       [ 3, 3, 4, 13, 25, 29, 34, 10, 4, 3 ]
- NodeOutputName:  'squeezenet0_conv12_fwd__2:0'
  Min:             -197.8346710205078125
  Max:             137.153839111328125
  Histogram:       [ 12, 93, 668, 3864, 20471, 46570, 18517, 2589, 477, 
                     51 ]
- NodeOutputName:  'squeezenet0_conv12_weight__1:0'
  Min:             -0.4525029361248016357421875
  Max:             0.535114705562591552734375
  Histogram:       [ 3, 20, 282, 5293, 24065, 6538, 575, 72, 13, 3 ]
- NodeOutputName:  'squeezenet0_conv13_bias:0'
  Min:             -0.0725610554218292236328125
  Max:             0.105924345552921295166015625
  Histogram:       [ 3, 4, 4, 8, 7, 6, 6, 1, 6, 3 ]
- NodeOutputName:  'squeezenet0_conv13_fwd__1:0'
  Min:             -125.53313446044921875
  Max:             303.79638671875
  Histogram:       [ 4, 36, 818, 3370, 2416, 1151, 251, 43, 16, 7 ]
- NodeOutputName:  'squeezenet0_conv13_fwd__1_bias:0'
  Min:             -125.53313446044921875
  Max:             303.79638671875
  Histogram:       [ 4, 36, 818, 3370, 2416, 1151, 251, 43, 16, 7 ]
- NodeOutputName:  'squeezenet0_conv13_fwd__1_dot:0'
  Min:             -125.6237640380859375
  Max:             303.86895751953125
  Histogram:       [ 4, 36, 818, 3369, 2418, 1149, 253, 42, 16, 7 ]
- NodeOutputName:  'squeezenet0_conv13_fwd__2:0'
  Min:             -125.53313446044921875
  Max:             303.79638671875
  Histogram:       [ 4, 36, 818, 3370, 2416, 1151, 251, 43, 16, 7 ]
- NodeOutputName:  'squeezenet0_conv13_fwd__2_ReshapeInput:0'
  Min:             0.0
  Max:             137.153839111328125
  Histogram:       [ 24432, 9701, 5624, 2144, 766, 354, 149, 60, 21, 13 ]
- NodeOutputName:  'squeezenet0_conv13_fwd__2_ReshapeOutput:0'
  Min:             -125.53313446044921875
  Max:             303.79638671875
  Histogram:       [ 4, 36, 818, 3370, 2416, 1151, 251, 43, 16, 7 ]
- NodeOutputName:  'squeezenet0_conv13_fwd__2_ReshapeWeights:0'
  Min:             -0.497271001338958740234375
  Max:             0.692627847194671630859375
  Histogram:       [ 8, 68, 733, 3941, 5367, 1745, 340, 67, 17, 2 ]
- NodeOutputName:  'squeezenet0_conv13_fwd__2_TransposeWeights:0'
  Min:             -0.497271001338958740234375
  Max:             0.692627847194671630859375
  Histogram:       [ 8, 68, 733, 3941, 5367, 1745, 340, 67, 17, 2 ]
- NodeOutputName:  'squeezenet0_conv13_weight__1:0'
  Min:             -0.497271001338958740234375
  Max:             0.692627847194671630859375
  Histogram:       [ 8, 68, 733, 3941, 5367, 1745, 340, 67, 17, 2 ]
- NodeOutputName:  'squeezenet0_conv14_bias:0'
  Min:             -0.02403825707733631134033203125
  Max:             0.0396986193954944610595703125
  Histogram:       [ 4, 10, 23, 45, 30, 26, 15, 18, 15, 6 ]
- NodeOutputName:  'squeezenet0_conv14_fwd__1:0'
  Min:             -158.98944091796875
  Max:             187.8151702880859375
  Histogram:       [ 9, 45, 609, 5512, 16608, 8450, 1120, 92, 1, 2 ]
- NodeOutputName:  'squeezenet0_conv14_fwd__1_bias:0'
  Min:             -158.98944091796875
  Max:             187.8151702880859375
  Histogram:       [ 9, 45, 609, 5512, 16608, 8450, 1120, 92, 1, 2 ]
- NodeOutputName:  'squeezenet0_conv14_fwd__1_dot:0'
  Min:             -159.01983642578125
  Max:             187.8188018798828125
  Histogram:       [ 9, 45, 609, 5507, 16607, 8457, 1119, 92, 1, 2 ]
- NodeOutputName:  'squeezenet0_conv14_fwd__2:0'
  Min:             -158.98944091796875
  Max:             187.8151702880859375
  Histogram:       [ 9, 45, 609, 5512, 16608, 8450, 1120, 92, 1, 2 ]
- NodeOutputName:  'squeezenet0_conv14_fwd__2_ReshapeInput:0'
  Min:             0.0
  Max:             303.79638671875
  Histogram:       [ 3011, 2212, 1509, 874, 374, 78, 27, 17, 6, 4 ]
- NodeOutputName:  'squeezenet0_conv14_fwd__2_ReshapeOutput:0'
  Min:             -158.98944091796875
  Max:             187.8151702880859375
  Histogram:       [ 9, 45, 609, 5512, 16608, 8450, 1120, 92, 1, 2 ]
- NodeOutputName:  'squeezenet0_conv14_fwd__2_ReshapeWeights:0'
  Min:             -0.4363412559032440185546875
  Max:             0.51821506023406982421875
  Histogram:       [ 4, 26, 324, 1897, 4220, 2233, 448, 52, 8, 4 ]
- NodeOutputName:  'squeezenet0_conv14_fwd__2_TransposeWeights:0'
  Min:             -0.4363412559032440185546875
  Max:             0.51821506023406982421875
  Histogram:       [ 4, 26, 324, 1897, 4220, 2233, 448, 52, 8, 4 ]
- NodeOutputName:  'squeezenet0_conv14_weight__1:0'
  Min:             -0.4363412559032440185546875
  Max:             0.51821506023406982421875
  Histogram:       [ 4, 26, 324, 1897, 4220, 2233, 448, 52, 8, 4 ]
- NodeOutputName:  'squeezenet0_conv15_bias:0'
  Min:             -0.02380142174661159515380859375
  Max:             0.0313474424183368682861328125
  Histogram:       [ 1, 1, 17, 39, 47, 47, 25, 14, 0, 1 ]
- NodeOutputName:  'squeezenet0_conv15_fwd__2:0'
  Min:             -431.69757080078125
  Max:             333.8541259765625
  Histogram:       [ 5, 33, 249, 1439, 8015, 17292, 4952, 428, 34, 1 ]
- NodeOutputName:  'squeezenet0_conv15_weight__1:0'
  Min:             -0.546131610870361328125
  Max:             0.506356537342071533203125
  Histogram:       [ 1, 8, 53, 685, 26852, 52249, 2901, 163, 19, 13 ]
- NodeOutputName:  'squeezenet0_conv16_bias:0'
  Min:             -0.07147784531116485595703125
  Max:             0.12490414083003997802734375
  Histogram:       [ 5, 3, 5, 6, 7, 9, 7, 3, 1, 2 ]
- NodeOutputName:  'squeezenet0_conv16_fwd__1:0'
  Min:             -131.5975341796875
  Max:             362.967803955078125
  Histogram:       [ 18, 182, 1425, 3049, 2272, 904, 197, 49, 10, 6 ]
- NodeOutputName:  'squeezenet0_conv16_fwd__1_bias:0'
  Min:             -131.5975341796875
  Max:             362.967803955078125
  Histogram:       [ 18, 182, 1425, 3049, 2272, 904, 197, 49, 10, 6 ]
- NodeOutputName:  'squeezenet0_conv16_fwd__1_dot:0'
  Min:             -131.5275726318359375
  Max:             362.926055908203125
  Histogram:       [ 18, 183, 1426, 3047, 2273, 902, 198, 49, 10, 6 ]
- NodeOutputName:  'squeezenet0_conv16_fwd__2:0'
  Min:             -131.5975341796875
  Max:             362.967803955078125
  Histogram:       [ 18, 182, 1425, 3049, 2272, 904, 197, 49, 10, 6 ]
- NodeOutputName:  'squeezenet0_conv16_fwd__2_ReshapeInput:0'
  Min:             0.0
  Max:             333.8541259765625
  Histogram:       [ 56825, 6153, 1371, 374, 108, 48, 11, 5, 0, 1 ]
- NodeOutputName:  'squeezenet0_conv16_fwd__2_ReshapeOutput:0'
  Min:             -131.5975341796875
  Max:             362.967803955078125
  Histogram:       [ 18, 182, 1425, 3049, 2272, 904, 197, 49, 10, 6 ]
- NodeOutputName:  'squeezenet0_conv16_fwd__2_ReshapeWeights:0'
  Min:             -0.57066142559051513671875
  Max:             0.64222049713134765625
  Histogram:       [ 4, 27, 88, 1811, 9930, 5819, 664, 71, 14, 4 ]
- NodeOutputName:  'squeezenet0_conv16_fwd__2_TransposeWeights:0'
  Min:             -0.57066142559051513671875
  Max:             0.64222049713134765625
  Histogram:       [ 4, 27, 88, 1811, 9930, 5819, 664, 71, 14, 4 ]
- NodeOutputName:  'squeezenet0_conv16_weight__1:0'
  Min:             -0.57066142559051513671875
  Max:             0.64222049713134765625
  Histogram:       [ 4, 27, 88, 1811, 9930, 5819, 664, 71, 14, 4 ]
- NodeOutputName:  'squeezenet0_conv17_bias:0'
  Min:             -0.01416126079857349395751953125
  Max:             0.058108888566493988037109375
  Histogram:       [ 22, 38, 43, 40, 21, 14, 9, 2, 2, 1 ]
- NodeOutputName:  'squeezenet0_conv17_fwd__1:0'
  Min:             -224.3120269775390625
  Max:             186.425567626953125
  Histogram:       [ 2, 6, 36, 759, 7455, 16637, 6560, 931, 56, 6 ]
- NodeOutputName:  'squeezenet0_conv17_fwd__1_bias:0'
  Min:             -224.3120269775390625
  Max:             186.425567626953125
  Histogram:       [ 2, 6, 36, 759, 7455, 16637, 6560, 931, 56, 6 ]
- NodeOutputName:  'squeezenet0_conv17_fwd__1_dot:0'
  Min:             -224.3084869384765625
  Max:             186.436248779296875
  Histogram:       [ 2, 6, 37, 759, 7456, 16641, 6555, 931, 55, 6 ]
- NodeOutputName:  'squeezenet0_conv17_fwd__2:0'
  Min:             -224.3120269775390625
  Max:             186.425567626953125
  Histogram:       [ 2, 6, 36, 759, 7455, 16637, 6560, 931, 56, 6 ]
- NodeOutputName:  'squeezenet0_conv17_fwd__2_ReshapeInput:0'
  Min:             0.0
  Max:             362.967803955078125
  Histogram:       [ 2803, 2234, 1693, 852, 367, 105, 38, 10, 5, 5 ]
- NodeOutputName:  'squeezenet0_conv17_fwd__2_ReshapeOutput:0'
  Min:             -224.3120269775390625
  Max:             186.425567626953125
  Histogram:       [ 2, 6, 36, 759, 7455, 16637, 6560, 931, 56, 6 ]
- NodeOutputName:  'squeezenet0_conv17_fwd__2_ReshapeWeights:0'
  Min:             -0.409724771976470947265625
  Max:             0.5295398235321044921875
  Histogram:       [ 4, 59, 481, 2544, 4089, 1672, 319, 43, 3, 2 ]
- NodeOutputName:  'squeezenet0_conv17_fwd__2_TransposeWeights:0'
  Min:             -0.409724771976470947265625
  Max:             0.5295398235321044921875
  Histogram:       [ 4, 59, 481, 2544, 4089, 1672, 319, 43, 3, 2 ]
- NodeOutputName:  'squeezenet0_conv17_weight__1:0'
  Min:             -0.409724771976470947265625
  Max:             0.5295398235321044921875
  Histogram:       [ 4, 59, 481, 2544, 4089, 1672, 319, 43, 3, 2 ]
- NodeOutputName:  'squeezenet0_conv18_bias:0'
  Min:             -0.01980684138834476470947265625
  Max:             0.02564824558794498443603515625
  Histogram:       [ 2, 10, 28, 36, 29, 42, 28, 11, 4, 2 ]
- NodeOutputName:  'squeezenet0_conv18_fwd__2:0'
  Min:             -354.268341064453125
  Max:             383.43292236328125
  Histogram:       [ 53, 390, 2337, 8608, 13249, 6527, 1149, 124, 8, 3 ]
- NodeOutputName:  'squeezenet0_conv18_weight__1:0'
  Min:             -0.4191139042377471923828125
  Max:             0.3343057334423065185546875
  Histogram:       [ 1, 6, 27, 529, 13354, 52223, 15681, 1039, 74, 10 ]
- NodeOutputName:  'squeezenet0_conv19_bias:0'
  Min:             -0.107101909816265106201171875
  Max:             0.116629727184772491455078125
  Histogram:       [ 1, 0, 5, 9, 9, 17, 12, 9, 1, 1 ]
- NodeOutputName:  'squeezenet0_conv19_fwd__1:0'
  Min:             -190.71246337890625
  Max:             412.698516845703125
  Histogram:       [ 35, 449, 2172, 4262, 2824, 830, 194, 39, 6, 5 ]
- NodeOutputName:  'squeezenet0_conv19_fwd__1_bias:0'
  Min:             -190.71246337890625
  Max:             412.698516845703125
  Histogram:       [ 35, 449, 2172, 4262, 2824, 830, 194, 39, 6, 5 ]
- NodeOutputName:  'squeezenet0_conv19_fwd__1_dot:0'
  Min:             -190.7575531005859375
  Max:             412.72705078125
  Histogram:       [ 34, 449, 2171, 4266, 2822, 830, 194, 39, 6, 5 ]
- NodeOutputName:  'squeezenet0_conv19_fwd__2:0'
  Min:             -190.71246337890625
  Max:             412.698516845703125
  Histogram:       [ 35, 449, 2172, 4262, 2824, 830, 194, 39, 6, 5 ]
- NodeOutputName:  'squeezenet0_conv19_fwd__2_ReshapeInput:0'
  Min:             0.0
  Max:             383.43292236328125
  Histogram:       [ 56504, 6205, 1579, 415, 131, 45, 13, 1, 0, 3 ]
- NodeOutputName:  'squeezenet0_conv19_fwd__2_ReshapeOutput:0'
  Min:             -190.71246337890625
  Max:             412.698516845703125
  Histogram:       [ 35, 449, 2172, 4262, 2824, 830, 194, 39, 6, 5 ]
- NodeOutputName:  'squeezenet0_conv19_fwd__2_ReshapeWeights:0'
  Min:             -0.555584013462066650390625
  Max:             0.475987732410430908203125
  Histogram:       [ 1, 1, 18, 455, 6099, 13252, 4163, 512, 64, 11 ]
- NodeOutputName:  'squeezenet0_conv19_fwd__2_TransposeWeights:0'
  Min:             -0.555584013462066650390625
  Max:             0.475987732410430908203125
  Histogram:       [ 1, 1, 18, 455, 6099, 13252, 4163, 512, 64, 11 ]
- NodeOutputName:  'squeezenet0_conv19_weight__1:0'
  Min:             -0.555584013462066650390625
  Max:             0.475987732410430908203125
  Histogram:       [ 1, 1, 18, 455, 6099, 13252, 4163, 512, 64, 11 ]
- NodeOutputName:  'squeezenet0_conv1_bias:0'
  Min:             -0.72509253025054931640625
  Max:             0.635230541229248046875
  Histogram:       [ 2, 1, 1, 1, 0, 0, 3, 3, 1, 4 ]
- NodeOutputName:  'squeezenet0_conv1_fwd__1:0'
  Min:             -42.978908538818359375
  Max:             28.9495601654052734375
  Histogram:       [ 12, 28, 49, 181, 873, 6758, 37909, 2324, 226, 40 ]
- NodeOutputName:  'squeezenet0_conv1_fwd__1_bias:0'
  Min:             -42.978908538818359375
  Max:             28.9495601654052734375
```
