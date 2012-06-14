#include <unistd.h>
#include <jni.h>
#include <android/log.h>

#include "broov_player.h"
#include "video_player.h"

#ifdef __cplusplus
#define C_LINKAGE "C"
#else
#define C_LINKAGE
#endif

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativePlayerInit(JNIEnv* env, jobject obj, jstring jfileName, jint subtitleShow, jint subtitleFontSize, jint subtitleEncodingType, jint rgb565)
{
        jboolean isCopy;
        char     lclFileName[FILE_NAME_SIZE];

        int my_subtitle_show;
        int my_subtitle_font_size;
        int my_subtitle_encoding_type;
        int my_rgb565;

#ifdef BROOV_C
        const char *fileString = (*env)->GetStringUTFChars(env, jfileName, &isCopy);
#else
        const char *fileString = env->GetStringUTFChars(jfileName, &isCopy);
#endif

        strncpy(lclFileName, fileString, FILE_NAME_SIZE);
#ifdef BROOV_C
        (*env)->ReleaseStringUTFChars(env, jfileName, fileString);
#else
        env->ReleaseStringUTFChars(jfileName, fileString);

#endif
        my_subtitle_show = subtitleShow;
        my_subtitle_font_size = subtitleFontSize;
        my_subtitle_encoding_type = subtitleEncodingType;
        my_rgb565 = rgb565;
        
        __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Starting Player init: subtitle_show:%d subtitle_font_size:%d", my_subtitle_show, my_subtitle_font_size);
        return player_init(lclFileName, my_subtitle_show, my_subtitle_font_size, my_subtitle_encoding_type, my_rgb565);

};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativePlayerExit(JNIEnv* env, jobject obj)
{
        __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Starting Player Exit");
        return player_exit();
};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativePlayerMain(JNIEnv* env, jobject obj, 
           jstring jfileName, 
           jint loopAfterPlay, 
           jint audioFileType,
           jint skipFrames,
           jint rgb565,
           jint yuvRgbAsm,
           jint skipBidirFrames,
           jint queueSizeMin,
           jint queueSizeMax,
           jint totalQueueSize,
           jint audioQueueSize,
           jint fastMode,
           jint debugMode,
           jint syncType,
           jint seekDuration,
           jint ffmpegFlags)
{
        jboolean isCopy;

	int argc = 1;

        int my_loop_after_play;
        int my_audio_file_type;
        int my_skip_frames;
        int my_rgb_565;
        int my_yuv_rgb_asm;
        int my_skip_bidir_frames;
        int my_queue_size_min;
        int my_queue_size_max;
        int my_total_queue_size;
        int my_audio_queue_size;
        int my_fast_mode;
        int my_debug_mode;
        int my_sync_type;
        int my_seek_duration;
        int my_ffmpeg_flags;

#ifdef BROOV_C
        const char *fileString = (*env)->GetStringUTFChars(env, jfileName, &isCopy);
#else
        const char *fileString     = env->GetStringUTFChars(jfileName, &isCopy);
#endif
        char lclFileName[FILE_NAME_SIZE];

        strncpy(lclFileName, fileString, FILE_NAME_SIZE);

#ifdef BROOV_C
        (*env)->ReleaseStringUTFChars(env, jfileName, fileString);
#else
        env->ReleaseStringUTFChars(jfileName, fileString);
#endif

	char *argv[] = { lclFileName };
         
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Waiting 30s for debugger");
        //sleep(2); //Wait for debugger to attach
        __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Starting Player main()");

        my_loop_after_play = loopAfterPlay;
        my_audio_file_type = audioFileType;
        my_skip_frames = skipFrames;
        my_rgb_565 = rgb565;
        my_yuv_rgb_asm = yuvRgbAsm;
        my_skip_bidir_frames = skipBidirFrames;
        my_queue_size_min = queueSizeMin;
        my_queue_size_max = queueSizeMax;
        my_total_queue_size= totalQueueSize;
        my_audio_queue_size= audioQueueSize;
        my_fast_mode = fastMode;
        my_debug_mode = debugMode;
        my_sync_type= syncType;
        my_seek_duration= seekDuration;
        my_ffmpeg_flags = ffmpegFlags;

        return player_main(argc, argv, my_loop_after_play, my_audio_file_type, 
                           my_skip_frames, my_rgb_565, my_yuv_rgb_asm, 
                           my_skip_bidir_frames, my_queue_size_min, my_queue_size_max, my_total_queue_size, my_audio_queue_size, my_fast_mode, my_debug_mode, my_sync_type, my_seek_duration, my_ffmpeg_flags);

        //return test_main(argc, argv);
        //return t8_main(argc, argv);

}

