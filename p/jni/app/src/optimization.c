#if 0
SDL_Init(Uint32 flags) == SDL_InitSubSystem
SDL_Quit(void) == SDL_QuitSubSystem

void SDL_QuitSubSystem(Uint32 flags)
{

    if ((flags & SDL_initialized & SDL_INIT_TIMER)) {
        SDL_TimerQuit();
        SDL_initialized &= ~SDL_INIT_TIMER;
    }
    if ((flags & SDL_initialized & SDL_INIT_AUDIO)) {
        SDL_AudioQuit();
        SDL_initialized &= ~SDL_INIT_AUDIO;
    }
    if ((flags & SDL_initialized & SDL_INIT_VIDEO)) {
        SDL_VideoQuit();
        SDL_initialized &= ~SDL_INIT_VIDEO;
    }
}

int SDL_InitSubSystem(Uint32 flags)
{

    /* Initialize the video/event subsystem */
    if ((flags & SDL_INIT_VIDEO) && !(SDL_initialized & SDL_INIT_VIDEO)) {
        if (SDL_VideoInit(NULL, (flags & SDL_INIT_EVENTTHREAD)) < 0) {
            return (-1);
        }
        SDL_initialized |= SDL_INIT_VIDEO;
    }

    /* Initialize the audio subsystem */
    if ((flags & SDL_INIT_AUDIO) && !(SDL_initialized & SDL_INIT_AUDIO)) {
        if (SDL_AudioInit(NULL) < 0) {
            return (-1);
        }
        SDL_initialized |= SDL_INIT_AUDIO;
    }

    /* Initialize the timer subsystem */
    if (!ticks_started) {
        SDL_StartTicks();
        ticks_started = 1;
    }
    if ((flags & SDL_INIT_TIMER) && !(SDL_initialized & SDL_INIT_TIMER)) {
        if (SDL_TimerInit() < 0) {
            return (-1);
        }
        SDL_initialized |= SDL_INIT_TIMER;
    }
    return (0);
}

int
SDL_AudioInit(const char *driver_name)
{
    int i = 0;
    int initialized = 0;
    int tried_to_init = 0;

    if (SDL_WasInit(SDL_INIT_AUDIO)) {
        SDL_AudioQuit();        /* shutdown driver if already running. */
    }

    SDL_memset(&current_audio, '\0', sizeof(current_audio));
    SDL_memset(open_devices, '\0', sizeof(open_devices));

    /* Select the proper audio driver */
    if (driver_name == NULL) {
        driver_name = SDL_getenv("SDL_AUDIODRIVER");
    }

    for (i = 0; (!initialized) && (bootstrap[i]); ++i) {
        /* make sure we should even try this driver before doing so... */
        const AudioBootStrap *backend = bootstrap[i];
        if (((driver_name) && (SDL_strcasecmp(backend->name, driver_name))) ||
            ((!driver_name) && (backend->demand_only))) {
            continue;
        }

        tried_to_init = 1;
        SDL_memset(&current_audio, 0, sizeof(current_audio));
        current_audio.name = backend->name;
        current_audio.desc = backend->desc;
        initialized = backend->init(&current_audio.impl);
    }

    if (!initialized) {
        /* specific drivers will set the error message if they fail... */
        if (!tried_to_init) {
            if (driver_name) {
                SDL_SetError("Audio target '%s' not available", driver_name);
            } else {
                SDL_SetError("No available audio device");
            }
        }

        SDL_memset(&current_audio, 0, sizeof(current_audio));
        return (-1);            /* No driver was available, so fail. */
    }

    finalize_audio_entry_points();

    return (0);
}

void
SDL_AudioQuit(void)
{
    SDL_AudioDeviceID i;
    for (i = 0; i < SDL_arraysize(open_devices); i++) {
        SDL_CloseAudioDevice(i);
    }

    /* Free the driver data */
    current_audio.impl.Deinitialize();
    SDL_memset(&current_audio, '\0', sizeof(current_audio));
    SDL_memset(open_devices, '\0', sizeof(open_devices));
}

