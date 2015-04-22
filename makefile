all:H264_RTP2
CC=arm-linux-gcc

LD=arm-linux-ld
#CFLAGS=	-c -Wall -std=gnu99
#INCS= -I/opt/ortp/include -L/opt/ortp/lib 
CXXFLAGS=	-c -std=gnu99  -I/opt/ortp/include -L/opt/ortp/lib 
OBJS_SERVER=	SsbSipMfcDecAPI.o SsbSipMfcEncAPI.o H264EncTest.o  rtpsend.o Encoder.o capture.o
LIBS_SERVER=	-lortp


%.o:%.c
	$(CC)  $(CXXFLAGS) $< -o $@  
	

H264_RTP2: $(OBJS_SERVER)
	$(CC) -o $@ -L/opt/ortp/lib $^    $(LIBS_SERVER)

clean:
	rm -f *.o
	rm -f H264_RTP2
