#!/bin/bash

export NDK=/Users/apple/Downloads/android-ndk-r8/
PREBUILT="$NDK/toolchains/x86-4.4.3/prebuilt/darwin-x86" 
PLATFORM="$NDK/platforms/android-9/arch-x86"

GCC=i686-android-linux-gcc
CROSPREFIX=i686-android-linux-
NM=i686-android-linux-nm
ELF=$PREBUILT/i686-android-linux/lib/ldscripts/elf_i386.x
ARCH=x86
CRTBEGIN=$PREBUILT/lib/gcc/i686-android-linux/4.4.3/crtbegin.o
CRTEND=$PREBUILT/lib/gcc/i686-android-linux/4.4.3/crtend.o
CPUDIR=x86
PREFIX=./android/$CPUDIR
OPTIMIZE_CFLAGS=" "

BZLIBPATH="/Users/apple/Downloads/dolphin_player/dolphin-player/p/jni/bzip2/"
BZLIB_LDPATHX86="/Users/apple/Downloads/dolphin_player/dolphin-player/p/jni/bzip2/x86"
BZLIB_LDPATH=$BZLIB_LDPATHX86

./configure --target-os=linux \
    --prefix=$PREFIX \
    --enable-cross-compile \
    --disable-yasm \
    --disable-asm \
    --arch=$ARCH \
    --cc=$PREBUILT/bin/$GCC \
    --cross-prefix=$PREBUILT/bin/$CROSPREFIX \
    --nm=$PREBUILT/bin/$NM \
    --extra-cflags=" -O3 -fpic -DANDROID -DHAVE_SYS_UIO_H=1 -Dipv6mr_interface=ipv6mr_ifindex -fasm -Wno-psabi -fno-short-enums -fno-strict-aliasing -finline-limit=300 $OPTIMIZE_CFLAGS -I$BZLIBPATH" \
    --extra-ldflags="-Wl,-T,$ELF -Wl,-rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib -nostdlib $CRTBEGIN $CRTEND -lc -lm -ldl -llog -L$BZLIB_LDPATH" \
    --enable-gpl \
    --enable-version3 \
    --disable-shared \
    --enable-static \
    --sysroot=$PLATFORM \
    --extra-libs="-lgcc" \
    --enable-parsers \
    --enable-decoders \
    --enable-demuxers \
    --enable-network \
    --enable-protocols \
    --enable-protocol=file \
    --enable-swscale  \
    --enable-swresample \
    --enable-avformat \
    --enable-zlib \
    --enable-avcodec \
    --disable-ffplay \
    --disable-ffprobe \
    --disable-ffmpeg \
    --disable-ffserver \
    --disable-avfilter \
    --disable-swscale-alpha \
    --disable-encoders \
    --disable-debug \
    --disable-muxers \
    --disable-devices \
    --disable-avdevice \
    --disable-postproc \
    --disable-indevs \
    --disable-doc \
    --disable-bsfs \
    --disable-parser=dca \
    --disable-demuxer=srt \
    --disable-demuxer=microdvd \
    --disable-demuxer=jacosub \
    --disable-demuxer=dts \
    --disable-decoder=ass \
    --disable-decoder=srt \
    --disable-decoder=microdvd \
    --disable-decoder=jacosub \
    --disable-decoder=dca \
    --enable-bzlib \
    --enable-optimizations \
    --enable-pic \
    $ADDITIONAL_CONFIGURE_FLAG

#exit
make clean
make  -j4 install

$PREBUILT/bin/i686-android-linux-ar d libavcodec/libavcodec.a inverse.o
$PREBUILT/bin/i686-android-linux-ld -rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib  -soname libffmpeg.so -shared -nostdlib  -z,noexecstack -Bsymbolic --whole-archive --no-undefined -o $PREFIX/libffmpeg.so libavcodec/libavcodec.a libavformat/libavformat.a libavutil/libavutil.a libswscale/libswscale.a libswresample/libswresample.a $BZLIB_LDPATH/libbz2.a -lc -lm -lz -ldl --warn-once  --dynamic-linker=/system/bin/linker $PREBUILT/lib/gcc/i686-android-linux/4.4.3/libgcc.a

cp config.h broov/x86/.