SDL_CloseAudio(void)
{
    SDL_CloseAudioDevice(1);
}

int SDL_OpenAudio(SDL_AudioSpec * desired, SDL_AudioSpec * obtained)
{
    SDL_AudioDeviceID id = 0;

    /* Start up the audio driver, if necessary. This is legacy behaviour! */
    if (!SDL_WasInit(SDL_INIT_AUDIO)) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
            return (-1);
        }
    }

    /* SDL_OpenAudio() is legacy and can only act on Device ID #1. */
    if (open_devices[0] != NULL) {
        SDL_SetError("Audio device is already opened");
        return (-1);
    }

    if (obtained) {
        id = open_audio_device(NULL, 0, desired, obtained,
                               SDL_AUDIO_ALLOW_ANY_CHANGE, 1);
    } else {
        id = open_audio_device(NULL, 0, desired, desired, 0, 1);
    }
    if (id > 1) {               /* this should never happen in theory... */
        SDL_CloseAudioDevice(id);
        SDL_SetError("Internal error"); /* MUST be Device ID #1! */
        return (-1);
    }

    return ((id == 0) ? -1 : 0);
}

SDL_AudioDeviceID
SDL_OpenAudioDevice(const char *device, int iscapture,
                    const SDL_AudioSpec * desired, SDL_AudioSpec * obtained,
                    int allowed_changes)
{
    return open_audio_device(device, iscapture, desired, obtained,
                             allowed_changes, 2);
}

