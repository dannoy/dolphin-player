#!/bin/bash

#export NDK=/media/DATA/Android/NDK/ndk
#PREBUILT=$NDK/build/prebuilt/linux-x86/i686-unknown-linux-gnu-4.2.1
#PLATFORM=$NDK/build/platforms/android-4/arch-x86
export NDK="/home/aatrala/android/android-ndk-r6b"
PREBUILT="$NDK/toolchains/x86-4.4.3/prebuilt/linux-x86" 
PLATFORM="$NDK/platforms/android-9/arch-x86"
GCC=i686-android-linux-gcc
CROSPREFIX=i686-android-linux
NM=i686-android-linux-nm
ELF=$PREBUILT/i686-android-linux/lib/ldscripts/elf_i386.x
ARCH=x86
CRTBEGIN=$PREBUILT/lib/gcc/i686-android-linux/4.4.3/crtbegin.o
CRTEND=$PREBUILT/lib/gcc/i686-android-linux/4.4.3/crtend.o
CPUDIR=x86
PREFIX=./android/$CPUDIR
OPTIMIZE_CFLAGS=" "

#./configure --target-os=linux --arch=$ARCH --enable-version3 --enable-gpl  --enable-shared --disable-static --disable-asm --disable-yasm --disable-ffserver --disable-ffprobe --disable-ffplay --enable-nonfree --enable-postproc --enable-avfilter --enable-cross-compile --enable-swscale --disable-stripping --cc=$PREBUILT/bin/$GCC --cross-prefix=$PREBUILT/bin/$CROSPREFIX- --nm=$PREBUILT/bin/$NM --extra-cflags="-fPIC -DANDROID" --disable-asm --extra-ldflags="-Wl,-T,$ELF -Wl,-rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib -nostdlib $CRTBEGIN $CRTENDo -lc -lm -ldl" --prefix=$PLATFORM/usr

#--enable-version3 --enable-gpl 
# --enable-nonfree --enable-postproc 
#--disable-yasm --disable-asm
# --extra-cflags="-fPIC -DANDROID"

#./configure --target-os=linux --arch=$ARCH --enable-cross-compile --enable-shared --disable-static --sysroot=$PLATFORM --extra-libs="-lgcc"  --disable-ffserver --disable-yasm --disable-asm --disable-ffprobe --disable-ffplay --disable-ffmpeg --enable-avfilter  --enable-swscale --disable-stripping --cc=$PREBUILT/bin/$GCC --cross-prefix=$PREBUILT/bin/$CROSPREFIX- --nm=$PREBUILT/bin/$NM --extra-cflags="-fPIC -DANDROID" --extra-ldflags="-Wl,-T,$ELF -Wl,-rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib -nostdlib $CRTBEGIN $CRTEND -lc -lm -ldl" --prefix=$PREFIX 
./configure --target-os=linux --arch=$ARCH --enable-cross-compile --disable-shared --enable-static --sysroot=$PLATFORM --extra-libs="-lgcc"  --disable-ffserver --disable-yasm --disable-asm --disable-ffprobe --disable-ffplay --disable-ffmpeg --enable-avfilter  --enable-swscale --disable-stripping --cc=$PREBUILT/bin/$GCC --cross-prefix=$PREBUILT/bin/$CROSPREFIX- --nm=$PREBUILT/bin/$NM --extra-cflags=" -O3 -fPIC -DANDROID" --extra-ldflags="-Wl,-T,$ELF -Wl,-rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib -nostdlib $CRTBEGIN $CRTEND -lc -lm -ldl" --prefix=$PREFIX 
# --enable-cross-compile
# --enable-version3 --enable-gpl  --enable-shared \
#    --disable-ffserver --disable-ffprobe --disable-ffplay \
#    --enable-nonfree --enable-postproc --enable-avfilter \
#    --enable-swscale --disable-stripping \
#    --extra-cflags="-fPIC -DANDROID" --disable-asm --extra-ldflags="-Wl,-T,$ELF -Wl,-rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib -nostdlib $CRTBEGIN $CRTENDo -lc -lm -ldl" --prefix=$PLATFORM/usr
 
# ./configure --target-os=linux \
#    --prefix=$PREFIX \
#     --enable-cross-compile \
#     --extra-libs="-lgcc" \
#     --arch=$ARCH \
#     --cc=$PREBUILT/bin/$GCC \
#     --cross-prefix=$PREBUILT/bin/i686-android-linux- \
#     --nm=$PREBUILT/bin/$NM \
#     --sysroot=$PLATFORM \
#     --extra-cflags=" -O3 -fpic -DANDROID -DHAVE_SYS_UIO_H=1 -Dipv6mr_interface=ipv6mr_ifindex -fasm -Wno-psabi -fno-short-enums  -fno-strict-aliasing -finline-limit=300 $OPTIMIZE_CFLAGS " \
#     --disable-shared \
#     --enable-static \
#     --extra-ldflags="-Wl,-T,$ELF -Wl,-rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib  -nostdlib $CRTBEGIN $CRTEND -lc -lm -ldl " \
#     --enable-parsers \
#     --disable-encoders  \
#     --enable-decoders \
#     --disable-muxers \
#     --enable-demuxers \
# #     --enable-swscale  \
#     --disable-ffplay \
#     --disable-ffprobe \
#     --disable-ffserver \
#     --enable-network \
#     --enable-indevs \
#     --disable-bsfs \
#     --disable-filters \
#     --enable-protocols  \
#     --enable-asm \
#--disable-ffmpeg

#make clean
#make  -j4 install

#$PREBUILT/bin/i686-android-linux-ar d libavcodec/libavcodec.a inverse.o
#$PREBUILT/bin/i686-android-linux-ld -rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib  -soname libffmpeg.so -shared -nostdlib  -z,noexecstack -Bsymbolic --whole-archive --no-undefined -o $PREFIX/libffmpeg.so libavcodec/libavcodec.a libavformat/libavformat.a libavutil/libavutil.a libswscale/libswscale.a -lc -lm -lz -ldl --warn-once  --dynamic-linker=/system/bin/linker $PREBUILT/lib/gcc/i686-android-linux/4.4.3/libgcc.a

