#ifndef ENCODER_H
#define ENCODER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>
//#include <linux/videodev2.h>
#include "mfc_interface.h"


#define w 640
#define h 480


void *openHandle;

SSBSIP_MFC_ENC_H264_PARAM *param;
SSBSIP_MFC_ERROR_CODE err;
SSBSIP_MFC_ENC_INPUT_INFO iinfo;
SSBSIP_MFC_ENC_OUTPUT_INFO oinfo;


int H264_Encode_init(FILE *stream);
void H264_Encode(const void* buffer, FILE* stream);

void Close_Encode();






#endif