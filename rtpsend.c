#include "rtpsend.h"

RtpSession * rtpInit(char * ipStr,int port)    
  {    
     // Rtp会话对象     
     RtpSession *session;    
      char *ssrc;    
          
      // 时间戳初始化     
      g_user_ts = 0;    
     
      // ortp的一些基本初始化操作    
      ortp_init();    
      ortp_scheduler_init();    
      // 创建新的rtp会话对象   
      session=rtp_session_new(RTP_SESSION_SENDONLY);      
          
      rtp_session_set_scheduling_mode(session,1);    
      rtp_session_set_blocking_mode(session,1);    
      // 设置远程RTP客户端的的IP和监听端口（即本rtp数据包的发送目的地址）    
      rtp_session_set_remote_addr(session,ipStr,port);    
      // 设置负载类型     
    
       rtp_session_set_payload_type(session, 96);   
      // 获取同68    步源标69   识     
      ssrc=getenv("SSRC");    
      if (ssrc!=NULL)     
      {    
          printf("using SSRC=%i.\n",atoi(ssrc));    
          rtp_session_set_ssrc(session,atoi(ssrc));    
      }    
          
      return session;    
  }    
     
  /**  发送rtp数据包     
   *       
   *   主要用于发送rtp数据包     
   *   @param:  RtpSession *session RTP会话对象的指85   针     
   *   @param:  const char *buffer 要发送的数据的缓冲区地址      
   *   @param: int len 要发送的数据长度     
   *   @return:  int 实际发送的数据包数目     
   *   @note:     如果要发送的数据包长度大于BYTES_PER_COUNT，90 本函数内部会进行分包处理     
   */   
/* int rtpSend(RtpSession *session,const char *buffer, int len)    
  {    
      int curOffset = 0;    
      int sendBytes = 0;    
      int clockslide=500;     
  
   
     
     ortp_message("send data len %i\n ",len);    
    
     // 是否全部发送完毕     
     while(curOffset < len )    
     {    
     // 如果需要发送的数据长度小于等于阙值，106    则直接发送   
         if( len <= BYTES_PER_COUNT )    
         {    
             sendBytes = len;    
         }    
         else   
         {    
            // 如果当前偏移 + 阈值 小于等于 总长度，114 则发送阈值大小的数据   
             if( curOffset + BYTES_PER_COUNT <= len )    
             {    
                 sendBytes = BYTES_PER_COUNT;    
             }    
             // 否则就发送剩余长度的数据   
            else   
             {    
                 sendBytes = len - curOffset;    
             }    
         }    
             
         ortp_message("send data bytes %i\n ",sendBytes);    
             
         rtp_session_send_with_ts(session,(char *)(buffer+curOffset),sendBytes,g_user_ts);    
             
         // 累加   
       
         curOffset += sendBytes;                     
         g_user_ts += TIME_STAMP_INC;    
         
         // 发送一定数据包后休眠一会   
           
    }    
     return 0;    
 }  */

 int  rtpSend(RtpSession *session, unsigned char  *buffer,  int  len)  
{    
    int  sendBytes = 0;   
    int status;         
    uint32_t valid_len=len-4;  
    unsigned char NALU=buffer[4];  
       
    //printf("send len=%d\n",len);  
  
    //如果数据小于MAX_RTP_PKT_LENGTH字节，直接发送：单一NAL单元模式  
    if(valid_len <= MAX_RTP_PKT_LENGTH)  
    {  
        sendBytes = rtp_session_send_with_ts(session,  
                                             &buffer[4],  
                                             valid_len,  
                                             g_user_ts);
       
        return sendBytes;  
    }  
    else if (valid_len > MAX_RTP_PKT_LENGTH)  
    {  
        //切分为很多个包发送，每个包前要对头进行处理，如第一个包  
        valid_len -= 1;  
        int k=0,l=0;  
        k=valid_len/MAX_RTP_PKT_LENGTH;  
        l=valid_len%MAX_RTP_PKT_LENGTH;  
        int t=0;  
        int pos=5;  
        if(l!=0)  
        {  
            k=k+1;  
        }  
        while(t<k)//||(t==k&&l>0))  
        {  
            if(t<(k-1))//(t<k&&l!=0)||(t<(k-1))&&(l==0))//(0==t)||(t<k&&0!=l))  
            {  
                buffer[pos-2]=(NALU & 0x60)|28;  
                buffer[pos-1]=(NALU & 0x1f);  
                if(0==t)  
                {  
                    buffer[pos-1]|=0x80;  
                }  
                sendBytes = rtp_session_send_with_ts(session,  
                                                     &buffer[pos-2],  
                                                     MAX_RTP_PKT_LENGTH+2,  
                                                     g_user_ts);
                 
                t++;  
                pos+=MAX_RTP_PKT_LENGTH;  
            }  
            else //if((k==t&&l>0)||((t==k-1)&&l==0))  
            {  
                int iSendLen;  
                if(l>0)  
                {  
                    iSendLen=valid_len-t*MAX_RTP_PKT_LENGTH;  
                }  
                else  
                    iSendLen=MAX_RTP_PKT_LENGTH;  
                buffer[pos-2]=(NALU & 0x60)|28;  
                buffer[pos-1]=(NALU & 0x1f);  
                buffer[pos-1]|=0x40;  
                sendBytes = rtp_session_send_with_ts(session,  
                                                     &buffer[pos-2],  
                                                     iSendLen+2,  
                                                     g_user_ts); 
                 
                t++;  
            }  
        }  
    }  
  
    g_user_ts += DefaultTimestampIncrement;//timestamp increase  
    return  len; 
    }  
    
 /**  结束ortp的发送，145  释放资源     
  *      
  *   @param:  RtpSession *session RTP会话对象的指148  针     
  *   @return:  0表示成功     
  *   @note:         
  */   
 int rtpExit(RtpSession *session)    
 {    
     g_user_ts = 0;    
         
     rtp_session_destroy(session);    
     ortp_exit();    
     ortp_global_stats_display();    
    
     return 0;    
 }  