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

./configure --target-os=linux \
    --prefix=$PREFIX \
    --enable-cross-compile \
    --disable-yasm \
    --disable-asm \
    --arch=$ARCH \
    --cc=$PREBUILT/bin/$GCC \
    --cross-prefix=$PREBUILT/bin/$CROSPREFIX \
    --nm=$PREBUILT/bin/$NM \
    --extra-cflags=" -O3 -fpic -DANDROID -DHAVE_SYS_UIO_H=1 -Dipv6mr_interface=ipv6mr_ifindex -fasm -Wno-psabi -fno-short-enums -fno-strict-aliasing -finline-limit=300 $OPTIMIZE_CFLAGS" \
    --extra-ldflags="-Wl,-T,$ELF -Wl,-rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib -nostdlib $CRTBEGIN $CRTEND -lc -lm -ldl" \
    --enable-gpl \
    --disable-shared \
    --enable-static \
    --sysroot=$PLATFORM \
    --extra-libs="-lgcc" \
    --enable-parsers \
    --disable-encoders \
    --enable-decoders \
    --disable-muxers \
    --enable-demuxers \
    --enable-network \
    --enable-indevs \
    --disable-bsfs \
    --enable-protocols \
    --enable-protocol=file \
    --enable-swscale  \
    --enable-swresample \
    --enable-avformat \
    --enable-avcodec \
    --enable-ffplay \
    --disable-ffprobe \
    --disable-ffmpeg \
    --disable-ffserver \
    --enable-zlib \
    --enable-avfilter \
    --disable-debug \
    --disable-avdevice \
    $ADDITIONAL_CONFIGURE_FLAG

#exit
make clean
make  -j4 install

$PREBUILT/bin/i686-android-linux-ar d libavcodec/libavcodec.a inverse.o
$PREBUILT/bin/i686-android-linux-ld -rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib  -soname libffmpeg.so -shared -nostdlib  -z,noexecstack -Bsymbolic --whole-archive --no-undefined -o $PREFIX/libffmpeg.so libavcodec/libavcodec.a libavformat/libavformat.a libavutil/libavutil.a libswscale/libswscale.a -lc -lm -lz -ldl --warn-once  --dynamic-linker=/system/bin/linker $PREBUILT/lib/gcc/i686-android-linux/4.4.3/libgcc.a

cp config.h broov/x86/.
