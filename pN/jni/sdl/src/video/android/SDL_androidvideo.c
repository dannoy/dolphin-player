/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2009 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/

#include <jni.h>
#include <android/log.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <string.h> // for memset()

#include "SDL_config.h"
#include "SDL_version.h"

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_mutex.h"
#include "SDL_thread.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "../SDL_sysvideo.h"
#include "SDL_androidvideo.h"
#include "jniwrapperstuff.h"


// The device screen dimensions to draw on
int SDL_ANDROID_sWindowWidth  = 0;
int SDL_ANDROID_sWindowHeight = 0;

// Extremely wicked JNI environment to call Java functions from C code
static JNIEnv* JavaEnv = NULL;
static jclass JavaRendererClass = NULL;
static jobject JavaRenderer = NULL;
static jmethodID JavaSwapBuffers = NULL;
static jmethodID JavaExitFromNativePlayerView = NULL;
static int glContextLost = 0;

static void appPutToBackgroundCallbackDefault(void)
{
	//SDL_ANDROID_PauseAudioPlayback();
        __android_log_print(ANDROID_LOG_INFO, "SDLLOG", "Inside app put to background callback default");
}
static void appRestoredCallbackDefault(void)
{
	//SDL_ANDROID_ResumeAudioPlayback();
        __android_log_print(ANDROID_LOG_INFO, "SDLLOG", "Inside app restored callback default");
}

static SDL_ANDROID_ApplicationPutToBackgroundCallback_t appPutToBackgroundCallback = appPutToBackgroundCallbackDefault;
static SDL_ANDROID_ApplicationPutToBackgroundCallback_t appRestoredCallback = appRestoredCallbackDefault;

int SDL_ANDROID_CallJavaSwapBuffers()
{

#ifndef BROOV_PLAYER_NO_DEBUG_LOG
    __android_log_print(ANDROID_LOG_INFO, "SDLLOG", "Inside SDL_ANDROID_CallJavaSwapBuffers");
#endif
	if( !glContextLost )
	{
		SDL_ANDROID_drawTouchscreenKeyboard();
		SDL_ANDROID_processAndroidTrackballDampening();
	}

	if( glContextLost )
	{
		glContextLost = 0;
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "OpenGL context recreated, refreshing textures");
		SDL_ANDROID_VideoContextRecreated();
		appRestoredCallback();
	}

	if( ! (*JavaEnv)->CallIntMethod( JavaEnv, JavaRenderer, JavaSwapBuffers ) )
		return 0;

	return 1;
}

int SDL_ANDROID_CallJavaExitFromNativePlayerView()
{
#ifndef BROOV_PLAYER_NO_DEBUG_LOG
    __android_log_print(ANDROID_LOG_INFO, "SDLLOG", "Inside SDL_ANDROID_CallJavaExitFromNativePlayerView");
#endif

	if ( ! (*JavaEnv)->CallIntMethod( JavaEnv, JavaRenderer, JavaExitFromNativePlayerView))
		return 0;
	return 1;
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoRenderer_nativeResize) ( JNIEnv*  env, jobject  thiz, jint w, jint h )
{
#ifndef BROOV_PLAYER_NO_DEBUG_LOG
    __android_log_print(ANDROID_LOG_INFO, "SDLLOG", "Inside DemoRenderer_nativeResize");
#endif

	if( SDL_ANDROID_sWindowWidth == 0 )
	{
		SDL_ANDROID_sWindowWidth = w;
		SDL_ANDROID_sWindowHeight = h;
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Physical screen resolution is %dx%d", w, h);
	} else 
        {
		SDL_ANDROID_sWindowWidth = w;
		SDL_ANDROID_sWindowHeight = h;
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Physical screen resolution changed to %dx%d", w, h);

        }
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(AudioPlayer_nativeAudioPlayerDone) ( JNIEnv*  env, jobject  thiz )
{
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "quitting...");
#if SDL_VERSION_ATLEAST(1,3,0)
	SDL_SendQuit();
#else
	SDL_PrivateQuit();
#endif
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "quit OK");
}


JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoRenderer_nativeDone) ( JNIEnv*  env, jobject  thiz )
{
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "quitting...");
#if SDL_VERSION_ATLEAST(1,3,0)
	SDL_SendQuit();
#else
	SDL_PrivateQuit();
#endif
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "quit OK");
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoRenderer_nativeGlContextLost) ( JNIEnv*  env, jobject  thiz )
{
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "OpenGL context lost, waiting for new OpenGL context");
	glContextLost = 1;
	appPutToBackgroundCallback();
#if SDL_VERSION_ATLEAST(1,3,0)
	if( ANDROID_CurrentWindow )
		SDL_SendWindowEvent(ANDROID_CurrentWindow, SDL_WINDOWEVENT_MINIMIZED, 0, 0);
#else
	SDL_PrivateAppActive(0, SDL_APPACTIVE|SDL_APPINPUTFOCUS|SDL_APPMOUSEFOCUS);
#endif

	SDL_ANDROID_VideoContextLost();
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoRenderer_nativeGlContextRecreated) ( JNIEnv*  env, jobject  thiz )
{
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "OpenGL context recreated, sending SDL_ACTIVEEVENT");
#if SDL_VERSION_ATLEAST(1,3,0)
	if( ANDROID_CurrentWindow )
		SDL_SendWindowEvent(ANDROID_CurrentWindow, SDL_WINDOWEVENT_RESTORED, 0, 0);
#else
	SDL_PrivateAppActive(1, SDL_APPACTIVE|SDL_APPINPUTFOCUS|SDL_APPMOUSEFOCUS);
#endif
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoRenderer_nativeInitJavaCallbacks) ( JNIEnv*  env, jobject thiz )
{
#ifndef BROOV_PLAYER_NO_DEBUG_LOG
    __android_log_print(ANDROID_LOG_INFO, "SDLLOG", "Inside DemoRenderer_nativeiInitJavaCallbacks");
#endif
	JavaEnv = env;
	JavaRenderer = thiz;
	
	JavaRendererClass = (*JavaEnv)->GetObjectClass(JavaEnv, thiz);
	JavaSwapBuffers = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "swapBuffers", "()I");
	JavaExitFromNativePlayerView = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "exitFromNativePlayerView", "()I");
	
	ANDROID_InitOSKeymap();
	
}

int SDL_ANDROID_SetApplicationPutToBackgroundCallback(
		SDL_ANDROID_ApplicationPutToBackgroundCallback_t appPutToBackground,
		SDL_ANDROID_ApplicationPutToBackgroundCallback_t appRestored )
{
#ifndef BROOV_PLAYER_NO_DEBUG_LOG
    __android_log_print(ANDROID_LOG_INFO, "SDLLOG", "Inside SDL_ANDROID_SetApplicationPutToBackgroundCallback");
#endif
	appPutToBackgroundCallback = appPutToBackgroundCallbackDefault;
	appRestoredCallback = appRestoredCallbackDefault;
	
	if( appPutToBackground )
		appPutToBackgroundCallback = appPutToBackground;

	if( appRestoredCallback )
		appRestoredCallback = appRestored;
}