static SDL_AudioDeviceID
open_audio_device(const char *devname, int iscapture,
                  const SDL_AudioSpec * desired, SDL_AudioSpec * obtained,
                  int allowed_changes, int min_id)
{
    SDL_AudioDeviceID id = 0;
    SDL_AudioSpec _obtained;
    SDL_AudioDevice *device;
    SDL_bool build_cvt;
    int i = 0;

    if (!SDL_WasInit(SDL_INIT_AUDIO)) {
        SDL_SetError("Audio subsystem is not initialized");
        return 0;
    }

    if ((iscapture) && (!current_audio.impl.HasCaptureSupport)) {
        SDL_SetError("No capture support");
        return 0;
    }

    if (!obtained) {
        obtained = &_obtained;
    }
    if (!prepare_audiospec(desired, obtained)) {
        return 0;
    }

    /* If app doesn't care about a specific device, let the user override. */
    if (devname == NULL) {
        devname = SDL_getenv("SDL_AUDIO_DEVICE_NAME");
    }

    /*
     * Catch device names at the high level for the simple case...
     * This lets us have a basic "device enumeration" for systems that
     *  don't have multiple devices, but makes sure the device name is
     *  always NULL when it hits the low level.
     *
     * Also make sure that the simple case prevents multiple simultaneous
     *  opens of the default system device.
     */

    if ((iscapture) && (current_audio.impl.OnlyHasDefaultInputDevice)) {
        if ((devname) && (SDL_strcmp(devname, DEFAULT_INPUT_DEVNAME) != 0)) {
            SDL_SetError("No such device");
            return 0;
        }
        devname = NULL;

        for (i = 0; i < SDL_arraysize(open_devices); i++) {
            if ((open_devices[i]) && (open_devices[i]->iscapture)) {
                SDL_SetError("Audio device already open");
                return 0;
            }
        }
    }

    if ((!iscapture) && (current_audio.impl.OnlyHasDefaultOutputDevice)) {
        if ((devname) && (SDL_strcmp(devname, DEFAULT_OUTPUT_DEVNAME) != 0)) {
            SDL_SetError("No such device");
            return 0;
        }
        devname = NULL;

        for (i = 0; i < SDL_arraysize(open_devices); i++) {
            if ((open_devices[i]) && (!open_devices[i]->iscapture)) {
                SDL_SetError("Audio device already open");
                return 0;
            }
        }
    }

    device = (SDL_AudioDevice *) SDL_AllocAudioMem(sizeof(SDL_AudioDevice));
    if (device == NULL) {
        SDL_OutOfMemory();
        return 0;
    }
    SDL_memset(device, '\0', sizeof(SDL_AudioDevice));
    device->spec = *obtained;
    device->enabled = 1;
    device->paused = 1;
    device->iscapture = iscapture;

    /* Create a semaphore for locking the sound buffers */
    if (!current_audio.impl.SkipMixerLock) {
        device->mixer_lock = SDL_CreateMutex();
        if (device->mixer_lock == NULL) {
            close_audio_device(device);
            SDL_SetError("Couldn't create mixer lock");
            return 0;
        }
    }

    if (!current_audio.impl.OpenDevice(device, devname, iscapture)) {
        close_audio_device(device);
        return 0;
    }
    device->opened = 1;

    /* Allocate a fake audio memory buffer */
    device->fake_stream = (Uint8 *)SDL_AllocAudioMem(device->spec.size);
    if (device->fake_stream == NULL) {
        close_audio_device(device);
        SDL_OutOfMemory();
        return 0;
    }

    /* If the audio driver changes the buffer size, accept it */
    if (device->spec.samples != obtained->samples) {
        obtained->samples = device->spec.samples;
        SDL_CalculateAudioSpec(obtained);
    }

    /* See if we need to do any conversion */
    build_cvt = SDL_FALSE;
    if (obtained->freq != device->spec.freq) {
        if (allowed_changes & SDL_AUDIO_ALLOW_FREQUENCY_CHANGE) {
            obtained->freq = device->spec.freq;
        } else {
            build_cvt = SDL_TRUE;
        }
    }
    if (obtained->format != device->spec.format) {
        if (allowed_changes & SDL_AUDIO_ALLOW_FORMAT_CHANGE) {
            obtained->format = device->spec.format;
        } else {
            build_cvt = SDL_TRUE;
        }
    }
    if (obtained->channels != device->spec.channels) {
        if (allowed_changes & SDL_AUDIO_ALLOW_CHANNELS_CHANGE) {
            obtained->channels = device->spec.channels;
        } else {
            build_cvt = SDL_TRUE;
        }
    }
    if (build_cvt) {
        /* Build an audio conversion block */
        if (SDL_BuildAudioCVT(&device->convert,
                              obtained->format, obtained->channels,
                              obtained->freq,
                              device->spec.format, device->spec.channels,
                              device->spec.freq) < 0) {
            close_audio_device(device);
            return 0;
        }
        if (device->convert.needed) {
            device->convert.len = (int) (((double) obtained->size) /
                                         device->convert.len_ratio);

            device->convert.buf =
                (Uint8 *) SDL_AllocAudioMem(device->convert.len *
                                            device->convert.len_mult);
            if (device->convert.buf == NULL) {
                close_audio_device(device);
                SDL_OutOfMemory();
                return 0;
            }
        }
    }

    /* Find an available device ID and store the structure... */
    for (id = min_id - 1; id < SDL_arraysize(open_devices); id++) {
        if (open_devices[id] == NULL) {
            open_devices[id] = device;
            break;
        }
    }

    if (id == SDL_arraysize(open_devices)) {
        SDL_SetError("Too many open audio devices");
        close_audio_device(device);
        return 0;
    }

    /* Start the audio thread if necessary */
    if (!current_audio.impl.ProvidesOwnCallbackThread) {
        /* Start the audio thread */
/* !!! FIXME: this is nasty. */
#if (defined(__WIN32__) && !defined(_WIN32_WCE)) && !defined(HAVE_LIBC)
#undef SDL_CreateThread
        device->thread = SDL_CreateThread(SDL_RunAudio, device, NULL, NULL);
#else
        device->thread = SDL_CreateThread(SDL_RunAudio, device);
#endif
        if (device->thread == NULL) {
            SDL_CloseAudioDevice(id + 1);
            SDL_SetError("Couldn't create audio thread");
            return 0;
        }
    }

    return id + 1;
}

