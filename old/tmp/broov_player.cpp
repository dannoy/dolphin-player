// Android Media Player - Based on FFPlay, KMP2, and dranger.com tutorials
//  
// Description:
//   - Support for playing audio and video files
//   - Support for subtitles from subtitle files
//   - Support for pausing and playing of audio & video files
//   - Support for forward and rewind of audio & video files
//   - Support for player on screen controls

//List of features to be enabled

#define BROOV_PLAYER_CONTROL
#define BROOV_PLAYER_REFRESH_THREAD
#define BROOV_PLAYER_DISPLAY_FPS

//#define BROOV_PLAYER_DISPLAY_RGB
#define BROOV_IMG_CONVERT

#include "broov_player.h"
#include "subreader.h"
#include "broov_font.h"

extern JavaVM *gBroovJniVM;

const char program_name[]     = "AV Player based on FFplay";
const int  program_birth_year = 2011;

static int     g_show_subtitle  = 1;
static int     g_autoexit       = 0;
static int     g_loop           = 0;
static int     g_debug          = 0;
static int     g_audio_file_type= 0;
static int64_t start_time       = AV_NOPTS_VALUE;
static int64_t duration         = AV_NOPTS_VALUE;

static int     audio_ms         = 100000;
static int     video_ms         = 10000;
static int     refresh_ms       = video_ms;

static SDL_Surface     *screen;

/* Since we only have one decoding thread, the Big Struct
   can be global in case we need it. */
static VideoState *global_video_state;
static VideoState *cur_stream;
static int64_t     audio_callback_time;

AVPacket           flush_pkt;

static int         is_full_screen   = 1;

#ifndef BROOV_IMG_CONVERT
static int         sws_flags = SWS_BICUBIC;
//static int         sws_flags = SWS_BILINEAR;
//static int         sws_flags = SWS_FAST_BILINEAR;
#endif

static int         rdftspeed=20;
static int         framedrop=0;

static int debug_mv = 0;
static int step = 0;
static int thread_count = 1;
static int workaround_bugs = 1;
static int fast = 1;
static int genpts = 0;
static int lowres = 0;
static int idct = FF_IDCT_AUTO;

static enum AVDiscard skip_frame= AVDISCARD_DEFAULT;
static enum AVDiscard skip_idct= AVDISCARD_DEFAULT;
static enum AVDiscard skip_loop_filter= AVDISCARD_DEFAULT;

static int error_recognition = FF_ER_CAREFUL;
static int error_concealment = 3;

static int seek_by_bytes=-1;

static int decoder_reorder_pts= -1;

static int av_sync_type = DEFAULT_AV_SYNC_TYPE;

static const char *input_filename;
static const char *window_title;
int fs_screen_width  = 640;
int fs_screen_height = 480;
static int fps_display_x = 500;
static int file_display_x = 200;
static int file_display_y = 160;
static int file_display_y2 = 180;
static int file_display_y3 = 200;
//static int fs_screen_width  = 320;
//static int fs_screen_height = 240;
static int screen_width = 0;
static int screen_height = 0;
static int frame_width = 0;
static int frame_height = 0;
static enum PixelFormat frame_pix_fmt = PIX_FMT_NONE;
static int audio_disable;
static int video_disable;

double player_fps;

uint64_t global_video_pkt_pts = AV_NOPTS_VALUE;
static double audio_clock_delta = 0;

static void do_exit(void);

/* get the current audio output buffer size, in samples. With SDL, we
   cannot have a precise information */
static int audio_write_get_buf_size(VideoState *is)
{
	return is->audio_buf_size - is->audio_buf_index;
}

static double get_audio_clock(VideoState *is) 
{
	double pts = 0;
	int hw_buf_size, bytes_per_sec;

	pts = is->audio_clock + audio_clock_delta; /* maintained in the audio thread */
	hw_buf_size = audio_write_get_buf_size(is);
	bytes_per_sec = 0;

	if (is->audio_st) {
	    bytes_per_sec = is->audio_st->codec->sample_rate * 2 * is->audio_st->codec->channels;
	}

	if (bytes_per_sec) {
	    pts -= (double)hw_buf_size / bytes_per_sec;
	}

	return pts;
}

/* get the current video clock value */
static double get_video_clock(VideoState *is)
{
	if (is->paused) {
		return is->video_current_pts;
	} else {
		return is->video_current_pts_drift + av_gettime() / 1000000.0;
	}
}

static double get_external_clock(VideoState *is) 
{
	//return av_gettime() / 1000000.0;
	int64_t ti;
	ti = av_gettime();
	return is->external_clock + ((ti - is->external_clock_time) * 1e-6);

}

static double get_master_clock(VideoState *is) 
{
	double val;

	if (is->av_sync_type == AV_SYNC_VIDEO_MASTER) {
		if (is->video_st)
			val = get_video_clock(is);
		else
			val = get_audio_clock(is);
	} else if (is->av_sync_type == AV_SYNC_AUDIO_MASTER) {
		if (is->audio_st)
			val = get_audio_clock(is);
		else
			val = get_video_clock(is);
	} else {
		val = get_external_clock(is);
	}
	return val;

}

void fill_rectangle(SDL_Surface *screen,
		int x, int y, int w, int h, int color)
{
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	SDL_FillRect(screen, &rect, color);
}

static void stream_seek(VideoState *is, int64_t pos, int64_t rel, int seek_by_bytes) 
{

	if(!is->seek_req) {
		is->seek_pos = pos;
		is->seek_rel = rel;
		is->seek_flags &= ~AVSEEK_FLAG_BYTE;

		if (seek_by_bytes)
			is->seek_flags |= AVSEEK_FLAG_BYTE;
		is->seek_req_special = 0;
		is->seek_req = 1;
	}
} /* stream_seek() */

static void stream_seek_special(VideoState *is, int64_t pos, int64_t rel, int seek_by_bytes) 
{

	if(!is->seek_req) {
		is->seek_pos = pos;
		is->seek_rel = rel;
		is->seek_flags &= ~AVSEEK_FLAG_BYTE;

		if (seek_by_bytes)
			is->seek_flags |= AVSEEK_FLAG_BYTE;
		is->seek_req_special = 1;
		is->seek_req = 1;
	}
} /* stream_seek() */


/* pause or resume the video */
static void stream_pause(VideoState *is)
{
	if (is->paused) {
		is->frame_timer += av_gettime() / 1000000.0 + is->video_current_pts_drift - is->video_current_pts;
		if(is->read_pause_return != AVERROR(ENOSYS)){
			is->video_current_pts = is->video_current_pts_drift + av_gettime() / 1000000.0;
		}
		is->video_current_pts_drift = is->video_current_pts - av_gettime() / 1000000.0;
	}
	is->paused = !is->paused;
}

static double compute_target_time(double frame_current_pts, VideoState *is)
{
	double delay, sync_threshold, diff;

	/* compute nominal delay */
	delay = frame_current_pts - is->frame_last_pts;
	if (delay <= 0 || delay >= 10.0) {
		/* if incorrect delay, use previous one */
		delay = is->frame_last_delay;
	} else {
		is->frame_last_delay = delay;
	}
	is->frame_last_pts = frame_current_pts;

	/* update delay to follow master synchronisation source */
	if (((is->av_sync_type == AV_SYNC_AUDIO_MASTER && is->audio_st) ||
				is->av_sync_type == AV_SYNC_EXTERNAL_CLOCK)) {
		/* if video is slave, we try to correct big delays by
		   duplicating or deleting a frame */
		diff = get_video_clock(is) - get_master_clock(is);

		/* skip or repeat frame. We take into account the
		   delay to compute the threshold. I still don't know
		   if it is the best guess */
		sync_threshold = FFMAX(AV_SYNC_THRESHOLD, delay);
		if (fabs(diff) < AV_NOSYNC_THRESHOLD) {
			if (diff <= -sync_threshold)
				delay = 0;
			else if (diff >= sync_threshold)
				delay = 2 * delay;
		}
	}
	is->frame_timer += delay;

#ifdef DEBUG_PLAYER
	{
		char log_msg[128];
		sprintf(log_msg, "video: delay=%0.3f actual_delay=%0.3f pts=%0.3f A-V=%f\n", delay, actual_delay, frame_current_pts, -diff);
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", log_msg);

	}
#endif

	return is->frame_timer;
}


/* return the new audio buffer size (samples can be added or deleted
   to get better sync if video or external master clock) */
static int synchronize_audio(VideoState *is, short *samples,
		int samples_size1, double pts) 
{
	int n;
	int samples_size;
	double ref_clock;

	n = 2 * is->audio_st->codec->channels;
	samples_size = samples_size1;

	/* if not master, then we try to remove or add samples to correct the clock */
	if (((is->av_sync_type == AV_SYNC_VIDEO_MASTER && is->video_st) ||
	      is->av_sync_type == AV_SYNC_EXTERNAL_CLOCK)) {
		double diff, avg_diff;
		int wanted_size, min_size, max_size, nb_samples;

		ref_clock = get_master_clock(is);
		diff = get_audio_clock(is) - ref_clock;

		if(diff < AV_NOSYNC_THRESHOLD) {
			// accumulate the diffs
			is->audio_diff_cum = diff + is->audio_diff_avg_coef
				* is->audio_diff_cum;
			if(is->audio_diff_avg_count < AUDIO_DIFF_AVG_NB) {
				/* not enough measures to have a correct estimate */
				is->audio_diff_avg_count++;
			} else {
				/* estimate the A-V difference */
				avg_diff = is->audio_diff_cum * (1.0 - is->audio_diff_avg_coef);

				if (fabs(avg_diff) >= is->audio_diff_threshold) {
					wanted_size = samples_size + ((int)(diff * is->audio_st->codec->sample_rate) * n);
					nb_samples = samples_size / n;

					min_size = ((nb_samples * (100 - SAMPLE_CORRECTION_PERCENT_MAX)) / 100) * n;
					max_size = ((nb_samples * (100 + SAMPLE_CORRECTION_PERCENT_MAX)) / 100) * n;
					if(wanted_size < min_size) {
						wanted_size = min_size;
					} else if (wanted_size > max_size) {
						wanted_size = max_size;
					}

					/* add or remove samples to correction the synchro */

					if (wanted_size < samples_size) {
						/* remove samples */
						samples_size = wanted_size;
					} else if(wanted_size > samples_size) {
						uint8_t *samples_end, *q;
						int nb;

						/* add samples by copying final sample*/
						nb = (samples_size - wanted_size);
						samples_end = (uint8_t *)samples + samples_size - n;
						q = samples_end + n;
						while(nb > 0) {
							memcpy(q, samples_end, n);
							q += n;
							nb -= n;
						}

						samples_size = wanted_size;
					}
				}
			}
		} else {

			/* too big difference : may be initial PTS errors, so
			   reset A-V filter */
			is->audio_diff_avg_count = 0;
			is->audio_diff_cum = 0;
		}
	}
	return samples_size;
}

