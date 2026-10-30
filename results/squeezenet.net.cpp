

#include "VTABundle.h"
#include "vta/runtime.h"
#include <fstream>
#include <iostream>
#include <time.h>
#include "squeezenet.h"
SymbolTableEntry symbolTableEntry_squeezenet[2]={{"data",0,150528,'1'},{"squeezenet0_flatten0_reshape0__1",602112,1000,'1'}};
BundleConfig squeezenet_config = {1247328, 606112, 0, 64, 2, symbolTableEntry_squeezenet};
namespace namespace_squeezenet {
int8_t* squeezenet0_conv0_weight__1;
int8_t* squeezenet0_conv0_bias;
int8_t* squeezenet0_conv1_weight__1;
int8_t* squeezenet0_conv1_bias;
int8_t* squeezenet0_conv2_weight__1;
int8_t* squeezenet0_conv2_bias;
int8_t* squeezenet0_conv3_weight__1;
int8_t* squeezenet0_conv3_bias;
int8_t* squeezenet0_conv4_weight__1;
int8_t* squeezenet0_conv4_bias;
int8_t* squeezenet0_conv5_weight__1;
int8_t* squeezenet0_conv5_bias;
int8_t* squeezenet0_conv6_weight__1;
int8_t* squeezenet0_conv6_bias;
int8_t* squeezenet0_conv7_weight__1;
int8_t* squeezenet0_conv7_bias;
int8_t* squeezenet0_conv8_weight__1;
int8_t* squeezenet0_conv8_bias;
int8_t* squeezenet0_conv9_weight__1;
int8_t* squeezenet0_conv9_bias;
int8_t* squeezenet0_conv10_weight__1;
int8_t* squeezenet0_conv10_bias;
int8_t* squeezenet0_conv11_weight__1;
int8_t* squeezenet0_conv11_bias;
int8_t* squeezenet0_conv12_weight__1;
int8_t* squeezenet0_conv12_bias;
int8_t* squeezenet0_conv13_weight__1;
int8_t* squeezenet0_conv13_bias;
int8_t* squeezenet0_conv14_weight__1;
int8_t* squeezenet0_conv14_bias;
int8_t* squeezenet0_conv15_weight__1;
int8_t* squeezenet0_conv15_bias;
int8_t* squeezenet0_conv16_weight__1;
int8_t* squeezenet0_conv16_bias;
int8_t* squeezenet0_conv17_weight__1;
int8_t* squeezenet0_conv17_bias;
int8_t* squeezenet0_conv18_weight__1;
int8_t* squeezenet0_conv18_bias;
int8_t* squeezenet0_conv19_weight__1;
int8_t* squeezenet0_conv19_bias;
int8_t* squeezenet0_conv20_weight__1;
int8_t* squeezenet0_conv20_bias;
int8_t* squeezenet0_conv21_weight__1;
int8_t* squeezenet0_conv21_bias;
int8_t* squeezenet0_conv22_weight__1;
int8_t* squeezenet0_conv22_bias;
int8_t* squeezenet0_conv23_weight__1;
int8_t* squeezenet0_conv23_bias;
int8_t* squeezenet0_conv24_weight__1;
int8_t* squeezenet0_conv24_bias;
int8_t* squeezenet0_conv25_weight__1;
int8_t* squeezenet0_conv25_bias;
}
using namespace namespace_squeezenet;
extern VTACommandHandle vtaCmdH;