static void close_audio_device(SDL_AudioDevice * device)
{
    device->enabled = 0;
    if (device->thread != NULL) {
        SDL_WaitThread(device->thread, NULL);
    }
    if (device->mixer_lock != NULL) {
        SDL_DestroyMutex(device->mixer_lock);
    }
    if (device->fake_stream != NULL) {
        SDL_FreeAudioMem(device->fake_stream);
    }
    if (device->convert.needed) {
        SDL_FreeAudioMem(device->convert.buf);
    }
    if (device->opened) {
        current_audio.impl.CloseDevice(device);
        device->opened = 0;
    }
    SDL_FreeAudioMem(device);
}

/*
 * Sanity check desired AudioSpec for SDL_OpenAudio() in (orig).
 *  Fills in a sanitized copy in (prepared).
 *  Returns non-zero if okay, zero on fatal parameters in (orig).
 */
static int
prepare_audiospec(const SDL_AudioSpec * orig, SDL_AudioSpec * prepared)
{
    SDL_memcpy(prepared, orig, sizeof(SDL_AudioSpec));

    if (orig->callback == NULL) {
        SDL_SetError("SDL_OpenAudio() passed a NULL callback");
        return 0;
    }

    if (orig->freq == 0) {
        const char *env = SDL_getenv("SDL_AUDIO_FREQUENCY");
        if ((!env) || ((prepared->freq = SDL_atoi(env)) == 0)) {
            prepared->freq = 22050;     /* a reasonable default */
        }
    }

    if (orig->format == 0) {
        const char *env = SDL_getenv("SDL_AUDIO_FORMAT");
        if ((!env) || ((prepared->format = SDL_ParseAudioFormat(env)) == 0)) {
            prepared->format = AUDIO_S16;       /* a reasonable default */
        }
    }

    switch (orig->channels) {
    case 0:{
            const char *env = SDL_getenv("SDL_AUDIO_CHANNELS");
            if ((!env) || ((prepared->channels = (Uint8) SDL_atoi(env)) == 0)) {
                prepared->channels = 2; /* a reasonable default */
            }
            break;
        }
    case 1:                    /* Mono */
    case 2:                    /* Stereo */
    case 4:                    /* surround */
    case 6:                    /* surround with center and lfe */
        break;
    default:
        SDL_SetError("Unsupported number of audio channels.");
        return 0;
    }

    if (orig->samples == 0) {
        const char *env = SDL_getenv("SDL_AUDIO_SAMPLES");
        if ((!env) || ((prepared->samples = (Uint16) SDL_atoi(env)) == 0)) {
            /* Pick a default of ~46 ms at desired frequency */
            /* !!! FIXME: remove this when the non-Po2 resampling is in. */
            const int samples = (prepared->freq / 1000) * 46;
            int power2 = 1;
            while (power2 < samples) {
                power2 *= 2;
            }
            prepared->samples = power2;
        }
    }

    /* Calculate the silence and size of the audio specification */
    SDL_CalculateAudioSpec(prepared);

    return 1;
}