#if 0
int test_main(int argc, char** argv );
#ifdef __cplusplus
extern "C" {
#endif

int t8_main(int argc, char** argv );

#ifdef __cplusplus
}
#endif

#endif

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativePlayerDuration(JNIEnv* env, jobject obj)
{
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Get Current FileDuration");
        return player_duration();
};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativePlayerTotalDuration(JNIEnv* env, jobject obj)
{
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Get Total Duration");
        return player_total_duration();
};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativePlayerPlay(JNIEnv* env, jobject obj)
{
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "nativePlayerPlay");
        return player_play();
};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativePlayerPause(JNIEnv* env, jobject obj)
{
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "nativePlayerPause");
        return player_pause();
};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativePlayerForward(JNIEnv* env, jobject obj)
{
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "nativePlayerForward");
        return player_forward();
};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativePlayerRewind(JNIEnv* env, jobject obj)
{
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "nativePlayerRewind");
        return player_rewind();
};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativePlayerPrev(JNIEnv* env, jobject obj)
{
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "nativePlayerPrev");
        return player_prev();
};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativePlayerNext(JNIEnv* env, jobject obj)
{
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "nativePlayerNext");
        return player_next();
};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativePlayerSeek(JNIEnv* env, jobject obj, jint percentage)
{
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "nativePlayerSeek");
        int my_percentage = percentage;
        return player_seek(percentage);
};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativePlayerSetAspectRatio(JNIEnv* env, jobject obj, jint aspectRatio)
{
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "nativePlayerSetAspectRatio");
        int my_aspect_ratio= aspectRatio;
        return player_set_aspect_ratio(my_aspect_ratio);
};


