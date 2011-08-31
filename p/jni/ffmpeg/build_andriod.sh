#!/bin/bash

######################################################
# FFmpeg builds script for Android+ARM platform
#
# This script is released under term of 
#   CDDL (http://www.opensource.org/licenses/cddl1) 
# Wrote by pinxue (~@gmail.com) from RockPlayer.com
#                                   2010-8 ~ 2011-4
######################################################

######################################################
# Usage:
#   put this script in top of FFmpeg source tree
#   ./build_android
#
# It generates binary for following architectures:
#     ARMv6 
#     ARMv6+VFP 
#     ARMv7+VFPv3-d16 (Tegra2) 
#     ARMv7+Neon (Cortex-A8)
#
# Customizing:
# 1. Feel free to change ./configure parameters for more features
# 2. To adapt other ARM variants
#       set $CPU and $OPTIMIZE_CFLAGS 
#       call build_one
######################################################

NDK=~/android/android-ndk-r4
PLATFORM=$NDK/build/platforms/android-8/arch-arm/
PREBUILT=$NDK/build/prebuilt/darwin-x86/arm-eabi-4.4.0


function build_one
{

# -fasm : required. Android header file uses asm keyword instead of __asm__ , but most of c dialect (like ansi,c99,gnu99) implies -fno-asm.
#   ~/android/android-ndk-r4/build/platforms/android-5/arch-arm//usr/include/asm/byteorder.h: In function '___arch__swab32':
#   ~/android/android-ndk-r4/build/platforms/android-5/arch-arm//usr/include/asm/byteorder.h:25: error: expected ')' before ':' token

# -fno-short-enums : optimized.  Else FFmpeg obj will generate a huge number of warning for variable-size enums, 
#   though we may suppress them by --no-enum-size-warning, it would be better to avoid it.
#   .../ld: warning: cmdutils.o uses variable-size enums yet the output is to use 32-bit enums; use of enum values across objects may fail

# --extra-libs="-lgcc" : required. Else cannot solve some runtime function symbols
#   ... undefined reference to `__aeabi_f2uiz'

# --enable-protocols : required. Without this option, the file open always fails mysteriously.
#   FFmpeg's av_open_input_file will invoke file format probing functions, but because most of useful demuxers has flag of zero 
#   which cause them are ignored during file format probling and fall to url stream parsing,
#   if protocols are disabled, the file:// url cannot be opened as well.

# $PREBUILT/bin/arm-eabi-ar d libavcodec/libavcodec.a inverse.o : required.
#   FFmpeg includes two copies of inverse.c both in libavutil and libavcodec for performance consideration (not sure the benifit yet)
#   Without this step, final ld of generating libffmpeg.so will fail silently, if invoke ld through gcc, gcc will collect more reasonable error message.

# -llog: debug only, FFmpeg itself doesn't require it at all.
#   With this option, we may simply includes "utils/Log.h" and use LOGx() to observe FFmpeg's behavior
#   PS, it seems the toolchain implies -DNDEBUG somewhere, it would be safer to use following syntax
#    #ifdef NDEBUG
#        #undef NDEBUG
#        #define HAVE_NDEBUG
#    #endif
#    #include "utils/Log.h"
#    #ifdef HAVE_NDEBUG
#        #define NDEBUG
#        #undef HAVE_NDEBUG
#    #endif

# --whole-archive : required. Else ld generate a small .so file (about 15k)

# --no-stdlib : required. Android doesn't use standard c runtime but invited its own wheal (bionic libc) because of license consideration.

# space before \ of configure lines: required for some options. Else next line will be merged into previous lines's content and cause problem.
#   Especially the --extra-cflags, the next line will pass to gcc in this case and configure will say gcc cannot create executable.

# many options mentioned by articles over internet are implied by -O2 or -O3 already, need not repeat at all.

# two or three common optimization cflags are omitted because not sure about the trade off yet. invoke NDK build system with V=1 to find them.

# -Wl,-T,$PREBUILT/arm-eabi/lib/ldscripts/armelf.x mentioned by almost every articles over internet, but it is not required to specify at all.

# -Dipv6mr_interface=ipv6mr_ifindex : required. Android inet header doesn't use ipv6mr_interface which is required by rfc, seems it generate this user space header file directly from kernel header file, but Linux kernel has decided to keep its own name for ever and ask user space header to use rfc name.

# HAVE_SYS_UIO_H : required. Else:
# In file included from ~/android/android-ndk-r4/build/platforms/android-5/arch-arm//usr/include/linux/socket.h:29,
#                 from ~/android/android-ndk-r4/build/platforms/android-5/arch-arm//usr/include/sys/socket.h:33,
#                 from libavformat/network.h:35,
#                 from libavformat/utils.c:46:
#~/android/android-ndk-r4/build/platforms/android-5/arch-arm//usr/include/linux/uio.h:19: error: redefinition of 'struct iovec'
#

# --disable-doc : required because of strange bug of toolchain.

./configure --target-os=linux \
    --prefix=$PREFIX \
    --enable-cross-compile \
    --extra-libs="-lgcc" \
    --arch=arm \
    --cc=$PREBUILT/bin/arm-eabi-gcc \
    --cross-prefix=$PREBUILT/bin/arm-eabi- \
    --nm=$PREBUILT/bin/arm-eabi-nm \
    --sysroot=$PLATFORM \
    --extra-cflags=" -O3 -fpic -DANDROID -DHAVE_SYS_UIO_H=1 -Dipv6mr_interface=ipv6mr_ifindex -fasm -Wno-psabi -fno-short-enums  -fno-strict-aliasing -finline-limit=300 $OPTIMIZE_CFLAGS " \
    --disable-shared \
    --enable-static \
    --extra-ldflags="-Wl,-rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib  -nostdlib -lc -lm -ldl -llog" \
    --enable-parsers \
    --disable-encoders  \
    --enable-decoders \
    --disable-muxers \
    --enable-demuxers \
    --enable-swscale  \
    --disable-ffplay \
    --disable-ffprobe \
    --disable-ffserver \
    --enable-network \
    --enable-indevs \
    --disable-bsfs \
    --disable-filters \
    --enable-protocols  \
    --enable-asm \
    $ADDITIONAL_CONFIGURE_FLAG


#make clean
make  -j4 install

$PREBUILT/bin/arm-eabi-ar d libavcodec/libavcodec.a inverse.o

$PREBUILT/bin/arm-eabi-ld -rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib  -soname libffmpeg.so -shared -nostdlib  -z,noexecstack -Bsymbolic --whole-archive --no-undefined -o $PREFIX/libffmpeg.so libavcodec/libavcodec.a libavformat/libavformat.a libavutil/libavutil.a  -lc -lm -lz -ldl -llog  --warn-once  --dynamic-linker=/system/bin/linker $PREBUILT/lib/gcc/arm-eabi/4.4.0/libgcc.a

}

#arm v6
CPU=armv6
OPTIMIZE_CFLAGS="-marm -march=$CPU"
PREFIX=./android/$CPU 
ADDITIONAL_CONFIGURE_FLAG=
#build_one

#arm v7vfpv3
CPU=armv7-a
OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=vfpv3-d16 -marm -march=$CPU "
PREFIX=./android/$CPU 
ADDITIONAL_CONFIGURE_FLAG=
build_one

#arm v7vfp
CPU=armv7-a
OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=vfp -marm -march=$CPU "
PREFIX=./android/$CPU-vfp
ADDITIONAL_CONFIGURE_FLAG=
build_one

#arm v7n
CPU=armv7-a
OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=neon -marm -march=$CPU -mtune=cortex-a8"
PREFIX=./android/$CPU 
ADDITIONAL_CONFIGURE_FLAG=--enable-neon
build_one

#arm v6+vfp
CPU=armv6
OPTIMIZE_CFLAGS="-DCMP_HAVE_VFP -mfloat-abi=softfp -mfpu=vfp -marm -march=$CPU"
PREFIX=./android/${CPU}_vfp 
ADDITIONAL_CONFIGURE_FLAG=
build_one