/* The general mixing thread function */
int SDLCALL
SDL_RunAudio(void *devicep)
{
    SDL_AudioDevice *device = (SDL_AudioDevice *) devicep;
    Uint8 *stream;
    int stream_len;
    void *udata;
    void (SDLCALL * fill) (void *userdata, Uint8 * stream, int len);
    int silence;
    Uint32 delay;

    /* For streaming when the buffer sizes don't match up */
    Uint8 *istream;
    int istream_len = 0;

    /* Perform any thread setup */
    device->threadid = SDL_ThreadID();
    current_audio.impl.ThreadInit(device);

    /* Set up the mixing function */
    fill = device->spec.callback;
    udata = device->spec.userdata;

    /* By default do not stream */
    device->use_streamer = 0;

    if (device->convert.needed) {
        if (device->convert.src_format == AUDIO_U8) {
            silence = 0x80;
        } else {
            silence = 0;
        }

#if 0                           
        /* !!! FIXME: I took len_div out of the structure. Use rate_incr instead? */
        /* If the result of the conversion alters the length, i.e. resampling is being used, use the streamer */
        if (device->convert.len_mult != 1 || device->convert.len_div != 1) {
            /* The streamer's maximum length should be twice whichever is larger: spec.size or len_cvt */
            stream_max_len = 2 * device->spec.size;
            if (device->convert.len_mult > device->convert.len_div) {
                stream_max_len *= device->convert.len_mult;
                stream_max_len /= device->convert.len_div;
            }
            if (SDL_StreamInit(&device->streamer, stream_max_len, silence) <
                0)
                return -1;
            device->use_streamer = 1;

            /* istream_len should be the length of what we grab from the callback and feed to conversion,
               so that we get close to spec_size. I.e. we want device.spec_size = istream_len * u / d
             */
            istream_len =
                device->spec.size * device->convert.len_div /
                device->convert.len_mult;
        }
#endif

        /* stream_len = device->convert.len; */
        stream_len = device->spec.size;
    } else {
        silence = device->spec.silence;
        stream_len = device->spec.size;
    }

    /* Calculate the delay while paused */
    delay = ((device->spec.samples * 1000) / device->spec.freq);

    /* Determine if the streamer is necessary here */
    if (device->use_streamer == 1) {
        /* This code is almost the same as the old code. The difference is, instead of reading
           directly from the callback into "stream", then converting and sending the audio off,
           we go: callback -> "istream" -> (conversion) -> streamer -> stream -> device.
           However, reading and writing with streamer are done separately:
           - We only call the callback and write to the streamer when the streamer does not
           contain enough samples to output to the device.
           - We only read from the streamer and tell the device to play when the streamer
           does have enough samples to output.
           This allows us to perform resampling in the conversion step, where the output of the
           resampling process can be any number. We will have to see what a good size for the
           stream's maximum length is, but I suspect 2*max(len_cvt, stream_len) is a good figure.
         */
        while (device->enabled) {

            if (device->paused) {
                SDL_Delay(delay);
                continue;
            }

            /* Only read in audio if the streamer doesn't have enough already (if it does not have enough samples to output) */
            if (SDL_StreamLength(&device->streamer) < stream_len) {
                /* Set up istream */
                if (device->convert.needed) {
                    if (device->convert.buf) {
                        istream = device->convert.buf;
                    } else {
                        continue;
                    }
                } else {
			/* FIXME: Ryan, this is probably wrong.  I imagine we don't want to get
			 * a device buffer both here and below in the stream output.
			 */
			istream = current_audio.impl.GetDeviceBuf(device);
                        if (istream == NULL) {
                            istream = device->fake_stream;
                        }
                }

                /* Read from the callback into the _input_ stream */
                SDL_mutexP(device->mixer_lock);
                (*fill) (udata, istream, istream_len);
                SDL_mutexV(device->mixer_lock);

                /* Convert the audio if necessary and write to the streamer */
                if (device->convert.needed) {
                    SDL_ConvertAudio(&device->convert);
                    if (istream == NULL) {
                        istream = device->fake_stream;
                    }
                    /*SDL_memcpy(istream, device->convert.buf, device->convert.len_cvt); */
                    SDL_StreamWrite(&device->streamer, device->convert.buf,
                                    device->convert.len_cvt);
                } else {
                    SDL_StreamWrite(&device->streamer, istream, istream_len);
                }
            }

            /* Only output audio if the streamer has enough to output */
            if (SDL_StreamLength(&device->streamer) >= stream_len) {
                /* Set up the output stream */
                if (device->convert.needed) {
                    if (device->convert.buf) {
                        stream = device->convert.buf;
                    } else {
                        continue;
                    }
                } else {
                    stream = current_audio.impl.GetDeviceBuf(device);
                    if (stream == NULL) {
                        stream = device->fake_stream;
                    }
                }

                /* Now read from the streamer */
                SDL_StreamRead(&device->streamer, stream, stream_len);

                /* Ready current buffer for play and change current buffer */
                if (stream != device->fake_stream) {
                    current_audio.impl.PlayDevice(device);
                    /* Wait for an audio buffer to become available */
                    current_audio.impl.WaitDevice(device);
                } else {
                    SDL_Delay(delay);
                }
            }

        }
    } else {
        /* Otherwise, do not use the streamer. This is the old code. */

        /* Loop, filling the audio buffers */
        while (device->enabled) {

            if (device->paused) {
                SDL_Delay(delay);
                continue;
            }

            /* Fill the current buffer with sound */
            if (device->convert.needed) {
                if (device->convert.buf) {
                    stream = device->convert.buf;
                } else {
                    continue;
                }
            } else {
                stream = current_audio.impl.GetDeviceBuf(device);
                if (stream == NULL) {
                    stream = device->fake_stream;
                }
            }

            SDL_mutexP(device->mixer_lock);
            (*fill) (udata, stream, stream_len);
            SDL_mutexV(device->mixer_lock);

            /* Convert the audio if necessary */
            if (device->convert.needed) {
                SDL_ConvertAudio(&device->convert);
                stream = current_audio.impl.GetDeviceBuf(device);
                if (stream == NULL) {
                    stream = device->fake_stream;
                }
                SDL_memcpy(stream, device->convert.buf,
                           device->convert.len_cvt);
            }

            /* Ready current buffer for play and change current buffer */
            if (stream != device->fake_stream) {
                current_audio.impl.PlayDevice(device);
                /* Wait for an audio buffer to become available */
                current_audio.impl.WaitDevice(device);
            } else {
                SDL_Delay(delay);
            }
        }
    }

    /* Wait for the audio to drain.. */
    current_audio.impl.WaitDone(device);

    /* If necessary, deinit the streamer */
    if (device->use_streamer == 1)
        SDL_StreamDeinit(&device->streamer);

    return (0);
}

