#!/bin/bash
######################################################
# Usage:
# put this script in top of FFmpeg source tree
# ./build_android
# It generates binary for following architectures:
# ARM V5
# ARM v6 
# ARM v6+VFP 
# ARM v7+VFPv3-d16 (Tegra2) 
# ARM v7+Neon (Cortex-A8)
# Intel X86
#
# Customizing:
# 1. Feel free to change ./configure parameters for more features
# 2. To adapt other ARM variants
# set $CPU and $OPTIMIZE_CFLAGS 
# call build_one
######################################################

NDK=/Users/apple/Downloads/android-ndk-r8/
PLATFORM=$NDK/platforms/android-8/arch-arm/
PREBUILT=$NDK/toolchains/arm-linux-androideabi-4.4.3/prebuilt/darwin-x86

#PREBUILT=$NDK/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86
#lseek to lseek64 -Change this to libavformat/file.c for large file support
#    --enable-libogg \
#    --enable-libvorbis \
#    --enable-libfaac \
#    --enable-libfaad \
#    --enable-libxvid \
#    --enable-libamr_nb \
#    --enable-libamr_wb \
#    --enable-libgsm \
#    --enable-liba52 \
#    --enable-nonfree \
#    --enable-indevs \
#    --enable-runtime-cpudetect \

BZLIBPATH="/Users/apple/Downloads/dolphin_player/dolphin-player/p/jni/bzip2/"
BZLIB_LDPATHARM="/Users/apple/Downloads/dolphin_player/dolphin-player/p/jni/bzip2"
BZLIB_LDPATHARMV7="/Users/apple/Downloads/dolphin_player/dolphin-player/p/jni/bzip2"

function build_one
{
./configure --target-os=linux \
    --prefix=$PREFIX \
    --enable-cross-compile \
    --extra-libs="-lgcc" \
    --cc=$PREBUILT/bin/arm-linux-androideabi-gcc \
    --cross-prefix=$PREBUILT/bin/arm-linux-androideabi- \
    --nm=$PREBUILT/bin/arm-linux-androideabi-nm \
    --sysroot=$PLATFORM \
    --extra-cflags=" -O3 -fpic -DANDROID -DHAVE_SYS_UIO_H=1 -Dipv6mr_interface=ipv6mr_ifindex -fasm -Wno-psabi -fno-short-enums -fno-strict-aliasing -finline-limit=300 $OPTIMIZE_CFLAGS -I$BZLIBPATH -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE" \
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
    --disable-decoder=ass \
    --disable-decoder=srt \
    --disable-decoder=microdvd \
    --disable-decoder=jacosub \
    --enable-bzlib \
    --enable-zlib \
    --enable-pic \
    --enable-optimizations \
    --disable-filters \
    $ADDITIONAL_CONFIGURE_FLAG

#exit
#read x
make clean
make  -j4 install
$PREBUILT/bin/arm-linux-androideabi-ar d libavcodec/libavcodec.a inverse.o

$PREBUILT/bin/arm-linux-androideabi-ld -rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib  -soname libffmpeg.so -shared -nostdlib  -z,noexecstack -Bsymbolic --whole-archive --no-undefined -o $PREFIX/libffmpeg.so libavcodec/libavcodec.a libavformat/libavformat.a libavutil/libavutil.a libswscale/libswscale.a libswresample/libswresample.a $BZLIB_LDPATH/libbz2.a -lc -lm -lz -ldl -llog  --warn-once  --dynamic-linker=/system/bin/linker $PREBUILT/lib/gcc/arm-linux-androideabi/4.4.3/libgcc.a

}

function build_arm_v5
{
        #arm v5
	CPU=armv5te
	OPTIMIZE_CFLAGS="-march=$CPU -Wno-multichar -fno-exceptions"
        #-D__thumb__ -mthumb
	PREFIX=./android/$CPU 
	ADDITIONAL_CONFIGURE_FLAG="--arch=arm --cpu=armv5te --enable-armv5te --enable-memalign-hack"
	build_one
}

function build_arm_v6
{
        #arm v6
	CPU=armv6
	OPTIMIZE_CFLAGS="-march=$CPU "
	PREFIX=./android/$CPU 
	ADDITIONAL_CONFIGURE_FLAG="--arch=armv6 --enable-armv5te --enable-armv6 --enable-memalign-hack"
	build_one
}

function build_arm_v6_vfp
{
        #arm v6+vfp
	CPU=armv6
	OPTIMIZE_CFLAGS="-DCMP_HAVE_VFP -mfloat-abi=softfp -mfpu=vfp -march=$CPU "
	PREFIX=./android/${CPU}_vfp 
	ADDITIONAL_CONFIGURE_FLAG="--arch=armv6 --enable-armv5te --enable-armv6 --enable-armvfp --enable-memalign-hack"
	build_one
}

function build_arm_v7_vfp
{
        #arm v7vfp
	CPU=armv7-a
	OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=vfp -marm -march=$CPU "
	PREFIX=./android/$CPU-vfp
	ADDITIONAL_CONFIGURE_FLAG="--arch=armv7-a --enable-armv5te --enable-armv6 --enable-armvfp --enable-memalign-hack"
	build_one
}

function build_arm_v7_vfpv3
{
        #arm tegra v7vfpv3
	CPU=armv7-a
	OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=vfpv3-d16 -march=$CPU"
	PREFIX=./android/$CPU-vfpv3
	ADDITIONAL_CONFIGURE_FLAG="--arch=armv7-a --enable-armv5te --enable-armv6 --enable-armvfp --enable-memalign-hack"
	build_one
}

function build_arm_v7_neon
{
        #arm v7n
	CPU=armv7-a
	OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=neon -march=$CPU -mtune=cortex-a8 -Wno-multichar -fno-exceptions "
	PREFIX=./android/$CPU 
	ADDITIONAL_CONFIGURE_FLAG="--arch=arm --cpu=armv7-a --enable-armv5te --enable-armv6 --enable-armvfp --enable-memalign-hack --enable-neon"
	build_one
}


function main
{
  BZLIB_LDPATH=$BZLIB_LDPATHARM
  build_arm_v5
  cp config.h broov/v5/.
  cp config.log broov/v5/.

  build_arm_v6
  cp config.h broov/v6/.
  cp config.log broov/v6/.

  build_arm_v6_vfp
  cp config.h broov/v6vfp/.
  cp config.log broov/v6vfp/.

  build_arm_v7_vfp
  cp config.h broov/v7vfp/.
  cp config.log broov/v7vfp/.

  build_arm_v7_vfpv3
  cp config.h broov/v7vfpv3/.
  cp config.log broov/v7vfpv3/.

  BZLIB_LDPATH=$BZLIB_LDPATHARMV7
  build_arm_v7_neon
  cp config.h broov/v7neon/.
  cp config.log broov/v7neon/.

#  //Strip the debug symbols after compilation using strip command
  $PREBUILT/bin/arm-linux-androideabi-strip android/armv5te/libffmpeg.so
  $PREBUILT/bin/arm-linux-androideabi-strip android/armv6/libffmpeg.so
  $PREBUILT/bin/arm-linux-androideabi-strip android/armv6_vfp/libffmpeg.so

  $PREBUILT/bin/arm-linux-androideabi-strip android/armv7-a-vfp/libffmpeg.so
  $PREBUILT/bin/arm-linux-androideabi-strip android/armv7-a-vfpv3/libffmpeg.so
  $PREBUILT/bin/arm-linux-androideabi-strip android/armv7-a/libffmpeg.so
}


main
