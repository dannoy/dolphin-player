package com.broov.player;

import android.app.Activity;

import android.content.Context;
import android.media.AudioManager;
import android.view.KeyEvent;
import android.view.MotionEvent;

class DemoGLSurfaceView
{
	public DemoGLSurfaceView(Activity context, DemoRenderer renderer) {		
		mParent = context;
		mRenderer = renderer;
	}
	
	public boolean onTouchEvent(final MotionEvent event) 
	{
		 //System.out.println("DemoGLSurfaceView onTouchEvent");
		// TODO: add multitouch support (added in Android 2.0 SDK)
		int action = -1;
		if (event.getAction() == MotionEvent.ACTION_DOWN)
			action = 0;
		if (event.getAction() == MotionEvent.ACTION_UP)
			action = 1;
		if (event.getAction() == MotionEvent.ACTION_MOVE)
			action = 2;
		if (action >= 0) {
			System.out.println("onTouchEvent X:"+(int)event.getX()+ " Y:"+(int)event.getY());
			nativeMouse((int)event.getX(), (int)event.getY(), action);
		}
		
		return true;
	}

	 public void exitApp() {
		 System.out.println("DemoGLSurfaceView exitApp");
		 mRenderer.exitApp();
	 };
	
	public boolean onKeyDown(int keyCode, final KeyEvent event) {
		 System.out.println("DemoGLSurfaceView onKeyDown");
		 
		 if (keyCode == KeyEvent.KEYCODE_BACK) {
			    System.out.println("DemoGLSurfaceView onKeyDown ExitApp");
			 	exitApp();
		 }
		 else if (keyCode == KeyEvent.KEYCODE_VOLUME_UP) {
			 VolumeUp();
			 return true;
			 //return false;
		 }
		 else if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN) {
			 VolumeDown();
			 return true;
			 //return false;
		 }
		 else if (keyCode == KeyEvent.KEYCODE_MUTE) {
			 VolumeMute();
		 }
		 nativeKey(keyCode, 1);
		 
		 return true;
	 }

	public void callNativeKey(int keyCode, int down){
		try{
			nativeKey(keyCode,down);
		} catch(Exception e){
			e.printStackTrace();
		}
	}
	
	public void VolumeUp(){
		 AudioManager a = (AudioManager) mParent.getSystemService(Context.AUDIO_SERVICE);
		 a.adjustVolume(AudioManager.ADJUST_LOWER, 0);
	}
	public void VolumeDown(){
		 AudioManager a = (AudioManager) mParent.getSystemService(Context.AUDIO_SERVICE);
		 a.adjustVolume(AudioManager.ADJUST_RAISE, 0);
	}
	
	public void VolumeMute(){
		 //AudioManager a = (AudioManager) mParent.getSystemService(Context.AUDIO_SERVICE);
		 //a.adjustVolume(AudioManager..ADJUST_RAISE, 0);
	}
	
	DemoRenderer mRenderer;
	Activity mParent;

	public native void nativeMouse(int x, int y, int action);
	public native void nativeKey(int keyCode, int down);
	
}