static unsigned char * audioBuffer = NULL;
static size_t audioBufferSize = 0;

// Extremely wicked JNI environment to call Java functions from C code
static jbyteArray audioBufferJNI = NULL;
static JavaVM *jniVM = NULL;
static jobject JavaAudioThread = NULL;
static jmethodID JavaInitAudio = NULL;
static jmethodID JavaDeinitAudio = NULL;
static jmethodID JavaPauseAudioPlayback = NULL;
static jmethodID JavaResumeAudioPlayback = NULL;

JavaVM *gBroovJniVM = NULL;

static Uint8 *ANDROIDAUD_GetAudioBuf(_THIS)
{
	return(audioBuffer);
}


static int ANDROIDAUD_OpenAudio (_THIS, const char *devname, int iscapture)
{
	SDL_AudioSpec *audioFormat = &this->spec;

	int bytesPerSample;
	JNIEnv * jniEnv = NULL;

	this->hidden = NULL;

	if( ! (audioFormat->format == AUDIO_S8 || audioFormat->format == AUDIO_S16) )
	{
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "Application requested unsupported audio format - only S8 and S16 are supported");
		return (-1); // TODO: enable format conversion? Don't know how to do that in SDL
	}

	bytesPerSample = (audioFormat->format & 0xFF) / 8;
	audioFormat->format = ( bytesPerSample == 2 ) ? AUDIO_S16 : AUDIO_S8;

	__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_OpenAudio(): app requested audio bytespersample %d freq %d channels %d samples %d", bytesPerSample, audioFormat->freq, (int)audioFormat->channels, (int)audioFormat->samples);

	if(audioFormat->samples <= 0)
		audioFormat->samples = 128; // Some sane value
	if( audioFormat->samples > 32768 ) // Why anyone need so huge audio buffer?
	{
		audioFormat->samples = 32768;
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_OpenAudio(): limiting samples size to ", (int)audioFormat->samples);
	}
	
	SDL_CalculateAudioSpec(audioFormat);
	
	
	(*jniVM)->AttachCurrentThread(jniVM, &jniEnv, NULL);

	if( !jniEnv )
	{
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "ANDROIDAUD_OpenAudio: Java VM AttachCurrentThread() failed");
		return (-1); // TODO: enable format conversion? Don't know how to do that in SDL
	}

	// The returned audioBufferSize may be huge, up to 100 Kb for 44100 because user may have selected large audio buffer to get rid of choppy sound
	audioBufferSize = (*jniEnv)->CallIntMethod( jniEnv, JavaAudioThread, JavaInitAudio, 
					(jint)audioFormat->freq, (jint)audioFormat->channels, 
					(jint)(( bytesPerSample == 2 ) ? 1 : 0), (jint)(audioFormat->size) );

	if( audioBufferSize == 0 )
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_OpenAudio(): failed to get audio buffer from JNI");
		ANDROIDAUD_CloseAudio(this);
		return(-1);
	}

	/* We cannot call DetachCurrentThread() from main thread or we'll crash */
	/* (*jniVM)->DetachCurrentThread(jniVM); */

	audioFormat->samples = audioBufferSize / bytesPerSample / audioFormat->channels;
	audioFormat->size = audioBufferSize;
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_OpenAudio(): app opened audio bytespersample %d freq %d channels %d bufsize %d", bytesPerSample, audioFormat->freq, (int)audioFormat->channels, audioBufferSize);

	SDL_CalculateAudioSpec(audioFormat);
	
	return(1);
}