/* decode one audio frame and returns its uncompressed size */
static int audio_decode_frame(VideoState *is, double *pts_ptr) 
{
	AVPacket *pkt_temp = &is->audio_pkt_temp;
	AVPacket *pkt = &is->audio_pkt;
	AVCodecContext *dec= is->audio_st->codec;
	int n, len1, data_size;
	double pts;

	for(;;) {
		/* NOTE: the audio packet can contain several frames */
		while (pkt_temp->size > 0) {
			data_size = sizeof(is->audio_buf1);
			len1 = avcodec_decode_audio3(dec,
					(int16_t *)is->audio_buf1, &data_size,
					pkt_temp);
			if (len1 < 0) {
				/* if error, we skip the frame */
				pkt_temp->size = 0;
				break;
			}

			pkt_temp->data += len1;
			pkt_temp->size -= len1;
			if (data_size <= 0)
				continue;

#ifdef BROOV_REFORMAT_CONTEXT
			if (dec->sample_fmt != is->audio_src_fmt) {
				if (is->reformat_ctx)
					av_audio_convert_free(is->reformat_ctx);
				is->reformat_ctx= av_audio_convert_alloc(AV_SAMPLE_FMT_S16, 1,
						dec->sample_fmt, 1, NULL, 0);
				if (!is->reformat_ctx) {
					fprintf(stderr, "Cannot convert %s sample format to %s sample format\n",
							av_get_sample_fmt_name(dec->sample_fmt),
							av_get_sample_fmt_name(AV_SAMPLE_FMT_S16));
					break;
				}
				is->audio_src_fmt= dec->sample_fmt;
			}

			if (is->reformat_ctx) {
				const void *ibuf[6]= {is->audio_buf1};
				void *obuf[6]= {is->audio_buf2};
				int istride[6]= {av_get_bits_per_sample_fmt(dec->sample_fmt)/8};
				int ostride[6]= {2};
				int len= data_size/istride[0];
				if (av_audio_convert(is->reformat_ctx, obuf, ostride, ibuf, istride, len)<0) {
					printf("av_audio_convert() failed\n");
					break;
				}
				is->audio_buf= is->audio_buf2;
				/* FIXME: existing code assume that data_size equals framesize*channels*2 remove this legacy cruft */

				data_size= len*2;
			}else{
				is->audio_buf= is->audio_buf1;
			}

#endif
			is->audio_buf= is->audio_buf1;

			/* if no pts, then compute it */
			pts = is->audio_clock;
			*pts_ptr = pts;
			n = 2 * dec->channels;
			is->audio_clock += (double)data_size /
				(double)(n * dec->sample_rate);

#ifdef DEBUG_PLAYER
			{
				static double last_clock;
				printf("audio: delay=%0.3f clock=%0.3f pts=%0.3f\n",
						is->audio_clock - last_clock,
						is->audio_clock, pts);
				last_clock = is->audio_clock;
			}
#endif
			return data_size;
		}

		/* free the current packet */
		if (pkt->data)
			av_free_packet(pkt);

		if (is->paused || is->audioq.abort_request) {
			return -1;
		}

		/* read next packet */
		if (packet_queue_get(&is->audioq, pkt, 1) < 0)
			return -1;
		if(pkt->data == flush_pkt.data){
			avcodec_flush_buffers(dec);
			continue;
		}

		pkt_temp->data = pkt->data;
		pkt_temp->size = pkt->size;

		/* if update the audio clock with the pts */
		if (pkt->pts != AV_NOPTS_VALUE) {
			is->audio_clock = av_q2d(is->audio_st->time_base)*pkt->pts;
		}
	}
}

static inline int compute_mod(int a, int b)
{
	a = a % b;
	if (a >= 0)
		return a;
	else
		return a + b;
}

static void audio_callback(void *userdata, Uint8 *stream, int len) 
{

	VideoState *is = (VideoState *)userdata;
	int len1, audio_size;
	double pts;

	audio_callback_time = av_gettime();

	//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "audio_callback::len::%d", len);

	while (len > 0) {

		if (is->audio_buf_index >= is->audio_buf_size) {
			/* We have already sent all our data; get more */
			audio_size = audio_decode_frame(is, &pts);
			if (audio_size < 0) {
				/* If error, output silence */
				is->audio_buf = is->audio_buf1;
				is->audio_buf_size = 1024;
				memset(is->audio_buf, 0, is->audio_buf_size);
			} else {

				audio_size = synchronize_audio(is, (int16_t *)is->audio_buf,
						audio_size, pts);
				is->audio_buf_size = audio_size;
			}
			is->audio_buf_index = 0;
		}
		len1 = is->audio_buf_size - is->audio_buf_index;
		if(len1 > len)
			len1 = len;
		memcpy(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, len1);
		len -= len1;
		stream += len1;
		is->audio_buf_index += len1;
	}
}

#ifdef BROOV_PLAYER_REFRESH_THREAD
static int refresh_thread(void *opaque)
{
	VideoState *is= (VideoState *)opaque;

	if (!is) {
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "refreshThread:is:Null");
		return -1;
	}

#ifdef BROOV_PLAYER_DISPLAY_RGB
	//Do not run the refresh thread for RGB based display
	if (refresh_ms == audio_ms) {
		while (!is->abort_request)
		{
			SDL_Event event;
			event.type = FF_REFRESH_EVENT;
			event.user.data1 = opaque;
			if (!is->refresh) {
				//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "VideoRefresh");
				is->refresh=1;
				SDL_PushEvent(&event);
			}

			usleep(refresh_ms); //Send the refresh event every 10ms.
		}

	}
#else
	while (!is->abort_request)
	{
		SDL_Event event;
		event.type = FF_REFRESH_EVENT;
		event.user.data1 = opaque;
		if (!is->refresh) {
		    //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "VideoRefresh");
		    is->refresh=1;
		    SDL_PushEvent(&event);
		}

		usleep(refresh_ms); //Send the refresh event every 10ms.
	}
#endif

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "RefreshThread Exit Success");

#ifdef BROOV_C
	(*gBroovJniVM)->DetachCurrentThread(gBroovJniVM);
#else
	gBroovJniVM->DetachCurrentThread();
#endif
	return 0;

} /* refresh_thread() */
#endif


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


static void calculate_frames_per_second()
{
	static double st=0;
	static double ct=0;
	static double seconds=0;
	static int frames=0;

	if (st == 0) {
		st = ct = seconds = current_time();
	}

	{
		double tt = current_time();

		double dt; //= tt - ct; //elapsed time

		ct = tt; //current time

		frames++;
		dt = ct - seconds;
		if (dt >= 5.0)
		{
			player_fps = frames/dt;

#ifdef ANDROID
			//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "%d frames in %3.1f seconds = %6.3f FPS", frames, dt, player_fps);
#endif
			seconds = ct;
			frames = 0;

		}

	}

}

void display_fps(SDL_Surface *screen)
{
        static double last_player_fps; 
	static char log_msg[16];
	//SDL_Rect rect = {0, 0, sw, sh};
	//SDL_SetRenderDrawColor(0, 0, 0, 0xFF);
	//SDL_RenderFillRect(&rect);
        if (last_player_fps != player_fps) {
	    sprintf(log_msg, "%6.1f FPS", player_fps);
	    __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", log_msg);
	    //fps_draw_msg(screen, fps_display_x, sy, log_msg); 
        }
        last_player_fps = player_fps;
}

void calculate_duration(AVFormatContext *ic)
{
	if (ic && (ic->duration != AV_NOPTS_VALUE)) 
	{
		int secs;
		secs = ic->duration / AV_TIME_BASE;
		//us = ic->duration % AV_TIME_BASE;

		broov_gui_set_total_duration(secs);
	} 
}