//Native Video Player Support Code Begin
extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativeVideoPlayerInit(JNIEnv* env, jobject obj, jstring jfileName, jint subtitleShow, jint subtitleFontSize, jint subtitleEncodingType, jint rgb565)
{
        jboolean isCopy;
        char     lclFileName[FILE_NAME_SIZE];

        int my_subtitle_show;
        int my_subtitle_font_size;
        int my_subtitle_encoding_type;
        int my_rgb565;

#ifdef BROOV_C
        const char *fileString = (*env)->GetStringUTFChars(env, jfileName, &isCopy);
#else
        const char *fileString = env->GetStringUTFChars(jfileName, &isCopy);
#endif

        strncpy(lclFileName, fileString, FILE_NAME_SIZE);
#ifdef BROOV_C
        (*env)->ReleaseStringUTFChars(env, jfileName, fileString);
#else
        env->ReleaseStringUTFChars(jfileName, fileString);

#endif
        my_subtitle_show = subtitleShow;
        my_subtitle_font_size = subtitleFontSize;
        my_subtitle_encoding_type = subtitleEncodingType;
        my_rgb565 = rgb565;
        
        __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Starting Player init: subtitle_show:%d subtitle_font_size:%d", my_subtitle_show, my_subtitle_font_size);
        return video_player_init(lclFileName, my_subtitle_show, my_subtitle_font_size, my_subtitle_encoding_type, my_rgb565);

};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativeVideoPlayerExit(JNIEnv* env, jobject obj)
{
        __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Starting Player Exit");
        return video_player_exit();
};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativeVideoPlayerMain(JNIEnv* env, jobject obj, 
           jstring jfileName, 
           jint loopAfterPlay, 
           jint audioFileType,
           jint skipFrames,
           jint rgb565,
           jint yuvRgbAsm,
           jint skipBidirFrames,
           jint queueSizeMin,
           jint queueSizeMax,
           jint totalQueueSize,
           jint audioQueueSize,
           jint fastMode,
           jint debugMode,
           jint syncType,
           jint seekDuration,
           jint ffmpegFlags)
{
        jboolean isCopy;

	int argc = 1;

        int my_loop_after_play;
        int my_audio_file_type;
        int my_skip_frames;
        int my_rgb_565;
        int my_yuv_rgb_asm;
        int my_skip_bidir_frames;
        int my_queue_size_min;
        int my_queue_size_max;
        int my_total_queue_size;
        int my_audio_queue_size;
        int my_fast_mode;
        int my_debug_mode;
        int my_sync_type;
        int my_seek_duration;
        int my_ffmpeg_flags;

#ifdef BROOV_C
        const char *fileString = (*env)->GetStringUTFChars(env, jfileName, &isCopy);
#else
        const char *fileString     = env->GetStringUTFChars(jfileName, &isCopy);
#endif
        char lclFileName[FILE_NAME_SIZE];

        strncpy(lclFileName, fileString, FILE_NAME_SIZE);

#ifdef BROOV_C
        (*env)->ReleaseStringUTFChars(env, jfileName, fileString);
#else
        env->ReleaseStringUTFChars(jfileName, fileString);
#endif

	char *argv[] = { lclFileName };
         
        my_loop_after_play = loopAfterPlay;
        my_audio_file_type = audioFileType;
        my_skip_frames = skipFrames;
        my_rgb_565 = rgb565;
        my_yuv_rgb_asm = yuvRgbAsm;
        my_skip_bidir_frames = skipBidirFrames;
        my_queue_size_min = queueSizeMin;
        my_queue_size_max = queueSizeMax;
        my_total_queue_size= totalQueueSize;
        my_audio_queue_size= audioQueueSize;
        my_fast_mode = fastMode;
        my_debug_mode = debugMode;
        my_sync_type= syncType;
        my_seek_duration= seekDuration;
        my_ffmpeg_flags = ffmpegFlags;

        return video_player_main(argc, argv, my_loop_after_play, my_audio_file_type, 
                           my_skip_frames, my_rgb_565, my_yuv_rgb_asm, 
                           my_skip_bidir_frames, my_queue_size_min, my_queue_size_max, my_total_queue_size, my_audio_queue_size, my_fast_mode, my_debug_mode, my_sync_type, my_seek_duration, my_ffmpeg_flags);

}

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativeVideoPlayerDuration(JNIEnv* env, jobject obj)
{
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Get Current FileDuration");
        return video_player_duration();
};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativeVideoPlayerTotalDuration(JNIEnv* env, jobject obj)
{
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Get Total Duration");
        return video_player_total_duration();
};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativeVideoPlayerPlay(JNIEnv* env, jobject obj)
{
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "nativePlayerPlay");
        return video_player_play();
};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativeVideoPlayerPause(JNIEnv* env, jobject obj)
{
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "nativePlayerPause");
        return video_player_pause();
};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativeVideoPlayerForward(JNIEnv* env, jobject obj)
{
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "nativePlayerForward");
        return video_player_forward();
};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativeVideoPlayerRewind(JNIEnv* env, jobject obj)
{
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "nativePlayerRewind");
        return video_player_rewind();
};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativeVideoPlayerPrev(JNIEnv* env, jobject obj)
{
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "nativePlayerPrev");
        return video_player_prev();
};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativeVideoPlayerNext(JNIEnv* env, jobject obj)
{
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "nativePlayerNext");
        return video_player_next();
};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativeVideoPlayerSeek(JNIEnv* env, jobject obj, jint percentage)
{
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "nativePlayerSeek");
        int my_percentage = percentage;
        return video_player_seek(percentage);
};

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativeVideoPlayerSetAspectRatio(JNIEnv* env, jobject obj, jint aspectRatio)
{
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "nativePlayerSetAspectRatio");
        int my_aspect_ratio= aspectRatio;
        return video_player_set_aspect_ratio(my_aspect_ratio);
};

#undef C_LINKAGE

