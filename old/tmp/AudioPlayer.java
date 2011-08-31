package com.broov.player;

import android.app.Activity;

public class AudioPlayer implements Runnable{
	
	Activity parent;
	private AudioThread mAudioThread;

	public Boolean usergeneratedexitApp 	 = false;
	public Boolean playnextfileFromDirectory = true;
	String         nextFile 				 = null;
	String         fileName;

	private native int nativeAudioPlayerInit();
	private native int nativeAudioPlayerMain(String fileName, int loop);
	private native int nativeAudioPlayerExit();
	private native void nativeAudioPlayerDone();
	
	public AudioPlayer(Activity arg0) {
		 parent = arg0;
		 mAudioThread = new AudioThread(parent);
	}
	
	public void setParent(Activity arg0) {
		parent = arg0;
	}
	
	public void setFileName(String fname) {
		fileName = fname;
	}
	
	public void quitAlreadyPlayingFile() {
		nativeAudioPlayerDone();
	}
	
	public void run() {
		int    loopselected              = 0;
		
		try {
			Thread.sleep(2000);
		}catch(Exception e) {
			System.out.println("Exception:"+e.toString());
		}
		
		nativeAudioPlayerInit();

		switch(FileManager.loopOptionForFile(fileName)){
		case Globals.PLAY_ONCE:
			System.out.println("PLAY_ONCE");
			playnextfileFromDirectory = false;
			loopselected = 0;
			break;
		case Globals.PLAY_ALL:
			System.out.println("PLAY_ALL");
			loopselected = 0;
			break;
		case Globals.REPEAT_ONE:
			System.out.println("REPEAT_ONE");
			loopselected=1;
			playnextfileFromDirectory =false;
			break;
		case Globals.REPEAT_ALL:
			System.out.println("REPEAT_ALL");
			loopselected=0;
			break;
		}

		System.out.println("nativeAudioPlayerMain(fileName:"+fileName+", loopselected:"+loopselected);
		////101 - Next button  100 - Previous button  0 - Song played finished

		int retValue = nativeAudioPlayerMain(fileName, loopselected);
		System.out.println("Returned from NativePlayerMainValue:"+ retValue);
		
		System.out.println("Exited after nativeAudioPlayerMain");
		nativeAudioPlayerExit();
		System.out.println("Exited after nativeAudioPlayerExit");
	}
	
		

}
