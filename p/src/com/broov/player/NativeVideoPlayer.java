package com.broov.player;

import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;

import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;

import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TableLayout;
import android.widget.TextView;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.os.PowerManager;

public class NativeVideoPlayer extends Activity  {

	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
	}

	@Override
	public void onBackPressed() {
		seekBarUpdater.stopIt();
		demoRenderer.exitApp();
	}

	@Override
	protected void onStop() 
	{
		if (wakeLock != null) {
			wakeLock.release();
			wakeLock = null;
		}

		super.onStop();
	}

	PhoneStateListener phoneStateListener = new PhoneStateListener() {
		@Override
		public void onCallStateChanged(int state, String incomingNumber) {
			if (state == TelephonyManager.CALL_STATE_RINGING) {
				//Incoming call: Pause music
				//System.out.println("call state idle");
				demoRenderer.nativeVideoPlayerPlay();

				//seekBarUpdater = new Updater();
				//mHandler.postDelayed(seekBarUpdater, 500);
			} else if(state == TelephonyManager.CALL_STATE_IDLE) {
				//Not in call: Play music
				demoRenderer.nativeVideoPlayerPause();
				//System.out.println("call sate ringing");
				//seekBarUpdater.stopIt();
			} else if(state == TelephonyManager.CALL_STATE_OFFHOOK) {
				//A call is dialing, active or on hold
				//System.out.println("call state offhook");
				demoRenderer.nativeVideoPlayerPlay();
				//seekBarUpdater = new Updater();
				//mHandler.postDelayed(seekBarUpdater, 500);
			}
			super.onCallStateChanged(state, incomingNumber);
		}
	};


	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

        Globals.setNativeVideoPlayer(true);
		System.out.println("Inside VideoPlayer onCreate");
		paused = false;

		// fullscreen mode
		requestWindowFeature(Window.FEATURE_NO_TITLE);		

		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);

		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

		setContentView(R.layout.video_player);

		//Utils.hideSystemUi(getWindow().getDecorView());
		//Utils.hideSystemUi(this.findViewById(R.id.glsurfaceview).getRootView());
		
		i = getIntent();

		if (i!= null) {
			Uri uri = i.getData();
			if (uri!= null) {
				openfileFromBrowser = uri.getEncodedPath();	

				//Change from 1.6
				String decodedOpenFileFromBrowser = null;
				try {
					decodedOpenFileFromBrowser = URLDecoder.decode(openfileFromBrowser,"UTF-8");
				} catch (UnsupportedEncodingException e) {
					// TODO Auto-generated catch block
					//e.printStackTrace();
				}
				if (decodedOpenFileFromBrowser != null)
				{
					openfileFromBrowser = decodedOpenFileFromBrowser; 
				}
			}	
		}
		System.out.println("openfileFromBrowser:"+openfileFromBrowser);

		if(FileManager.isVideoFile(openfileFromBrowser)){
			Globals.setFileName(openfileFromBrowser);	
			System.out.println("================openfileFromBrowser:"+openfileFromBrowser+"=============");			

		}	
		else {
			Bundle extras = i.getExtras();
			if (extras != null) {
				String tmpFileName = extras.getString("videofilename");

				if (FileManager.isVideoFile(tmpFileName)) {
					Globals.setFileName(tmpFileName);
					System.out.println("================extras.getString videofilename:"+tmpFileName+"============");
				}
			}
		}

		System.out.println("=======================Playing filename:" + Globals.fileName);

		TelephonyManager mgr = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
		System.out.println("TelephoneManager : "+mgr);
		if(mgr != null) {
			System.out.println("telephonemanager start");
			mgr.listen(phoneStateListener, PhoneStateListener.LISTEN_CALL_STATE);
		}

		// Find the views whose visibility will change
		mSeekBar = (SeekBar) findViewById(R.id.progressbar);

		currentTime = (TextView) findViewById(R.id.currenttime);     
		totalTime = (TextView) findViewById(R.id.totaltime);        
		controlPanel = (TableLayout) findViewById(R.id.controlPanel);
		controlPanel.getBackground().setAlpha(85);

		imgPlay = findViewById(R.id.img_vp_play);
		imgForward = findViewById(R.id.img_vp_forward);
		imgBackward = findViewById(R.id.img_vp_backward);
		imgAspectRatio = findViewById(R.id.fs_shadow);

		mHideContainer = findViewById(R.id.hidecontainer);
		mHideContainer.setOnClickListener(mVisibleListener);
		imgAspectRatio.setOnTouchListener(imgAspectRatioTouchListener);
		imgPlay.setOnTouchListener(imgPlayTouchListener);
		imgForward.setOnTouchListener(imgForwardTouchListener);
		imgBackward.setOnTouchListener(imgBackwardTouchListener);
		mSeekBar.setOnSeekBarChangeListener(mSeekBarChangeListener);

		initSDL();
	}

	public void initSDL()
	{
		//Wake lock code
		try {
			PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
			wakeLock = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK, Globals.ApplicationName);
			wakeLock.acquire();
		} catch (Exception e) {
			System.out.println("Inside wake lock exception"+e.toString());
		}
		System.out.println("Acquired wakeup lock");

		//Native libraries loading code
		Globals.LoadNativeLibraries();
		System.out.println("native libraries loaded");

		//Audio thread initializer
		mAudioThread = new AudioThread(this);
		System.out.println("Audio thread initialized");

		GLSurfaceView_SDL surfaceView = (GLSurfaceView_SDL) findViewById(R.id.glsurfaceview);
		System.out.println("got the surface view:");

		surfaceView.setOnClickListener(mGoneListener);

		DemoRenderer demoRenderer = new DemoRenderer(this);
		this.demoRenderer = demoRenderer;
		surfaceView.setRenderer(demoRenderer); 
		System.out.println("Set the surface view renderer");

		SurfaceHolder holder = surfaceView.getHolder();
		holder.addCallback(surfaceView);
		System.out.println("Added the holder callback");
		holder.setType(SurfaceHolder.SURFACE_TYPE_GPU);
		System.out.println("Hold type set");

		surfaceView.setFocusable(true);
		surfaceView.requestFocus();

		totalDuration = demoRenderer.nativeVideoPlayerTotalDuration();
		totalTime.setText(Utils.formatTime(totalDuration));

		mHandler.postDelayed(seekBarUpdater, 100);
	}

	public void restartUpdater() {
		seekBarUpdater.stopIt();
		seekBarUpdater = new Updater();
		mHandler.postDelayed(seekBarUpdater, 100);
	}

	private class Updater implements Runnable {
		private boolean stop;

		public void stopIt() {
			System.out.println("Stopped updater");
			stop = true;
		}

		@Override
		public void run() {
			//Thread.currentThread().setPriority(Thread.MIN_PRIORITY);
			if(currentTime != null && demoRenderer != null) {
				long playedDuration = demoRenderer.nativeVideoPlayerDuration();				
				currentTime.setText(Utils.formatTime(playedDuration));
				totalDuration = demoRenderer.nativeVideoPlayerTotalDuration();
				if(totalDuration != 0) {
					int progress = (int)((1000 * playedDuration) / totalDuration);
					mSeekBar.setProgress(progress);							
					totalTime.setText(Utils.formatTime(totalDuration));
				}						
				if(demoRenderer.fileInfoUpdated) {
					demoRenderer.fileInfoUpdated = false;
				}
			}

			if(!stop) {
				if (Globals.fileName != null) {
					//Restart the updater if file is still playing
					mHandler.postDelayed(seekBarUpdater, 500);
				}
			}
		}
	}



	OnClickListener mGoneListener = new OnClickListener() 
	{
		public void onClick(View v) 
		{
			System.out.println("Inside mGone Click");
			if ((mHideContainer.getVisibility() == View.INVISIBLE) ||
					(mHideContainer.getVisibility() == View.GONE))
			{
				mHideContainer.setVisibility(View.VISIBLE);
				restartUpdater();
			}	else 
			{
				mHideContainer.setVisibility(View.INVISIBLE);
				seekBarUpdater.stopIt();
			}
		}
	};


	OnClickListener mVisibleListener = new OnClickListener() 
	{
		public void onClick(View v) 
		{
			if ((mHideContainer.getVisibility() == View.GONE) ||
					(mHideContainer.getVisibility() == View.INVISIBLE)) 
			{
				mHideContainer.setVisibility(View.VISIBLE);
				restartUpdater();
			} else 
			{
				mHideContainer.setVisibility(View.INVISIBLE);
				seekBarUpdater.stopIt();
			}
		}
	};

	OnTouchListener imgAspectRatioTouchListener = new OnTouchListener() {			
		@Override
		public boolean onTouch(View v, MotionEvent event) {
			ImageView img = (ImageView) v;
			if (event.getAction() == MotionEvent.ACTION_DOWN) {
				//Do nothing for now
			}
			else if (event.getAction() == MotionEvent.ACTION_UP) {
				if (current_aspect_ratio_type == 3) {
					img.setImageResource(R.drawable.fs_shadow_4_3);
					demoRenderer.nativeVideoPlayerSetAspectRatio(0);
					current_aspect_ratio_type = 1;
				} else if (current_aspect_ratio_type == 1) {
					img.setImageResource(R.drawable.fs_shadow);
					demoRenderer.nativeVideoPlayerSetAspectRatio(3);
					current_aspect_ratio_type = 2;
				} else if (current_aspect_ratio_type == 2) {
					img.setImageResource(R.drawable.fs_shadow_16_9);
					demoRenderer.nativeVideoPlayerSetAspectRatio(2);
					current_aspect_ratio_type = 3;

				}
			}						
			//resetAutoHider();
			return true;
		}
	};

	OnTouchListener imgPlayTouchListener = new OnTouchListener() {			
		@Override
		public boolean onTouch(View v, MotionEvent event) {
			ImageView img = (ImageView) v;				

			if (event.getAction() == MotionEvent.ACTION_DOWN ) {	
				System.out.println("Down paused:" + paused);
				if(paused) {
					img.setImageResource(R.drawable.vp_play);
				}
				else {
					img.setImageResource(R.drawable.vp_pause);
				}
			}
			else if (event.getAction() == MotionEvent.ACTION_UP ) {
				System.out.println("Up paused:" + paused);		  
				System.out.println("Total:" + demoRenderer.nativeVideoPlayerTotalDuration() + "---Current:" + demoRenderer.nativeVideoPlayerDuration());
				if(paused) {
					demoRenderer.nativeVideoPlayerPause();
					seekBarUpdater = new Updater();
					mHandler.postDelayed(seekBarUpdater, 500);
					img.setImageResource(R.drawable.vp_pause_shadow);
				}
				else {
					demoRenderer.nativeVideoPlayerPlay();
					seekBarUpdater.stopIt();						
					img.setImageResource(R.drawable.vp_play_shadow);
				}		        	
				paused = !paused;
			}				
			//resetAutoHider();
			return true;
		}
	};

	OnTouchListener imgForwardTouchListener = new OnTouchListener() {			
		@Override
		public boolean onTouch(View v, MotionEvent event) {
			ImageView img = (ImageView) v;
			if (event.getAction() == MotionEvent.ACTION_DOWN ) {		            		            
				img.setImageResource(R.drawable.vp_forward_glow);
			}
			else if (event.getAction() == MotionEvent.ACTION_UP ) {		        	
				img.setImageResource(R.drawable.vp_forward);										
				demoRenderer.nativeVideoPlayerForward();
			}							
			//resetAutoHider();
			return true;
		}
	};

	OnTouchListener imgBackwardTouchListener = new OnTouchListener() {			
		@Override
		public boolean onTouch(View v, MotionEvent event) {
			ImageView img = (ImageView) v;
			if (event.getAction() == MotionEvent.ACTION_DOWN ) {		            		            
				img.setImageResource(R.drawable.vp_backward_glow_80x60);
			}
			else if (event.getAction() == MotionEvent.ACTION_UP ) {		        	
				img.setImageResource(R.drawable.vp_backward);										
				demoRenderer.nativeVideoPlayerRewind();
			}						
			//resetAutoHider();
			return true;
		}
	};

	OnSeekBarChangeListener mSeekBarChangeListener = new OnSeekBarChangeListener() {
		@Override
		public void onStopTrackingTouch(SeekBar seekBar) {
			int progress = seekBar.getProgress();
			demoRenderer.nativeVideoPlayerSeek(progress);
			if (!paused) {
				restartUpdater();
			} 
		}

		@Override
		public void onStartTrackingTouch(SeekBar seekBar) {
			//	// TODO Auto-generated method stub
			//	trScrolledTime.setVisibility(View.VISIBLE);
		}

		@Override
		public void onProgressChanged(SeekBar seekBar, int progress,
				boolean fromUser) {
			// TODO Auto-generated method stub
			//System.out.println("Progress Changed = " + progress);
			//System.out.println("Progres change percent=" + (float) (progress / 10F ));				
			if(fromUser) {
				long currentSecsMoved = (long)((totalDuration * ((float) (progress / 10F ))) / 100);
				String timeMoved = Utils.formatTime(currentSecsMoved);
				//scrolledtime.setText(timeMoved);
				currentTime.setText(timeMoved);
				//resetAutoHider();
			}
		}
	};

	View mHideContainer;

	View imgPlay; 
	View imgBackward; View imgForward;
	View imgAspectRatio;
	SeekBar mSeekBar;
	TextView currentTime; TextView totalTime; 

	long totalDuration;

	DemoRenderer demoRenderer;
	TableLayout controlPanel;

	private AudioThread 		  mAudioThread = null;
	private PowerManager.WakeLock wakeLock     = null;
	private Handler mHandler = new Handler();

	private Updater seekBarUpdater = new Updater();
	private static int current_aspect_ratio_type=1; //Default Aspect Ratio of the file
	private static boolean paused;
	String openfileFromBrowser = "";
	Intent i = getIntent();

}
