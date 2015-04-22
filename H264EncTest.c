#include "capture.h"
#include "rtpsend.h"

#define w 640
#define h 480
#define TARGET_IP "192.168.1.109"
#define TARGET_PORT 5000
#define VIDEO_FPS 25.0




FILE *fp_h264;



RtpSession * pRtpSession = NULL; 
FILE*fp_h264;

//////////////////////////////////////////////////////
//获取一帧数据
//////////////////////////////////////////////////////







int main (int argc,char ** argv)
{   
pRtpSession = rtpInit(TARGET_IP,TARGET_PORT); 
 if(pRtpSession==NULL)    
     {    
         printf("error rtpInit");    
         return 0;    
     }    
 fp_h264=fopen("test.h264","wb");
 capture_init();
 if(!H264_Encode_init(fp_h264))
  printf("H264Encode_init fail");

while(1)
{
for (;;) //这一段涉及到异步IO
{
   fd_set fds;
   struct timeval tv;
   int r;

   FD_ZERO (&fds);//将指定的文件描述符集清空
   FD_SET (fd, &fds);//在文件描述符集合中增加一个新的文件描述符

   /* Timeout. */
   tv.tv_sec = 2;
   tv.tv_usec = 0;

   r = select (fd + 1, &fds, NULL, NULL, &tv);//判断是否可读（即摄像头是否准备好），tv是定时

   if (-1 == r) {
    if (EINTR == errno)
     continue;
    printf ("select err\n");
                        }
   if (0 == r) {
    fprintf (stderr, "select timeout\n");
    exit (EXIT_FAILURE);
                        }

   if(read_frame ())
  {
    H264_Encode(nv12buffer,fp_h264);
    rtpSend(pRtpSession,oinfo.StrmVirAddr,oinfo.dataSize);

  }
      break;//如果可读，执行read_frame ()函数，并跳出循环
  
}

} 


  
Close_Encode(); 
close_cap();
//free(param);
//fclose(fp_nv12);
 fclose(fp_h264);
exit (EXIT_SUCCESS);
 rtpExit(pRtpSession);    
return 0;
}
