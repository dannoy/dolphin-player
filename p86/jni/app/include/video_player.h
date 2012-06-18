#ifndef VIDEO_PLAYER_H
#define VIDEO_PLAYER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <math.h>
#include <limits.h>
#include <signal.h>

#include <unistd.h>
#include <assert.h>

#include "libavutil/avstring.h"
#include "libavutil/colorspace.h"
#include "libavutil/mathematics.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libavutil/dict.h"
#include "libavutil/parseutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/avassert.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/opt.h"
#include "libavcodec/avfft.h"
#include "libswresample/swresample.h"

# include "libavfilter/avcodec.h"
# include "libavfilter/avfilter.h"
# include "libavfilter/avfiltergraph.h"
# include "libavfilter/buffersink.h"

#include "libavcodec/avcodec.h"
#include "libavcodec/audioconvert.h"


#ifdef __cplusplus
}
#endif

#include "broov_player.h"

//File playing sequence of methods
int video_player_init(char *font_fname, int subtitle_show, int subtitle_font_size, int subtitle_encoding_type, int rgb565);

int video_player_main(int argc, char *argv[], 
		int loop_after_play, int audio_file_type, int skip_frames, int rgb565, int yuv_rgb_asm,
                int skip_bidir_frames, int vqueue_size_min, int vqueue_size_max, int total_queue_size, 
		int audio_queue_size, int fast_mode, int debug_mode, int sync_type, int seek_duration,
		int ffmpeg_flags);

int video_player_exit();

//Player status and control methods

// Current Duration and Total Duration of the playing file
int video_player_duration();
int video_player_total_duration();

// Play and Pause Button Clicks
int video_player_play();
int video_player_pause();

// Forward and Rewind Button Clicks
int video_player_forward();
int video_player_rewind();

// Previous and Next Button clicks
int video_player_prev();
int video_player_next();

// Seek to the given percentage in the playing file
int video_player_seek(int percentage);

// Set the aspect ratio to 4:3, 16:9, and full screen for the playing file
int video_player_set_aspect_ratio(int aspect_ratio_type);

#endif /* #ifndef VIDEO_PLAYER_H */
