#ifndef BROOV_PLAYER_H
#define BROOV_PLAYER_H

#include <jni.h>
#include <sys/time.h>
#include <unistd.h>

#include <android/log.h>
#include <math.h>
#include <limits.h>

#include <stdio.h>
#include <math.h>

#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_ttf.h"
#include "SDL_image.h"

#include "broov_version.h"
#include "broov_types.h"
#include "broov_config.h"
#include "broov_gui.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "libavutil/avstring.h"
#include "libavutil/pixfmt.h"
#include "libavutil/pixdesc.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavcodec/audioconvert.h"
#include "libavcodec/avfft.h"

#include "libswscale/swscale.h"

#include "broov_font.h"
#include "broov_queue.h"
#include "subreader.h"

int SDL_ANDROID_CallJavaExitFromNativePlayerView();

#ifdef __cplusplus
}
#endif

#define SDL_AUDIO_BUFFER_SIZE                1024

#define FF_ALLOC_EVENT                       (SDL_USEREVENT)
#define FF_REFRESH_EVENT                     (SDL_USEREVENT + 1)
#define FF_QUIT_EVENT                        (SDL_USEREVENT + 2)
#define FF_PAUSE_EVENT                       (SDL_USEREVENT + 3)
#define FF_PLAY_EVENT                        (SDL_USEREVENT + 4)
#define FF_ALLOC_RGB_EVENT                   (SDL_USEREVENT + 5)
#define FF_REFRESH_RGB_EVENT                 (SDL_USEREVENT + 6)

#define FF_GUI_PLAY_EVENT                    (SDL_USEREVENT + 7)
#define FF_GUI_PAUSE_EVENT                   (SDL_USEREVENT + 8)
#define FF_GUI_FORWARD_EVENT                 (SDL_USEREVENT + 9)
#define FF_GUI_REWIND_EVENT                  (SDL_USEREVENT + 10)
#define FF_GUI_PREV_EVENT                    (SDL_USEREVENT + 11)
#define FF_GUI_NEXT_EVENT                    (SDL_USEREVENT + 12)
#define FF_GUI_SEEK_EVENT                    (SDL_USEREVENT + 13)
#define FF_GUI_ASPECT_RATIO_EVENT            (SDL_USEREVENT + 14)
#define FF_FRAME_ALLOC_EVENT                 (SDL_USEREVENT + 15)

#define AV_SYNC_THRESHOLD                    0.01

#define AV_NOSYNC_THRESHOLD                  10.0

#define FRAME_SKIP_FACTOR                    0.05

/* maximum audio speed change to get correct sync */
#define SAMPLE_CORRECTION_PERCENT_MAX        10

/* we use about AUDIO_DIFF_AVG_NB A-V differences to make the average */
#define AUDIO_DIFF_AVG_NB                    20

#define VIDEO_PICTURE_QUEUE_SIZE             3
#define SUBPICTURE_QUEUE_SIZE                4

//160 KB (10 * 16 * 1024)
//#define MAX_AUDIOQ_SIZE (163840)
//512 KB (512 * 1024)
//#define MAX_AUDIOQ_SIZE (524288)

#define DEFAULT_AV_SYNC_TYPE AV_SYNC_AUDIO_MASTER
//#define DEFAULT_AV_SYNC_TYPE AV_SYNC_VIDEO_MASTER
//#define DEFAULT_AV_SYNC_TYPE AV_SYNC_EXTERNAL_CLOCK

#define FILE_NAME_SIZE 1024

typedef struct VideoPicture 
{
  int          width, height; /* source width & height */
  int          allocated;
  double       pts;
  double       target_clock;  ///<av_gettime() time at which this should be displayed ideally
  int64_t      pos;           ///<byte position in file
  enum PixelFormat pix_fmt;

  AVFrame      *pFrameRGB;
  int           numBytes;
  uint8_t      *buffer;
  SDL_TextureID display_rgb_text; 

  // Position of the Display Frame based on current aspect ratio
  //   4:3, 16:9, and full screen
  int          x, y;
  int          dst_width, dst_height; /* dest width & height */

} VideoPicture;

