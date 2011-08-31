// tutorial01.c
// Code based on a tutorial by Martin Bohme (boehme@inb.uni-luebeckREMOVETHIS.de)
// Tested on Gentoo, CVS version 5/01/07 compiled with GCC 4.1.1

// A small sample program that shows how to use libavformat and libavcodec to
// read video from a file.
//
// Use
//
// gcc -o tutorial01 tutorial01.c -lavformat -lavcodec -lz
//
// to build (assuming libavformat and libavcodec are correctly installed
// your system).
//
// Run using
//
// tutorial01 myvideofile.mpg
//
// to write the first five frames from "myvideofile.mpg" to disk in PPM
// format.

#include "broov_player.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
//#include <ffmpeg/avcodec.h>
//#include <ffmpeg/avformat.h>
//#include "SDL.h"
#include "SDL_opengles.h"
#include "SDL_main.h"
//#include "IL/il.h"
//#include "GLES/gl.h"

//#include <GLES2/gl2.h>
//#include <GLES2/gl2ext.h>

#include <stdio.h>

/* return current time (in seconds) */
double current_time(void)
{
	struct timeval tv;
#ifdef __VMS
	(void) gettimeofday(&tv, NULL );
#else
	struct timezone tz;
	(void) gettimeofday(&tv, &tz);
#endif
	return (double) tv.tv_sec + tv.tv_usec / 1000000.0;
}


int player_init(char *font_fname, int subtitle_show, int subtitle_font_size)
{
}
int player_exit()
{
}
SDL_Surface* screen;

static SDL_Surface *image = NULL;
SDL_TextureID text; 
static int display_rgb_picture_type1(AVFrame *pFrame, int width, int height, int iFrame)
{
                     
	//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Processed Display Picture LineSize:%d, W:%d H:%d", pFrame->linesize[0], width, height);

        int bpp=24;
	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "BeforeCRGB");
        image = SDL_CreateRGBSurfaceFrom(pFrame->data[0], 
                            width, height, bpp,
                            pFrame->linesize[0], 
                            0, 0, 0, 0);
	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "AfterCRGB");

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "BCT");
        text = SDL_CreateTextureFromSurface(0, image);
	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "BRC");
	SDL_RenderCopy(text, NULL, NULL);
	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "BRP");
	SDL_RenderPresent();
	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "BDT");
	SDL_DestroyTexture(text);
	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "ADT");

        SDL_FreeSurface(image);

}

static int display_rgb_picture(AVFrame *pFrame, int width, int height, int iFrame)
{
	if (image == NULL) {
		int bpp=24;

		//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Processed Display Picture LineSize:%d, W:%d H:%d", pFrame->linesize[0], width, height);
		image = SDL_CreateRGBSurfaceFrom(pFrame->data[0], 
				width, height, bpp,
				pFrame->linesize[0], 
				0, 0, 0, 0);

		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Created RGB Surface");
	}
        
        SDL_BlitSurface(image, 0, screen, NULL);

        // finally, update the screen :)
        //SDL_Flip(screen);
        SDL_UpdateRect(screen, 0, 0, 640, 480);

        //SDL_FreeSurface(image);

}

static int display_rgb_picture_type2(AVFrame *pFrame, int width, int height, int iFrame)

{

}


AVPacket           flush_pkt;
int fs_screen_width  = 640;
int fs_screen_height = 480;
//int main(int argc, char *argv[]) {
int player_main(int argc, char *argv[], 
		int loop_after_play, int audio_image_idx, int audio_file_type)
{
  AVFormatContext *pFormatCtx;
  int             i, videoStream;
  AVCodecContext  *pCodecCtx;
  AVCodec         *pCodec;
  AVFrame         *pFrame; 
  AVFrame         *pFrameRGB;
  AVPacket        packet;
  int             frameFinished;
  int             numBytes;
  uint8_t         *buffer;
  
  if (argc < 2) {
    printf("Please provide a movie file\n");
    return -1;
  }

  // initialize SDL video
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
  }

  // make sure SDL cleans up before exit
  atexit(SDL_Quit);

  // create a new window
  //screen = SDL_SetVideoMode(640, 480, 32,
  //                      SDL_HWSURFACE|SDL_DOUBLEBUF);
  screen = SDL_SetVideoMode(640, 480, 32,
                        SDL_OPENGL|SDL_HWSURFACE);

  glViewport(0, 0, 640, 480);
  glEnable(GL_TEXTURE_2D);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrthof(0.0, 640.0, 480.0, 0.0, 0.0, 100.0);
  glMatrixMode(GL_MODELVIEW);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClearDepthf(0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (!screen)
  {
	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Unable to set 640x480 video: %s\n", SDL_GetError());
        return 1;
  }

  // Register all formats and codecs
  av_register_all();
  
  // Open video file
  if (av_open_input_file(&pFormatCtx, argv[1], NULL, 0, NULL)!=0)
    return -1; // Couldn't open file
  
  // Retrieve stream information
  if(av_find_stream_info(pFormatCtx)<0)
    return -1; // Couldn't find stream information
  
  // Dump information about file onto standard error
  dump_format(pFormatCtx, 0, argv[1], 0);
  
  // Find the first video stream
  videoStream=-1;
  for(i=0; i<pFormatCtx->nb_streams; i++)
    if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO) {
      videoStream=i;
      break;
    }
  if(videoStream==-1)
    return -1; // Didn't find a video stream
  
  // Get a pointer to the codec context for the video stream
  pCodecCtx=pFormatCtx->streams[videoStream]->codec;
  
  // Find the decoder for the video stream
  pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
  if(pCodec==NULL) {
    fprintf(stderr, "Unsupported codec!\n");
    return -1; // Codec not found
  }
  // Open codec
  if(avcodec_open(pCodecCtx, pCodec)<0)
    return -1; // Could not open codec
  
  // Allocate video frame
  pFrame=avcodec_alloc_frame();
  
  // Allocate an AVFrame structure
  pFrameRGB=avcodec_alloc_frame();
  if(pFrameRGB==NULL)
    return -1;
  
  // Determine required buffer size and allocate buffer
  numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,
			      pCodecCtx->height);
  buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
  
  // Assign appropriate parts of buffer to image planes in pFrameRGB
  // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
  // of AVPicture
  avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
		 pCodecCtx->width, pCodecCtx->height);
  
  // clear screen
  SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 255, 80, 255));

  // Read frames and save first five frames to disk
  i=0;
  while(av_read_frame(pFormatCtx, &packet)>=0) {
    // Is this a packet from the video stream?
    if(packet.stream_index==videoStream) {
      // Decode video frame
      __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "BeforeDV");
      avcodec_decode_video(pCodecCtx, pFrame, &frameFinished, 
			   packet.data, packet.size);
      __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "AfterDV");
      
      // Did we get a video frame?
      if(frameFinished) {
	// Convert the image from its native format to RGB
        __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "BeforeIC");
	img_convert((AVPicture *)pFrameRGB, PIX_FMT_RGB24, 
                    (AVPicture*)pFrame, pCodecCtx->pix_fmt, pCodecCtx->width, 
                    pCodecCtx->height);
        __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "AfterIC");
	
	// Show the frame
	display_rgb_picture_type2
                   (pFrameRGB, pCodecCtx->width, pCodecCtx->height, i); 
		  
      }
    }
    
    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);
  }
  
  // Free the RGB image
  av_free(buffer);
  av_free(pFrameRGB);
  
  // Free the YUV frame
  av_free(pFrame);
  
  // Close the codec
  avcodec_close(pCodecCtx);
  
  // Close the video file
  av_close_input_file(pFormatCtx);
  
  return 0;
}