static void video_image_display(VideoState *is)
{
	SDL_Rect rect;
	VideoPicture *vp;
	AVPicture pict;
	float aspect_ratio;
	int w, h, x, y;
	int i;

	//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Inside video image display");

#ifdef SUBTITLE_FROM_STREAM
	SubPicture *sp;
#endif

#ifdef BROOV_PLAYER_DISPLAY_FPS
	calculate_frames_per_second();
#endif

	vp = &is->pictq[is->pictq_rindex];
	if (vp->bmp) {

		if (is->video_st->codec->sample_aspect_ratio.num == 0) {
			aspect_ratio = 0;
		} else {
			aspect_ratio = av_q2d(is->video_st->codec->sample_aspect_ratio) *
				is->video_st->codec->width / is->video_st->codec->height;
		}

		if (aspect_ratio <= 0.0) {
			aspect_ratio = (float)is->video_st->codec->width /
				(float)is->video_st->codec->height;
		}

		h = screen->h;
		w = ((int)rint(h * aspect_ratio)) & -3;

		if (w > screen->w) {
			w = screen->w;
			h = ((int)rint(w / aspect_ratio)) & -3;
		}
		x = (screen->w - w) / 2;
		y = (screen->h - h) / 2;

		// __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Display X:%d Y:%d W:%d H:%d ScreenW:%d ScreenH:%d", x, y, w, h, screen->w, screen->h);


#ifdef SUBTITLE_FROM_STREAM
		if (is->subtitle_st)
		{
			if (is->subpq_size > 0)
			{
				sp = &is->subpq[is->subpq_rindex];

				if (vp->pts >= sp->pts + ((float) sp->sub.start_display_time / 1000))
				{
					SDL_LockYUVOverlay (vp->bmp);

					pict.data[0] = vp->bmp->pixels[0];
					pict.data[1] = vp->bmp->pixels[2];
					pict.data[2] = vp->bmp->pixels[1];

					pict.linesize[0] = vp->bmp->pitches[0];
					pict.linesize[1] = vp->bmp->pitches[2];
					pict.linesize[2] = vp->bmp->pitches[1];

					for (i = 0; i < sp->sub.num_rects; i++)
						blend_subrect(&pict, sp->sub.rects[i],
								vp->bmp->w, vp->bmp->h);

					SDL_UnlockYUVOverlay (vp->bmp);
				}
			}
		}
#endif

		rect.x = x;
		rect.y = y;
		rect.w = w;
		rect.h = h;

		{
			//char log_msg[128];
			//sprintf(log_msg, "X:%d, Y:%d, W:%d, H:%d", rect.x, rect.y, rect.w, rect.h);
			//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", log_msg);
		}

		//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "B");
		SDL_DisplayYUVOverlayRenderCopy(vp->bmp, &rect);
		//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "A");

		if (is->use_sub)
		{
			//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Using subTitles");
			ULONG ulRefClock = get_master_clock(is) * 100;
			if (subInTime(ulRefClock)) 
			{
				subDisplay(screen);
			}
			else 
			{
				subClearDisplay(screen);
				subFindNext(ulRefClock);
			}

		}

#ifdef BROOV_PLAYER_DISPLAY_FPS
		//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Before FPS");
		display_fps(screen);
		//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "After FPS");
#endif

#ifdef BROOV_PLAYER_CONTROL
		if (previously_frame_shown && !frameShown) {

			broov_gui_hide_controls();
			previously_frame_shown++;
			if (previously_frame_shown >= 3) {
				previously_frame_shown = 0;

			}
		}

		if (frameShown)  {
			broov_gui_set_current_duration((ULONG)get_master_clock(is));
			broov_gui_render_all(screen);
			previously_frame_shown=1;
		} 
#endif
		SDL_RenderPresent();

	} 
}

static int try_to_set_best_video_mode(int w, int h)
{

	const SDL_VideoInfo *vi = SDL_GetVideoInfo();
	if (vi) 
	{
		fs_screen_width  = vi->current_w;
		fs_screen_height = vi->current_h;
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "FullScreen Width:%d Height:%d", fs_screen_width, fs_screen_height);

		fps_display_x = (int) ((double)560.0 * fs_screen_width/640.0);
		file_display_x = (int) ((double)200.0 * fs_screen_width/640.0);
		file_display_y = (int) ((double)120.0 * fs_screen_height/480.0);
		file_display_y2 = (int) ((double)160.0 * fs_screen_height/480.0);
		file_display_y3 = (int) ((double)200.0 * fs_screen_height/480.0);
		broov_gui_update_positions();
		w = fs_screen_width;
		h = fs_screen_height;
	}

	//int flags = SDL_SWSURFACE;
	//int flags = SDL_SWSURFACE|SDL_FULLSCREEN;

	//int flags = SDL_HWSURFACE | SDL_ASYNCBLIT | SDL_HWACCEL | 
	//                SDL_FULLSCREEN;

	int flags = SDL_HWSURFACE | SDL_ASYNCBLIT | SDL_HWACCEL | 
		SDL_RESIZABLE | SDL_FULLSCREEN;
	int bpp;

	bpp=24;
	screen = SDL_SetVideoMode(w, h, bpp, flags);

	if (!screen) {
		bpp=32;
		screen = SDL_SetVideoMode(w, h, bpp, flags);

		if (!screen) {
			bpp=24;
			flags = SDL_SWSURFACE;
			screen = SDL_SetVideoMode(w, h, bpp, flags);
			if (!screen) {
				bpp = 32;
				screen = SDL_SetVideoMode(w, h, bpp, flags);

			}
		}
	}

	if (screen) {
		return 1;
	}


	return 0;

} /* End of try_to_set_best_video_mode method */

static void video_refresh_timer(void *userdata) 
{

	VideoState *is = (VideoState *)userdata;
	VideoPicture *vp;

#ifdef SUBTITLE_FROM_STREAM
	SubPicture *sp, *sp2;
#endif

	if (is->video_st) {

retry:
		//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "VRT");

		if (is->pictq_size == 0) {
			//nothing to do, no picture to display in the queue
#if 0
#ifdef BROOV_PLAYER_CONTROL

		       if (!previously_frame_shown && frameShown)  {
			   broov_gui_set_current_duration((ULONG)get_master_clock(is));
			   broov_gui_render_all(screen);
			   previously_frame_shown=1;
		           SDL_RenderPresent();
		       } 
#endif
#endif



		} else {

			double frame_delay;
			double time= av_gettime()/1000000.0;
			double next_target;
			/* dequeue the picture */
			vp = &is->pictq[is->pictq_rindex];

			//Frame delay is already computed and timer accordingly started
			if (time < vp->target_clock) {
				return;
			}

			/* update current video pts */
			is->video_current_pts = vp->pts;
			is->video_current_pts_drift = is->video_current_pts - time;
			is->video_current_pos = vp->pos;
			is->video_current_pts_time = av_gettime();

			if (is->pictq_size > 1){
				VideoPicture *nextvp= &is->pictq[(is->pictq_rindex+1)%VIDEO_PICTURE_QUEUE_SIZE];
				//assert(nextvp->target_clock >= vp->target_clock);
				next_target= nextvp->target_clock;
			}else {
				next_target= vp->target_clock + is->video_clock - vp->pts; //FIXME pass durations cleanly
			}


#ifdef BROOV_PLAYER_UNOPTIMIZED
			if(framedrop && time > next_target){
				is->skip_frames *= 1.0 + FRAME_SKIP_FACTOR;
				if(is->pictq_size > 1 || time > next_target + 0.5){
					/* update queue size and signal for next picture */
					if (++is->pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE)
						is->pictq_rindex = 0;

					SDL_LockMutex(is->pictq_mutex);
					is->pictq_size--;
					SDL_CondSignal(is->pictq_cond);
					SDL_UnlockMutex(is->pictq_mutex);
					goto retry;
				}
			}
#endif


#ifdef SUBTITLE_FROM_STREAM
			if(is->subtitle_st) {
				if (is->subtitle_stream_changed) {
					SDL_LockMutex(is->subpq_mutex);

					while (is->subpq_size) {
						free_subpicture(&is->subpq[is->subpq_rindex]);

						/* update queue size and signal for next picture */
						if (++is->subpq_rindex == SUBPICTURE_QUEUE_SIZE)
							is->subpq_rindex = 0;

						is->subpq_size--;
					}
					is->subtitle_stream_changed = 0;

					SDL_CondSignal(is->subpq_cond);
					SDL_UnlockMutex(is->subpq_mutex);
				} else {
					if (is->subpq_size > 0) {
						sp = &is->subpq[is->subpq_rindex];

						if (is->subpq_size > 1)
							sp2 = &is->subpq[(is->subpq_rindex + 1) % SUBPICTURE_QUEUE_SIZE];
						else
							sp2 = NULL;

						if ((is->video_current_pts > (sp->pts + ((float) sp->sub.end_display_time / 1000)))
								|| (sp2 && is->video_current_pts > (sp2->pts + ((float) sp2->sub.start_display_time / 1000))))
						{
							free_subpicture(sp);

							/* update queue size and signal for next picture */
							if (++is->subpq_rindex == SUBPICTURE_QUEUE_SIZE)
								is->subpq_rindex = 0;

							SDL_LockMutex(is->subpq_mutex);
							is->subpq_size--;
							SDL_CondSignal(is->subpq_cond);
							SDL_UnlockMutex(is->subpq_mutex);
						}
					} //subpq size > 0

				} //else subtitle stream changed

			} // if subtitle stream
#endif

			/* show the picture! */
			__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Before Video Display");
			//if (frame_delay > 0.010) {
			//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "B");
			video_image_display(is);
			//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "A");
			//}
			__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "After Video Display");

			/* update queue for next picture! */
			if(++is->pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE) {
				is->pictq_rindex = 0;
			}
			SDL_LockMutex(is->pictq_mutex);
			is->pictq_size--;
			SDL_CondSignal(is->pictq_cond);
			SDL_UnlockMutex(is->pictq_mutex);

			//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "After Video DisplayExternalClock:%lf", av_gettime()/1000000.0);
			//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "After Video DisplayExternalClock:%lf VideoClock:%lf", get_external_clock(is), get_video_clock(is));

		}
	}
	else if (is->audio_st) {
		/* draw the next audio frame */
		if (!screen) {
			try_to_set_best_video_mode(fs_screen_width, fs_screen_height);
		}

		if (screen && g_audio_file_type) {

			if (previously_frame_shown && frameShown) {
				broov_gui_show_image(screen);
				subtitle_draw_msg(screen, file_display_x, file_display_y, is->fileshowname);
				subtitle_draw_msg(screen, file_display_x, file_display_y2, is->filetypename);
				subtitle_draw_msg(screen, file_display_x, file_display_y3, is->filesizename);

				previously_frame_shown++;
				if (previously_frame_shown >= 3) {
					previously_frame_shown = 0;
				}

			}

			if (!frameShown) 
			{
				broov_gui_show();
				broov_gui_show_image(screen);

				subtitle_draw_msg(screen, file_display_x, file_display_y, is->fileshowname);
				subtitle_draw_msg(screen, file_display_x, file_display_y2, is->filetypename);
				subtitle_draw_msg(screen, file_display_x, file_display_y3, is->filesizename);
				frameShown=1;
				previously_frame_shown=1;
			}


			broov_gui_set_current_duration((ULONG)get_master_clock(is));
			broov_gui_render_all(screen);
			SDL_RenderPresent();
		}

	} else {
		// none is matched

	}
	//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "E");