void squeezenet_load_module(uint8_t *constantWeight){
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
  squeezenet0_conv4_weight__1 = (int8_t *)VTABufferAlloc(2048);
  VTABufferCopy((int8_t *)(constantWeight + 13824), 0, squeezenet0_conv4_weight__1, 0, 2048, 1);
  squeezenet0_conv4_bias = (int8_t *)VTABufferAlloc(64);
  VTABufferCopy((int8_t *)(constantWeight + 15872), 0, squeezenet0_conv4_bias, 0, 64, 1);
  squeezenet0_conv5_weight__1 = (int8_t *)VTABufferAlloc(1024);
  VTABufferCopy((int8_t *)(constantWeight + 15936), 0, squeezenet0_conv5_weight__1, 0, 1024, 1);
  squeezenet0_conv5_bias = (int8_t *)VTABufferAlloc(256);
  VTABufferCopy((int8_t *)(constantWeight + 16960), 0, squeezenet0_conv5_bias, 0, 256, 1);
  squeezenet0_conv6_weight__1 = (int8_t *)VTABufferAlloc(9216);
  VTABufferCopy((int8_t *)(constantWeight + 17216), 0, squeezenet0_conv6_weight__1, 0, 9216, 1);
  squeezenet0_conv6_bias = (int8_t *)VTABufferAlloc(256);
  VTABufferCopy((int8_t *)(constantWeight + 26432), 0, squeezenet0_conv6_bias, 0, 256, 1);
  squeezenet0_conv7_weight__1 = (int8_t *)VTABufferAlloc(4096);
  VTABufferCopy((int8_t *)(constantWeight + 26688), 0, squeezenet0_conv7_weight__1, 0, 4096, 1);
  squeezenet0_conv7_bias = (int8_t *)VTABufferAlloc(128);
  VTABufferCopy((int8_t *)(constantWeight + 30784), 0, squeezenet0_conv7_bias, 0, 128, 1);
  squeezenet0_conv8_weight__1 = (int8_t *)VTABufferAlloc(4096);
  VTABufferCopy((int8_t *)(constantWeight + 30912), 0, squeezenet0_conv8_weight__1, 0, 4096, 1);
  squeezenet0_conv8_bias = (int8_t *)VTABufferAlloc(512);
  VTABufferCopy((int8_t *)(constantWeight + 35008), 0, squeezenet0_conv8_bias, 0, 512, 1);
  squeezenet0_conv9_weight__1 = (int8_t *)VTABufferAlloc(36864);
  VTABufferCopy((int8_t *)(constantWeight + 35520), 0, squeezenet0_conv9_weight__1, 0, 36864, 1);
  squeezenet0_conv9_bias = (int8_t *)VTABufferAlloc(512);
  VTABufferCopy((int8_t *)(constantWeight + 72384), 0, squeezenet0_conv9_bias, 0, 512, 1);
  squeezenet0_conv10_weight__1 = (int8_t *)VTABufferAlloc(8192);
  VTABufferCopy((int8_t *)(constantWeight + 72896), 0, squeezenet0_conv10_weight__1, 0, 8192, 1);
  squeezenet0_conv10_bias = (int8_t *)VTABufferAlloc(128);
  VTABufferCopy((int8_t *)(constantWeight + 81088), 0, squeezenet0_conv10_bias, 0, 128, 1);
  squeezenet0_conv11_weight__1 = (int8_t *)VTABufferAlloc(4096);
  VTABufferCopy((int8_t *)(constantWeight + 81216), 0, squeezenet0_conv11_weight__1, 0, 4096, 1);
  squeezenet0_conv11_bias = (int8_t *)VTABufferAlloc(512);
  VTABufferCopy((int8_t *)(constantWeight + 85312), 0, squeezenet0_conv11_bias, 0, 512, 1);
  squeezenet0_conv12_weight__1 = (int8_t *)VTABufferAlloc(36864);
  VTABufferCopy((int8_t *)(constantWeight + 85824), 0, squeezenet0_conv12_weight__1, 0, 36864, 1);
  squeezenet0_conv12_bias = (int8_t *)VTABufferAlloc(512);
  VTABufferCopy((int8_t *)(constantWeight + 122688), 0, squeezenet0_conv12_bias, 0, 512, 1);
  squeezenet0_conv13_weight__1 = (int8_t *)VTABufferAlloc(12288);
  VTABufferCopy((int8_t *)(constantWeight + 123200), 0, squeezenet0_conv13_weight__1, 0, 12288, 1);
  squeezenet0_conv13_bias = (int8_t *)VTABufferAlloc(192);
  VTABufferCopy((int8_t *)(constantWeight + 135488), 0, squeezenet0_conv13_bias, 0, 192, 1);
  squeezenet0_conv14_weight__1 = (int8_t *)VTABufferAlloc(9216);
  VTABufferCopy((int8_t *)(constantWeight + 135680), 0, squeezenet0_conv14_weight__1, 0, 9216, 1);
  squeezenet0_conv14_bias = (int8_t *)VTABufferAlloc(768);
  VTABufferCopy((int8_t *)(constantWeight + 144896), 0, squeezenet0_conv14_bias, 0, 768, 1);
  squeezenet0_conv15_weight__1 = (int8_t *)VTABufferAlloc(82944);
  VTABufferCopy((int8_t *)(constantWeight + 145664), 0, squeezenet0_conv15_weight__1, 0, 82944, 1);
  squeezenet0_conv15_bias = (int8_t *)VTABufferAlloc(768);
  VTABufferCopy((int8_t *)(constantWeight + 228608), 0, squeezenet0_conv15_bias, 0, 768, 1);
  squeezenet0_conv16_weight__1 = (int8_t *)VTABufferAlloc(18432);
  VTABufferCopy((int8_t *)(constantWeight + 229376), 0, squeezenet0_conv16_weight__1, 0, 18432, 1);
  squeezenet0_conv16_bias = (int8_t *)VTABufferAlloc(192);
  VTABufferCopy((int8_t *)(constantWeight + 247808), 0, squeezenet0_conv16_bias, 0, 192, 1);
  squeezenet0_conv17_weight__1 = (int8_t *)VTABufferAlloc(9216);
  VTABufferCopy((int8_t *)(constantWeight + 248000), 0, squeezenet0_conv17_weight__1, 0, 9216, 1);
  squeezenet0_conv17_bias = (int8_t *)VTABufferAlloc(768);
  VTABufferCopy((int8_t *)(constantWeight + 257216), 0, squeezenet0_conv17_bias, 0, 768, 1);
  squeezenet0_conv18_weight__1 = (int8_t *)VTABufferAlloc(82944);
  VTABufferCopy((int8_t *)(constantWeight + 257984), 0, squeezenet0_conv18_weight__1, 0, 82944, 1);
  squeezenet0_conv18_bias = (int8_t *)VTABufferAlloc(768);
  VTABufferCopy((int8_t *)(constantWeight + 340928), 0, squeezenet0_conv18_bias, 0, 768, 1);
  squeezenet0_conv19_weight__1 = (int8_t *)VTABufferAlloc(24576);
  VTABufferCopy((int8_t *)(constantWeight + 341696), 0, squeezenet0_conv19_weight__1, 0, 24576, 1);
  squeezenet0_conv19_bias = (int8_t *)VTABufferAlloc(256);
  VTABufferCopy((int8_t *)(constantWeight + 366272), 0, squeezenet0_conv19_bias, 0, 256, 1);
  squeezenet0_conv20_weight__1 = (int8_t *)VTABufferAlloc(16384);
  VTABufferCopy((int8_t *)(constantWeight + 366528), 0, squeezenet0_conv20_weight__1, 0, 16384, 1);
  squeezenet0_conv20_bias = (int8_t *)VTABufferAlloc(1024);
  VTABufferCopy((int8_t *)(constantWeight + 382912), 0, squeezenet0_conv20_bias, 0, 1024, 1);
  squeezenet0_conv21_weight__1 = (int8_t *)VTABufferAlloc(147456);
  VTABufferCopy((int8_t *)(constantWeight + 383936), 0, squeezenet0_conv21_weight__1, 0, 147456, 1);
  squeezenet0_conv21_bias = (int8_t *)VTABufferAlloc(1024);
  VTABufferCopy((int8_t *)(constantWeight + 531392), 0, squeezenet0_conv21_bias, 0, 1024, 1);
  squeezenet0_conv22_weight__1 = (int8_t *)VTABufferAlloc(32768);
  VTABufferCopy((int8_t *)(constantWeight + 532416), 0, squeezenet0_conv22_weight__1, 0, 32768, 1);
  squeezenet0_conv22_bias = (int8_t *)VTABufferAlloc(256);
  VTABufferCopy((int8_t *)(constantWeight + 565184), 0, squeezenet0_conv22_bias, 0, 256, 1);
  squeezenet0_conv23_weight__1 = (int8_t *)VTABufferAlloc(16384);
  VTABufferCopy((int8_t *)(constantWeight + 565440), 0, squeezenet0_conv23_weight__1, 0, 16384, 1);
  squeezenet0_conv23_bias = (int8_t *)VTABufferAlloc(1024);
  VTABufferCopy((int8_t *)(constantWeight + 581824), 0, squeezenet0_conv23_bias, 0, 1024, 1);
  squeezenet0_conv24_weight__1 = (int8_t *)VTABufferAlloc(147456);
  VTABufferCopy((int8_t *)(constantWeight + 582848), 0, squeezenet0_conv24_weight__1, 0, 147456, 1);
  squeezenet0_conv24_bias = (int8_t *)VTABufferAlloc(1024);
  VTABufferCopy((int8_t *)(constantWeight + 730304), 0, squeezenet0_conv24_bias, 0, 1024, 1);
  squeezenet0_conv25_weight__1 = (int8_t *)VTABufferAlloc(512000);
  VTABufferCopy((int8_t *)(constantWeight + 731328), 0, squeezenet0_conv25_weight__1, 0, 512000, 1);
  squeezenet0_conv25_bias = (int8_t *)VTABufferAlloc(4000);
  VTABufferCopy((int8_t *)(constantWeight + 1243328), 0, squeezenet0_conv25_bias, 0, 4000, 1);
}

