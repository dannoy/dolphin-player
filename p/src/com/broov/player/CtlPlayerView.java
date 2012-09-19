package com.broov.player;

import android.content.Context;
import android.os.Handler;
import android.os.PowerManager;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener; 
import android.widget.TableLayout;
import android.widget.TextView;

public class CtlPlayerView extends FrameLayout{
    public String TAG = "CtlPlayerView";
	private Context 		mContext;
    private WindowManager.LayoutParams wmParams = null;
    private WindowManager wm=(WindowManager)getContext().getApplicationContext().getSystemService("window");
    private float mTouchStartX;
    private float mTouchStartY;
    private float x;
    private float y;
	View mHideContainer;
	View mControlPanelContainer;

	View imgPlay; 
	View imgBackward; View imgForward;
	View imgAspectRatio;
	SeekBar mSeekBar;
	TextView currentTime; TextView totalTime; 
	DemoRenderer demoRenderer;
	TableLayout controlPanel;
	private AudioThread 		  mAudioThread = null;
	private PowerManager.WakeLock wakeLock     = null;
	private Handler mHandler = new Handler();
	private static boolean paused;
	long totalDuration;
	private static int current_aspect_ratio_type=1; //Default Aspect Ratio of the file

	private Updater seekBarUpdater = new Updater();
    private int click_status = 0;

