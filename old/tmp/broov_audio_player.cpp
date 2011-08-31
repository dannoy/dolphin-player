// Android Audio Player - Based on FFPlay
//  
// Description:
//   - Support for playing audio files
//   - Support for pausing and playing of audio files
//   - Support for forward and rewind of audio files

#include "broov_audio_player.h"

extern JavaVM *gBroovJniVM;
extern AVPacket           flush_pkt;

static int     g_autoexit       = 0;
static int     g_loop           = 0;
static int     g_debug          = 0;
static int64_t start_time       = AV_NOPTS_VALUE;
static int64_t duration         = AV_NOPTS_VALUE;

/* Since we only have one decoding thread, the Big Struct
   can be global in case we need it. */
static AudioState *global_audio_state;
static AudioState *cur_stream;
static int64_t     audio_callback_time;

static int debug_mv = 0;
static int step = 0;
static int thread_count = 1;
static int workaround_bugs = 1;
static int fast = 1;
static int genpts = 0;
static int lowres = 0;
static int idct = FF_IDCT_AUTO;

static int error_recognition = FF_ER_CAREFUL;
static int error_concealment = 3;

static enum AVDiscard skip_frame= AVDISCARD_DEFAULT;
static enum AVDiscard skip_idct= AVDISCARD_DEFAULT;
static enum AVDiscard skip_loop_filter= AVDISCARD_DEFAULT;

static int seek_by_bytes=-1;

static int decoder_reorder_pts= -1;

static int av_sync_type = AUDIO_PLAYER_AV_SYNC_AUDIO_MASTER;

static const char *input_filename;
static int audio_disable;
static int video_disable=1;

static void do_exit(void);

/* get the current audio output buffer size, in samples. With SDL, we
   cannot have a precise information */
static int audio_write_get_buf_size(AudioState *is)
{
	return is->audio_buf_size - is->audio_buf_index;
}

static double get_audio_clock(AudioState *is) 
{
	double pts = 0;
	int hw_buf_size, bytes_per_sec;

	pts = is->audio_clock ; /* maintained in the audio thread */
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

static double get_external_clock(AudioState *is) 
{
	//return av_gettime() / 1000000.0;
	int64_t ti;
	ti = av_gettime();
	return is->external_clock + ((ti - is->external_clock_time) * 1e-6);

}

static double get_master_clock(AudioState *is) 
{
	double val;

        if ((is->av_sync_type == AUDIO_PLAYER_AV_SYNC_AUDIO_MASTER)  &&
	    (is->audio_st)) {
		val = get_audio_clock(is);
        }else {
		val = get_external_clock(is);
	}
	return val;

}

static void stream_seek(AudioState *is, int64_t pos, int64_t rel, int seek_by_bytes) 
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

static void stream_seek_special(AudioState *is, int64_t pos, int64_t rel, int seek_by_bytes) 
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


/* pause or resume the audio */
static void stream_pause(AudioState *is)
{
	is->paused = !is->paused;
}


/* return the new audio buffer size (samples can be added or deleted
   to get better sync if external master clock) */
static int synchronize_audio(AudioState *is, short *samples,
		int samples_size1, double pts) 
{
	int n;
	int samples_size;
	double ref_clock;

	n = 2 * is->audio_st->codec->channels;
	samples_size = samples_size1;

	/* if not master, then we try to remove or add samples to correct the clock */
	if (is->av_sync_type == AUDIO_PLAYER_AV_SYNC_EXTERNAL_CLOCK) {
	   
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
static int audio_decode_frame(AudioState *is, double *pts_ptr) 
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

	AudioState *is = (AudioState *)userdata;
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

/* return current time (in seconds) */
static double current_time(void)
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


static void calculate_duration(AVFormatContext *ic)
{
	if (ic && (ic->duration != AV_NOPTS_VALUE)) 
	{
		int secs;
		secs = ic->duration / AV_TIME_BASE;
		//us = ic->duration % AV_TIME_BASE;

		//broov_gui_set_total_duration(secs);
	} 
}

static int stream_component_open(AudioState *is, int stream_index) 
{
	AVFormatContext *pFormatCtx = is->pFormatCtx;
	AVCodecContext *codecCtx;
	AVCodec *codec;
	SDL_AudioSpec wanted_spec, spec;

	if (stream_index < 0 || stream_index >= pFormatCtx->nb_streams) {
		return -1;
	}

	// Get a pointer to the codec context for the stream
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

			break;

		default:
			break;
	}

}

static void stream_component_close(AudioState *is, int stream_index)
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
			__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Stream Component Close Video Done");
			break;

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
		default:
			break;
	}

} /* stream_component_close() */


static int decode_interrupt_cb(void) 
{
	return (global_audio_state && global_audio_state->abort_request);
}