#ifdef DEBUG_PLAYER
	print_status(is);
#endif	

}

static void alloc_picture(void *userdata) 
{
	VideoState *is = (VideoState *)userdata;
	VideoPicture *vp;

	vp = &is->pictq[is->pictq_windex];
	if(vp->bmp) {
		// we already have one make another, bigger/smaller
		SDL_FreeYUVOverlay(vp->bmp);
	}

	vp->width = is->video_st->codec->width;
	vp->height = is->video_st->codec->height;
	vp->pix_fmt = is->video_st->codec->pix_fmt;

	// Allocate a place to put our YUV image on that screen
	vp->bmp = SDL_CreateYUVOverlay(vp->width, 
			vp->height,
			SDL_YV12_OVERLAY,
			screen);

	if (!vp->bmp || vp->bmp->pitches[0] < vp->width) {
		/* SDL allocates a buffer smaller than requested if the video
		 * overlay hardware is unable to support the requested size. */
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Error: the video system does not support an image\n"
				"size of %dx%d pixels. Try using -lowres or -vf \"scale=w:h\"\n"
				"to reduce the image size.\n", vp->width, vp->height );

		do_exit();

	} else {

		SDL_LockMutex(is->pictq_mutex);
		vp->allocated = 1;
		SDL_CondSignal(is->pictq_cond);
		SDL_UnlockMutex(is->pictq_mutex);
	}

}
#ifdef BROOV_PLAYER_DISPLAY_RGB
void SaveFrame(AVFrame *pFrame, int w, int h, int iFrame) 
{
   FILE *pFile;

   char szFilename[64];
   int y;

   //Open file
   sprintf(szFilename, "/sdcard/frame%d.ppm", iFrame);
   pFile = fopen(szFilename, "wb+");

   if (pFile == NULL) {
       sprintf(szFilename, "/mnt/sdcard/frame%d.ppm", iFrame);
       pFile = fopen(szFilename, "wb+");
       if (pFile == NULL) {
	  __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Unable to open file for save frame");
          return;
       }
   }

   // Write header
   fprintf(pFile, "P6\n%d %d\n255\n", w, h);

#ifdef BROOV_PLAYER_RGB_24
   // Write pixel data
   for (y=0; y<h; y++)
    fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, w*3, pFile);
#else
   // Write pixel data
   for (y=0; y<h; y++)
    fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, w*4, pFile);
#endif

   // Close file
   fclose(pFile);

}
static int display_rgb_picture(VideoState *is, AVFrame *pFrame, double pts, int64_t pos) 
{
        static int iFrame;
        int width=is->video_st->codec->width;
        int height=is->video_st->codec->height;
#ifdef BROOV_PLAYER_RGB_24
        int bpp=8;
        img_convert( (AVPicture *)is->pFrameRGBA, PIX_FMT_RGB24,
                     (AVPicture *)pFrame, is->video_st->codec->pix_fmt,
                     width, height);    
#else
        int bpp=8;
        img_convert( (AVPicture *)is->pFrameRGBA, PIX_FMT_RGB32,
                     (AVPicture *)pFrame, is->video_st->codec->pix_fmt,
                     width, height);    
#endif
                     
        //unsigned char *frameBuffer = is->pFrameRGBA->data[0]; 
        //SaveFrame(is->pFrameRGBA, width, height, iFrame++);

#ifdef BROOV_PLAYER_DISPLAY_FPS
	calculate_frames_per_second();
#endif
#ifdef BROOV_PLAYER_DISPLAY_FPS
	display_fps(screen);
#endif
	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Processed Display Picture LineSize:%d, W:%d H:%d", is->pFrameRGBA->linesize[0], width, height);
        SDL_Surface *image = SDL_CreateRGBSurfaceFrom(is->buffer, 
                            width, height, bpp,
                            is->pFrameRGBA->linesize[0], 
                            RMASK, GMASK, BMASK, AMASK);
	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Created RGB Surface");
          SDL_Rect dest= {0, 0, 100, 100};
          SDL_TextureID  text = SDL_CreateTextureFromSurface(0, image);
	  __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Created Texture RGB");
	  SDL_RenderCopy(text, NULL, &dest);
	  __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Copied Texture RGB");
	  SDL_RenderPresent();
	  __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Rendered Texture RGB");
	  SDL_DestroyTexture(text);
	  __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Free RGB surface");

          SDL_FreeSurface(image);


}
#endif

/**
 *
 * @param pts the dts of the pkt / pts of the frame and guessed if not known
 */
static int queue_picture(VideoState *is, AVFrame *pFrame, double pts, int64_t pos) 
{
	VideoPicture *vp;
	static enum PixelFormat src_pix_fmt;
	PixelFormat dst_pix_fmt;

	//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "VT QPB");

	/* wait until we have space for a new pic */
	SDL_LockMutex(is->pictq_mutex);

#ifdef BROOV_PLAYER_UNOPTIMIZED
	if(is->pictq_size>=VIDEO_PICTURE_QUEUE_SIZE && !is->refresh) {
		is->skip_frames= FFMAX(1.0 - FRAME_SKIP_FACTOR, is->skip_frames * (1.0-FRAME_SKIP_FACTOR));
	}
#endif

	while(is->pictq_size >= VIDEO_PICTURE_QUEUE_SIZE &&
	      !is->abort_request) {
		SDL_CondWait(is->pictq_cond, is->pictq_mutex);
	}
	SDL_UnlockMutex(is->pictq_mutex);

	if (is->videoq.abort_request)
		return -1;

	// windex is set to 0 initially
	vp = &is->pictq[is->pictq_windex];

	/* allocate or resize the buffer! */
	if (!vp->bmp ||
	    vp->width  != is->video_st->codec->width ||
	    vp->height != is->video_st->codec->height) {
		SDL_Event event;

		vp->allocated = 0;

		/* the allocation must be done in the main thread to avoid
		   locking problems */
		event.type = FF_ALLOC_EVENT;
		event.user.data1 = is;
		SDL_PushEvent(&event);

		/* wait until we have a picture allocated */
		SDL_LockMutex(is->pictq_mutex);
		while (!vp->allocated && !is->videoq.abort_request) {
			SDL_CondWait(is->pictq_cond, is->pictq_mutex);
		}
		SDL_UnlockMutex(is->pictq_mutex);
		if (is->videoq.abort_request) {
			return -1;
		}
	}

	/* if the frame is not skipped, then display it */
	if (vp->bmp) {
		AVPicture pict;

		//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "VT QP LOB");
		SDL_LockYUVOverlay(vp->bmp);
		//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "VT QP LOE");

		dst_pix_fmt = PIX_FMT_YUV420P;
		memset(&pict,0,sizeof(AVPicture));
		/* point pict at the queue */

		pict.data[0] = vp->bmp->pixels[0];
		pict.data[1] = vp->bmp->pixels[2];
		pict.data[2] = vp->bmp->pixels[1];

		pict.linesize[0] = vp->bmp->pitches[0];
		pict.linesize[1] = vp->bmp->pitches[2];
		pict.linesize[2] = vp->bmp->pitches[1];

		//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "VT QP ICB");
#ifdef BROOV_IMG_CONVERT
		img_convert(&pict, dst_pix_fmt,
				(AVPicture *)pFrame, is->video_st->codec->pix_fmt, 
				is->video_st->codec->width, is->video_st->codec->height);
#else

		is->img_convert_ctx = sws_getCachedContext(is->img_convert_ctx,
				vp->width, vp->height, vp->pix_fmt, vp->width,
				vp->height, dst_pix_fmt, sws_flags, 
				NULL, NULL, NULL);

		if (is->img_convert_ctx == NULL) {
			__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Unable to create a context");
			return 1;
		}
		sws_scale(is->img_convert_ctx, pFrame->data, pFrame->linesize,
				0, vp->height, pict.data, pict.linesize);
#endif
		//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "VT QP ICE");

		//{
		//  char log_msg[256];
		//  sprintf(log_msg, "Displaying Picture::width:%d, height:%d", is->video_st->codec->width, is->video_st->codec->height);
		//  __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", log_msg);
		//}

		//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "VT QP ULOB");
		SDL_UnlockYUVOverlay(vp->bmp);
		//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "VT QP ULOE");
		vp->pts = pts;
		vp->pos = pos;

		/* now we inform our display thread that we have a pic ready */
		if(++is->pictq_windex == VIDEO_PICTURE_QUEUE_SIZE) {
			is->pictq_windex = 0;
		}
		SDL_LockMutex(is->pictq_mutex);
		vp->target_clock= compute_target_time(vp->pts, is);

		is->pictq_size++;
		SDL_UnlockMutex(is->pictq_mutex);
	}

	//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "VT QPE");
	return 0;
}

