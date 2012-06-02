package com.broov.player;

import javax.microedition.khronos.opengles.GL10;

import javax.microedition.khronos.egl.EGLConfig;
import android.app.Activity;

class DemoRenderer extends GLSurfaceView_SDL.Renderer {

	public Boolean usergeneratedexitApp 	 = false;
	public Boolean playnextfileFromDirectory = true;
	String         nextFile 				 = Globals.fileName;
	public boolean fileInfoUpdated			 = false;
	private int    loopselected              = 0;
	private int    skipFrames                = 0;
	private int    rgb565                    = 0;
    private int    yuvRgbAsm                 = 0;  
    private int    skipBidirFrames           = 1;
    private int    queueSizeMin              = (1024 * 1024);
    private int    queueSizeMax              = (5000 * 1024);
    private int    queueSizeTotal            = (6000 * 1024);
    private int    queueSizeAudio            = (512 * 1024);
    
    DemoRenderer(Activity _context)
	{
		System.out.println("DemoRenderer instance created:");
		context = _context;
	}

	public void onSurfaceCreated(GL10 gl, EGLConfig config) 
	{
		System.out.println("Surface Created");
	}

	public void onSurfaceChanged(GL10 gl, int w, int h) 
	{
		System.out.println("OnSurfaceChanged");
		nativeResize(w, h);
	}

	public void onDrawFrame(GL10 gl) 
	{
		System.out.println("Inside on DrawFrame");

		nativeInitJavaCallbacks();

		// Make main thread priority lower so audio thread won't get underrun
		//Thread.currentThread().setPriority((Thread.currentThread().getPriority() + Thread.MIN_PRIORITY)/2);
		//Thread.currentThread().setPriority(Thread.MAX_PRIORITY-1);
		
		//System.out.println("Calling playerInit");
		System.out.println("Show subtitle:"+FileManager.getshow_subtitle());
		System.out.println("Subtitle size:"+FileManager.getSubTitleSize());

		nativePlayerInit(Globals.dbSubtitleFont, FileManager.getshow_subtitle(), FileManager.getSubTitleSize(), Globals.dbSubtitleEncoding, rgb565);

		System.out.println("Player Filename:"+Globals.fileName);

		switch(FileManager.loopOptionForFile(Globals.fileName)){
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

		int audioFileType;
		if (FileManager.isAudioFile(Globals.fileName)) {
			audioFileType = 1; 
		} else {
			audioFileType = 0;
		}


		System.out.println("nativePlayerMain(NewPlayer.fileName:"+Globals.fileName+", loopselected:"+loopselected+", audioFileType: "+audioFileType+");");
		////101 - Next button  100 - Previous button  0 - Song played finished
		
		if (Globals.dbSkipframes) {
			skipFrames = 1;
		} else {
			skipFrames = 0;
		}

		//int retValue = nativePlayerMain(Globals.fileName, loopselected, audioFileType, skipFrames);
		int retValue = nativePlayerMain(Globals.fileName, loopselected, audioFileType, skipFrames, rgb565, yuvRgbAsm, skipBidirFrames, queueSizeMin, queueSizeMax, queueSizeTotal, queueSizeAudio);
		
		//Initializing the arraylist
		//clear the array of already played items 
		FileManager.alreadyPlayed.clear();
		
		System.out.println("Returned from NativePlayerMainValue:"+ retValue);
		while (!usergeneratedexitApp && playnextfileFromDirectory ){
			if (retValue == 100){
				nextFile =	FileManager.getPrevFileInDirectory(nextFile);
				Globals.fileName = nextFile;
				fileInfoUpdated = true;
			}else{
				nextFile =	FileManager.getNextFileInDirectory(nextFile);
				Globals.fileName = nextFile;
				fileInfoUpdated = true;
				
			}
			if (nextFile == "") {
				System.out.println("All files are played in directory:");
				break;
			}

			System.out.println("nextFile before:"+nextFile);
			if (FileManager.isAudioFile(nextFile)) {
				audioFileType = 1; 
			} else {
				audioFileType = 0;
			}
			System.out.println("nativePlayerMain(fileName:"+nextFile+", loopselected:"+loopselected+", audioFileType: "+audioFileType+");");
			if (Globals.dbSkipframes) {
				skipFrames = 1;
			} else {
				skipFrames = 0;
			}
		
			retValue = nativePlayerMain(nextFile, loopselected, audioFileType, skipFrames, rgb565, yuvRgbAsm, skipBidirFrames, queueSizeMin, queueSizeMax, queueSizeTotal, queueSizeAudio);

			System.out.println("Returned from NativePlayerMainValue:"+ retValue);
		}

		System.out.println("Exited after nativePlayerMain");
		nativePlayerExit();
		System.out.println("Exited after nativePlayerExit");
		context.finish();
	}

	public int swapBuffers() // Called from native code, returns 1 on success, 0 when GL context lost (user put app to background)
	{
		return super.SwapBuffers() ? 1 : 0;
	}

	public void exitApp() 
	{
		System.out.println("Calling nativeDone");

		usergeneratedexitApp = true;
		nativeDone();
	};

	public int exitFromNativePlayerView()
	{
		System.out.println("Inside exitFromNativePlayerView()");
		return 1;
	}

	private native void nativeInitJavaCallbacks();

	/**
	 * 
	 * @param fontFileName
	 * Any other font file provided or selected by the user to be used for subtitles
	 * 
	 * @param subtitleShow
	 * 0 - Do not show subtitle
	 * 1 - Show subtitle
	 * 
	 * @param subtitleFontSize
	 * Valid values are 9, 11, 13 as of now
	 * @return
	 */
	private native int nativePlayerInit(String fileName, int subtitleShow, int subtitleFontSize, int subtitleEncodingType, int rgb565);

	/**
	 * 
	 * @param fileName
	 * Name of the file to be played
	 * 
	 * @param loop
	 * 0 - Dont loop
	 * 1 - Loop the same file
	 * 
	 * @param audioImageIndex
	 * audio Image index from 0 - 4 (Hardcoded images)
	 * 
	 * @param audioFileType
	 * audio File Type = 0 or 1 (0 means video file, 1 means audio file
	 * 
	 * @param file name show string
	 * string descrbing the file name that is being played now
	 * 
	 * @param file type string 
	 * string describing the file type and any other
	 * 
	 * @param file size string 
	 * string describing the size of the file 
	 * @return
	 */
	private native int nativePlayerMain(String fileName, int loop,int audioFileType, int skipFrames, int rgb565, int yuvRgbAsm, int skipBidirFrames, int queueSizeMin, int queueSizeMax, int queueSizeTotal, int queueSizeAudio);

	private native int nativePlayerExit();

	private native void nativeResize(int w, int h);
	private native void nativeDone();
	
	public native int nativePlayerDuration();
	public native int nativePlayerTotalDuration();
	
	public native int nativePlayerPlay();
	public native int nativePlayerPause();
	public native int nativePlayerForward();
	public native int nativePlayerRewind();
	public native int nativePlayerPrev();
	public native int nativePlayerNext();
	public native int nativePlayerSeek(int percent); //Number in the range of 0-1000, meaning 99.1, 99.2, 0.1,0.2..
	public native int nativePlayerSetAspectRatio(int aspectRatioType); // 0-default, 1- 4:3, 2- 16:9, 3- FullScreen
	
	private Activity context = null;
}