void squeezenet_destroy_module(){
  VTABufferFree(squeezenet0_conv0_weight__1);
  VTABufferFree(squeezenet0_conv0_bias);
  VTABufferFree(squeezenet0_conv1_weight__1);
  VTABufferFree(squeezenet0_conv1_bias);
  VTABufferFree(squeezenet0_conv2_weight__1);
  VTABufferFree(squeezenet0_conv2_bias);
  VTABufferFree(squeezenet0_conv3_weight__1);
  VTABufferFree(squeezenet0_conv3_bias);
  VTABufferFree(squeezenet0_conv4_weight__1);
  VTABufferFree(squeezenet0_conv4_bias);
  VTABufferFree(squeezenet0_conv5_weight__1);
  VTABufferFree(squeezenet0_conv5_bias);
  VTABufferFree(squeezenet0_conv6_weight__1);
  VTABufferFree(squeezenet0_conv6_bias);
  VTABufferFree(squeezenet0_conv7_weight__1);
  VTABufferFree(squeezenet0_conv7_bias);
  VTABufferFree(squeezenet0_conv8_weight__1);
  VTABufferFree(squeezenet0_conv8_bias);
  VTABufferFree(squeezenet0_conv9_weight__1);
  VTABufferFree(squeezenet0_conv9_bias);
  VTABufferFree(squeezenet0_conv10_weight__1);
  VTABufferFree(squeezenet0_conv10_bias);
  VTABufferFree(squeezenet0_conv11_weight__1);
  VTABufferFree(squeezenet0_conv11_bias);
  VTABufferFree(squeezenet0_conv12_weight__1);
  VTABufferFree(squeezenet0_conv12_bias);
  VTABufferFree(squeezenet0_conv13_weight__1);
  VTABufferFree(squeezenet0_conv13_bias);
  VTABufferFree(squeezenet0_conv14_weight__1);
  VTABufferFree(squeezenet0_conv14_bias);
  VTABufferFree(squeezenet0_conv15_weight__1);
  VTABufferFree(squeezenet0_conv15_bias);
  VTABufferFree(squeezenet0_conv16_weight__1);
  VTABufferFree(squeezenet0_conv16_bias);
  VTABufferFree(squeezenet0_conv17_weight__1);
  VTABufferFree(squeezenet0_conv17_bias);
  VTABufferFree(squeezenet0_conv18_weight__1);
  VTABufferFree(squeezenet0_conv18_bias);
  VTABufferFree(squeezenet0_conv19_weight__1);
  VTABufferFree(squeezenet0_conv19_bias);
  VTABufferFree(squeezenet0_conv20_weight__1);
  VTABufferFree(squeezenet0_conv20_bias);
  VTABufferFree(squeezenet0_conv21_weight__1);
  VTABufferFree(squeezenet0_conv21_bias);
  VTABufferFree(squeezenet0_conv22_weight__1);
  VTABufferFree(squeezenet0_conv22_bias);
  VTABufferFree(squeezenet0_conv23_weight__1);
  VTABufferFree(squeezenet0_conv23_bias);
  VTABufferFree(squeezenet0_conv24_weight__1);
  VTABufferFree(squeezenet0_conv24_bias);
  VTABufferFree(squeezenet0_conv25_weight__1);
  VTABufferFree(squeezenet0_conv25_bias);
}
int squeezenet(uint8_t *constantWeight, uint8_t *mutableWeight, uint8_t *activations){

  //Run allocactivation : squeezenet0_conv0_fwd__1_quantize_res
  int8_t *squeezenet0_conv0_fwd__1_quantize_res = (int8_t *)malloc(150528);

  //Run quantize : squeezenet0_conv0_fwd__1_quantize
  int8_t* data = (int8_t*)mutableWeight + 0;
  quantize(data, squeezenet0_conv0_fwd__1_quantize_res, 150528, 1/32.000000, 0 );

  //Run allocactivation : squeezenet0_conv0_fwd__9_res
  int8_t *squeezenet0_conv0_fwd__9_res = (int8_t *)malloc(150528);

  //Run transpose : squeezenet0_conv0_fwd__9
  transpose(squeezenet0_conv0_fwd__1_quantize_res, squeezenet0_conv0_fwd__9_res, 1, 3, 224, 224, 1, 224, 224, 3, 0, 2, 3, 1 );

  //Run deallocactivation : dealloc_squeezenet0_conv0_fwd__1_quantize_res
  free(squeezenet0_conv0_fwd__1_quantize_res);

  //Run allocactivation : squeezenet0_conv0_fwd__2_res
  int8_t *squeezenet0_conv0_fwd__2_res = (int8_t *)malloc(788544);

  //Run convolution : squeezenet0_conv0_fwd__2
  nonvtaconvolution(squeezenet0_conv0_fwd__9_res, 1.0/32.000000, 0, (int8_t *)VTABufferGetVirtAddr(squeezenet0_conv0_weight__1), 1.0/128.000000, 0, (int8_t *)VTABufferGetVirtAddr(squeezenet0_conv0_bias), 1.0/4096.000000, 0, squeezenet0_conv0_fwd__2_res, 1.0/4.000000, 0, 1, 224, 224, 3, 64, 3, 3, 0, 2, 1, 1, 1, 1, 111, 111 );

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

  //Run allocactivation : squeezenet0_conv2_fwd__2_res
  int8_t *squeezenet0_conv2_fwd__2_res = (int8_t *)malloc(193600);

  //Run convolution : squeezenet0_conv2_fwd__2
  int8_t* squeezenet0_conv2_fwd__2_input_transpose = (int8_t *)VTABufferAlloc(48400);
  transpose_nhwc2vtaio(squeezenet0_conv1_fwd__2_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv2_fwd__2_input_transpose), 1, 55, 55, 16);
  int8_t* squeezenet0_conv2_fwd__2_output_bef_transpose = (int8_t *)VTABufferAlloc(193600);
  convolution_wo_tr(squeezenet0_conv2_fwd__2_input_transpose, squeezenet0_conv2_weight__1, (int32_t *)squeezenet0_conv2_bias, squeezenet0_conv2_fwd__2_output_bef_transpose, 1, 55, 55, 16, 64, 1, 1, 0, 1, 0, 1, 7, 55, 55, vtaCmdH, 1, 8, 55, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv2_fwd__2_output_bef_transpose), squeezenet0_conv2_fwd__2_res, 1, 55, 55, 64 );
  VTABufferFree(squeezenet0_conv2_fwd__2_input_transpose);
  VTABufferFree(squeezenet0_conv2_fwd__2_output_bef_transpose);

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

  //Run allocactivation : squeezenet0_concat0__3
  int8_t *squeezenet0_concat0__3 = (int8_t *)malloc(387200);

  //Run touch : squeezenet0_concat0__1

  //Run inserttensor : squeezenet0_concat0__3_squeezenet0_conv2_fwd__2
  const dim_t _it0_dd[] = {1, 55, 55, 128};
  const dim_t _it0_sd[] = {1, 55, 55, 64};
  const dim_t _it0_of[] = {0, 0, 0, 0};
  insert_tensor(squeezenet0_concat0__3, squeezenet0_conv2_fwd__2_res, _it0_of, _it0_dd, _it0_sd, 4, 4, 4, 1, 3);

  //Run deallocactivation : dealloc_squeezenet0_conv2_fwd__2_res
  free(squeezenet0_conv2_fwd__2_res);

  //Run inserttensor : squeezenet0_concat0__3_squeezenet0_conv3_fwd__2
  const dim_t _it1_dd[] = {1, 55, 55, 128};
  const dim_t _it1_sd[] = {1, 55, 55, 64};
  const dim_t _it1_of[] = {0, 0, 0, 64};
  insert_tensor(squeezenet0_concat0__3, squeezenet0_conv3_fwd__2_res, _it1_of, _it1_dd, _it1_sd, 4, 4, 4, 1, 3);

  //Run deallocactivation : dealloc_squeezenet0_conv3_fwd__2_res
  free(squeezenet0_conv3_fwd__2_res);

  //Run allocactivation : squeezenet0_relu2_fwd__2_res
  int8_t *squeezenet0_relu2_fwd__2_res = (int8_t *)malloc(387200);

  //Run relu : squeezenet0_relu2_fwd__2
  relu(squeezenet0_concat0__3, squeezenet0_relu2_fwd__2_res, 387200, 0.500000, 0.250000 );

  //Run deallocactivation : dealloc_squeezenet0_concat0__3
  free(squeezenet0_concat0__3);

  //Run allocactivation : squeezenet0_conv4_fwd__2_res
  int8_t *squeezenet0_conv4_fwd__2_res = (int8_t *)malloc(48400);

  //Run convolution : squeezenet0_conv4_fwd__2
  int8_t* squeezenet0_conv4_fwd__2_input_transpose = (int8_t *)VTABufferAlloc(387200);
  transpose_nhwc2vtaio(squeezenet0_relu2_fwd__2_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv4_fwd__2_input_transpose), 1, 55, 55, 128);
  int8_t* squeezenet0_conv4_fwd__2_output_bef_transpose = (int8_t *)VTABufferAlloc(48400);
  convolution_wo_tr(squeezenet0_conv4_fwd__2_input_transpose, squeezenet0_conv4_weight__1, (int32_t *)squeezenet0_conv4_bias, squeezenet0_conv4_fwd__2_output_bef_transpose, 1, 55, 55, 128, 16, 1, 1, 0, 1, 1, 1, 8, 55, 55, vtaCmdH, 1, 8, 55, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv4_fwd__2_output_bef_transpose), squeezenet0_conv4_fwd__2_res, 1, 55, 55, 16 );
  VTABufferFree(squeezenet0_conv4_fwd__2_input_transpose);
  VTABufferFree(squeezenet0_conv4_fwd__2_output_bef_transpose);

  //Run deallocactivation : dealloc_squeezenet0_relu2_fwd__2_res
  free(squeezenet0_relu2_fwd__2_res);

  //Run allocactivation : squeezenet0_conv5_fwd__7_res
  int8_t *squeezenet0_conv5_fwd__7_res = (int8_t *)malloc(193600);

  //Run convolution : squeezenet0_conv5_fwd__7
  int8_t* squeezenet0_conv5_fwd__7_input_transpose = (int8_t *)VTABufferAlloc(48400);
  transpose_nhwc2vtaio(squeezenet0_conv4_fwd__2_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv5_fwd__7_input_transpose), 1, 55, 55, 16);
  int8_t* squeezenet0_conv5_fwd__7_output_bef_transpose = (int8_t *)VTABufferAlloc(193600);
  convolution_wo_tr(squeezenet0_conv5_fwd__7_input_transpose, squeezenet0_conv5_weight__1, (int32_t *)squeezenet0_conv5_bias, squeezenet0_conv5_fwd__7_output_bef_transpose, 1, 55, 55, 16, 64, 1, 1, 0, 1, 0, 1, 7, 55, 55, vtaCmdH, 1, 8, 55, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv5_fwd__7_output_bef_transpose), squeezenet0_conv5_fwd__7_res, 1, 55, 55, 64 );
  VTABufferFree(squeezenet0_conv5_fwd__7_input_transpose);
  VTABufferFree(squeezenet0_conv5_fwd__7_output_bef_transpose);

  //Run allocactivation : squeezenet0_conv6_fwd__2_res
  int8_t *squeezenet0_conv6_fwd__2_res = (int8_t *)malloc(193600);

  //Run convolution : squeezenet0_conv6_fwd__2
  int8_t* squeezenet0_conv6_fwd__2_input_transpose = (int8_t *)VTABufferAlloc(48400);
  transpose_nhwc2vtaio(squeezenet0_conv4_fwd__2_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv6_fwd__2_input_transpose), 1, 55, 55, 16);
  int8_t* squeezenet0_conv6_fwd__2_output_bef_transpose = (int8_t *)VTABufferAlloc(193600);
  convolution_wo_tr(squeezenet0_conv6_fwd__2_input_transpose, squeezenet0_conv6_weight__1, (int32_t *)squeezenet0_conv6_bias, squeezenet0_conv6_fwd__2_output_bef_transpose, 1, 55, 55, 16, 64, 3, 3, 1, 1, 0, 1, 7, 55, 55, vtaCmdH, 1, 8, 55, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv6_fwd__2_output_bef_transpose), squeezenet0_conv6_fwd__2_res, 1, 55, 55, 64 );
  VTABufferFree(squeezenet0_conv6_fwd__2_input_transpose);
  VTABufferFree(squeezenet0_conv6_fwd__2_output_bef_transpose);

  //Run deallocactivation : dealloc_squeezenet0_conv4_fwd__2_res
  free(squeezenet0_conv4_fwd__2_res);

  //Run allocactivation : squeezenet0_concat1__3
  int8_t *squeezenet0_concat1__3 = (int8_t *)malloc(387200);

  //Run touch : squeezenet0_concat1__1

  //Run inserttensor : squeezenet0_concat1__3_squeezenet0_conv5_fwd__7
  const dim_t _it2_dd[] = {1, 55, 55, 128};
  const dim_t _it2_sd[] = {1, 55, 55, 64};
  const dim_t _it2_of[] = {0, 0, 0, 0};
  insert_tensor(squeezenet0_concat1__3, squeezenet0_conv5_fwd__7_res, _it2_of, _it2_dd, _it2_sd, 4, 4, 4, 1, 3);

  //Run deallocactivation : dealloc_squeezenet0_conv5_fwd__7_res
  free(squeezenet0_conv5_fwd__7_res);

  //Run inserttensor : squeezenet0_concat1__3_squeezenet0_conv6_fwd__2
  const dim_t _it3_dd[] = {1, 55, 55, 128};
  const dim_t _it3_sd[] = {1, 55, 55, 64};
  const dim_t _it3_of[] = {0, 0, 0, 64};
  insert_tensor(squeezenet0_concat1__3, squeezenet0_conv6_fwd__2_res, _it3_of, _it3_dd, _it3_sd, 4, 4, 4, 1, 3);

  //Run deallocactivation : dealloc_squeezenet0_conv6_fwd__2_res
  free(squeezenet0_conv6_fwd__2_res);

  //Run relu : squeezenet0_relu5_fwd__2
  relu(squeezenet0_concat1__3, squeezenet0_concat1__3, 387200 );

  //Run allocactivation : squeezenet0_pool1_fwd__1_res
  int8_t *squeezenet0_pool1_fwd__1_res = (int8_t *)malloc(93312);

  //Run maxpool : squeezenet0_pool1_fwd__2
  maxpool(squeezenet0_concat1__3, squeezenet0_pool1_fwd__1_res, 1, 55, 55, 128, 3, 3, 0, 2 );

  //Run deallocactivation : dealloc_squeezenet0_concat1__3
  free(squeezenet0_concat1__3);

  //Run allocactivation : squeezenet0_conv7_fwd__2_res
  int8_t *squeezenet0_conv7_fwd__2_res = (int8_t *)malloc(23328);

  //Run convolution : squeezenet0_conv7_fwd__2
  int8_t* squeezenet0_conv7_fwd__2_input_transpose = (int8_t *)VTABufferAlloc(93312);
  transpose_nhwc2vtaio(squeezenet0_pool1_fwd__1_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv7_fwd__2_input_transpose), 1, 27, 27, 128);
  int8_t* squeezenet0_conv7_fwd__2_output_bef_transpose = (int8_t *)VTABufferAlloc(23328);
  convolution_wo_tr(squeezenet0_conv7_fwd__2_input_transpose, squeezenet0_conv7_weight__1, (int32_t *)squeezenet0_conv7_bias, squeezenet0_conv7_fwd__2_output_bef_transpose, 1, 27, 27, 128, 32, 1, 1, 0, 1, 1, 1, 8, 27, 27, vtaCmdH, 1, 14, 27, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv7_fwd__2_output_bef_transpose), squeezenet0_conv7_fwd__2_res, 1, 27, 27, 32 );
  VTABufferFree(squeezenet0_conv7_fwd__2_input_transpose);
  VTABufferFree(squeezenet0_conv7_fwd__2_output_bef_transpose);

  //Run deallocactivation : dealloc_squeezenet0_pool1_fwd__1_res
  free(squeezenet0_pool1_fwd__1_res);

  //Run allocactivation : squeezenet0_conv8_fwd__7_res
  int8_t *squeezenet0_conv8_fwd__7_res = (int8_t *)malloc(93312);

  //Run convolution : squeezenet0_conv8_fwd__7
  int8_t* squeezenet0_conv8_fwd__7_input_transpose = (int8_t *)VTABufferAlloc(23328);
  transpose_nhwc2vtaio(squeezenet0_conv7_fwd__2_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv8_fwd__7_input_transpose), 1, 27, 27, 32);
  int8_t* squeezenet0_conv8_fwd__7_output_bef_transpose = (int8_t *)VTABufferAlloc(93312);
  convolution_wo_tr(squeezenet0_conv8_fwd__7_input_transpose, squeezenet0_conv8_weight__1, (int32_t *)squeezenet0_conv8_bias, squeezenet0_conv8_fwd__7_output_bef_transpose, 1, 27, 27, 32, 128, 1, 1, 0, 1, 0, 1, 8, 27, 27, vtaCmdH, 1, 14, 27, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv8_fwd__7_output_bef_transpose), squeezenet0_conv8_fwd__7_res, 1, 27, 27, 128 );
  VTABufferFree(squeezenet0_conv8_fwd__7_input_transpose);
  VTABufferFree(squeezenet0_conv8_fwd__7_output_bef_transpose);

  //Run allocactivation : squeezenet0_conv9_fwd__2_res
  int8_t *squeezenet0_conv9_fwd__2_res = (int8_t *)malloc(93312);

  //Run convolution : squeezenet0_conv9_fwd__2
  int8_t* squeezenet0_conv9_fwd__2_input_transpose = (int8_t *)VTABufferAlloc(23328);
  transpose_nhwc2vtaio(squeezenet0_conv7_fwd__2_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv9_fwd__2_input_transpose), 1, 27, 27, 32);
  int8_t* squeezenet0_conv9_fwd__2_output_bef_transpose = (int8_t *)VTABufferAlloc(93312);
  convolution_wo_tr(squeezenet0_conv9_fwd__2_input_transpose, squeezenet0_conv9_weight__1, (int32_t *)squeezenet0_conv9_bias, squeezenet0_conv9_fwd__2_output_bef_transpose, 1, 27, 27, 32, 128, 3, 3, 1, 1, 0, 1, 9, 27, 27, vtaCmdH, 1, 14, 27, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv9_fwd__2_output_bef_transpose), squeezenet0_conv9_fwd__2_res, 1, 27, 27, 128 );
  VTABufferFree(squeezenet0_conv9_fwd__2_input_transpose);
  VTABufferFree(squeezenet0_conv9_fwd__2_output_bef_transpose);

  //Run deallocactivation : dealloc_squeezenet0_conv7_fwd__2_res
  free(squeezenet0_conv7_fwd__2_res);

  //Run allocactivation : squeezenet0_concat2__3
  int8_t *squeezenet0_concat2__3 = (int8_t *)malloc(186624);

  //Run touch : squeezenet0_concat2__1

  //Run inserttensor : squeezenet0_concat2__3_squeezenet0_conv8_fwd__7
  const dim_t _it4_dd[] = {1, 27, 27, 256};
  const dim_t _it4_sd[] = {1, 27, 27, 128};
  const dim_t _it4_of[] = {0, 0, 0, 0};
  insert_tensor(squeezenet0_concat2__3, squeezenet0_conv8_fwd__7_res, _it4_of, _it4_dd, _it4_sd, 4, 4, 4, 1, 3);

  //Run deallocactivation : dealloc_squeezenet0_conv8_fwd__7_res
  free(squeezenet0_conv8_fwd__7_res);

  //Run inserttensor : squeezenet0_concat2__3_squeezenet0_conv9_fwd__2
  const dim_t _it5_dd[] = {1, 27, 27, 256};
  const dim_t _it5_sd[] = {1, 27, 27, 128};
  const dim_t _it5_of[] = {0, 0, 0, 128};
  insert_tensor(squeezenet0_concat2__3, squeezenet0_conv9_fwd__2_res, _it5_of, _it5_dd, _it5_sd, 4, 4, 4, 1, 3);

  //Run deallocactivation : dealloc_squeezenet0_conv9_fwd__2_res
  free(squeezenet0_conv9_fwd__2_res);

  //Run allocactivation : squeezenet0_relu8_fwd__2_res
  int8_t *squeezenet0_relu8_fwd__2_res = (int8_t *)malloc(186624);

  //Run relu : squeezenet0_relu8_fwd__2
  relu(squeezenet0_concat2__3, squeezenet0_relu8_fwd__2_res, 186624, 2.000000, 1.000000 );

  //Run deallocactivation : dealloc_squeezenet0_concat2__3
  free(squeezenet0_concat2__3);

  //Run allocactivation : squeezenet0_conv10_fwd__2_res
  int8_t *squeezenet0_conv10_fwd__2_res = (int8_t *)malloc(23328);

  //Run convolution : squeezenet0_conv10_fwd__2
  int8_t* squeezenet0_conv10_fwd__2_input_transpose = (int8_t *)VTABufferAlloc(186624);
  transpose_nhwc2vtaio(squeezenet0_relu8_fwd__2_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv10_fwd__2_input_transpose), 1, 27, 27, 256);
  int8_t* squeezenet0_conv10_fwd__2_output_bef_transpose = (int8_t *)VTABufferAlloc(23328);
  convolution_wo_tr(squeezenet0_conv10_fwd__2_input_transpose, squeezenet0_conv10_weight__1, (int32_t *)squeezenet0_conv10_bias, squeezenet0_conv10_fwd__2_output_bef_transpose, 1, 27, 27, 256, 32, 1, 1, 0, 1, 1, 1, 8, 27, 27, vtaCmdH, 1, 14, 27, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv10_fwd__2_output_bef_transpose), squeezenet0_conv10_fwd__2_res, 1, 27, 27, 32 );
  VTABufferFree(squeezenet0_conv10_fwd__2_input_transpose);
  VTABufferFree(squeezenet0_conv10_fwd__2_output_bef_transpose);

  //Run deallocactivation : dealloc_squeezenet0_relu8_fwd__2_res
  free(squeezenet0_relu8_fwd__2_res);

  //Run allocactivation : squeezenet0_conv11_fwd__7_res
  int8_t *squeezenet0_conv11_fwd__7_res = (int8_t *)malloc(93312);

  //Run convolution : squeezenet0_conv11_fwd__7
  int8_t* squeezenet0_conv11_fwd__7_input_transpose = (int8_t *)VTABufferAlloc(23328);
  transpose_nhwc2vtaio(squeezenet0_conv10_fwd__2_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv11_fwd__7_input_transpose), 1, 27, 27, 32);
  int8_t* squeezenet0_conv11_fwd__7_output_bef_transpose = (int8_t *)VTABufferAlloc(93312);
  convolution_wo_tr(squeezenet0_conv11_fwd__7_input_transpose, squeezenet0_conv11_weight__1, (int32_t *)squeezenet0_conv11_bias, squeezenet0_conv11_fwd__7_output_bef_transpose, 1, 27, 27, 32, 128, 1, 1, 0, 1, 0, 1, 7, 27, 27, vtaCmdH, 1, 14, 27, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv11_fwd__7_output_bef_transpose), squeezenet0_conv11_fwd__7_res, 1, 27, 27, 128 );
  VTABufferFree(squeezenet0_conv11_fwd__7_input_transpose);
  VTABufferFree(squeezenet0_conv11_fwd__7_output_bef_transpose);

  //Run allocactivation : squeezenet0_conv12_fwd__2_res
  int8_t *squeezenet0_conv12_fwd__2_res = (int8_t *)malloc(93312);

  //Run convolution : squeezenet0_conv12_fwd__2
  int8_t* squeezenet0_conv12_fwd__2_input_transpose = (int8_t *)VTABufferAlloc(23328);
  transpose_nhwc2vtaio(squeezenet0_conv10_fwd__2_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv12_fwd__2_input_transpose), 1, 27, 27, 32);
  int8_t* squeezenet0_conv12_fwd__2_output_bef_transpose = (int8_t *)VTABufferAlloc(93312);
  convolution_wo_tr(squeezenet0_conv12_fwd__2_input_transpose, squeezenet0_conv12_weight__1, (int32_t *)squeezenet0_conv12_bias, squeezenet0_conv12_fwd__2_output_bef_transpose, 1, 27, 27, 32, 128, 3, 3, 1, 1, 0, 1, 7, 27, 27, vtaCmdH, 1, 14, 27, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv12_fwd__2_output_bef_transpose), squeezenet0_conv12_fwd__2_res, 1, 27, 27, 128 );
  VTABufferFree(squeezenet0_conv12_fwd__2_input_transpose);
  VTABufferFree(squeezenet0_conv12_fwd__2_output_bef_transpose);

  //Run deallocactivation : dealloc_squeezenet0_conv10_fwd__2_res
  free(squeezenet0_conv10_fwd__2_res);

  //Run allocactivation : squeezenet0_concat3__3
  int8_t *squeezenet0_concat3__3 = (int8_t *)malloc(186624);

  //Run touch : squeezenet0_concat3__1

  //Run inserttensor : squeezenet0_concat3__3_squeezenet0_conv11_fwd__7
  const dim_t _it6_dd[] = {1, 27, 27, 256};
  const dim_t _it6_sd[] = {1, 27, 27, 128};
  const dim_t _it6_of[] = {0, 0, 0, 0};
  insert_tensor(squeezenet0_concat3__3, squeezenet0_conv11_fwd__7_res, _it6_of, _it6_dd, _it6_sd, 4, 4, 4, 1, 3);

  //Run deallocactivation : dealloc_squeezenet0_conv11_fwd__7_res
  free(squeezenet0_conv11_fwd__7_res);

  //Run inserttensor : squeezenet0_concat3__3_squeezenet0_conv12_fwd__2
  const dim_t _it7_dd[] = {1, 27, 27, 256};
  const dim_t _it7_sd[] = {1, 27, 27, 128};
  const dim_t _it7_of[] = {0, 0, 0, 128};
  insert_tensor(squeezenet0_concat3__3, squeezenet0_conv12_fwd__2_res, _it7_of, _it7_dd, _it7_sd, 4, 4, 4, 1, 3);

  //Run deallocactivation : dealloc_squeezenet0_conv12_fwd__2_res
  free(squeezenet0_conv12_fwd__2_res);

  //Run relu : squeezenet0_relu11_fwd__2
  relu(squeezenet0_concat3__3, squeezenet0_concat3__3, 186624 );

  //Run allocactivation : squeezenet0_pool2_fwd__1_res
  int8_t *squeezenet0_pool2_fwd__1_res = (int8_t *)malloc(43264);

  //Run maxpool : squeezenet0_pool2_fwd__2
  maxpool(squeezenet0_concat3__3, squeezenet0_pool2_fwd__1_res, 1, 27, 27, 256, 3, 3, 0, 2 );

  //Run deallocactivation : dealloc_squeezenet0_concat3__3
  free(squeezenet0_concat3__3);

  //Run allocactivation : squeezenet0_conv13_fwd__2_res
  int8_t *squeezenet0_conv13_fwd__2_res = (int8_t *)malloc(8112);

  //Run convolution : squeezenet0_conv13_fwd__2
  int8_t* squeezenet0_conv13_fwd__2_input_transpose = (int8_t *)VTABufferAlloc(43264);
  transpose_nhwc2vtaio(squeezenet0_pool2_fwd__1_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv13_fwd__2_input_transpose), 1, 13, 13, 256);
  int8_t* squeezenet0_conv13_fwd__2_output_bef_transpose = (int8_t *)VTABufferAlloc(8112);
  convolution_wo_tr(squeezenet0_conv13_fwd__2_input_transpose, squeezenet0_conv13_weight__1, (int32_t *)squeezenet0_conv13_bias, squeezenet0_conv13_fwd__2_output_bef_transpose, 1, 13, 13, 256, 48, 1, 1, 0, 1, 1, 1, 8, 13, 13, vtaCmdH, 1, 13, 13, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv13_fwd__2_output_bef_transpose), squeezenet0_conv13_fwd__2_res, 1, 13, 13, 48 );
  VTABufferFree(squeezenet0_conv13_fwd__2_input_transpose);
  VTABufferFree(squeezenet0_conv13_fwd__2_output_bef_transpose);

  //Run deallocactivation : dealloc_squeezenet0_pool2_fwd__1_res
  free(squeezenet0_pool2_fwd__1_res);

  //Run allocactivation : squeezenet0_conv14_fwd__7_res
  int8_t *squeezenet0_conv14_fwd__7_res = (int8_t *)malloc(32448);

  //Run convolution : squeezenet0_conv14_fwd__7
  int8_t* squeezenet0_conv14_fwd__7_input_transpose = (int8_t *)VTABufferAlloc(8112);
  transpose_nhwc2vtaio(squeezenet0_conv13_fwd__2_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv14_fwd__7_input_transpose), 1, 13, 13, 48);
  int8_t* squeezenet0_conv14_fwd__7_output_bef_transpose = (int8_t *)VTABufferAlloc(32448);
  convolution_wo_tr(squeezenet0_conv14_fwd__7_input_transpose, squeezenet0_conv14_weight__1, (int32_t *)squeezenet0_conv14_bias, squeezenet0_conv14_fwd__7_output_bef_transpose, 1, 13, 13, 48, 192, 1, 1, 0, 1, 0, 1, 7, 13, 13, vtaCmdH, 1, 13, 13, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv14_fwd__7_output_bef_transpose), squeezenet0_conv14_fwd__7_res, 1, 13, 13, 192 );
  VTABufferFree(squeezenet0_conv14_fwd__7_input_transpose);
  VTABufferFree(squeezenet0_conv14_fwd__7_output_bef_transpose);

  //Run allocactivation : squeezenet0_conv15_fwd__2_res
  int8_t *squeezenet0_conv15_fwd__2_res = (int8_t *)malloc(32448);

  //Run convolution : squeezenet0_conv15_fwd__2
  int8_t* squeezenet0_conv15_fwd__2_input_transpose = (int8_t *)VTABufferAlloc(8112);
  transpose_nhwc2vtaio(squeezenet0_conv13_fwd__2_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv15_fwd__2_input_transpose), 1, 13, 13, 48);
  int8_t* squeezenet0_conv15_fwd__2_output_bef_transpose = (int8_t *)VTABufferAlloc(32448);
  convolution_wo_tr(squeezenet0_conv15_fwd__2_input_transpose, squeezenet0_conv15_weight__1, (int32_t *)squeezenet0_conv15_bias, squeezenet0_conv15_fwd__2_output_bef_transpose, 1, 13, 13, 48, 192, 3, 3, 1, 1, 0, 1, 7, 13, 13, vtaCmdH, 1, 13, 13, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv15_fwd__2_output_bef_transpose), squeezenet0_conv15_fwd__2_res, 1, 13, 13, 192 );
  VTABufferFree(squeezenet0_conv15_fwd__2_input_transpose);
  VTABufferFree(squeezenet0_conv15_fwd__2_output_bef_transpose);

  //Run deallocactivation : dealloc_squeezenet0_conv13_fwd__2_res
  free(squeezenet0_conv13_fwd__2_res);

  //Run allocactivation : squeezenet0_concat4__3
  int8_t *squeezenet0_concat4__3 = (int8_t *)malloc(64896);

  //Run touch : squeezenet0_concat4__1

  //Run inserttensor : squeezenet0_concat4__3_squeezenet0_conv14_fwd__7
  const dim_t _it8_dd[] = {1, 13, 13, 384};
  const dim_t _it8_sd[] = {1, 13, 13, 192};
  const dim_t _it8_of[] = {0, 0, 0, 0};
  insert_tensor(squeezenet0_concat4__3, squeezenet0_conv14_fwd__7_res, _it8_of, _it8_dd, _it8_sd, 4, 4, 4, 1, 3);

  //Run deallocactivation : dealloc_squeezenet0_conv14_fwd__7_res
  free(squeezenet0_conv14_fwd__7_res);

  //Run inserttensor : squeezenet0_concat4__3_squeezenet0_conv15_fwd__2
  const dim_t _it9_dd[] = {1, 13, 13, 384};
  const dim_t _it9_sd[] = {1, 13, 13, 192};
  const dim_t _it9_of[] = {0, 0, 0, 192};
  insert_tensor(squeezenet0_concat4__3, squeezenet0_conv15_fwd__2_res, _it9_of, _it9_dd, _it9_sd, 4, 4, 4, 1, 3);

  //Run deallocactivation : dealloc_squeezenet0_conv15_fwd__2_res
  free(squeezenet0_conv15_fwd__2_res);

  //Run relu : squeezenet0_relu14_fwd__2
  relu(squeezenet0_concat4__3, squeezenet0_concat4__3, 64896 );

  //Run allocactivation : squeezenet0_conv16_fwd__2_res
  int8_t *squeezenet0_conv16_fwd__2_res = (int8_t *)malloc(8112);

  //Run convolution : squeezenet0_conv16_fwd__2
  int8_t* squeezenet0_conv16_fwd__2_input_transpose = (int8_t *)VTABufferAlloc(64896);
  transpose_nhwc2vtaio(squeezenet0_concat4__3, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv16_fwd__2_input_transpose), 1, 13, 13, 384);
  int8_t* squeezenet0_conv16_fwd__2_output_bef_transpose = (int8_t *)VTABufferAlloc(8112);
  convolution_wo_tr(squeezenet0_conv16_fwd__2_input_transpose, squeezenet0_conv16_weight__1, (int32_t *)squeezenet0_conv16_bias, squeezenet0_conv16_fwd__2_output_bef_transpose, 1, 13, 13, 384, 48, 1, 1, 0, 1, 1, 1, 7, 13, 13, vtaCmdH, 1, 13, 13, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv16_fwd__2_output_bef_transpose), squeezenet0_conv16_fwd__2_res, 1, 13, 13, 48 );
  VTABufferFree(squeezenet0_conv16_fwd__2_input_transpose);
  VTABufferFree(squeezenet0_conv16_fwd__2_output_bef_transpose);

  //Run deallocactivation : dealloc_squeezenet0_concat4__3
  free(squeezenet0_concat4__3);

  //Run allocactivation : squeezenet0_conv17_fwd__7_res
  int8_t *squeezenet0_conv17_fwd__7_res = (int8_t *)malloc(32448);

  //Run convolution : squeezenet0_conv17_fwd__7
  int8_t* squeezenet0_conv17_fwd__7_input_transpose = (int8_t *)VTABufferAlloc(8112);
  transpose_nhwc2vtaio(squeezenet0_conv16_fwd__2_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv17_fwd__7_input_transpose), 1, 13, 13, 48);
  int8_t* squeezenet0_conv17_fwd__7_output_bef_transpose = (int8_t *)VTABufferAlloc(32448);
  convolution_wo_tr(squeezenet0_conv17_fwd__7_input_transpose, squeezenet0_conv17_weight__1, (int32_t *)squeezenet0_conv17_bias, squeezenet0_conv17_fwd__7_output_bef_transpose, 1, 13, 13, 48, 192, 1, 1, 0, 1, 0, 1, 7, 13, 13, vtaCmdH, 1, 13, 13, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv17_fwd__7_output_bef_transpose), squeezenet0_conv17_fwd__7_res, 1, 13, 13, 192 );
  VTABufferFree(squeezenet0_conv17_fwd__7_input_transpose);
  VTABufferFree(squeezenet0_conv17_fwd__7_output_bef_transpose);

  //Run allocactivation : squeezenet0_conv18_fwd__2_res
  int8_t *squeezenet0_conv18_fwd__2_res = (int8_t *)malloc(32448);

  //Run convolution : squeezenet0_conv18_fwd__2
  int8_t* squeezenet0_conv18_fwd__2_input_transpose = (int8_t *)VTABufferAlloc(8112);
  transpose_nhwc2vtaio(squeezenet0_conv16_fwd__2_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv18_fwd__2_input_transpose), 1, 13, 13, 48);
  int8_t* squeezenet0_conv18_fwd__2_output_bef_transpose = (int8_t *)VTABufferAlloc(32448);
  convolution_wo_tr(squeezenet0_conv18_fwd__2_input_transpose, squeezenet0_conv18_weight__1, (int32_t *)squeezenet0_conv18_bias, squeezenet0_conv18_fwd__2_output_bef_transpose, 1, 13, 13, 48, 192, 3, 3, 1, 1, 0, 1, 8, 13, 13, vtaCmdH, 1, 13, 13, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv18_fwd__2_output_bef_transpose), squeezenet0_conv18_fwd__2_res, 1, 13, 13, 192 );
  VTABufferFree(squeezenet0_conv18_fwd__2_input_transpose);
  VTABufferFree(squeezenet0_conv18_fwd__2_output_bef_transpose);

  //Run deallocactivation : dealloc_squeezenet0_conv16_fwd__2_res
  free(squeezenet0_conv16_fwd__2_res);

  //Run allocactivation : squeezenet0_concat5__3
  int8_t *squeezenet0_concat5__3 = (int8_t *)malloc(64896);

  //Run touch : squeezenet0_concat5__1

  //Run inserttensor : squeezenet0_concat5__3_squeezenet0_conv17_fwd__7
  const dim_t _it10_dd[] = {1, 13, 13, 384};
  const dim_t _it10_sd[] = {1, 13, 13, 192};
  const dim_t _it10_of[] = {0, 0, 0, 0};
  insert_tensor(squeezenet0_concat5__3, squeezenet0_conv17_fwd__7_res, _it10_of, _it10_dd, _it10_sd, 4, 4, 4, 1, 3);

  //Run deallocactivation : dealloc_squeezenet0_conv17_fwd__7_res
  free(squeezenet0_conv17_fwd__7_res);

  //Run inserttensor : squeezenet0_concat5__3_squeezenet0_conv18_fwd__2
  const dim_t _it11_dd[] = {1, 13, 13, 384};
  const dim_t _it11_sd[] = {1, 13, 13, 192};
  const dim_t _it11_of[] = {0, 0, 0, 192};
  insert_tensor(squeezenet0_concat5__3, squeezenet0_conv18_fwd__2_res, _it11_of, _it11_dd, _it11_sd, 4, 4, 4, 1, 3);

  //Run deallocactivation : dealloc_squeezenet0_conv18_fwd__2_res
  free(squeezenet0_conv18_fwd__2_res);

  //Run relu : squeezenet0_relu17_fwd__2
  relu(squeezenet0_concat5__3, squeezenet0_concat5__3, 64896 );

  //Run allocactivation : squeezenet0_conv19_fwd__2_res
  int8_t *squeezenet0_conv19_fwd__2_res = (int8_t *)malloc(10816);

  //Run convolution : squeezenet0_conv19_fwd__2
  int8_t* squeezenet0_conv19_fwd__2_input_transpose = (int8_t *)VTABufferAlloc(64896);
  transpose_nhwc2vtaio(squeezenet0_concat5__3, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv19_fwd__2_input_transpose), 1, 13, 13, 384);
  int8_t* squeezenet0_conv19_fwd__2_output_bef_transpose = (int8_t *)VTABufferAlloc(10816);
  convolution_wo_tr(squeezenet0_conv19_fwd__2_input_transpose, squeezenet0_conv19_weight__1, (int32_t *)squeezenet0_conv19_bias, squeezenet0_conv19_fwd__2_output_bef_transpose, 1, 13, 13, 384, 64, 1, 1, 0, 1, 1, 1, 7, 13, 13, vtaCmdH, 1, 13, 13, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv19_fwd__2_output_bef_transpose), squeezenet0_conv19_fwd__2_res, 1, 13, 13, 64 );
  VTABufferFree(squeezenet0_conv19_fwd__2_input_transpose);
  VTABufferFree(squeezenet0_conv19_fwd__2_output_bef_transpose);

  //Run deallocactivation : dealloc_squeezenet0_concat5__3
  free(squeezenet0_concat5__3);

  //Run allocactivation : squeezenet0_conv20_fwd__7_res
  int8_t *squeezenet0_conv20_fwd__7_res = (int8_t *)malloc(43264);

  //Run convolution : squeezenet0_conv20_fwd__7
  int8_t* squeezenet0_conv20_fwd__7_input_transpose = (int8_t *)VTABufferAlloc(10816);
  transpose_nhwc2vtaio(squeezenet0_conv19_fwd__2_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv20_fwd__7_input_transpose), 1, 13, 13, 64);
  int8_t* squeezenet0_conv20_fwd__7_output_bef_transpose = (int8_t *)VTABufferAlloc(43264);
  convolution_wo_tr(squeezenet0_conv20_fwd__7_input_transpose, squeezenet0_conv20_weight__1, (int32_t *)squeezenet0_conv20_bias, squeezenet0_conv20_fwd__7_output_bef_transpose, 1, 13, 13, 64, 256, 1, 1, 0, 1, 0, 1, 8, 13, 13, vtaCmdH, 1, 13, 13, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv20_fwd__7_output_bef_transpose), squeezenet0_conv20_fwd__7_res, 1, 13, 13, 256 );
  VTABufferFree(squeezenet0_conv20_fwd__7_input_transpose);
  VTABufferFree(squeezenet0_conv20_fwd__7_output_bef_transpose);

  //Run allocactivation : squeezenet0_conv21_fwd__2_res
  int8_t *squeezenet0_conv21_fwd__2_res = (int8_t *)malloc(43264);

  //Run convolution : squeezenet0_conv21_fwd__2
  int8_t* squeezenet0_conv21_fwd__2_input_transpose = (int8_t *)VTABufferAlloc(10816);
  transpose_nhwc2vtaio(squeezenet0_conv19_fwd__2_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv21_fwd__2_input_transpose), 1, 13, 13, 64);
  int8_t* squeezenet0_conv21_fwd__2_output_bef_transpose = (int8_t *)VTABufferAlloc(43264);
  convolution_wo_tr(squeezenet0_conv21_fwd__2_input_transpose, squeezenet0_conv21_weight__1, (int32_t *)squeezenet0_conv21_bias, squeezenet0_conv21_fwd__2_output_bef_transpose, 1, 13, 13, 64, 256, 3, 3, 1, 1, 0, 1, 8, 13, 13, vtaCmdH, 1, 13, 13, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv21_fwd__2_output_bef_transpose), squeezenet0_conv21_fwd__2_res, 1, 13, 13, 256 );
  VTABufferFree(squeezenet0_conv21_fwd__2_input_transpose);
  VTABufferFree(squeezenet0_conv21_fwd__2_output_bef_transpose);

  //Run deallocactivation : dealloc_squeezenet0_conv19_fwd__2_res
  free(squeezenet0_conv19_fwd__2_res);

  //Run allocactivation : squeezenet0_concat6__3
  int8_t *squeezenet0_concat6__3 = (int8_t *)malloc(86528);

  //Run touch : squeezenet0_concat6__1

  //Run inserttensor : squeezenet0_concat6__3_squeezenet0_conv20_fwd__7
  const dim_t _it12_dd[] = {1, 13, 13, 512};
  const dim_t _it12_sd[] = {1, 13, 13, 256};
  const dim_t _it12_of[] = {0, 0, 0, 0};
  insert_tensor(squeezenet0_concat6__3, squeezenet0_conv20_fwd__7_res, _it12_of, _it12_dd, _it12_sd, 4, 4, 4, 1, 3);

  //Run deallocactivation : dealloc_squeezenet0_conv20_fwd__7_res
  free(squeezenet0_conv20_fwd__7_res);

  //Run inserttensor : squeezenet0_concat6__3_squeezenet0_conv21_fwd__2
  const dim_t _it13_dd[] = {1, 13, 13, 512};
  const dim_t _it13_sd[] = {1, 13, 13, 256};
  const dim_t _it13_of[] = {0, 0, 0, 256};
  insert_tensor(squeezenet0_concat6__3, squeezenet0_conv21_fwd__2_res, _it13_of, _it13_dd, _it13_sd, 4, 4, 4, 1, 3);

  //Run deallocactivation : dealloc_squeezenet0_conv21_fwd__2_res
  free(squeezenet0_conv21_fwd__2_res);

  //Run relu : squeezenet0_relu20_fwd__2
  relu(squeezenet0_concat6__3, squeezenet0_concat6__3, 86528 );

  //Run allocactivation : squeezenet0_conv22_fwd__2_res
  int8_t *squeezenet0_conv22_fwd__2_res = (int8_t *)malloc(10816);

  //Run convolution : squeezenet0_conv22_fwd__2
  int8_t* squeezenet0_conv22_fwd__2_input_transpose = (int8_t *)VTABufferAlloc(86528);
  transpose_nhwc2vtaio(squeezenet0_concat6__3, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv22_fwd__2_input_transpose), 1, 13, 13, 512);
  int8_t* squeezenet0_conv22_fwd__2_output_bef_transpose = (int8_t *)VTABufferAlloc(10816);
  convolution_wo_tr(squeezenet0_conv22_fwd__2_input_transpose, squeezenet0_conv22_weight__1, (int32_t *)squeezenet0_conv22_bias, squeezenet0_conv22_fwd__2_output_bef_transpose, 1, 13, 13, 512, 64, 1, 1, 0, 1, 1, 1, 8, 13, 13, vtaCmdH, 1, 13, 13, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv22_fwd__2_output_bef_transpose), squeezenet0_conv22_fwd__2_res, 1, 13, 13, 64 );
  VTABufferFree(squeezenet0_conv22_fwd__2_input_transpose);
  VTABufferFree(squeezenet0_conv22_fwd__2_output_bef_transpose);

  //Run deallocactivation : dealloc_squeezenet0_concat6__3
  free(squeezenet0_concat6__3);

  //Run allocactivation : squeezenet0_conv23_fwd__7_res
  int8_t *squeezenet0_conv23_fwd__7_res = (int8_t *)malloc(43264);

  //Run convolution : squeezenet0_conv23_fwd__7
  int8_t* squeezenet0_conv23_fwd__7_input_transpose = (int8_t *)VTABufferAlloc(10816);
  transpose_nhwc2vtaio(squeezenet0_conv22_fwd__2_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv23_fwd__7_input_transpose), 1, 13, 13, 64);
  int8_t* squeezenet0_conv23_fwd__7_output_bef_transpose = (int8_t *)VTABufferAlloc(43264);
  convolution_wo_tr(squeezenet0_conv23_fwd__7_input_transpose, squeezenet0_conv23_weight__1, (int32_t *)squeezenet0_conv23_bias, squeezenet0_conv23_fwd__7_output_bef_transpose, 1, 13, 13, 64, 256, 1, 1, 0, 1, 0, 1, 9, 13, 13, vtaCmdH, 1, 13, 13, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv23_fwd__7_output_bef_transpose), squeezenet0_conv23_fwd__7_res, 1, 13, 13, 256 );
  VTABufferFree(squeezenet0_conv23_fwd__7_input_transpose);
  VTABufferFree(squeezenet0_conv23_fwd__7_output_bef_transpose);

  //Run allocactivation : squeezenet0_conv24_fwd__2_res
  int8_t *squeezenet0_conv24_fwd__2_res = (int8_t *)malloc(43264);

  //Run convolution : squeezenet0_conv24_fwd__2
  int8_t* squeezenet0_conv24_fwd__2_input_transpose = (int8_t *)VTABufferAlloc(10816);
  transpose_nhwc2vtaio(squeezenet0_conv22_fwd__2_res, (int8_t* )VTABufferGetVirtAddr(squeezenet0_conv24_fwd__2_input_transpose), 1, 13, 13, 64);
  int8_t* squeezenet0_conv24_fwd__2_output_bef_transpose = (int8_t *)VTABufferAlloc(43264);
  convolution_wo_tr(squeezenet0_conv24_fwd__2_input_transpose, squeezenet0_conv24_weight__1, (int32_t *)squeezenet0_conv24_bias, squeezenet0_conv24_fwd__2_output_bef_transpose, 1, 13, 13, 64, 256, 3, 3, 1, 1, 0, 1, 10, 13, 13, vtaCmdH, 1, 13, 13, 0);
  transpose_vtaio2nhwc((int8_t* )VTABufferGetVirtAddr(squeezenet0_conv24_fwd__2_output_bef_transpose), squeezenet0_conv24_fwd__2_res, 1, 13, 13, 256 );
  VTABufferFree(squeezenet0_conv24_fwd__2_input_transpose);
  VTABufferFree(squeezenet0_conv24_fwd__2_output_bef_transpose);

  //Run deallocactivation : dealloc_squeezenet0_conv22_fwd__2_res
  free(squeezenet0_conv22_fwd__2_res);

  //Run allocactivation : squeezenet0_concat7__3
  int8_t *squeezenet0_concat7__3 = (int8_t *)malloc(86528);

  //Run touch : squeezenet0_concat7__1

  //Run inserttensor : squeezenet0_concat7__3_squeezenet0_conv23_fwd__7
  const dim_t _it14_dd[] = {1, 13, 13, 512};
  const dim_t _it14_sd[] = {1, 13, 13, 256};
  const dim_t _it14_of[] = {0, 0, 0, 0};
  insert_tensor(squeezenet0_concat7__3, squeezenet0_conv23_fwd__7_res, _it14_of, _it14_dd, _it14_sd, 4, 4, 4, 1, 3);

  //Run deallocactivation : dealloc_squeezenet0_conv23_fwd__7_res
  free(squeezenet0_conv23_fwd__7_res);

  //Run inserttensor : squeezenet0_concat7__3_squeezenet0_conv24_fwd__2
  const dim_t _it15_dd[] = {1, 13, 13, 512};
  const dim_t _it15_sd[] = {1, 13, 13, 256};
  const dim_t _it15_of[] = {0, 0, 0, 256};
  insert_tensor(squeezenet0_concat7__3, squeezenet0_conv24_fwd__2_res, _it15_of, _it15_dd, _it15_sd, 4, 4, 4, 1, 3);

  //Run deallocactivation : dealloc_squeezenet0_conv24_fwd__2_res
  free(squeezenet0_conv24_fwd__2_res);

  //Run allocactivation : squeezenet0_relu23_fwd__2_res
  int8_t *squeezenet0_relu23_fwd__2_res = (int8_t *)malloc(86528);

  //Run relu : squeezenet0_relu23_fwd__2
  relu(squeezenet0_concat7__3, squeezenet0_relu23_fwd__2_res, 86528, 8.000000, 4.000000 );

  //Run deallocactivation : dealloc_squeezenet0_concat7__3
  free(squeezenet0_concat7__3);

  //Run allocactivation : squeezenet0_conv25_fwd__2_res
  int8_t *squeezenet0_conv25_fwd__2_res = (int8_t *)malloc(169000);

  //Run convolution : squeezenet0_conv25_fwd__2
  nonvtaconvolution(squeezenet0_relu23_fwd__2_res, 1.0/0.250000, 0, (int8_t *)VTABufferGetVirtAddr(squeezenet0_conv25_weight__1), 1.0/512.000000, 0, (int8_t *)VTABufferGetVirtAddr(squeezenet0_conv25_bias), 1.0/128.000000, 0, squeezenet0_conv25_fwd__2_res, 1.0/1.000000, 0, 1, 13, 13, 512, 1000, 1, 1, 0, 1, 1, 1, 1, 1, 13, 13 );

  //Run deallocactivation : dealloc_squeezenet0_relu23_fwd__2_res
  free(squeezenet0_relu23_fwd__2_res);

  //Run allocactivation : squeezenet0_pool3_fwd__1_res
  int8_t *squeezenet0_pool3_fwd__1_res = (int8_t *)malloc(1000);

  //Run avgpool : squeezenet0_pool3_fwd__1
  avgpool(squeezenet0_conv25_fwd__2_res, 1.0/1.000000, 0, squeezenet0_pool3_fwd__1_res, 1.0/4.000000, 0, 1, 13, 13, 1000, 1, 1, 1, 1000, 13, 13, 0, 13 );

  //Run deallocactivation : dealloc_squeezenet0_conv25_fwd__2_res
  free(squeezenet0_conv25_fwd__2_res);

  //Run tensorview : squeezenet0_flatten0_reshape0__2
  int8_t* squeezenet0_flatten0_reshape0__1 = (int8_t*)mutableWeight + 602112;
  int8_t* squeezenet0_flatten0_reshape0__2 = squeezenet0_flatten0_reshape0__1;

  //Run dequantize : squeezenet0_flatten0_reshape0__2_dequantize__1
  dequantize(squeezenet0_pool3_fwd__1_res, squeezenet0_flatten0_reshape0__2, 1000, 1/4.000000, 0 );

  //Run deallocactivation : dealloc_squeezenet0_pool3_fwd__1_res
  free(squeezenet0_pool3_fwd__1_res);
  return 0;
}