package com.broov.playerN;

import android.app.Activity;
import android.media.AudioTrack;
import android.media.AudioManager;
import android.media.AudioFormat;

import java.lang.Thread;


class AudioThread {

	private AudioTrack mAudio;
	private byte[] mAudioBuffer;

	public AudioThread(Activity parent)
	{
		System.out.println("Inside AudioThread");
		mAudio       = null;
		mAudioBuffer = null;
		nativeAudioInitJavaCallbacks();
	}
	
	public int fillBuffer()
	{
		if ((mAudio !=null) && (mAudioBuffer != null)) {
			mAudio.write(mAudioBuffer, 0, mAudioBuffer.length);
		}
		return 1;
	}
	
	public int fillBufferOfSize(int len)
	{
		if ((mAudio !=null) && (mAudioBuffer != null)) {
			if (len >= mAudioBuffer.length) len = mAudioBuffer.length;
			mAudio.write(mAudioBuffer, 0, len);
		}
		return 1;
	}
	
	public int initAudio(int rate, int channels, int encoding, int bufSize)
	{
		    System.out.println("Inside initAudio");
			if (mAudio == null)
			{
					//channels = (channels == 1) ? AudioFormat.CHANNEL_CONFIGURATION_MONO : 
					//							 AudioFormat.CHANNEL_CONFIGURATION_STEREO;
				    channels = (channels == 1) ? AudioFormat.CHANNEL_OUT_MONO : 
				                                 AudioFormat.CHANNEL_OUT_STEREO;

					encoding = (encoding == 1) ? AudioFormat.ENCODING_PCM_16BIT :
													AudioFormat.ENCODING_PCM_8BIT;

					//System.out.println("Channels:"+channels+" Encoding:"+encoding);
					
					if (AudioTrack.getMinBufferSize(rate, channels, encoding ) > bufSize)
						bufSize = AudioTrack.getMinBufferSize(rate, channels, encoding);

					System.out.println("AudioTrack BufferSize:"+bufSize);
					//bufSize = (int)((float)bufSize * (((float)Globals.AudioBufferConfig * 2.5f) + 1.0f));
					bufSize = (int)((float)bufSize * (((float)Globals.AudioBufferConfig) + 1.0f));

					mAudioBuffer = new byte[bufSize];

					mAudio = new AudioTrack(AudioManager.STREAM_MUSIC, 
											rate,
											channels,
											encoding,
											bufSize,
											AudioTrack.MODE_STREAM);
					mAudio.play();
			}
			return mAudioBuffer.length;
	}
	
	public byte[] getBuffer()
	{
		return mAudioBuffer;
	}
	
	public int deinitAudio()
	{
		System.out.println("Inside deinitAudio");
		if (mAudio != null)
		{
			mAudio.stop();
			mAudio.release();
			mAudio = null;
		}
		mAudioBuffer = null;
		return 1;
	}
	
	public int initAudioThread()
	{
	    //System.out.println("Inside initAudioThread set high priority");	
		// Make audio thread priority higher so audio thread won't get underrun
		Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
		return 1;
	}
	
	public int pauseAudioPlayback()
	{
		System.out.println("Inside pauseAudioPlayback");
		if (mAudio != null)
		{
			mAudio.pause();
			return 1;
		}
		return 0;
	}

	public int resumeAudioPlayback()
	{
		System.out.println("Inside resumeAudioPlayback");
		if (mAudio != null)
		{
			mAudio.play();
			return 1;
		}
		return 0;
	}
	
	private native int nativeAudioInitJavaCallbacks();
}