static void ANDROIDAUD_CloseAudio(_THIS)
{
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_CloseAudio()");
	JNIEnv * jniEnv = NULL;
	(*jniVM)->AttachCurrentThread(jniVM, &jniEnv, NULL);

	(*jniEnv)->DeleteGlobalRef(jniEnv, audioBufferJNI);
	audioBufferJNI = NULL;
	audioBuffer = NULL;
	audioBufferSize = 0;
	
	(*jniEnv)->CallIntMethod( jniEnv, JavaAudioThread, JavaDeinitAudio );

	/* We cannot call DetachCurrentThread() from main thread or we'll crash */
	/* (*jniVM)->DetachCurrentThread(jniVM); */
	
}

/* This function waits until it is possible to write a full sound buffer */
static void ANDROIDAUD_WaitAudio(_THIS)
{
	/* We will block in PlayAudio(), do nothing here */
}

static JNIEnv * jniEnvPlaying = NULL;
static jmethodID JavaFillBuffer = NULL;

static void ANDROIDAUD_ThreadInit(_THIS)
{
	jclass JavaAudioThreadClass = NULL;
	jmethodID JavaInitThread = NULL;
	jmethodID JavaGetBuffer = NULL;
	jboolean isCopy = JNI_TRUE;

	(*jniVM)->AttachCurrentThread(jniVM, &jniEnvPlaying, NULL);

	JavaAudioThreadClass = (*jniEnvPlaying)->GetObjectClass(jniEnvPlaying, JavaAudioThread);
	JavaFillBuffer = (*jniEnvPlaying)->GetMethodID(jniEnvPlaying, JavaAudioThreadClass, "fillBuffer", "()I");

	/* HACK: raise our own thread priority to max to get rid of "W/AudioFlinger: write blocked for 54 msecs" errors */
	JavaInitThread = (*jniEnvPlaying)->GetMethodID(jniEnvPlaying, JavaAudioThreadClass, "initAudioThread", "()I");
	(*jniEnvPlaying)->CallIntMethod( jniEnvPlaying, JavaAudioThread, JavaInitThread );

	JavaGetBuffer = (*jniEnvPlaying)->GetMethodID(jniEnvPlaying, JavaAudioThreadClass, "getBuffer", "()[B");
	audioBufferJNI = (*jniEnvPlaying)->CallObjectMethod( jniEnvPlaying, JavaAudioThread, JavaGetBuffer );
	audioBufferJNI = (*jniEnvPlaying)->NewGlobalRef(jniEnvPlaying, audioBufferJNI);
	audioBuffer = (unsigned char *) (*jniEnvPlaying)->GetByteArrayElements(jniEnvPlaying, audioBufferJNI, &isCopy);
	if( !audioBuffer )
	{
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "ANDROIDAUD_ThreadInit() JNI::GetByteArrayElements() failed! we will crash now");
		return;
	}
	if( isCopy == JNI_TRUE )
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "ANDROIDAUD_ThreadInit(): JNI returns a copy of byte array - no audio will be played");

	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_ThreadInit()");
	SDL_memset(audioBuffer, this->spec.silence, this->spec.size);
};