/* this thread gets the stream from the disk or the network */
static int decode_thread(void *arg) 
{

	AudioState *is = (AudioState *)arg;
	AVFormatContext *pFormatCtx;
	AVPacket pkt1, *packet = &pkt1;

	int err;
	int ret = -1;
	int eof=0;
	int pkt_in_play_range = 0;

	int i;

	int video_index = -1;
	int audio_index = -1;

	is->videoStream=-1;
	is->audioStream=-1;

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Inside decodeThread");
	global_audio_state = is;

	// will interrupt blocking functions if we quit!
	url_set_interrupt_cb(decode_interrupt_cb);

	// Open audio from file
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


		if (ret < 0) {
			__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "%s: could not seek to position %0.3f\n",
					is->filename, (double)timestamp / AV_TIME_BASE);
		}
	}

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Before Calculate Duration");
	calculate_duration(pFormatCtx);

	// Find the first stream
	for (i=0; i<pFormatCtx->nb_streams; i++) {

		if (pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO &&
		    video_index < 0) {
			video_index=i;

		}

		if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_AUDIO &&
				audio_index < 0) {
			audio_index=i;
		}

	}

	if (audio_index >= 0) {
		stream_component_open(is, audio_index);
	} else {
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Audio not found");
	}


	if (is->audioStream < 0) {
		__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "%s: could not open codecs\n", is->filename);
		goto fail;
	}

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

				is->seek_req = 0;
				eof= 0;

			}
		}

                if (is->audioq.nb_packets>50 && is->audioStream>=0)
                {
			//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "AudioQ Size:%d", is->audioq.size);
			/* fine tune this delay for audio packet in Q */
			SDL_Delay(15);
			continue;
		}

		if (eof) {
				if (is->audioq.size==0) 
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

		if (packet->stream_index == is->audioStream) {
			packet_queue_put(&is->audioq, packet);
		}

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
	global_audio_state = NULL;

	/* close each stream */
	if (is->audioStream >= 0)
		stream_component_close(is, is->audioStream);

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

static AudioState *stream_open(const char *filename, AVInputFormat *iformat)
{
	AudioState *is;

	is = (AudioState *)av_mallocz(sizeof(AudioState));
	if (!is)
		return NULL;
	av_strlcpy(is->filename, filename, sizeof(is->filename));
	is->iformat = iformat;
	is->ytop = 0;
	is->xleft = 0;

	is->av_sync_type = av_sync_type;
	is->parse_tid = SDL_CreateThread(decode_thread, is);
	if (!is->parse_tid) {
		av_free(is);
		return NULL;
	}
	return is;
}

static void stream_close(AudioState *is)
{
	int i;

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Stream Close");
	/* XXX: use a special url_shutdown call to abort parse cleanly */
	is->abort_request = 1;
	SDL_WaitThread(is->parse_tid, NULL);

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
		stream_pause(cur_stream);

	}
}
int audio_player_init()
{
	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "AudioPlayerInit called");
	// Register all formats and codecs
	av_register_all();

	//if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTTHREAD)) 
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER)) 
	{
		return 2;
	}
	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "AudioPlayerInit done");

	return 0;
}

int audio_player_exit()
{
	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "AudioPlayerExit called");
	SDL_Quit();

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "AudioPlayerExit done");

	return 0;
}

int audio_player_done()
{
	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "AudioPlayerDone called");

	//SDL_SendQuit();

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "AudioPlayerDone finished");

	return 0;
}

int audio_player_main(int argc, char *argv[], 
		int loop_after_play)

{
	static int prev_x, prev_y;
	int ret_value = 0;

	int             iter = 0;
	SDL_Event       event;
	double          pts;
	AudioState      *is;

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Inside AudioPlayerMain:Loop_After_Play:%d ", loop_after_play);

	if (loop_after_play == 0) {
		g_autoexit = 1;
		g_loop = 0;
	} else if (loop_after_play == 1) {
		g_autoexit = 0;
		g_loop = 1;
	}

	seek_by_bytes = -1;
	start_time = AV_NOPTS_VALUE;
	duration   = AV_NOPTS_VALUE;

	is = (AudioState *)av_mallocz(sizeof(AudioState));

	strncpy(is->filename, argv[1], FILE_NAME_SIZE);

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
		//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "event.type:%d", event.type);
		{
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
							//__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Audio sync correction : %+.1fs", audio_clock_delta);
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
									if(cur_stream->audio_st >= 0 && cur_stream->audio_pkt.pos>=0){
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

				case SDL_FINGERDOWN:
					//toggle_pause();
					break;

				case SDL_FINGERMOTION:
					//toggle_pause();
					break;

				case FF_QUIT_EVENT:
					is->abort_request = 1;
					do_exit();
					goto audio_player_done;
					break;
				case SDL_QUIT:
					is->abort_request = 1;
					do_exit();
					goto audio_player_done;
					break;

				case SDL_MOUSEBUTTONUP:
				case SDL_MOUSEMOTION:
#if 0
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
#endif
					break;
				case SDL_MOUSEBUTTONDOWN:
#if 0
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
							goto audio_player_done;
							break;
						}
						else if (broov_gui_next_button_clicked(event.button.x, event.button.y))
						{
							ret_value = BROOV_NEXT_BUTTON_CLICKED; 
							is->abort_request = 1;
							do_exit();
							goto audio_player_done;
							break;
						}
						else if (frameShown && !g_audio_file_type)
						{
							broov_gui_hide();
							frameShown=0;
                                                        frame_shown_in_screen=0;
						        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "frameShown set to false");
						}
						else if (!frameShown && !g_audio_file_type) 
						{
						        //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "frameShown set to true");
							broov_gui_show();
							frameShown=1;
						}
					}
#endif
					break;

do_audio_seek_special:
					__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Do Seek Special");
				        if (cur_stream) {
					    stream_seek_special(cur_stream, (int64_t)(incr * AV_TIME_BASE), (int64_t)(incr * AV_TIME_BASE), 0);
					}

					break;



				default:
					break;
			}
		}
                SDL_Delay(1000);
	}

audio_player_done:

	__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "AudioPlayer is Quit now()");

	return ret_value;
}

