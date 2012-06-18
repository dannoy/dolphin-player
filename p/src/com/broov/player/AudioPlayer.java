package com.broov.player;

import java.io.File;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.util.ArrayList;
import java.util.HashMap;
//import java.util.List;

import android.app.Activity;
//import android.app.ActivityManager;
//import android.app.ActivityManager.RunningTaskInfo;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.database.Cursor;
import android.graphics.drawable.Drawable;

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

public class AudioPlayer extends Activity  {
	private static boolean paused;
	private ArrayList<HashMap<String, String>> mp3List = new ArrayList<HashMap<String, String>>();
	String openfileFromBrowser = "";
	Intent i = getIntent();
	Context context;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		Globals.setNativeVideoPlayer(false);
		//paused = false;
		System.out.println("Inside New Player onCreate");

		// fullscreen mode
		requestWindowFeature(Window.FEATURE_NO_TITLE);		

		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);

		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

		setContentView(R.layout.audio_player);

		//Utils.hideSystemUi(getWindow().getDecorView());
		
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
		if (FileManager.isAudioFile(openfileFromBrowser)) {
			Globals.setFileName(openfileFromBrowser);	
			System.out.println("openfileFromBrowser:"+openfileFromBrowser);			
		}else{
			Bundle extras = i.getExtras();
			if (extras!= null) {
				String tmpFileName = extras.getString("audiofilename");
				System.out.println("Extras. get string audioFilename:" + Globals.fileName);
				if (FileManager.isAudioFile(tmpFileName)) {
					Globals.setFileName(tmpFileName);
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
		controlPanel.getBackground().setAlpha(55);
		mAudioPanel = findViewById(R.id.audioPanel);
		mAudioPanel.getBackground().setAlpha(55);
		albumcover = (ImageView) findViewById(R.id.albumcover);
		songtitle = (TextView) findViewById(R.id.songtitle);

		albumname = (TextView) findViewById(R.id.albumname);
		artistname = (TextView) findViewById(R.id.artistname);
		samplerate = (TextView) findViewById(R.id.samplerate);
		bitrate = (TextView) findViewById(R.id.bitrate);

		imgPlay = findViewById(R.id.img_play);
		imgPrev = findViewById(R.id.img_prev);
		imgNext = findViewById(R.id.img_next);
		imgForward = findViewById(R.id.img_forward);
		imgBackward = findViewById(R.id.img_backward);

		trScrolledTime = findViewById(R.id.trscrolledtime);
		scrolledtime = (TextView) findViewById(R.id.scrolledtime);

		trScrolledTime.setVisibility(View.GONE);

		imgPlay.setOnTouchListener(new OnTouchListener() {			
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				ImageView img = (ImageView) v;				

				if (event.getAction() == MotionEvent.ACTION_DOWN ) {	
					System.out.println("Down paused:" + paused);
					if(paused) {
						img.setImageResource(R.drawable.play);
					}
					else {
						img.setImageResource(R.drawable.pause);
					}
				}
				else if (event.getAction() == MotionEvent.ACTION_UP ) {
					System.out.println("Up paused:" + paused);		  
					System.out.println("Total:" + demoRenderer.nativePlayerTotalDuration() + "---Current:" + demoRenderer.nativePlayerDuration());
					if(paused) {
						demoRenderer.nativePlayerPause();
						seekBarUpdater = new Updater();
						mHandler.postDelayed(seekBarUpdater, 500);
						img.setImageResource(R.drawable.pause_shadow);
					}
					else {
						demoRenderer.nativePlayerPlay();
						//seekBarUpdater.stopIt();						
						img.setImageResource(R.drawable.play_shadow);
					}		        	
					paused = !paused;
				}				
				return true;
			}
		});

		imgPlay.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				System.out.println("On Click paused:" + paused);								
			}
		});

		imgPrev.setOnTouchListener(new OnTouchListener() {			
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				ImageView img = (ImageView) v;
				if (event.getAction() == MotionEvent.ACTION_DOWN ) {		            		            
					img.setImageResource(R.drawable.prev);

				}
				else if (event.getAction() == MotionEvent.ACTION_UP ) {		        	
					img.setImageResource(R.drawable.prev_shadow);
					totalDuration = 0;
					System.out.println("Calling KeyCode_1");
					demoRenderer.nativePlayerPrev();
					if(paused==true)
					{
						demoRenderer.nativePlayerPlay();
					}


				}
				demoRenderer.fileInfoUpdated = false;				
				return true;
			}
		});

		imgNext.setOnTouchListener(new OnTouchListener() {			
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				ImageView img = (ImageView) v;
				if (event.getAction() == MotionEvent.ACTION_DOWN ) {		            		            
					img.setImageResource(R.drawable.next);
				}
				else if (event.getAction() == MotionEvent.ACTION_UP ) {		        	
					img.setImageResource(R.drawable.next_shadow);
					totalDuration = 0;
					System.out.println("Calling KeyCode_2");
					demoRenderer.nativePlayerNext();
					if(paused==true)
					{
						demoRenderer.nativePlayerPlay();
					}

				}							

				demoRenderer.fileInfoUpdated = false;
				return true;
			}
		});

		imgForward.setOnTouchListener(new OnTouchListener() {			
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				ImageView img = (ImageView) v;
				if (event.getAction() == MotionEvent.ACTION_DOWN ) {		            		            
					img.setImageResource(R.drawable.forward_glow);
				}
				else if (event.getAction() == MotionEvent.ACTION_UP ) {		        	
					img.setImageResource(R.drawable.forward);										
					demoRenderer.nativePlayerForward();
				}											
				return true;
			}
		});

		imgBackward.setOnTouchListener(new OnTouchListener() {			
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				ImageView img = (ImageView) v;
				if (event.getAction() == MotionEvent.ACTION_DOWN ) {		            		            
					img.setImageResource(R.drawable.backward_glow_80x60);
				}
				else if (event.getAction() == MotionEvent.ACTION_UP ) {		        	
					img.setImageResource(R.drawable.backward);										
					demoRenderer.nativePlayerRewind();
				}											
				return true;
			}
		});


		mSeekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
			@Override
			public void onStopTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub
				int progress = seekBar.getProgress();
				trScrolledTime.setVisibility(View.GONE);
				System.out.println("Seeked to" + (float) (progress / 10F ));
				System.out.println("Progress:"+progress);
				demoRenderer.nativePlayerSeek(progress);
				if(!paused) {
					restartUpdater();
				} 
			}

			@Override
			public void onStartTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub
				trScrolledTime.setVisibility(View.VISIBLE);
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
					scrolledtime.setText(timeMoved);
					currentTime.setText(timeMoved);					
				}
			}
		});

		System.out.println("Start - InitSDL()");
		initSDL();
		System.out.println("End - InitSDL()");
		System.out.println("End of onCreate() for NewPlayer Activity");
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

		totalDuration = demoRenderer.nativePlayerTotalDuration();
		totalTime.setText(Utils.formatTime(totalDuration));

		setAudioInfo();

		mHandler.postDelayed(seekBarUpdater, 100);

	}

	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
	}

	@Override
	public void onBackPressed() {
		seekBarUpdater.stopIt();
		demoRenderer.exitApp();
	}

	public void setAudioInfo() {

		if (Globals.fileName == null) return;

		try {
			//System.out.println("Setting filename to " + Globals.fileName);
			getMp3Tags();
			HashMap<String, String> mp3Info = getMp3Info();
			artistname.setText(mp3Info.get("ARTIST"));
			albumname.setText(mp3Info.get("ALBUM"));
			songtitle.setText(mp3Info.get("TITLE"));
			if (mp3Info.get("ARTIST").length() < 3) {
				songtitle.setText(Globals.getFileName());
			}

			String duration = mp3Info.get("DURATION");
			long filesize = new File(Globals.fileName).length();
			int durationInSeconds = Integer.parseInt(duration) / 1000;
			long filesizeInBits = filesize * 8;
			long bits = (filesizeInBits/durationInSeconds) / 1000;
			bitrate.setText(""+bits);
			//Set the image
			String currentDir = new File(Globals.fileName).getParent();
			ArrayList<String> images = FileManager.listofImageFiles(currentDir);
			if(images != null && images.size() > 0) {
				Uri imageUri = Uri.fromFile(new File(images.get(0)));
				albumcover.setImageDrawable(Drawable.createFromStream(
						getContentResolver().openInputStream(imageUri),
						null));
			}
		}
		catch (Exception e) {
			//e.printStackTrace();
			try {
				songtitle.setText(Globals.getFileName());
			} catch(Exception e2) {

			}
			// TODO: handle exception
		}
		//================
	}

	private HashMap<String, String> getMp3Info() {
		String currentDisplayName = new File(Globals.fileName).getName();
		for(int i=0;i < mp3List.size();i++) {
			HashMap<String, String> mp3Info = mp3List.get(i);
			if(mp3Info.get("DISPLAY_NAME").equalsIgnoreCase(currentDisplayName)) {
				return mp3Info;
			}
		}
		return null;
	}

	private void getMp3Tags() {
		Uri uri = android.provider.MediaStore.Audio.Media.EXTERNAL_CONTENT_URI; //uri to sd-card
		String[] selection = new String[] {
				android.provider.MediaStore.Audio.Media._ID,
				android.provider.MediaStore.Audio.Media.TITLE,
				android.provider.MediaStore.Audio.Media.DATA,
				android.provider.MediaStore.Audio.Media.ARTIST,
				android.provider.MediaStore.Audio.Media.ALBUM,
				android.provider.MediaStore.Audio.Media.DURATION,
				android.provider.MediaStore.Audio.Media.DISPLAY_NAME
		};
		Cursor mCursor = managedQuery(uri, selection,null, null, null);
		mCursor.moveToFirst(); 

		while(mCursor.moveToNext()) {
			HashMap<String, String> mp3Info = new HashMap<String, String>();
			String artist = mCursor.getString(mCursor.getColumnIndexOrThrow("ARTIST"));
			String title = mCursor.getString(mCursor.getColumnIndexOrThrow("TITLE"));
			String album = mCursor.getString(mCursor.getColumnIndexOrThrow("ALBUM"));
			String duration = mCursor.getString(mCursor.getColumnIndexOrThrow("DURATION"));
			String displayName = mCursor.getString(mCursor.getColumnIndexOrThrow(android.provider.MediaStore.Audio.Media.DISPLAY_NAME));
			mp3Info.put("ARTIST", artist);
			mp3Info.put("TITLE", title);
			mp3Info.put("ALBUM", album);
			mp3Info.put("DISPLAY_NAME", displayName);
			mp3Info.put("DURATION", duration);
			mp3List.add(mp3Info); //Adds the mp3 information to the list 
		}	     	   

	}

	public void restartUpdater() {
		seekBarUpdater.stopIt();
		seekBarUpdater = new Updater();
		mHandler.postDelayed(seekBarUpdater, 100);
	}

	