static void ANDROIDAUD_ThreadDeinit(_THIS)
{
	(*jniVM)->DetachCurrentThread(jniVM);
};

static void ANDROIDAUD_PlayAudio(_THIS)
{
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_PlayAudio()");
	jboolean isCopy = JNI_TRUE;

	(*jniEnvPlaying)->ReleaseByteArrayElements(jniEnvPlaying, audioBufferJNI, (jbyte *)audioBuffer, 0);
	audioBuffer = NULL;

	(*jniEnvPlaying)->CallIntMethod( jniEnvPlaying, JavaAudioThread, JavaFillBuffer );

	audioBuffer = (unsigned char *) (*jniEnvPlaying)->GetByteArrayElements(jniEnvPlaying, audioBufferJNI, &isCopy);
	if( !audioBuffer )
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "ANDROIDAUD_PlayAudio() JNI::GetByteArrayElements() failed! we will crash now");

	if( isCopy == JNI_TRUE )
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_PlayAudio() JNI returns a copy of byte array - that's slow");
}

int SDL_ANDROID_PauseAudioPlayback(void)
{
	JNIEnv * jniEnv = NULL;
	(*jniVM)->AttachCurrentThread(jniVM, &jniEnv, NULL);
	return (*jniEnv)->CallIntMethod( jniEnv, JavaAudioThread, JavaPauseAudioPlayback );
};
int SDL_ANDROID_ResumeAudioPlayback(void)
{
	JNIEnv * jniEnv = NULL;
	(*jniVM)->AttachCurrentThread(jniVM, &jniEnv, NULL);
	return (*jniEnv)->CallIntMethod( jniEnv, JavaAudioThread, JavaResumeAudioPlayback );
};


JNIEXPORT jint JNICALL Java_com_broov_player_AudioThread_nativeAudioInitJavaCallbacks (JNIEnv * jniEnv, jobject thiz)
{
	jclass JavaAudioThreadClass = NULL;
	JavaAudioThread = (*jniEnv)->NewGlobalRef(jniEnv, thiz);
	JavaAudioThreadClass = (*jniEnv)->GetObjectClass(jniEnv, JavaAudioThread);
	JavaInitAudio = (*jniEnv)->GetMethodID(jniEnv, JavaAudioThreadClass, "initAudio", "(IIII)I");
	JavaDeinitAudio = (*jniEnv)->GetMethodID(jniEnv, JavaAudioThreadClass, "deinitAudio", "()I");
	JavaPauseAudioPlayback = (*jniEnv)->GetMethodID(jniEnv, JavaAudioThreadClass, "pauseAudioPlayback", "()I");
	JavaResumeAudioPlayback = (*jniEnv)->GetMethodID(jniEnv, JavaAudioThreadClass, "resumeAudioPlayback", "()I");
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
	jniVM = vm;
        gBroovJniVM = vm;
	return JNI_VERSION_1_2;
};

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved)
{
	/* TODO: free JavaAudioThread */
	jniVM = vm;
};

#endif
