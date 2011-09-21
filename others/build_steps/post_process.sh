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

$PREBUILT/bin/i686-android-linux-ar d libavcodec/libavcodec.a inverse.o
$PREBUILT/bin/i686-android-linux-ld -rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib  -soname libffmpeg.so -shared -nostdlib  -z,noexecstack -Bsymbolic --whole-archive --no-undefined -o $PREFIX/libffmpeg.so libavcodec/libavcodec.a libavformat/libavformat.a libavutil/libavutil.a libswscale/libswscale.a -lc -lm -lz -ldl --warn-once  --dynamic-linker=/system/bin/linker $PREBUILT/lib/gcc/i686-android-linux/4.4.3/libgcc.a