/**
 * compute the exact PTS for the picture if it is omitted in the stream
 * @param pts1 the dts of the pkt / pts of the frame
 */
static int output_picture2(VideoState *is, AVFrame *src_frame, double pts1, int64_t pos)
{
	double frame_delay, pts;

	pts = pts1;

	if (pts != 0) {
		/* update video clock with pts, if present */
		is->video_clock = pts;
	} else {
		pts = is->video_clock;
	}

	/* update video clock for next frame */
	frame_delay = av_q2d(is->video_st->codec->time_base);
	/* for MPEG2, the frame can be repeated, so we update the
	   clock accordingly */
	frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);
	is->video_clock += frame_delay;

#ifdef DEBUG_PLAYER
	printf("frame_type=%c clock=%0.3f pts=%0.3f\n",
			av_get_pict_type_char(src_frame->pict_type), pts, pts1);
#endif

#if 0
	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer",
			"frame_type=%c clock=%0.3f pts=%0.3f\n",
			av_get_pict_type_char(src_frame->pict_type), pts, pts1);
#endif

#ifdef BROOV_PLAYER_DISPLAY_RGB
	return display_rgb_picture(is, src_frame, pts, pos);
#else
	//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Before QP");
	return queue_picture(is, src_frame, pts, pos);
#endif
}

static int get_video_frame(VideoState *is, AVFrame *frame, int64_t *pts, AVPacket *pkt)
{
	int len1, got_picture, i;

#ifdef BROOV_FINE_TUNING_TESTING
	static int alternateSkip;
        static double diff_window_min = 1.0;
        static double diff_window_median = 1.5;
        static double diff_window_max = 2.5;
        static double diff_window = diff_window_max;
#endif

	if (packet_queue_get(&is->videoq, pkt, 1) < 0)
		return -1;

	if (pkt->data == flush_pkt.data){
		avcodec_flush_buffers(is->video_st->codec);

		SDL_LockMutex(is->pictq_mutex);

		//Make sure there are no long delay timers (ideally we should just flush the que but thats harder)
		for(i=0; i<VIDEO_PICTURE_QUEUE_SIZE; i++){
			is->pictq[i].target_clock= 0;
		}

		while (is->pictq_size && !is->videoq.abort_request) {
			SDL_CondWait(is->pictq_cond, is->pictq_mutex);
		}
		is->video_current_pos= -1;
		SDL_UnlockMutex(is->pictq_mutex);

#ifdef BROOV_PTS_CORRECTION_CONTEXT
		init_pts_correction(&is->pts_ctx);
#endif
		is->frame_last_pts= AV_NOPTS_VALUE;
		is->frame_last_delay = 0;
		is->frame_timer = (double)av_gettime() / 1000000.0;
		is->skip_frames= 1;
		is->skip_frames_index= 0;
		return 0;
	}


	//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "PktType:%c", pkt->data[0]);

	/* NOTE: ipts is the PTS of the _first_ picture beginning in
	   this packet, if any */
	is->video_st->codec->reordered_opaque= pkt->pts;
	len1 = avcodec_decode_video2(is->video_st->codec,
			frame, &got_picture,
			pkt);

	if (got_picture) {
#ifdef BROOV_PTS_CORRECTION_CONTEXT
		if (decoder_reorder_pts == -1) {
			*pts = guess_correct_pts(&is->pts_ctx, frame->reordered_opaque, pkt->dts);
		} else 
			if (decoder_reorder_pts) {
				*pts = frame->reordered_opaque;
			} else {
				*pts = pkt->dts;
			}
#else
		*pts = pkt->dts;
#endif

		if (*pts == AV_NOPTS_VALUE) {
			*pts = 0;
		}
	}

	//            if (len1 < 0)
	//                break;
#ifdef BROOV_PLAYER_UNOPTIMIZED
	if (got_picture){
		is->skip_frames_index += 1;
		if(is->skip_frames_index >= is->skip_frames){
			is->skip_frames_index -= FFMAX(is->skip_frames, 1.0);
			return 1;
		}

	}
#else
	if (got_picture){
		//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "GPTS:%ld AudioClock:%3.2f", pkt->pts, is->audio_clock);

#ifdef BROOV_FINE_TUNING_TESTING
	/* Need to free the packet av_free_packet that is being skipped */
	{
		double diff = get_master_clock(is) - get_video_clock(is);
		if ((diff > diff_window)) {
			if (alternateSkip) {
			    alternateSkip = 0;
			} else {
			    alternateSkip = 1;
			}

                        //(pict_type == 'B' || pict_type == 'b') ||
                        char pict_type = av_get_pict_type_char(frame->pict_type);
			if ( 
                            (pict_type == 'B' || pict_type == 'b') ||
                            (frame->repeat_pict)) {
		                //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "VC:%3.2f MC:%3.2f Diff:%3.2f", get_video_clock(is), get_master_clock(is), diff);
				//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Skipping Frame: PictType:%c RepeatPict:%d", pict_type, frame->repeat_pict);
                                av_free_packet(pkt);
				return 0;
			}

                        if (diff_window >= diff_window_max) {
                            diff_window = diff_window_min;
                        } 

                        if (diff < diff_window_median) {
                            diff_window = diff_window_max;
                        }
		}
	}
#endif




		return 1;
	}
#endif
	return 0;
}



/* These are called whenever we allocate a frame
 * buffer. We use this to store the global_pts in
 * a frame at the time it is allocated.
 */
static int our_get_buffer(struct AVCodecContext *c, AVFrame *pic) 
{
	//int ret = avcodec_default_get_buffer(c, pic);
	//uint64_t *pts = (uint64_t *)av_malloc(sizeof(uint64_t));
	//*pts = global_video_pkt_pts;
	//pic->opaque = pts;
	//return ret;
	return avcodec_default_get_buffer(c, pic);
}
static void our_release_buffer(struct AVCodecContext *c, AVFrame *pic) 
{
	//if (pic) av_freep(&pic->opaque);
	avcodec_default_release_buffer(c, pic);
}

static int video_thread(void *arg) 
{
	VideoState *is = (VideoState *)arg;
	AVFrame *frame= avcodec_alloc_frame();

	int64_t pts_int;
	double pts;
	int ret;

	for(;;) {
		AVPacket pkt;

		while (is->paused && !is->videoq.abort_request) {
			//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Video Paused");
			SDL_Delay(10);
		}

		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "VT BFR");
		ret = get_video_frame(is, frame, &pts_int, &pkt);
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "VT AFR");

		if (ret < 0) goto the_end;

		if (!ret)
			continue;

		pts = pts_int*av_q2d(is->video_st->time_base);

		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "VT BOP");
		ret = output_picture2(is, frame, pts,  pkt.pos);
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "VT AOP");
	        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "After QP");
		av_free_packet(&pkt);

		if (ret < 0)
			goto the_end;

	}

the_end:
	av_free(frame);

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "VideoThread Aborting Done");

#ifdef BROOV_C
	(*gBroovJniVM)->DetachCurrentThread(gBroovJniVM);
#else
	gBroovJniVM->DetachCurrentThread();
#endif
	return 0;
}

static int stream_component_open(VideoState *is, int stream_index) 
{
	AVFormatContext *pFormatCtx = is->pFormatCtx;
	AVCodecContext *codecCtx;
	AVCodec *codec;
	SDL_AudioSpec wanted_spec, spec;

	if (stream_index < 0 || stream_index >= pFormatCtx->nb_streams) {
		return -1;
	}

	// Get a pointer to the codec context for the video stream
	codecCtx = pFormatCtx->streams[stream_index]->codec;

	/* prepare audio output */
	if (codecCtx->codec_type == AVMEDIA_TYPE_AUDIO) {
		if (codecCtx->channels > 0) {
			codecCtx->request_channels = FFMIN(2, codecCtx->channels);
		} else {
			codecCtx->request_channels = 2;
		}
	}


	codec = avcodec_find_decoder(codecCtx->codec_id);

#if 1
	codecCtx->debug_mv = debug_mv;
	codecCtx->debug = g_debug;
	codecCtx->workaround_bugs = workaround_bugs;
	codecCtx->lowres = lowres;
	if(lowres) codecCtx->flags |= CODEC_FLAG_EMU_EDGE;
	codecCtx->idct_algo= idct;
	if(fast) codecCtx->flags2 |= CODEC_FLAG2_FAST;
	codecCtx->skip_frame= skip_frame;
	codecCtx->skip_idct= skip_idct;
	codecCtx->skip_loop_filter= skip_loop_filter;
	codecCtx->error_recognition= error_recognition;
	codecCtx->error_concealment= error_concealment;
	//avcodec_thread_init(codecCtx, thread_count);

	//set_context_opts(codecCtx, avcodec_opts[codecCtx->codec_type], 0, codec);
#endif
	if(!codec || (avcodec_open(codecCtx, codec) < 0)) {
		fprintf(stderr, "Unsupported codec!\n");
		return -1;
	}

	/* prepare audio output */
	if(codecCtx->codec_type == CODEC_TYPE_AUDIO) {

		// Set audio settings from codec info
		wanted_spec.freq = codecCtx->sample_rate;
		wanted_spec.format = AUDIO_S16SYS;
		wanted_spec.channels = codecCtx->channels;
		wanted_spec.silence = 0;
		wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
		wanted_spec.callback = audio_callback;
		wanted_spec.userdata = is;

		if(SDL_OpenAudio(&wanted_spec, &spec) < 0) {
			fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
			return -1;
		}
		is->audio_hw_buf_size = spec.size;

#ifdef BROOV_REFORMAT_CONTEXT
		is->audio_src_fmt= AV_SAMPLE_FMT_S16;
#endif

	}

	pFormatCtx->streams[stream_index]->discard = AVDISCARD_DEFAULT;

	switch(codecCtx->codec_type) {
		case CODEC_TYPE_AUDIO:
			is->audioStream = stream_index;
			is->audio_st = pFormatCtx->streams[stream_index];
			is->audio_buf_size = 0;
			is->audio_buf_index = 0;

			/* averaging filter for audio sync */
			is->audio_diff_avg_coef = exp(log(0.01 / AUDIO_DIFF_AVG_NB));
			is->audio_diff_avg_count = 0;
			/* Correct audio only if larger error than this */
			is->audio_diff_threshold = 2.0 * SDL_AUDIO_BUFFER_SIZE / codecCtx->sample_rate;

			memset(&is->audio_pkt, 0, sizeof(is->audio_pkt));
			packet_queue_init(&is->audioq);
			SDL_PauseAudio(0);
			break;
		case CODEC_TYPE_VIDEO:
			is->videoStream = stream_index;
			is->video_st = pFormatCtx->streams[stream_index];

			//is->frame_timer = (double)av_gettime() / 1000000.0;
			//is->frame_last_delay = 40e-3;
			//is->video_current_pts_time = av_gettime();

			packet_queue_init(&is->videoq);
			is->video_tid = SDL_CreateThread(video_thread, is);
			codecCtx->get_buffer = our_get_buffer;
			codecCtx->release_buffer = our_release_buffer;

			break;

#ifdef SUBTITLE_FROM_STREAM
		case CODEC_TYPE_SUBTITLE:
			is->subtitleStream = stream_index;
			is->subtitle_st = pFormatCtx->streams[stream_index];
			packet_queue_init(&is->subtitleq);
			is->subtitle_tid = SDL_CreateThread(subtitle_thread, is);
			break;
#endif

		default:
			break;
	}

}