    public CtlPlayerView(Context context) {
        super(context);
        mContext = context;
        init();
    }
    public CtlPlayerView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
        init();
    }

    private void init() {
        LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View layout = inflater.inflate(R.layout.video_player, this);
        //View layout = inflater.inflate(R.layout.video_player, null);
        Globals.setFileName("/sdcard/yongqi.3gp");

		mSeekBar = (SeekBar) layout.findViewById(R.id.progressbar);

		currentTime = (TextView) layout.findViewById(R.id.currenttime);     
		totalTime = (TextView) layout.findViewById(R.id.totaltime);        
		controlPanel = (TableLayout) layout.findViewById(R.id.controlPanel);
		controlPanel.getBackground().setAlpha(85);

		imgPlay = layout.findViewById(R.id.img_vp_play);
		imgForward = layout.findViewById(R.id.img_vp_forward);
		imgBackward = layout.findViewById(R.id.img_vp_backward);
		imgAspectRatio = layout.findViewById(R.id.fs_shadow);

		mHideContainer = layout.findViewById(R.id.hidecontainer);
		mHideContainer.setOnClickListener(mVisibleListener);
		
		mControlPanelContainer = layout.findViewById(R.id.controlPanel);
		mControlPanelContainer.setOnClickListener(mControlPanelListener);

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
            PowerManager pm = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
            //wakeLock = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK, Globals.ApplicationName);
            wakeLock = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK, Globals.ApplicationName);

            wakeLock.acquire();
        } catch (Exception e) {
            System.out.println("Inside wake lock exception"+e.toString());
        }
        System.out.println("Acquired wakeup lock");

        //Native libraries loading code
        Globals.LoadNativeLibraries();
        System.out.println("native libraries loaded");

        //Audio thread initializer
        mAudioThread = new AudioThread();
        System.out.println("Audio thread initialized");

        GLSurfaceView_SDL surfaceView = (GLSurfaceView_SDL) findViewById(R.id.glsurfaceview);
        System.out.println("got the surface view:");

        surfaceView.setOnClickListener(mGoneListener);

        DemoRenderer demoRenderer = new DemoRenderer();
        this.demoRenderer = demoRenderer;
        surfaceView.setRenderer(demoRenderer); 
        surfaceView.setLayoutView(this);
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

        mHandler.postDelayed(seekBarUpdater, 100);

        //Hide ICS System bar/Navigation bar
        //		Window win = getWindow();
        //	     WindowManager.LayoutParams winParams = win.getAttributes();
        //	     winParams.flags |= WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN;
        //	     win.setAttributes(winParams);
        //		
        //Utils.hideSystemUi(surfaceView);
        //Utils.hideSystemUi(this.findViewById(R.id.glsurfaceview).getRootView());

        //Utils.hideSystemUi(getWindow().getDecorView());

    }

	private void updateViewPosition(){
	   
        wmParams = ((MyApp)getContext().getApplicationContext()).getMywmParams2();
		wmParams.x=(int)( x-mTouchStartX);
		wmParams.y=(int) (y-mTouchStartY);
		wm.updateViewLayout(this, wmParams);  //刷新显示
	}
    public boolean onInterceptTouchEvent(MotionEvent event) {
        //Log.w(TAG, "#####onInterceptTouchEvent");
        //return true;
	    return onTouchEvent(event);

        //return true;
    }
    
	public boolean onTouchEvent(MotionEvent event) {
        float oldx = x;
        float oldy = y;
        x = event.getRawX();
        y = event.getRawY()-25;   //25是系统状态栏的高度
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:    //捕获手指触摸按下动作
                //获取相对View的坐标，即以此View左上角为原点
                mTouchStartX =  event.getX();
                mTouchStartY =  event.getY();
                //Log.w(TAG, "##### X "+mTouchStartX + " Y " + mTouchStartY);
                if(mTouchStartY > 100) {
                    //Log.w(TAG, "#####onTouchEvent false");
                    //mTouchStartX=mTouchStartY=0;
                    //x = oldx;
                    //y = oldy;
                    click_status = 1;
                    return false;
                }
        //click_status = 1;
                click_status = 2;
                break;

            case MotionEvent.ACTION_MOVE:   //捕获手指触摸移动动作
                //Log.w(TAG, "#####onTouchEvent move");
                updateViewPosition();
                if(2 == click_status)
                    click_status = 3;
                break;

            case MotionEvent.ACTION_UP:    //捕获手指触摸离开动作
                //Log.w(TAG, "#####onTouchEvent up");
                mTouchStartX=mTouchStartY=0;
                if(3 == click_status) {
                    //updateViewPosition();
                    //Log.w(TAG, "#####onTouchEvent up update");
                }
                else {
                    //Log.w(TAG, "#####onTouchEvent false");
                    return false;
                }
                //if(0 == click_status 
                    //||click_status == 1)
                    //click_status = 3;
                //else{
                    //updateViewPosition();
                //}
                break;
        }
        //return true;
        //if(click_status == 3 
                //|| click_status == 1) {
            //click_status = 0;
        //Log.w(TAG, "#####onTouchEvent false");
            //return false;
        //}

        //Log.w(TAG, "#####onTouchEvent true");
        return true;
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
				long playedDuration = demoRenderer.nativePlayerDuration();				
				currentTime.setText(Utils.formatTime(playedDuration));
				totalDuration = demoRenderer.nativePlayerTotalDuration();
				if(totalDuration != 0) {
					int progress = (int)((1000 * playedDuration) / totalDuration);
					mSeekBar.setProgress(progress);							
					totalTime.setText(Utils.formatTime(totalDuration));
				}						
				if (demoRenderer.fileInfoUpdated) {
					//if (Globals.fileName != null) {
					//	videoInfo.setText(FileManager.getFileName(Globals.fileName));
					//}
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

	public void restartUpdater() {
		seekBarUpdater.stopIt();
		seekBarUpdater = new Updater();
		mHandler.postDelayed(seekBarUpdater, 100);
	}

	OnClickListener mControlPanelListener = new OnClickListener() 
	{
		public void onClick(View v) 
		{
			//Do not hide the control panel par, when clicked
			//System.out.println("CONTROL PANEL  LISTENER ONCLICK ");
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
					demoRenderer.nativePlayerSetAspectRatio(0);
					current_aspect_ratio_type = 1;
				} else if (current_aspect_ratio_type == 1) {
					img.setImageResource(R.drawable.fs_shadow);
					demoRenderer.nativePlayerSetAspectRatio(3);
					current_aspect_ratio_type = 2;
				} else if (current_aspect_ratio_type == 2) {
					img.setImageResource(R.drawable.fs_shadow_16_9);
					demoRenderer.nativePlayerSetAspectRatio(2);
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
				System.out.println("Total:" + demoRenderer.nativePlayerTotalDuration() + "---Current:" + demoRenderer.nativePlayerDuration());
				if(paused) {
					demoRenderer.nativePlayerPause();
					seekBarUpdater = new Updater();
					mHandler.postDelayed(seekBarUpdater, 500);
					img.setImageResource(R.drawable.vp_pause_shadow);
				}
				else {
					demoRenderer.nativePlayerPlay();
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
				demoRenderer.nativePlayerForward();
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
				demoRenderer.nativePlayerRewind();
			}						
			//resetAutoHider();
			return true;
		}
	};

	OnSeekBarChangeListener mSeekBarChangeListener = new OnSeekBarChangeListener() {
		@Override
		public void onStopTrackingTouch(SeekBar seekBar) {
			//System.out.println("Should be visible:" + trScrolledTime.getVisibility());
			//trScrolledTime.setVisibility(View.GONE);
			//trScrolledTime.setVisibility(View.INVISIBLE);
			//System.out.println("Should be gone:" + trScrolledTime.getVisibility());

			int progress = seekBar.getProgress();
			//System.out.println("Seeked to new progress" + (float) (progress / 10F ));
			//System.out.println("Progress new:"+progress);
			demoRenderer.nativePlayerSeek(progress);
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

}
