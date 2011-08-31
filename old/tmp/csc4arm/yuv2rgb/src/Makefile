TEST=1

OBJS := __ABGR8888toYUV422.o __ARGB8888toYUV422.o __BGR888toYUV422.o __RGB888toYUV422.o __RGBA8888toYUV422.o

OBJS += __UYVYtoBGR888.o __YUV420PtoBGR888.o

ifdef TEST
 OBJS += __BGR888toYUV422_C.o
endif

CFLAGS := -O2 -march=armv5te -mtune=arm926ej-s -I.

.S.o:
	$(CC) $(CFLAGS) -c $<

all: libcsc-armv5te.a 

libcsc-armv5te.a: $(OBJS)
	ar rcu $@ $^

clean:
	rm -f *.o libcsc-armv5te.a
