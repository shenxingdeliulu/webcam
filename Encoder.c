
#include "Encoder.h"
#include "rtpsend.h"



int H264_Encode_init(FILE *stream)
{
  int err;
  unsigned int buf_type=NO_CACHE;
   openHandle = SsbSipMfcEncOpen(&buf_type);
    if(openHandle == NULL) {
        fprintf(stderr,"Error: SsbSipMfcEncOpen\n");
        return 0;
       
    }else {
        printf("MfcEncOpen succeeded\n");
    }
    
    param=malloc(sizeof(*param));
    if(param==NULL) {
        fprintf(stderr,"Error: malloc param\n");
        return 0;
    }
    memset(param,0,sizeof(*param));
     param->codecType = H264_ENC;
     param->SourceWidth = 640;
    param->SourceHeight = 480;
    param->IDRPeriod = 100;
    param->SliceMode = 0; // 0,1,2,4
    param->RandomIntraMBRefresh = 0;
    param->EnableFRMRateControl = 1; // this must be 1 otherwise init error
    param->Bitrate = 9216000;
    param->FrameQp = 30; //<=51, the bigger the lower quality
    param->FrameQp_P = 20;
    param->QSCodeMin = 10; // <=51
    param->QSCodeMax = 51; // <=51
    param->CBRPeriodRf = 120;
    param->PadControlOn = 0;
    param->LumaPadVal = 0; // <=255
    param->CbPadVal = 0; //<=255
    param->CrPadVal = 0; //<=255
    param->FrameMap = 0; // encoding input mode (0=linear, 1=tiled) 
     param->ProfileIDC = 1; // 0=main,1=high,2=baseline
    param->LevelIDC = 40; // level 4.0
    param->FrameQp_B = 10;
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
    err = SsbSipMfcEncInit(openHandle,param);
    if(err<0) {
        fprintf(stderr,"Error: SsbSipMfcEncInit. Code %d\n",err);
      return 0;
    }else {
        printf("SsbSipMfcEncInit succeeded\n");
    }
      err = SsbSipMfcEncGetOutBuf(openHandle,&oinfo);
    if(err<0) {
        fprintf(stderr,"Error: SsbSipMfcEncGetOutBuf. Code %d\n",err);
        return 0;
    }else {
        printf("SsbSipMfcEncGetOutBuf suceeded\n");
        fwrite(oinfo.StrmVirAddr,1,oinfo.headerSize,stream);
    }
     memset(&iinfo,0,sizeof(iinfo));
    err = SsbSipMfcEncGetInBuf(openHandle,&iinfo);
    if(err<0) {
        fprintf(stderr,"Error: SsbSipMfcEncGetInBuf. Code %d\n",err);
       return 0;
    }else {
        printf("SsbSipMfcEncGetInBuf succeeded\n");
    }
    return 1;
}

void H264_Encode(const void* buffer, FILE* stream)
{
  memcpy(iinfo.YVirAddr,buffer,w*h);
  memcpy(iinfo.CVirAddr,buffer+w*h,w*h/2);
  
   err = SsbSipMfcEncSetInBuf(openHandle,&iinfo);
        if(err<0) {
            fprintf(stderr,"Error: SsbSipMfcEncSetInBuf. Code %d\n",err);
           
        }
         err = SsbSipMfcEncExe(openHandle);
        if(err<0) {
            fprintf(stderr,"Error: SsbSipMfcEncExe. Code %d\n",err);
           
        }
        
        memset(&oinfo,0,sizeof(oinfo));
        err = SsbSipMfcEncGetOutBuf(openHandle,&oinfo);
        if(err<0) {
            fprintf(stderr,"Error: SsbSipMfcEncGetOutBuf. Code %d\n",err);
            
        }
      
        //printf("oinfo.dataSize=%d",oinfo.dataSize);
        //fwrite(oinfo.StrmVirAddr,1,oinfo.dataSize,stream);

       // rtpSend(pRtpSession,oinfo.StrmVirAddr,oinfo.dataSize);

      
}
 

 void Close_Encode()
 {

    SsbSipMfcEncClose(openHandle);
    free(param);
 }