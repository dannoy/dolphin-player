#ifndef BROOV_AUDIO_PLAYER_H
#define BROOV_AUDIO_PLAYER_H

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

#include "broov_version.h"
#include "broov_types.h"

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

//Broov fine tuned size for player Options 1
//Hardcoded values from ffplay
#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_AUDIOQ_SIZE (20 * 16 * 1024)
#define MIN_FRAMES 5

#define MAX_AUDIOQ_SIZE 81920

#define AV_SYNC_THRESHOLD                    0.01

#define AV_NOSYNC_THRESHOLD                  10.0

#define FRAME_SKIP_FACTOR                    0.05

/* maximum audio speed change to get correct sync */
#define SAMPLE_CORRECTION_PERCENT_MAX        10

/* we use about AUDIO_DIFF_AVG_NB A-V differences to make the average */
#define AUDIO_DIFF_AVG_NB                    20


#define FILE_NAME_SIZE 1024

#define BPP 1

#define AUDIO_PLAYER_AV_SYNC_AUDIO_MASTER   0
#define AUDIO_PLAYER_AV_SYNC_EXTERNAL_CLOCK 1

typedef struct AudioState 
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
  AVStream        *video_st;
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

  SDL_Thread      *parse_tid;

  char             filename[FILE_NAME_SIZE];

  int              abort_request;

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

#ifdef BROOV_REFORMAT_CONTEXT
  enum AVSampleFormat audio_src_fmt;
  AVAudioConvert *reformat_ctx;
#endif

} AudioState;

int audio_player_init();
int audio_player_main(int argc, char *argv[], int loop_after_play); 
int audio_player_exit();
int audio_player_done();
#endif /* #ifndef BROOV_AUDIO_PLAYER_H */
