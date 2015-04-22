#include "capture.h"
#define CLEAR(x) memset (&(x), 0, sizeof (x))
static char * dev_name = "/dev/video0";//摄像头设备名

struct buffer * buffers = NULL;

static unsigned int n_buffers = 0;
unsigned char *yuyv;
const int yuyv_frm_sz=w*h*2;
static unsigned long file_length;
 int yuv2nv12(const void* buffer, size_t size, size_t count,unsigned char *nvbuffer)
{
  
   int i, r,j;
  memcpy(yuyv,buffer,size);
    
        for(i=0,j=0;i<size;i+=2,j++)
        {
            //putc(yuyv[i],stream);
            nvbuffer[j]=yuyv[i];
          }
        for(r=0;r<h;r+=2) {
            for(i=1;i<w*2;i+=2)
            {
               // putc(yuyv[r*w*2+i],stream);
                nvbuffer[j]=yuyv[r*w*2+i];
                j++;
              }
        }
    
    return 1;
}

 int read_frame (void)
{
struct v4l2_buffer buf;
unsigned int i;

CLEAR (buf);
buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
buf.memory = V4L2_MEMORY_MMAP;

ioctl (fd, VIDIOC_DQBUF, &buf); //出列采集的帧缓冲

assert (buf.index < n_buffers);
   printf ("buf.index dq is %d,\n",buf.index);

//fwrite(buffers[buf.index].start, buffers[buf.index].length, 1, file_fd); //将其写入文件中
if(yuv2nv12(buffers[buf.index].start, buffers[buf.index].length,1, nv12buffer))

printf("yuv2nv12success\n");
//H264_Encode(nv12buffer,fp_h264);
//rtpSend(pRtpSession,oinfo.StrmVirAddr,oinfo.dataSize);

ioctl (fd, VIDIOC_QBUF, &buf); //再将其入列

return 1;
}



void capture_init()
{
	struct v4l2_capability cap;
struct v4l2_format fmt;
unsigned int i;
enum v4l2_buf_type type;
fd = open (dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);//打开设备
ioctl (fd, VIDIOC_QUERYCAP, &cap);//获取摄像头参数

CLEAR (fmt);
fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
fmt.fmt.pix.width = 640;
fmt.fmt.pix.height = 480;
fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
ioctl (fd, VIDIOC_S_FMT, &fmt); //设置图像格式

file_length = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height; //计算图片大小

struct v4l2_requestbuffers req;
CLEAR (req);
req.count = 4;
req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
req.memory = V4L2_MEMORY_MMAP;

ioctl (fd, VIDIOC_REQBUFS, &req); //申请缓冲，count是申请的数量

if (req.count < 2)
   printf("Insufficient buffer memory\n");
buffers = calloc (req.count, sizeof (*buffers));//内存中建立对应空间
nv12buffer=malloc (yuyv_frm_sz);
yuyv=malloc (yuyv_frm_sz);
for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
{
   struct v4l2_buffer buf; //驱动中的一帧
   CLEAR (buf);
   buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   buf.memory = V4L2_MEMORY_MMAP;
   buf.index = n_buffers;

   if (-1 == ioctl (fd, VIDIOC_QUERYBUF, &buf)) //映射用户空间
    printf ("VIDIOC_QUERYBUF error\n");

   buffers[n_buffers].length = buf.length;
   buffers[n_buffers].start =
   mmap (NULL /* start anywhere */, //通过mmap建立映射关系
    buf.length,
    PROT_READ | PROT_WRITE /* required */,
    MAP_SHARED /* recommended */,
    fd, buf.m.offset);

   if (MAP_FAILED == buffers[n_buffers].start)
    printf ("mmap failed\n");
        }

for (i = 0; i < n_buffers; ++i)
{
   struct v4l2_buffer buf;
   CLEAR (buf);

   buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   buf.memory = V4L2_MEMORY_MMAP;
   buf.index = i;

   if (-1 == ioctl (fd, VIDIOC_QBUF, &buf))//申请到的缓冲进入列队
    printf ("VIDIOC_QBUF failed\n");
}
               
type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

if (-1 == ioctl (fd, VIDIOC_STREAMON, &type)) //开始捕捉图像数据
   printf ("VIDIOC_STREAMON failed\n");
}

void close_cap()
{
	close(fd);
	free(yuyv);
	free(nv12buffer);
}