//	public boolean isServiceRunning() { 
//
//		ActivityManager activityManager = (ActivityManager)AudioPlayer.this.getSystemService (Context.ACTIVITY_SERVICE); 
//		List<RunningTaskInfo> services = activityManager.getRunningTasks(Integer.MAX_VALUE); 
//
//		for (int i = 0; i < services.size(); i++) { 
//			if (services.get(i).topActivity.toString().equalsIgnoreCase("ComponentInfo{com.broov.player/com.broov.player.AudioPlayer}")) {
//				return true;
//			}
//		} 
//		return false; 
//	} 
	//@Override
	//protected void onPause() {
	//	this.
	//}

	//@Override
	//protected void onPause() {
	// TODO: if application pauses it's screen is messed up
	//	if (wakeLock != null)
	//		wakeLock.release();
	//	mAudioThread.pauseAudioPlayback();
	//	super.onPause();
	//}

	//@Override
	//protected void onNewIntent(Intent incomingIntent) {
	//		//super.onNewIntent();
	//      System.out.println("AudioPlayer onNewIntent incomingIntent.toString : "+incomingIntent.toString());		
	//}


	//@Override
	//protected void onResume() {
	//	if (wakeLock != null)
	//		wakeLock.acquire();
	//
	//	mAudioThread.resumeAudioPlayback();
	//	super.onResume();
	//}

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
				System.out.println("call state idle");
				if (demoRenderer != null)
					demoRenderer.nativePlayerPlay();

				//seekBarUpdater = new Updater();
				//mHandler.postDelayed(seekBarUpdater, 500);
			} else if(state == TelephonyManager.CALL_STATE_IDLE) {
				//Not in call: Play music
				if (demoRenderer != null)
					demoRenderer.nativePlayerPause();
				System.out.println("call sate ringing");
				//seekBarUpdater.stopIt();
			} else if(state == TelephonyManager.CALL_STATE_OFFHOOK) {
				//A call is dialing, active or on hold
				System.out.println("call state offhook");
				if (demoRenderer != null)
					demoRenderer.nativePlayerPlay();
				//seekBarUpdater = new Updater();
				//mHandler.postDelayed(seekBarUpdater, 500);
			}
			super.onCallStateChanged(state, incomingNumber);
		}
	};


	/*
	@Override
	public boolean onKeyDown(int keyCode, final KeyEvent event) {
		// Overrides Back key to use in our app
		System.out.println("keyCode onKeyDown"+keyCode); 
		if (mGLView != null)
			mGLView.nativeKey(keyCode, 1);
		if (keyCode == KeyEvent.KEYCODE_BACK)
			onStop();
		return true;
	}

	@Override
	public boolean onKeyUp(int keyCode, final KeyEvent event) {
		if (mGLView != null)
			mGLView.nativeKey(keyCode, 0);
		System.out.println("keyCode onKeyUp"+keyCode);
		return true;
	}
	 */

	class HideMeListener implements OnClickListener {
		final View mTarget;

		HideMeListener(View target) {
			mTarget = target;
		}

		public void onClick(View v) {
			mTarget.setVisibility(View.INVISIBLE);
		}
	}

	View mAudioPanel;
	SeekBar mSeekBar;
	View imgPlay;
	View imgNext;
	View imgPrev;
	View imgBackward;
	View imgForward;
	View trScrolledTime;

	TextView currentTime;
	TextView totalTime;
	TextView albumname;
	TextView artistname;
	TextView bitrate;
	TextView samplerate;
	TextView songtitle;
	TextView scrolledtime;
	ImageView albumcover;

	long totalDuration;
	DemoRenderer demoRenderer;
	TableLayout controlPanel;

	private AudioThread 		  mAudioThread = null;
	private PowerManager.WakeLock wakeLock     = null;
	private Handler mHandler = new Handler();

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
				long playedDuration = demoRenderer.nativePlayerDuration();
				//System.out.println("PlayedDuration:"+playedDuration);
				currentTime.setText(Utils.formatTime(playedDuration));
				totalDuration = demoRenderer.nativePlayerTotalDuration();
				if(totalDuration != 0) {
					int progress = (int)((1000 * playedDuration) / totalDuration);
					mSeekBar.setProgress(progress);							
					totalTime.setText(Utils.formatTime(totalDuration));
				}						
				if(demoRenderer.fileInfoUpdated) {
					setAudioInfo();
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
	private Updater seekBarUpdater = new Updater();			

}