typedef struct VideoState 
{

#ifdef BROOV_VIDEO_THREAD
  SDL_Thread      *video_tid;
  PacketQueue      videoq;
#endif

  SDL_Thread      *parse_tid;
  AVFormatContext *pFormatCtx;

  int              videoStream;
  int              audioStream;

  int              av_sync_type;

  int              seek_req;
  int              seek_req_special;
  int              seek_flags;
  int64_t          seek_pos;
  int64_t          seek_rel;

  double           audio_clock;
  AVStream        *audio_st;
  PacketQueue      audioq;

  /* samples output by the codec. we reserve more space for avsync
     compensation */
  DECLARE_ALIGNED(16,uint8_t,audio_buf1)[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
  uint8_t *audio_buf;

  unsigned int     audio_buf_size;
  unsigned int     audio_buf_index;
  AVPacket         audio_pkt;
  uint8_t         *audio_pkt_data;
  int              audio_pkt_size;
  int              audio_hw_buf_size;  

  double           audio_diff_cum; /* used for AV difference average computation */
  double           audio_diff_avg_coef;
  double           audio_diff_threshold;
  int              audio_diff_avg_count;

  double           frame_timer;
  double           frame_last_pts;
  double           frame_last_delay;

  double           video_clock;             ///<pts of last decoded frame / predicted pts of next decoded frame
  double           video_current_pts;       ///<current displayed pts (different from video_clock if frame fifos are used)
  double           video_current_pts_drift; ///<video_current_pts - time (av_gettime) at which we updated video_current_pts - used to have running video pts
  int64_t          video_current_pts_time;  ///<time (av_gettime) at which we updated video_current_pts - used to have running video pts
  int64_t          video_current_pos;       ///<current displayed file pos
  AVStream        *video_st;

  VideoPicture     pictq[VIDEO_PICTURE_QUEUE_SIZE];
  int              pictq_size, pictq_rindex, pictq_windex;
  SDL_mutex       *pictq_mutex;
  SDL_cond        *pictq_cond;

  char             filename[FILE_NAME_SIZE];

  int              abort_request;

  int              paused;
  int              last_paused;
  int              read_pause_return;

  int              refresh;

  AVPacket audio_pkt_temp;

  int use_sub;

  struct SwsContext *img_convert_ctx;

} VideoState;


enum 
{
    AV_SYNC_AUDIO_MASTER, /* default choice */
    AV_SYNC_VIDEO_MASTER,
    AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};

//File playing sequence of methods
int player_init(char *font_fname, int subtitle_show, int subtitle_font_size, int subtitle_encoding_type, int rgb565);

int player_main(int argc, char *argv[], 
		int loop_after_play, int audio_file_type, int skip_frames, int rgb565, int yuv_rgb_asm,
                int skip_bidir_frames, int vqueue_size_min, int vqueue_size_max, int total_queue_size, int audio_queue_size);
int player_exit();

//Player status and control methods

// Current Duration and Total Duration of the playing file
int player_duration();
int player_total_duration();

// Play and Pause Button Clicks
int player_play();
int player_pause();

// Forward and Rewind Button Clicks
int player_forward();
int player_rewind();

// Previous and Next Button clicks
int player_prev();
int player_next();

// Seek to the given percentage in the playing file
int player_seek(int percentage);

// Set the aspect ratio to 4:3, 16:9, and full screen for the playing file
int player_set_aspect_ratio(int aspect_ratio_type);


//ARMV5
//#define BROOV_VIDEO_MIN_BUFFER_SIZE  256000
//#define BROOV_VIDEO_MAX_BUFFER_SIZE  2048576 
//#define BROOV_TOTAL_MAX_BUFFER_SIZE  3048576
//#define MIN_TIME_TO_WAIT_FOR_SKIP    0.25
//512 KB (512 * 1024)
//#define MAX_AUDIOQ_SIZE (524288)

//ARMV6VFP
//#define BROOV_VIDEO_MIN_BUFFER_SIZE 256000
//#define BROOV_VIDEO_MAX_BUFFER_SIZE 3048576 
//#define BROOV_TOTAL_MAX_BUFFER_SIZE 4048576
//#define MIN_TIME_TO_WAIT_FOR_SKIP   0.25
//512 KB (512 * 1024)
//#define MAX_AUDIOQ_SIZE (524288)

//NEON
//#define BROOV_VIDEO_MIN_BUFFER_SIZE 512000
//#define BROOV_VIDEO_MAX_BUFFER_SIZE 12048576 
//#define BROOV_TOTAL_MAX_BUFFER_SIZE 15048576
//#define MIN_TIME_TO_WAIT_FOR_SKIP   0.25
//3MB
//#define MAX_AUDIOQ_SIZE (3048576)

//X86
//#define BROOV_VIDEO_MIN_BUFFER_SIZE 256000
//#define BROOV_VIDEO_MAX_BUFFER_SIZE 2048576 
//#define BROOV_TOTAL_MAX_BUFFER_SIZE 25048576
//#define MIN_TIME_TO_WAIT_FOR_SKIP   0.25
//3MB
//#define MAX_AUDIOQ_SIZE (524288)

//GENERIC values for Buffer sizes
//#define BROOV_VIDEO_MIN_BUFFER_SIZE  256000
//#define BROOV_VIDEO_MAX_BUFFER_SIZE  2048576 
//#define BROOV_TOTAL_MAX_BUFFER_SIZE  3048576
#define MIN_TIME_TO_WAIT_FOR_SKIP    0.25

//512 KB (512 * 1024)
//#define MAX_AUDIOQ_SIZE (524288)

#endif /* #ifndef BROOV_PLAYER_H */