static void stream_component_close(VideoState *is, int stream_index)
{
	AVFormatContext *pFormatCtx = is->pFormatCtx;
	AVCodecContext *avctx;

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Stream Component Close");
	if (stream_index < 0 || stream_index >= pFormatCtx->nb_streams)
		return;
	avctx = pFormatCtx->streams[stream_index]->codec;

	switch(avctx->codec_type) {
		case AVMEDIA_TYPE_AUDIO:
			packet_queue_abort(&is->audioq);

			SDL_CloseAudio();

			packet_queue_end(&is->audioq);

#ifdef BROOV_REFORMAT_CONTEXT
			if (is->reformat_ctx)
				av_audio_convert_free(is->reformat_ctx);
			is->reformat_ctx = NULL;
#endif

			break;

		case AVMEDIA_TYPE_VIDEO:
			packet_queue_abort(&is->videoq);

			/* note: we also signal this mutex to make sure we deblock the
			   video thread in all cases */
			SDL_LockMutex(is->pictq_mutex);
			SDL_CondSignal(is->pictq_cond);
			SDL_UnlockMutex(is->pictq_mutex);

			SDL_WaitThread(is->video_tid, NULL);

			packet_queue_end(&is->videoq);
			__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Stream Component Close Video Done");
			break;

#ifdef SUBTITLE_FROM_STREAM
		case AVMEDIA_TYPE_SUBTITLE:
			packet_queue_abort(&is->subtitleq);

			/* note: we also signal this mutex to make sure we deblock the
			   video thread in all cases */
			SDL_LockMutex(is->subpq_mutex);
			//is->subtitle_stream_changed = 1;

			SDL_CondSignal(is->subpq_cond);
			SDL_UnlockMutex(is->subpq_mutex);

			SDL_WaitThread(is->subtitle_tid, NULL);

			packet_queue_end(&is->subtitleq);
			break;
#endif

		default:
			break;
	}

	pFormatCtx->streams[stream_index]->discard = AVDISCARD_ALL;
	avcodec_close(avctx);
	switch(avctx->codec_type) {
		case AVMEDIA_TYPE_AUDIO:
			is->audio_st = NULL;
			is->audioStream = -1;
			break;
		case AVMEDIA_TYPE_VIDEO:
			is->video_st = NULL;
			is->videoStream = -1;
			break;
#ifdef SUBTITLE_FROM_STREAM
		case AVMEDIA_TYPE_SUBTITLE:
			is->subtitle_st = NULL;
			is->subtitleStream = -1;
			break;
#endif
		default:
			break;
	}

} /* stream_component_close() */


static int decode_interrupt_cb(void) 
{
	return (global_video_state && global_video_state->abort_request);
}

/* this thread gets the stream from the disk or the network */
static int decode_thread(void *arg) 
{

	VideoState *is = (VideoState *)arg;
	AVFormatContext *pFormatCtx;
	AVPacket pkt1, *packet = &pkt1;

	int err;
	int ret = -1;
	int eof=0;
	int pkt_in_play_range = 0;

	int i;

	int video_index = -1;
	int audio_index = -1;

#ifdef SUBTITLE_FROM_STREAM
	int subtitle_index = -1;
#endif

	is->videoStream=-1;
	is->audioStream=-1;

#ifdef SUBTITLE_FROM_STREAM
	is->subtitleStream=-1;
#endif

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Inside decodeThread");
	global_video_state = is;

	// will interrupt blocking functions if we quit!
	url_set_interrupt_cb(decode_interrupt_cb);

	// Open video file
	err = av_open_input_file(&pFormatCtx, is->filename, NULL, 0, NULL);

	if (err < 0) {
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "File open failed");
		ret = -1;
		goto fail;

	}

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "File open successful");

	is->pFormatCtx = pFormatCtx;

	if (genpts) {
		pFormatCtx->flags |= AVFMT_FLAG_GENPTS;
	}

	// Retrieve stream information
	if (av_find_stream_info(pFormatCtx)<0) {
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "%s: could not find codec parameters\n", is->filename);

		ret = -1;
		goto fail;
	}

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Finding streams successful");

	if(pFormatCtx->pb)
		pFormatCtx->pb->eof_reached= 0; //FIXME hack, ffplay maybe should not use url_feof() to test for the end

	if (seek_by_bytes<0) {
		seek_by_bytes= !!(pFormatCtx->iformat->flags & AVFMT_TS_DISCONT);
	}

	/* if seeking requested, we execute it */
	if (start_time != AV_NOPTS_VALUE) {
		int64_t timestamp;

		timestamp = start_time;
		/* add the stream start time */
		if (pFormatCtx->start_time != AV_NOPTS_VALUE)
			timestamp += pFormatCtx->start_time;
		ret = avformat_seek_file(pFormatCtx, -1, INT64_MIN, timestamp, INT64_MAX, 0);

		if (ret < 0) {
			__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "%s: could not seek to position %0.3f\n",
					is->filename, (double)timestamp / AV_TIME_BASE);
		}
	}

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Before Calculate Duration");
	calculate_duration(pFormatCtx);

	// Find the first video stream
	for (i=0; i<pFormatCtx->nb_streams; i++) {

		if (pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO &&
				video_index < 0) {
			video_index=i;

			/* init subtitle here to avoid latency */
			if (g_show_subtitle) {
				is->use_sub = !subInit(is->filename, 
						1/av_q2d(pFormatCtx->streams[i]->time_base));
			} else {
				is->use_sub = 0;
			}
		}

		if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_AUDIO &&
				audio_index < 0) {
			audio_index=i;
		}

#ifdef SUBTITLE_FROM_STREAM
		if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_SUBTITLE &&
				subtitle_index < 0) {
			subtitle_index=i;
		}
#endif

	}

	if (audio_index >= 0) {
		stream_component_open(is, audio_index);
	} else {
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Audio not found");
	}

	if (video_index >= 0) {
		stream_component_open(is, video_index);

	}  else {
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Video not found");
	}

#ifdef BROOV_PLAYER_REFRESH_THREAD

	is->refresh_tid = SDL_CreateThread(refresh_thread, is);

	if(!is->refresh_tid) {
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Refresh Thread ID invalid");
	}
#endif 

#ifdef SUBTITLE_FROM_STREAM
	if (subtitle_index >= 0) {
		stream_component_open(is, subtitle_index);
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Subtitle found");
	} else {
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Subtitle not found");
	}
#endif

	if (is->videoStream < 0 && is->audioStream < 0) {
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "%s: could not open codecs\n", is->filename);
		goto fail;
	}

#ifdef BROOV_PLAYER_DISPLAY_RGB
	if (is->videoStream >= 0) {
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "RGB display code execution");
		is->pFrameRGBA = avcodec_alloc_frame();
		if (is->pFrameRGBA == NULL) {
			__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Failed to allocate RGB frame");
			goto fail;

		}
#ifdef BROOV_PLAYER_RGB_24
		is->numBytes = avpicture_get_size(PIX_FMT_RGB24, is->video_st->codec->width, is->video_st->codec->height); 
		is->buffer = (uint8_t *) av_malloc(is->numBytes * sizeof(uint8_t));
		avpicture_fill( (AVPicture*)is->pFrameRGBA, is->buffer, 
                                PIX_FMT_RGB24,
				is->video_st->codec->width, is->video_st->codec->height);
#else
		is->numBytes = avpicture_get_size(PIX_FMT_RGB32, is->video_st->codec->width, is->video_st->codec->height); 
		is->buffer = (uint8_t *) av_malloc(is->numBytes * sizeof(uint8_t));
		avpicture_fill( (AVPicture*)is->pFrameRGBA, is->buffer, 
                                PIX_FMT_RGB32,
				is->video_st->codec->width, is->video_st->codec->height);
#endif
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "RGB display: W:%d H:%d NumBytes:%d", is->video_st->codec->width, is->video_st->codec->height, is->numBytes);
	}
