
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>

#include "mfc_interface.h"

#define TEST_H264
//#define TEST_H263
//#define TEST_MPEG4

int main()
{
    unsigned int buf_type = NO_CACHE;
    void *openHandle;
#if defined TEST_H264
    SSBSIP_MFC_ENC_H264_PARAM *param;
#elif defined TEST_H263
    SSBSIP_MFC_ENC_H263_PARAM *param;
#else
    SSBSIP_MFC_ENC_MPEG4_PARAM *param;
#endif
    
    SSBSIP_MFC_ERROR_CODE err;
    SSBSIP_MFC_ENC_INPUT_INFO iinfo;
    SSBSIP_MFC_ENC_OUTPUT_INFO oinfo;
    
    FILE *fp_nv12, *fp_strm;
    
    int retv = 0;
    
    fp_nv12 = fopen("test.nv12","rb");
    if(fp_nv12==NULL) {
        fprintf(stderr,"Error: open test.nv12\n");
        retv = 1;
        goto exit_end;
    }
#if defined TEST_H264
    fp_strm = fopen("test.h264","wb");
#elif defined TEST_H263
    fp_strm = fopen("test.h263","wb");
#else
    fp_strm = fopen("test.mpeg4","wb");
#endif
    if(fp_strm==NULL) {
        fprintf(stderr,"Error: open output file\n");
        retv = 1;
        goto exit_fp_nv12;
    }
    
    openHandle = SsbSipMfcEncOpen(&buf_type);
    if(openHandle == NULL) {
        fprintf(stderr,"Error: SsbSipMfcEncOpen\n");
        retv = 1;
        goto exit_fp_strm;
    }else {
        printf("MfcEncOpen succeeded\n");
    }
    
    param=malloc(sizeof(*param));
    if(param==NULL) {
        fprintf(stderr,"Error: malloc param\n");
        retv = 1;
        goto exit_handle;
    }
    memset(param,0,sizeof(*param));
    
    //common parameters
#if defined TEST_H264
    param->codecType = H264_ENC;
#elif defined TEST_H263
    param->codecType = H263_ENC;
#else
    param->codecType = MPEG4_ENC;
#endif
    param->SourceWidth = 640;
    param->SourceHeight = 480;
    param->IDRPeriod = 100;
    param->SliceMode = 0; // 0,1,2,4
    param->RandomIntraMBRefresh = 0;
    param->EnableFRMRateControl = 1; // this must be 1 otherwise init error
    param->Bitrate = 128000;
    param->FrameQp = 20; //<=51, the bigger the lower quality
    param->FrameQp_P = 20;
    param->QSCodeMin = 10; // <=51
    param->QSCodeMax = 51; // <=51
    param->CBRPeriodRf = 120;
    param->PadControlOn = 0;
    param->LumaPadVal = 0; // <=255
    param->CbPadVal = 0; //<=255
    param->CrPadVal = 0; //<=255
    param->FrameMap = 0; // encoding input mode (0=linear, 1=tiled) 
    
#if defined TEST_H264
    // H264 specific
    param->ProfileIDC = 1; // 0=main,1=high,2=baseline
    param->LevelIDC = 40; // level 4.0
    param->FrameQp_B = 20;
    param->FrameRate = 30000; // real frame rate = FrameRate/1000 (refer to S5PV210 datasheet Section 6.3.4.2.2)
    param->SliceArgument = 0;
    param->NumberBFrames = 0; //<=2
    param->NumberReferenceFrames = 2; // <=2
    param->NumberRefForPframes = 2; // <=2
    param->LoopFilterDisable = 1; // 0=enable, 1=disable
    param->LoopFilterAlphaC0Offset = 0; // <=6
    param->LoopFilterBetaOffset = 0; // <=6
    param->SymbolMode = 1; // 0=CAVLC, 1=CABAC
    param->PictureInterlace = 0; // Picture AFF 0=frame coding, 1=field coding, 2=adaptive
    param->Transform8x8Mode = 1; // 0=only 4x4 transform, 1=allow 8x8 trans, 2=only 8x8 trans
    param->EnableMBRateControl = 0;
    param->DarkDisable = 0;
    param->SmoothDisable = 0;
    param->StaticDisable = 0;
    param->ActivityDisable = 0;
#elif defined TEST_H263
    // H263 specific
    param->FrameRate = 30000;
#else
    // MPEG4 specific
    param->ProfileIDC = 0; // 0=main,1=high,2=baseline
    param->LevelIDC = 40; // level 4.0
    param->FrameQp_B = 20;
    param->TimeIncreamentRes = 0;
    param->VopTimeIncreament = 0;
    param->SliceArgument = 0;
    param->NumberBFrames = 0; //<=2
    param->DisableQpelME = 0;
#endif
    
    err = SsbSipMfcEncInit(openHandle,param);
    if(err<0) {
        fprintf(stderr,"Error: SsbSipMfcEncInit. Code %d\n",err);
        retv = 1;
        goto exit_param;
    }else {
        printf("SsbSipMfcEncInit succeeded\n");
    }
    
#ifndef TEST_H263
    err = SsbSipMfcEncGetOutBuf(openHandle,&oinfo);
    if(err<0) {
        fprintf(stderr,"Error: SsbSipMfcEncGetOutBuf. Code %d\n",err);
        retv =1;
        goto exit_param;
    }else {
        printf("SsbSipMfcEncGetOutBuf suceeded\n");
        fwrite(oinfo.StrmVirAddr,1,oinfo.headerSize,fp_strm);
    }
#endif

    memset(&iinfo,0,sizeof(iinfo));
    err = SsbSipMfcEncGetInBuf(openHandle,&iinfo);
    if(err<0) {
        fprintf(stderr,"Error: SsbSipMfcEncGetInBuf. Code %d\n",err);
        retv = 1;
        goto exit_param;
    }else {
        printf("SsbSipMfcEncGetInBuf succeeded\n");
    }

    
    int w=param->SourceWidth;
    int h=param->SourceHeight;
    int frmcnt = 0;
    while(fread(iinfo.YVirAddr,1,w*h,fp_nv12)==w*h && fread(iinfo.CVirAddr,1,w*h/2,fp_nv12)==w*h/2) {
        err = SsbSipMfcEncSetInBuf(openHandle,&iinfo);
        if(err<0) {
            fprintf(stderr,"Error: SsbSipMfcEncSetInBuf. Code %d\n",err);
            retv = 1;
            goto exit_param;
        }
#if 0
        err = SsbSipMfcEncSetOutBuf(openHandle,phyOutbuf,virOutbuf,outbufSize);
        if(err<0) {
            fprintf(stderr,"Error: SsbSipMfcEncSetOutBuf. Code %d\n",err);
            retv = 1;
            goto exit_param;
        }
#endif
        err = SsbSipMfcEncExe(openHandle);
        if(err<0) {
            fprintf(stderr,"Error: SsbSipMfcEncExe. Code %d\n",err);
            retv = 1;
            goto exit_param;
        }
        
        memset(&oinfo,0,sizeof(oinfo));
        err = SsbSipMfcEncGetOutBuf(openHandle,&oinfo);
        if(err<0) {
            fprintf(stderr,"Error: SsbSipMfcEncGetOutBuf. Code %d\n",err);
            retv = 1;
            goto exit_param;
        }
        
        fwrite(oinfo.StrmVirAddr,1,oinfo.dataSize,fp_strm);
        //printf("oinfo.StrmVirAddr=0x%x, oinfo.dataSize=%d.\n",(unsigned)oinfo.StrmVirAddr,oinfo.dataSize);
        //printf("Frame # %d encoded\n", frmcnt++);
    }
    
    // clear up
exit_param:
    free(param);
    
exit_handle:
    err = SsbSipMfcEncClose(openHandle);
    if(err<0) {
        fprintf(stderr,"Error: SsbSipMfcEncClose. Code %d\n",err);
    }
    
exit_fp_strm:
    fclose(fp_strm);
    
exit_fp_nv12:
    fclose(fp_nv12);
    
exit_end:
    return retv;
}

