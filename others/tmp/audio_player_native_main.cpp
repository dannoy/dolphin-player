#include <unistd.h>
#include <jni.h>
#include <android/log.h>

#include "broov_player.h"
#include "broov_audio_player.h"

#ifdef __cplusplus
#define C_LINKAGE "C"
#else
#define C_LINKAGE
#endif

extern C_LINKAGE int
Java_com_broov_player_DemoRenderer_nativePlayerInit(JNIEnv* env, jobject obj, jstring jfileName, jint subtitleShow, jint subtitleFontSize)
{
        jboolean isCopy;
        char     lclFileName[FILE_NAME_SIZE];

        int my_subtitle_show;
        int my_subtitle_font_size;

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
        
        __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Starting Player init: subtitle_show:%d subtitle_font_size:%d", my_subtitle_show, my_subtitle_font_size);
        return player_init(lclFileName, my_subtitle_show, my_subtitle_font_size);
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
           jint audioFileType)
{
        jboolean isCopy;

	int argc = 2;

        int my_loop_after_play;
        int my_audio_file_type;

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

        char sdl_string[] = "sdl";
	char *argv[] = { sdl_string, lclFileName };
         
        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Waiting 30s for debugger");
        //sleep(2); //Wait for debugger to attach
        __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Starting Player main()");

        my_loop_after_play = loopAfterPlay;
        my_audio_file_type = audioFileType;

        return player_main(argc, argv, my_loop_after_play, my_audio_file_type) ;
        //return test_main(argc, argv);
        //return t8_main(argc, argv);

};

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
Java_com_broov_player_AudioPlayer_nativeAudioPlayerInit(JNIEnv* env, jobject obj)
{

        __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Starting AudioPlayer init");
        return audio_player_init();
};

extern C_LINKAGE int
Java_com_broov_player_AudioPlayer_nativeAudioPlayerExit(JNIEnv* env, jobject obj)
{
        __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Starting AudioPlayer Exit");
        return audio_player_exit();
};

#if 0
extern C_LINKAGE int
Java_com_broov_player_AudioPlayer_nativeAudioPlayerDone(JNIEnv* env, jobject obj)
{
        __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Starting AudioPlayer Exit");
        return audio_player_done();
};
#endif


extern C_LINKAGE int
Java_com_broov_player_AudioPlayer_nativeAudioPlayerMain(JNIEnv* env, jobject obj, 
           jstring jfileName, 
           jint loopAfterPlay)
{
        jboolean isCopy;

	int argc = 2;

        int my_loop_after_play;

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

        char sdl_string[] = "sdl";
	char *argv[] = { sdl_string, lclFileName};
         
        __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Starting AudioPlayer main()");

        my_loop_after_play = loopAfterPlay;

        return audio_player_main(argc, argv, my_loop_after_play);
};

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

#undef C_LINKAGE