#endif

	// main decode loop

	for(;;) {
		if(is->abort_request) {
			__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "PlayerAbort received");
			break;
		}

		if (is->paused != is->last_paused) {
			is->last_paused = is->paused;
			if (is->paused)
				is->read_pause_return= av_read_pause(pFormatCtx);
			else
				av_read_play(pFormatCtx);
		}

		// seek stuff goes here
		if (is->seek_req) {
                        if (is->seek_req_special) {

		                /* add the stream start time */
		                if (is->pFormatCtx->start_time != AV_NOPTS_VALUE)
			           is->seek_pos+= pFormatCtx->start_time;
				ret = avformat_seek_file(is->pFormatCtx, -1, INT64_MIN, is->seek_pos, INT64_MAX, 0);
                                is->seek_req_special=0;

                        }
			else {
				int64_t seek_target = is->seek_pos;
				int64_t seek_min= is->seek_rel > 0 ? seek_target - is->seek_rel + 2: INT64_MIN;
				int64_t seek_max= is->seek_rel < 0 ? seek_target - is->seek_rel - 2: INT64_MAX;
				//FIXME the +-2 is due to rounding being not done in the correct direction in generation
				//of the seek_pos/seek_rel variables
				ret = avformat_seek_file(is->pFormatCtx, -1, seek_min, seek_target, seek_max, is->seek_flags);
			}

			if (ret < 0) {
				__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "%s: error while seeking\n", is->pFormatCtx->filename);
				is->seek_req = 0;
				eof= 0;
			} else {
				if (is->audioStream >= 0) {
					packet_queue_flush(&is->audioq);
					packet_queue_put(&is->audioq, &flush_pkt);
				}
#ifdef SUBTITLE_FROM_STREAM
				if (is->subtitleStream >= 0) {
					packet_queue_flush(&is->subtitleq);
					packet_queue_put(&is->subtitleq, &flush_pkt);
				}
#endif
				if (is->videoStream >= 0) {
					packet_queue_flush(&is->videoq);
					packet_queue_put(&is->videoq, &flush_pkt);
				}

				is->seek_req = 0;
				eof= 0;

			}
		}

#if 1

		/* if the queue are full, no need to read more */
                if (is->audioq.size > MAX_AUDIOQ_SIZE ||
                    is->videoq.size > MAX_VIDEOQ_SIZE)
                {
			/* wait 10 ms */
			SDL_Delay(20);
			continue;

		}
#else

#if 1
		/* if the queue are full, no need to read more */
		if (is->audioq.size + is->videoq.size > MAX_QUEUE_SIZE
		    || ((is->audioq.size  > MIN_AUDIOQ_SIZE || is->audioStream<0)
			&& (is->videoq.nb_packets > MIN_FRAMES || is->videoStream<0)
				   )) {
			/* wait 10 ms */
			SDL_Delay(10);
			continue;
		}
#else 
		/* if the queue are full, no need to read more */
		if (is->audioq.size > MAX_AUDIOQ_SIZE ||
				is->videoq.size > MAX_VIDEOQ_SIZE) {
			//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "QueueFull:AQ:%d VQ:%d", is->audioq.size, is->videoq.size);
			SDL_Delay(10);
			continue;
		}

#endif
#endif

		if (eof) {
			if (is->videoStream >= 0){
				av_init_packet(packet);
				packet->data=NULL;
				packet->size=0;
				packet->stream_index= is->videoStream;
				packet_queue_put(&is->videoq, packet);
			}
			SDL_Delay(10);
#ifdef SUBTITLE_FROM_STREAM
			if(is->audioq.size + is->videoq.size + is->subtitleq.size ==0)
#else
				if(is->audioq.size + is->videoq.size ==0) 
#endif
				{
					__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Decode Thread EOF");
					if (g_autoexit) {
						ret=AVERROR_EOF;
						goto fail;
					} 
					else if(g_loop==1){
						__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Looping the file");
						stream_seek(cur_stream, start_time != AV_NOPTS_VALUE ? start_time : 0, 0, 0);
					}
				}
			continue;
		}

		ret = av_read_frame(pFormatCtx, packet);

		if (ret < 0)  {

			if (ret == AVERROR_EOF || url_feof(pFormatCtx->pb))
				eof=1;
			if (url_ferror(pFormatCtx->pb)) {
				break;
			}

			SDL_Delay(100); /* no error; wait for user input */
			continue;
		}

#ifdef BROOV_PLAYER_UNOPTIMIZED
		/* check if packet is in play range specified by user, then queue, otherwise discard */
		pkt_in_play_range = duration == AV_NOPTS_VALUE || 
			(packet->pts - pFormatCtx->streams[packet->stream_index]->start_time) * 
			av_q2d(pFormatCtx->streams[packet->stream_index]->time_base) - 
			(double)(start_time != AV_NOPTS_VALUE ? start_time : 0)/1000000 
			<= ((double)duration/1000000);


		//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Duration:%lf", ((double) duration/1000000));

		// Is this a packet from the video stream?
		if(packet->stream_index == is->videoStream && pkt_in_play_range) {
			packet_queue_put(&is->videoq, packet);
			//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Frame put in VideoQ");
		} else if(packet->stream_index == is->audioStream && pkt_in_play_range) {

			packet_queue_put(&is->audioq, packet);
			//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Frame put in AudioQ");

		}
#else
		// Is this a packet from the video stream?
		if (packet->stream_index == is->videoStream) {
			packet_queue_put(&is->videoq, packet);
		} else if (packet->stream_index == is->audioStream) {
			packet_queue_put(&is->audioq, packet);
		}

#endif

#ifdef SUBTITLE_FROM_STREAM
		else if(packet->stream_index == is->subtitleStream && pkt_in_play_range){
			packet_queue_put(&is->subtitleq, packet);
		}
#endif
		else {
			av_free_packet(packet);
		}

	}

	/* all done - wait for it */
	while(!is->abort_request) {
		SDL_Delay(100);
	}

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Decode Thread Exit Success");
	ret = 0;
fail:
	if (eof) {
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "EOF Reached");

	}
	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Decode Thread Fail code");
	/* disable interrupting */
	global_video_state = NULL;

#ifdef BROOV_PLAYER_DISPLAY_RGB
        //Free the RGBA image
	if (is->videoStream >= 0) {
            av_free(is->buffer);
            av_free(is->pFrameRGBA);
        }
#endif

	if (is->videoStream >= 0)
		stream_component_close(is, is->videoStream);

	/* close each stream */
	if (is->audioStream >= 0)
		stream_component_close(is, is->audioStream);
#ifdef SUBTITLE_FROM_STREAM
	if (is->subtitleStream >= 0)
		stream_component_close(is, is->subtitleStream);
#endif
	if (is->pFormatCtx) {
		av_close_input_file(is->pFormatCtx);
		is->pFormatCtx = NULL; /* safety */
	}
	url_set_interrupt_cb(NULL);

	if (ret != 0) 
	{
		SDL_Event event;
		event.type = FF_QUIT_EVENT;
		event.user.data1 = is;
		SDL_PushEvent(&event);
	}

#ifdef BROOV_C
	(*gBroovJniVM)->DetachCurrentThread(gBroovJniVM);
#else
	gBroovJniVM->DetachCurrentThread();
#endif

	return 0;
} /* decode_thread() */

static VideoState *stream_open(const char *filename, AVInputFormat *iformat)
{
	VideoState *is;

	is = (VideoState *)av_mallocz(sizeof(VideoState));
	if (!is)
		return NULL;
	av_strlcpy(is->filename, filename, sizeof(is->filename));
	is->iformat = iformat;
	is->ytop = 0;
	is->xleft = 0;

	/* start video display */
	is->pictq_mutex = SDL_CreateMutex();
	is->pictq_cond = SDL_CreateCond();

#ifdef SUBTITLE_FROM_STREAM
	is->subpq_mutex = SDL_CreateMutex();
	is->subpq_cond = SDL_CreateCond();
#endif

	is->av_sync_type = av_sync_type;
	is->parse_tid = SDL_CreateThread(decode_thread, is);
	if (!is->parse_tid) {
		av_free(is);
		return NULL;
	}
	return is;
}

static void stream_close(VideoState *is)
{
	VideoPicture *vp;
	int i;

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Stream Close");
	/* XXX: use a special url_shutdown call to abort parse cleanly */
	is->abort_request = 1;
	SDL_WaitThread(is->parse_tid, NULL);

#ifdef BROOV_PLAYER_REFRESH_THREAD
	SDL_WaitThread(is->refresh_tid, NULL);
#endif

	//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Stream Close_WaitOver");

	/* free all pictures */
	for(i=0;i<VIDEO_PICTURE_QUEUE_SIZE; i++) {
		vp = &is->pictq[i];
		if (vp->bmp) {
			SDL_FreeYUVOverlay(vp->bmp);
			vp->bmp = NULL;
		}
	}
	//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Stream Close_FreedImages");
	SDL_DestroyMutex(is->pictq_mutex);
	SDL_DestroyCond(is->pictq_cond);
	//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Stream Close_FreedMutex");

#ifdef SUBTITLE_FROM_STREAM
	SDL_DestroyMutex(is->subpq_mutex);
	SDL_DestroyCond(is->subpq_cond);
#endif

#ifdef SWSCALE_CONTEXT
	if (is->img_convert_ctx)
		sws_freeContext(is->img_convert_ctx);
#endif

	av_free(is);
	//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Stream Close_FreeDS");
}

static void do_exit(void)
{
	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Playerdo_exit");
	if (cur_stream) {
		stream_close(cur_stream);
		cur_stream = NULL;
	}

}

static void toggle_pause(void)
{
	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Toggle Pause");
	if (cur_stream) {
		broov_gui_toggle_pause();
		stream_pause(cur_stream);

	}
}

