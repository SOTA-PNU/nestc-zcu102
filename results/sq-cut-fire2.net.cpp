

#include "VTABundle.h"
#include "vta/runtime.h"
#include <fstream>
#include <iostream>
#include <time.h>
#include "cut.h"
SymbolTableEntry symbolTableEntry_cut[2]={{"data",0,150528,'1'},{"squeezenet0_concat0__1",602112,387200,'1'}};
BundleConfig cut_config = {13824, 2150912, 0, 64, 2, symbolTableEntry_cut};
namespace namespace_cut {
int8_t* squeezenet0_conv0_weight__1;
int8_t* squeezenet0_conv0_bias;
int8_t* squeezenet0_conv1_weight__1;
int8_t* squeezenet0_conv1_bias;
int8_t* squeezenet0_conv2_weight__1;
int8_t* squeezenet0_conv2_bias;
int8_t* squeezenet0_conv3_weight__1;
int8_t* squeezenet0_conv3_bias;
}
using namespace namespace_cut;
extern VTACommandHandle vtaCmdH;

void cut_load_module(uint8_t *constantWeight){
  squeezenet0_conv0_weight__1 = (int8_t *)VTABufferAlloc(1728);
  VTABufferCopy((int8_t *)(constantWeight + 0), 0, squeezenet0_conv0_weight__1, 0, 1728, 1);
  squeezenet0_conv0_bias = (int8_t *)VTABufferAlloc(256);
  VTABufferCopy((int8_t *)(constantWeight + 1728), 0, squeezenet0_conv0_bias, 0, 256, 1);
  squeezenet0_conv1_weight__1 = (int8_t *)VTABufferAlloc(1024);
  VTABufferCopy((int8_t *)(constantWeight + 1984), 0, squeezenet0_conv1_weight__1, 0, 1024, 1);
  squeezenet0_conv1_bias = (int8_t *)VTABufferAlloc(64);
  VTABufferCopy((int8_t *)(constantWeight + 3008), 0, squeezenet0_conv1_bias, 0, 64, 1);
  squeezenet0_conv2_weight__1 = (int8_t *)VTABufferAlloc(1024);
  VTABufferCopy((int8_t *)(constantWeight + 3072), 0, squeezenet0_conv2_weight__1, 0, 1024, 1);
  squeezenet0_conv2_bias = (int8_t *)VTABufferAlloc(256);
  VTABufferCopy((int8_t *)(constantWeight + 4096), 0, squeezenet0_conv2_bias, 0, 256, 1);
  squeezenet0_conv3_weight__1 = (int8_t *)VTABufferAlloc(9216);
  VTABufferCopy((int8_t *)(constantWeight + 4352), 0, squeezenet0_conv3_weight__1, 0, 9216, 1);
  squeezenet0_conv3_bias = (int8_t *)VTABufferAlloc(256);
  VTABufferCopy((int8_t *)(constantWeight + 13568), 0, squeezenet0_conv3_bias, 0, 256, 1);
}

