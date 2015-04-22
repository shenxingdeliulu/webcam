#ifndef CAPTURE_H
#define CAPTURE_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h> 

#include <fcntl.h> 
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h> 
#include <linux/videodev2.h>
#include "Encoder.h"
struct buffer {
        void * start;
        size_t length;
};

unsigned char *nv12buffer;
int  fd ;
 void capture_init();
 int read_frame();
int yuv2nv12(const void* buffer, size_t size, size_t count,unsigned char *nvbuffer);
void close_cap();
#endif