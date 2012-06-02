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
#    --disable-everything \
#    --disable-avfilter \
#    --disable-avdevice \
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

function build_one
{
./configure --target-os=linux \
    --prefix=$PREFIX \
    --enable-cross-compile \
    --extra-libs="-lgcc" \
    --arch=arm \
    --cc=$PREBUILT/bin/arm-linux-androideabi-gcc \
    --cross-prefix=$PREBUILT/bin/arm-linux-androideabi- \
    --nm=$PREBUILT/bin/arm-linux-androideabi-nm \
    --sysroot=$PLATFORM \
    --extra-cflags=" -O3 -fpic -DANDROID -DHAVE_SYS_UIO_H=1 -Dipv6mr_interface=ipv6mr_ifindex -fasm -Wno-psabi -fno-short-enums -fno-strict-aliasing -finline-limit=300 $OPTIMIZE_CFLAGS" \
    --enable-gpl \
    --disable-shared \
    --enable-static \
    --extra-ldflags="-Wl,-rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib -nostdlib -lc -lm -ldl -llog" \
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
    --disable-ffserver \
    --enable-zlib \
    --disable-debug \
    $ADDITIONAL_CONFIGURE_FLAG

#exit
make clean
make  -j4 install
$PREBUILT/bin/arm-linux-androideabi-ar d libavcodec/libavcodec.a inverse.o

$PREBUILT/bin/arm-linux-androideabi-ld -rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib  -soname libffmpeg.so -shared -nostdlib  -z,noexecstack -Bsymbolic --whole-archive --no-undefined -o $PREFIX/libffmpeg.so libavcodec/libavcodec.a libavformat/libavformat.a libavutil/libavutil.a libswscale/libswscale.a libswresample/libswresample.a libavresample/libavresample.a libavfilter/libavfilter.a libavdevice/libavdevice.a libpostproc/libpostproc.a -lc -lm -lz -ldl -llog  --warn-once  --dynamic-linker=/system/bin/linker $PREBUILT/lib/gcc/arm-linux-androideabi/4.4.3/libgcc.a

}

function build_arm_v5
{
        #arm v5
	CPU=armv5
	OPTIMIZE_CFLAGS="-marm -march=$CPU"
	PREFIX=./android/$CPU 
	ADDITIONAL_CONFIGURE_FLAG=
	build_one
}

function build_arm_v6
{
        #arm v6
	CPU=armv6
	OPTIMIZE_CFLAGS="-marm -march=$CPU"
	PREFIX=./android/$CPU 
	ADDITIONAL_CONFIGURE_FLAG=
	build_one
}

function build_arm_v6_vfp
{
        #arm v6+vfp
	CPU=armv6
	OPTIMIZE_CFLAGS="-DCMP_HAVE_VFP -mfloat-abi=softfp -mfpu=vfp -marm -march=$CPU"
	PREFIX=./android/${CPU}_vfp 
	ADDITIONAL_CONFIGURE_FLAG=
	build_one
}

function build_arm_v7_vfp
{
        #arm v7vfp
	CPU=armv7-a
	OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=vfp -marm -march=$CPU "
	PREFIX=./android/$CPU-vfp
	ADDITIONAL_CONFIGURE_FLAG=
	build_one
}

function build_arm_v7_vfpv3
{
        #arm tegra v7vfpv3
	CPU=armv7-a
	OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=vfpv3-d16 -marm -march=$CPU "
	PREFIX=./android/$CPU-vfpv3
	ADDITIONAL_CONFIGURE_FLAG=
	build_one
}

function build_arm_v7_neon
{
        #arm v7n
	CPU=armv7-a
	OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=neon -marm -march=$CPU -mtune=cortex-a8"
	PREFIX=./android/$CPU 
	ADDITIONAL_CONFIGURE_FLAG=--enable-neon
	build_one
}


function main
{
  build_arm_v5
  build_arm_v6
  build_arm_v6_vfp
  build_arm_v7_vfp
  build_arm_v7_vfpv3
  build_arm_v7_neon
}


main