int player_init(char *font_fname, int subtitle_show, int subtitle_font_size)
{
	// Register all formats and codecs
	av_register_all();

	//if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE)) 
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) 
	{
		return 2;
	}

	if (!try_to_set_best_video_mode(fs_screen_width, fs_screen_height)) {
		return 3;

	}

	{
		int font_init = broov_font_init(font_fname, subtitle_font_size);
		if (font_init == 0) {
			__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "BroovFont init success");

		} else {
			__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "BroovFont init failed");
		}
	}

	{
		int img_flags = IMG_INIT_JPG | IMG_INIT_PNG;
		int initted = IMG_Init(img_flags);

		if (initted & img_flags != img_flags) {
			__android_log_print(ANDROID_LOG_INFO, "libSDL", "SDL_Image Initialization failure");
			return (4);
		}

	}

	if (subtitle_show == 0) {
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Subtitle not to be shown");
		g_show_subtitle = 0;
	}
	else  {
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Subtitle to be shown");
		g_show_subtitle = 1;
	}

	//broov_gui_init_audio_image(BROOV_LOADING_IMAGE);
	//broov_gui_show_video_image(screen);

	broov_gui_init();

	return 0;
}

int player_exit()
{
	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "PlayerExit called");
	TTF_Quit();  /* remember to quit SDL_ttf */
	free_font(); /* remember to free any loaded font and glyph cache */
	subFree();

	broov_gui_clean();

	SDL_Quit();

	SDL_ANDROID_CallJavaExitFromNativePlayerView();
	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "PlayerExit done");

	return 0;
}

int player_main(int argc, char *argv[], 
		int loop_after_play, int audio_image_idx, int audio_file_type)

{
	static int prev_x, prev_y;
	int ret_value = 0;

	int             iter = 0;
	SDL_Event       event;
	double          pts;
	VideoState      *is;

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Loop_After_Play:%d Audio_Image_idx:%d", loop_after_play, audio_image_idx);
	if (loop_after_play == 0) {
		g_autoexit = 1;
		g_loop = 0;
	} else if (loop_after_play == 1) {
		g_autoexit = 0;
		g_loop = 1;
	}

	if (audio_file_type == 1) {
		g_audio_file_type = 1;
                refresh_ms = audio_ms;
	} else {
		// the file is a video file
		// load a black background screen
		g_audio_file_type = 0;
                refresh_ms = video_ms;
		audio_image_idx = BROOV_BLACK_SCREEN_IMAGE;
	}
	seek_by_bytes = -1;
	start_time = AV_NOPTS_VALUE;
	duration   = AV_NOPTS_VALUE;
	broov_gui_init_ds();

	broov_gui_init_audio_image(audio_image_idx);

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Before Frame Display");
	//broov_gui_test_display_image(screen);
	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "After Frame Display");
        //SDL_Delay(2500);

	is = (VideoState *)av_mallocz(sizeof(VideoState));

	strncpy(is->filename, argv[1], FILE_NAME_SIZE);
	strncpy(is->fileshowname, argv[2], FILE_NAME_SIZE);
	strncpy(is->filetypename, argv[3], FILE_NAME_SIZE);
	strncpy(is->filesizename, argv[4], FILE_NAME_SIZE);

	/* start video display */
	is->pictq_mutex = SDL_CreateMutex();
	is->pictq_cond = SDL_CreateCond();

#ifdef SUBTITLE_FROM_STREAM
	is->subpq_mutex = SDL_CreateMutex();
	is->subpq_cond  = SDL_CreateCond();
#endif 

	is->av_sync_type = av_sync_type;
	is->parse_tid = SDL_CreateThread(decode_thread, is);
	if (!is->parse_tid) {
	     av_free(is);
	     return -1;
	}

	cur_stream = is;

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Decode Thread Creation Successful");

	av_init_packet(&flush_pkt);
	flush_pkt.data = (uint8_t *)"FLUSH";

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Flush Pkt Init Successful");

	for(;;) {
		double incr, pos;
		SDL_WaitEvent(&event);
#ifdef BROOV_PLAYER_CONTROL
		if (frameShown)  {
			broov_gui_process_event(&event);
		}
#endif
		{
			//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "event.type:%d", event.type);
			switch(event.type) {
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym) {
						case SDLK_p:
						case SDLK_SPACE:
						case SDLK_UP:
						case SDLK_DOWN:
							toggle_pause();
							break;

						case SDLK_d:
							g_debug = 1;
							break;
						case SDLK_z:
						case SDLK_x:
							audio_clock_delta += (event.key.keysym.sym == SDLK_z) ? -0.1 : 0.1;
							__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Audio sync correction : %+.1fs", audio_clock_delta);
							break;

						case SDLK_LEFT:
							__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Seek left 10s");
							incr = -10.0;
							goto do_seek;
						case SDLK_RIGHT:
							incr = 10.0;
							__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Seek right");
							goto do_seek;
do_seek:

					                __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Do Seek");
							if (cur_stream) {
								if (seek_by_bytes) {
									if (cur_stream->video_st >= 0 && cur_stream->video_current_pos>=0){
										pos= cur_stream->video_current_pos;
									}else if(cur_stream->audio_st >= 0 && cur_stream->audio_pkt.pos>=0){
										pos= cur_stream->audio_pkt.pos;
									}else {
										pos = url_ftell(cur_stream->pFormatCtx->pb);
									}
									if (cur_stream->pFormatCtx->bit_rate)
										incr *= cur_stream->pFormatCtx->bit_rate / 8.0;
									else
										incr *= 180000.0;
									pos += incr;
									stream_seek(cur_stream, pos, incr, 1);
								} else {
									pos = get_master_clock(cur_stream);
									pos += incr;
									stream_seek(cur_stream, (int64_t)(pos * AV_TIME_BASE), (int64_t)(incr * AV_TIME_BASE), 0);
								}
							}
							break;

						default:
							break;

					}
					break;
				case SDL_ACTIVEEVENT:
					{
                                                //if (event.active.gain == 0 && active.state & SDL_APPACTIVE) {
						//    __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "VideoResize event received W:%d, H:%d", event.resize.w, event.resize.h);
						//    try_to_set_best_video_mode(event.resize.w, event.resize.h);
                                                //}
					}
					break;
				case SDL_VIDEORESIZE:
					{
						__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "VideoResize event received W:%d, H:%d", event.resize.w, event.resize.h);
						try_to_set_best_video_mode(event.resize.w, event.resize.h);
					}
					break;

				case SDL_FINGERDOWN:
					//toggle_pause();
					break;

				case SDL_FINGERMOTION:
					//toggle_pause();
					break;

				case FF_QUIT_EVENT:
					is->abort_request = 1;
					do_exit();
					goto player_done;
					break;
				case SDL_QUIT:
					is->abort_request = 1;
					do_exit();
					goto player_done;
					break;
				case FF_ALLOC_EVENT:
					alloc_picture(event.user.data1);
					break;
				case FF_REFRESH_EVENT:
					video_refresh_timer(event.user.data1);
					//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "VideoRefresh Done");
					cur_stream->refresh=0;
					break;
				case SDL_MOUSEBUTTONUP:
				case SDL_MOUSEMOTION:
					{
						int x= event.button.x; 
						int y= event.button.y; 

						char log_msg[64];
						sprintf(log_msg, "Mouse Up: X:%d, Y:%d, State:%d", x, y, event.motion.state);
						__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", log_msg);

						if ((x== prev_x) &&
						    (y== prev_y)) {
							/* same x & y is received. Hence skip this event */
							break;
						}
						prev_x = x;
						prev_y = y;
						if (broov_gui_slider_used(event.button.x, event.button.y, prev_x))
						{
							if (slider_duration_to_incr == 0) {
								break;
							}
							incr = slider_duration_to_incr;
							goto do_seek_special;
						}

					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					{
						int x= event.button.x; 
						int y= event.button.y; 
						char log_msg[64];
						sprintf(log_msg, "Mouse Down: X:%d, Y:%d, State:%d", x, y, event.motion.state);
						__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", log_msg);

						if (broov_gui_play_button_clicked(event.button.x, event.button.y))
						{
							toggle_pause();
							break;
						}
						else if (broov_gui_slider_used(event.button.x, event.button.y, prev_x))
						{
							if ((x== prev_x) &&
							    (y== prev_y)) {

								/* same x & y is 
								   received. 
								   Hence skip this 
								   event */
								break;
							}

							prev_x = x;
							prev_y = y;

							if (slider_duration_to_incr == 0) {
								break;
							}
							incr = slider_duration_to_incr;
							goto do_seek_special;
						}
						else if (broov_gui_prev_button_clicked(event.button.x, event.button.y))
						{
							ret_value = BROOV_PREV_BUTTON_CLICKED; 
							is->abort_request = 1;
							do_exit();
							goto player_done;
							break;
						}
						else if (broov_gui_next_button_clicked(event.button.x, event.button.y))
						{
							ret_value = BROOV_NEXT_BUTTON_CLICKED; 
							is->abort_request = 1;
							do_exit();
							goto player_done;
							break;
						}
						else if (frameShown && !g_audio_file_type)
						{
							broov_gui_hide();
							frameShown=0;

						        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "frameShown set to false");
						}
						else if (!frameShown && !g_audio_file_type) 
						{
						        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "frameShown set to true");
							broov_gui_show();
							frameShown=1;
						}
					}
					break;

do_seek_special:
					__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Do Seek Special");
				        if (cur_stream) {
					    stream_seek_special(cur_stream, (int64_t)(incr * AV_TIME_BASE), (int64_t)(incr * AV_TIME_BASE), 0);
					}

					break;



				default:
					break;
			}
		}
	}

player_done:

	//broov_gui_show_video_image(screen);

	//Free the allocated subtitles for the current video, if any
	subFree();
	broov_gui_clean_audio_image();

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Player is Quit now()");

	return ret_value;
}
