#!/bin/bash
######################################################
# Usage:
# put this script in top of FFmpeg source tree
# ./build_android
# It generates binary for following architectures:
# MIPS
#
# Customizing:
# 1. Feel free to change ./configure parameters for more features
# 2. To adapt other variants
# set $CPU and $OPTIMIZE_CFLAGS 
# call build_one
######################################################

NDK=/Users/apple/Downloads/android-ndk-r8/
PLATFORM=$NDK/platforms/android-9/arch-mips/
PREBUILT=$NDK/toolchains/mipsel-linux-android-4.4.3/prebuilt/darwin-x86

BZLIBPATH="/Users/apple/Downloads/dolphin_player/dolphin-player/p/jni/bzip2/"
BZLIB_LDPATHMIPS="/Users/apple/Downloads/dolphin_player/dolphin-player/p/jni/bzip2/mips"

function build_one_configure
{
./configure --target-os=linux \
    --arch=mips32 \
    --prefix=$PREFIX \
    --enable-cross-compile \
    --extra-libs="-lgcc" \
    --cc=$PREBUILT/bin/mipsel-linux-android-gcc \
    --cross-prefix=$PREBUILT/bin/mipsel-linux-android- \
    --nm=$PREBUILT/bin/mipsel-linux-android-nm \
    --sysroot=$PLATFORM \
    --extra-cflags=" -O3 -DANDROID -DHAVE_SYS_UIO_H=1 -Dipv6mr_interface=ipv6mr_ifindex -fasm -Wno-psabi -fno-short-enums -fno-strict-aliasing -finline-limit=300 $OPTIMIZE_CFLAGS -I$BZLIBPATH" \
    --enable-gpl \
    --enable-version3 \
    --disable-shared \
    --enable-static \
    --extra-ldflags="-Wl,-rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib -nostdlib -lc -lm -ldl -llog -L$BZLIB_LDPATH" \
    --enable-parsers \
    --enable-decoders \
    --enable-demuxers \
    --enable-network \
    --enable-protocols \
    --enable-protocol=file \
    --enable-swscale  \
    --enable-swresample \
    --enable-avformat \
    --enable-avcodec \
    --disable-ffmpeg \
    --disable-ffplay \
    --disable-ffprobe \
    --disable-ffserver \
    --disable-devices \
    --disable-avdevice \
    --disable-postproc \
    --disable-avfilter \
    --disable-swscale-alpha \
    --disable-bsfs \
    --disable-encoders \
    --disable-muxers \
    --disable-indevs \
    --disable-debug \
    --disable-doc \
    --disable-demuxer=srt \
    --disable-demuxer=microdvd \
    --disable-demuxer=jacosub \
    --disable-demuxer=dts \
    --disable-decoder=ass \
    --disable-decoder=srt \
    --disable-decoder=microdvd \
    --disable-decoder=jacosub \
    --enable-bzlib \
    --enable-zlib \
    --enable-pic \
    --enable-optimizations \
    $ADDITIONAL_CONFIGURE_FLAG

exit
}

function build_one_compile
{
#read x
#make clean
make  -j4 install

}

function build_one_link
{
$PREBUILT/bin/mipsel-linux-android-ar d libavcodec/libavcodec.a inverse.o

$PREBUILT/bin/mipsel-linux-android-ld -rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib  -soname libffmpeg.so -shared -nostdlib  -z,noexecstack -Bsymbolic --whole-archive --no-undefined -o $PREFIX/libffmpeg.so libavcodec/libavcodec.a libavformat/libavformat.a libavutil/libavutil.a libswscale/libswscale.a libswresample/libswresample.a $BZLIB_LDPATH/libbz2.a -lc -lm -lz -ldl -llog  --warn-once  --dynamic-linker=/system/bin/linker $PREBUILT/lib/gcc/mipsel-linux-android/4.4.3/libgcc.a

}


function build_mips
{
        #mips build
	CPU=mips
	OPTIMIZE_CFLAGS="-Wno-multichar -fno-exceptions "
	PREFIX=./android/$CPU 
	ADDITIONAL_CONFIGURE_FLAG="--cpu=mips "
	#build_one_configure
	build_one_compile
	build_one_link
}


function main
{
  BZLIB_LDPATH=$BZLIB_LDPATHMIPS
  build_mips
  cp config.h broov/mips/.
  cp config.log broov/mips/.

  $PREBUILT/bin/mipsel-linux-android-strip android/mips/libffmpeg.so
}


main
