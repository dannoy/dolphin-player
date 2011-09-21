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

//Broov fine tuned size for player Options 1
//Hardcoded values from ffplay
#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_AUDIOQ_SIZE (20 * 16 * 1024)
#define MIN_FRAMES 5

//#define MAX_AUDIOQ_SIZE (5 * 16 * 1024)
//#define MAX_VIDEOQ_SIZE (5 * 1024 * 1024)

#define MAX_AUDIOQ_SIZE 81920
#define MAX_VIDEOQ_SIZE 5242880 
//(5 * 1024 * 1024) = 5242880
//(5 * 16 * 1024) = 81920

#define AV_SYNC_THRESHOLD                    0.01

#define AV_NOSYNC_THRESHOLD                  10.0

#define FRAME_SKIP_FACTOR                    0.05

/* maximum audio speed change to get correct sync */
#define SAMPLE_CORRECTION_PERCENT_MAX        10

/* we use about AUDIO_DIFF_AVG_NB A-V differences to make the average */
#define AUDIO_DIFF_AVG_NB                    20

/* VIDEO_PICTURE_QUEUE_SIZE should be 1, else shaking phenomenon occur.
   To be understood */
//#define VIDEO_PICTURE_QUEUE_SIZE             1
#define VIDEO_PICTURE_QUEUE_SIZE             2
#define SUBPICTURE_QUEUE_SIZE                4

#define DEFAULT_AV_SYNC_TYPE AV_SYNC_VIDEO_MASTER
//#define DEFAULT_AV_SYNC_TYPE AV_SYNC_EXTERNAL_CLOCK
//#define DEFAULT_AV_SYNC_TYPE AV_SYNC_AUDIO_MASTER
#define MAX_VIDEOFORMAT_SIZE 5242880

#define FILE_NAME_SIZE 1024

#define BPP 1

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

  int          x, y, dst_width, dst_height; /* dest width & height */

} VideoPicture;

typedef struct SubPicture 
{
  double     pts; /* presentation time stamp for this picture */
  AVSubtitle sub;
} SubPicture;

typedef struct VideoState 
{
  AVFormatContext *pFormatCtx;
  int              videoStream;
  int              audioStream;

  int              av_sync_type;
  double           external_clock; /* external clock base */
  int64_t          external_clock_time;
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
#ifdef BROOV_REFORMAT_CONTEXT
  DECLARE_ALIGNED(16,uint8_t,audio_buf2)[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
#endif

  uint8_t *audio_buf;
  //uint8_t          audio_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
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
  PacketQueue      videoq;

  VideoPicture     pictq[VIDEO_PICTURE_QUEUE_SIZE];
  int              pictq_size, pictq_rindex, pictq_windex;
  SDL_mutex       *pictq_mutex;
  SDL_cond        *pictq_cond;

  SDL_Thread      *parse_tid;
  SDL_Thread      *video_tid;

  SDL_Thread      *refresh_tid;

  char             filename[FILE_NAME_SIZE];

  int              abort_request;
  int              abort_from_gui;

  int              paused;
  int              last_paused;
  int              read_pause_return;

  float            skip_frames;
  float            skip_frames_index;

  int              refresh;

  int width, height, xleft, ytop;
  AVPacket audio_pkt_temp;

  AVInputFormat *iformat;

  int use_sub;

#ifdef BROOV_PTS_CORRECTION_CONTEXT
  PtsCorrectionContext pts_ctx;
#endif

#ifdef BROOV_REFORMAT_CONTEXT
  enum AVSampleFormat audio_src_fmt;
  AVAudioConvert *reformat_ctx;
#endif

  struct SwsContext *img_convert_ctx;

} VideoState;


enum 
{
    AV_SYNC_AUDIO_MASTER, /* default choice */
    AV_SYNC_VIDEO_MASTER,
    AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};

int player_init(char *font_fname, int subtitle_show, int subtitle_font_size, int subtitle_encoding_type);
int player_main(int argc, char *argv[],
                int loop_after_play, int audio_file_type, int skip_frames);
int player_exit();
int player_duration();
int player_total_duration();

int player_play();
int player_pause();
int player_forward();
int player_rewind();
int player_prev();
int player_next();
int player_seek(int percentage);
int player_set_aspect_ratio(int aspect_ratio_type);

double current_time(void);
#endif /* #ifndef BROOV_PLAYER_H */