void cut_destroy_module(){
  VTABufferFree(squeezenet0_conv0_weight__1);
  VTABufferFree(squeezenet0_conv0_bias);
  VTABufferFree(squeezenet0_conv1_weight__1);
  VTABufferFree(squeezenet0_conv1_bias);
  VTABufferFree(squeezenet0_conv2_weight__1);
  VTABufferFree(squeezenet0_conv2_bias);
  VTABufferFree(squeezenet0_conv3_weight__1);
  VTABufferFree(squeezenet0_conv3_bias);
}
int cut(uint8_t *constantWeight, uint8_t *mutableWeight, uint8_t *activations){

  //Run allocactivation : squeezenet0_conv0_fwd__1_quantize_res
  int8_t *squeezenet0_conv0_fwd__1_quantize_res = (int8_t *)malloc(150528);

  //Run quantize : squeezenet0_conv0_fwd__1_quantize
  int8_t* data = (int8_t*)mutableWeight + 0;
  quantize(data, squeezenet0_conv0_fwd__1_quantize_res, 150528, 1/64.000000, 0 );

  //Run allocactivation : squeezenet0_conv0_fwd__9_res
  int8_t *squeezenet0_conv0_fwd__9_res = (int8_t *)malloc(150528);

  //Run transpose : squeezenet0_conv0_fwd__9
  transpose(squeezenet0_conv0_fwd__1_quantize_res, squeezenet0_conv0_fwd__9_res, 1, 3, 224, 224, 1, 224, 224, 3, 0, 2, 3, 1 );

  //Run deallocactivation : dealloc_squeezenet0_conv0_fwd__1_quantize_res
  free(squeezenet0_conv0_fwd__1_quantize_res);

  //Run allocactivation : squeezenet0_conv0_fwd__2_res
  int8_t *squeezenet0_conv0_fwd__2_res = (int8_t *)malloc(788544);

  //Run convolution : squeezenet0_conv0_fwd__2
  nonvtaconvolution(squeezenet0_conv0_fwd__9_res, 1.0/64.000000, 0, (int8_t *)VTABufferGetVirtAddr(squeezenet0_conv0_weight__1), 1.0/128.000000, 0, (int8_t *)VTABufferGetVirtAddr(squeezenet0_conv0_bias), 1.0/8192.000000, 0, squeezenet0_conv0_fwd__2_res, 1.0/16.000000, 0, 1, 224, 224, 3, 64, 3, 3, 0, 2, 1, 1, 1, 1, 111, 111 );

  //Run deallocactivation : dealloc_squeezenet0_conv0_fwd__9_res
  free(squeezenet0_conv0_fwd__9_res);

  //Run allocactivation : squeezenet0_pool0_fwd__1_res
  int8_t *squeezenet0_pool0_fwd__1_res = (int8_t *)malloc(193600);

  //Run maxpool : squeezenet0_pool0_fwd__2
  maxpool(squeezenet0_conv0_fwd__2_res, squeezenet0_pool0_fwd__1_res, 1, 111, 111, 64, 3, 3, 0, 2 );

  //Run deallocactivation : dealloc_squeezenet0_conv0_fwd__2_res
  free(squeezenet0_conv0_fwd__2_res);

  //Run allocactivation : squeezenet0_conv1_fwd__2_res
  int8_t *squeezenet0_conv1_fwd__2_res = (int8_t *)malloc(48400);

  //Run convolution : squeezenet0_conv1_fwd__2
  int8_t* squeezenet0_conv1_fwd__2_input_transpose = (int8_t *)VTABufferAlloc(193600);
  transpose_nhwc2vtaio(squeezenet0_pool0_fwd__1_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv1_fwd__2_input_transpose), 1, 55, 55, 64);
  int8_t* squeezenet0_conv1_fwd__2_output_bef_transpose = (int8_t *)VTABufferAlloc(48400);
  convolution_wo_tr(squeezenet0_conv1_fwd__2_input_transpose, squeezenet0_conv1_weight__1, (int32_t *)squeezenet0_conv1_bias, squeezenet0_conv1_fwd__2_output_bef_transpose, 1, 55, 55, 64, 16, 1, 1, 0, 1, 1, 1, 6, 55, 55, vtaCmdH, 1, 8, 55, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv1_fwd__2_output_bef_transpose), squeezenet0_conv1_fwd__2_res, 1, 55, 55, 16 );
  VTABufferFree(squeezenet0_conv1_fwd__2_input_transpose);
  VTABufferFree(squeezenet0_conv1_fwd__2_output_bef_transpose);

  //Run deallocactivation : dealloc_squeezenet0_pool0_fwd__1_res
  free(squeezenet0_pool0_fwd__1_res);

  //Run allocactivation : squeezenet0_conv2_fwd__11_res
  int8_t *squeezenet0_conv2_fwd__11_res = (int8_t *)malloc(193600);

  //Run convolution : squeezenet0_conv2_fwd__11
  int8_t* squeezenet0_conv2_fwd__11_input_transpose = (int8_t *)VTABufferAlloc(48400);
  transpose_nhwc2vtaio(squeezenet0_conv1_fwd__2_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv2_fwd__11_input_transpose), 1, 55, 55, 16);
  int8_t* squeezenet0_conv2_fwd__11_output_bef_transpose = (int8_t *)VTABufferAlloc(193600);
  convolution_wo_tr(squeezenet0_conv2_fwd__11_input_transpose, squeezenet0_conv2_weight__1, (int32_t *)squeezenet0_conv2_bias, squeezenet0_conv2_fwd__11_output_bef_transpose, 1, 55, 55, 16, 64, 1, 1, 0, 1, 0, 1, 7, 55, 55, vtaCmdH, 1, 8, 55, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv2_fwd__11_output_bef_transpose), squeezenet0_conv2_fwd__11_res, 1, 55, 55, 64 );
  VTABufferFree(squeezenet0_conv2_fwd__11_input_transpose);
  VTABufferFree(squeezenet0_conv2_fwd__11_output_bef_transpose);

  //Run allocactivation : squeezenet0_conv3_fwd__2_res
  int8_t *squeezenet0_conv3_fwd__2_res = (int8_t *)malloc(193600);

  //Run convolution : squeezenet0_conv3_fwd__2
  int8_t* squeezenet0_conv3_fwd__2_input_transpose = (int8_t *)VTABufferAlloc(48400);
  transpose_nhwc2vtaio(squeezenet0_conv1_fwd__2_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv3_fwd__2_input_transpose), 1, 55, 55, 16);
  int8_t* squeezenet0_conv3_fwd__2_output_bef_transpose = (int8_t *)VTABufferAlloc(193600);
  convolution_wo_tr(squeezenet0_conv3_fwd__2_input_transpose, squeezenet0_conv3_weight__1, (int32_t *)squeezenet0_conv3_bias, squeezenet0_conv3_fwd__2_output_bef_transpose, 1, 55, 55, 16, 64, 3, 3, 1, 1, 0, 1, 7, 55, 55, vtaCmdH, 1, 8, 55, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv3_fwd__2_output_bef_transpose), squeezenet0_conv3_fwd__2_res, 1, 55, 55, 64 );
  VTABufferFree(squeezenet0_conv3_fwd__2_input_transpose);
  VTABufferFree(squeezenet0_conv3_fwd__2_output_bef_transpose);

  //Run deallocactivation : dealloc_squeezenet0_conv1_fwd__2_res
  free(squeezenet0_conv1_fwd__2_res);

  //Run allocactivation : squeezenet0_concat0__4
  int8_t *squeezenet0_concat0__4 = (int8_t *)malloc(387200);

  //Run touch : squeezenet0_concat0__2

  //Run inserttensor : squeezenet0_concat0__4_squeezenet0_conv2_fwd__11
  const dim_t _it0_dd[] = {1, 55, 55, 128};
  const dim_t _it0_sd[] = {1, 55, 55, 64};
  const dim_t _it0_of[] = {0, 0, 0, 0};
  insert_tensor(squeezenet0_concat0__4, squeezenet0_conv2_fwd__11_res, _it0_of, _it0_dd, _it0_sd, 4, 4, 4, 1, 3);

  //Run deallocactivation : dealloc_squeezenet0_conv2_fwd__11_res
  free(squeezenet0_conv2_fwd__11_res);

  //Run inserttensor : squeezenet0_concat0__4_squeezenet0_conv3_fwd__2
  const dim_t _it1_dd[] = {1, 55, 55, 128};
  const dim_t _it1_sd[] = {1, 55, 55, 64};
  const dim_t _it1_of[] = {0, 0, 0, 64};
  insert_tensor(squeezenet0_concat0__4, squeezenet0_conv3_fwd__2_res, _it1_of, _it1_dd, _it1_sd, 4, 4, 4, 1, 3);

  //Run deallocactivation : dealloc_squeezenet0_conv3_fwd__2_res
  free(squeezenet0_conv3_fwd__2_res);

  //Run relu : squeezenet0_relu2_fwd__2
  relu(squeezenet0_concat0__4, squeezenet0_concat0__4, 387200 );

  //Run allocactivation : squeezenet0_relu2_fwd__2_dequantize_res
  int8_t *squeezenet0_relu2_fwd__2_dequantize_res = (int8_t *)malloc(1548800);

  //Run dequantize : squeezenet0_relu2_fwd__2_dequantize
  dequantize(squeezenet0_concat0__4, squeezenet0_relu2_fwd__2_dequantize_res, 387200, 1/8.000000, 0 );

  //Run deallocactivation : dealloc_squeezenet0_concat0__4
  free(squeezenet0_concat0__4);

  //Run transpose : squeezenet0_conv2_fwd__10
  int8_t* squeezenet0_concat0__1 = (int8_t*)mutableWeight + 602112;
  transpose(squeezenet0_relu2_fwd__2_dequantize_res, squeezenet0_concat0__1, 1, 55, 55, 128, 1, 128, 55, 55, 0, 3, 1, 2 );

  //Run deallocactivation : dealloc_squeezenet0_relu2_fwd__2_dequantize_res
  free(squeezenet0_relu2_fwd__2_dequantize_res);
  return 0;
}