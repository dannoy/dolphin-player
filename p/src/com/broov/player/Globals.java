package com.broov.player;

import java.io.File;

import android.graphics.Color;
import android.os.Environment;

class Globals {

	public static String fileName; 
	// Set this value to true if you're planning to render 3D using OpenGL - it eats some GFX resources, so disabled for 2D
	public static boolean NeedDepthBuffer = false;
	public static int AudioBufferConfig   = 2;
	public static int numberofImages      = 4;  //Random images to be shown for audio file, being played

	//public static String ApplicationName = "Dolphin Player Universal";
	public static String ApplicationName 	 = "Dolphin Player ARM V5";
	public static String VersionName     	 = "1.8 Build 12June2012";
	public static String defaultDir      	 = "/sdcard";
	public static String defaultSubtitleFont = "/sdcard/broov.ttf";
	
	public static void setFileName(String fName) {
		fileName = fName;
	}
	
	public static String getFileName() {
		if (fileName == null) {
			return "";
		}
		return fileName;
	}

	public static String supportedVideoFileFormats[] = 
	{   "mp4","wmv","avi","mkv","dv",
		"rm","mpg","mpeg","flv","divx",
		"swf","dat","h264","h263","h261",
		"3gp","3gpp","asf","mov","m4v", "ogv",
		"vob", "vstream", "ts", "webm"
		//"ttf"
	};

	public static String supportedAudioFileFormats[] = 
	{   "mp3","wma","ogg","mp2","flac",
		"aac","ac3","amr","pcm","wav",
		"au","aiff","3g2","m4a", "astream"
	};		
	
	public static String supportedFontFileType[] = 
	{   "ttf"
	};
	
	public static String supportedImageFileFormats[] = 
	{
		"gif","bmp","png","jpg"
	};
	
	public static String supportedAudioStreamFileFormats[] = 
	{
			"astream"
	};
	
	public static String supportedVideoStreamFileFormats[] = 
	{
			"vstream"
	};
	
	public static String supportedFileFormats[] =concat(supportedAudioFileFormats, supportedVideoFileFormats, supportedFontFileType);

	public static final String PREFS_NAME = "BroovPrefsFile";	//user preference file name
	public static final String PREFS_HIDDEN = "hidden";
	public static final String PREFS_COLOR = "color";
	public static final String PREFS_SUBTITLE = "subtitle";
	public static final String PREFS_THUMBNAIL = "thumbnail";
	public static final String PREFS_AUDIOLOOP = "audioloop";
	public static final String PREFS_VIDEOLOOP = "videoloop";
	public static final String PREFS_SUBTITLESIZE = "subtitlesize";
	public static final String PREFS_SUBTITLEENCODING = "subtitleencoding";
	public static final String PREFS_SKIPFRAME = "skipframe";

	public static final String PREFS_LANGUAGE = "language";
	public static final String PREFS_LASTOPENDIR = "lastopeneddir";
	public static final String PREFS_DEFAULTHOME = "defaulthomedir";
	public static final String PREFS_SUBTITLEFONT = "subtitlefont";
	public static final String PREFS_SORT = "sort";

	public static final int PLAY_ONCE =0;
	public static final int PLAY_ALL =1;
	public static final int REPEAT_ONE =2;
	public static final int REPEAT_ALL =3;

	public static final int SWIPE_MIN_DISTANCE = 120;
	public static final int SWIPE_MAX_OFF_PATH = 250;
	public static final int SWIPE_THRESHOLD_VELOCITY = 200;

	public static final int SUBTITLE_FONT_SMALL = 9;
	public static final int SUBTITLE_FONT_MEDIUM = 11;
	public static final int SUBTITLE_FONT_LARGE = 13;

	/**
	 * This method is used to concat 2 string arrays to one
	 * @param A
	 * First string Array
	 * @param B
	 * Second string Array
	 * @return
	 * Concatenated string Array of First and Second
	 */
	public static String[] concat(String[] A, String[] B, String[] C) {
		String[] temp= new String[A.length+B.length+C.length];
		System.arraycopy(A, 0, temp, 0, A.length);
		System.arraycopy(B, 0, temp, A.length, B.length);
		System.arraycopy(C, 0, temp, A.length+B.length, C.length);
		return temp;
	}

	public static String aboutUsContent = 
		"<font color='white'>" +
		"<P ALIGN=CENTER><b>" +Globals.ApplicationName+ " " +Globals.VersionName+"</b>" +
		"</p>" +
		"<P ALIGN=CENTER> " +
		"<b>Credits</b>"+
		"</p>"+
		"<P ALIGN=JUSTIFY>"+
		"Our thanks to opensource community members of FFmpeg, SDL, freetype, SDL_image, SDL_ttf, libpng, libjpeg, Theorarm, universalchardet, iconv, andprof, Pelya for SDL porting to Android, KMP"+
		"</p>" +
		"<P ALIGN=JUSTIFY>"+
		"This is an open source media player for Android. Source can be availed at http://code.google.com/p/dolphin-player. Contributions are welcome from open source community members." +
		"</p>"+
		"<P ALIGN=JUSTIFY>"+
		"Users of Dolphin Player Premium Edition, can avail one licensed copy of BullsHit Converter 3.0 Ultimate Edition. HD Videos can be converted for optimal viewing in mobiles using this software" +
		"<br>"+
		"(Supports conversion for most of the Audio and Video files)"+
		"</p>" +
		"<P ALIGN=CENTER>"+
		"<b>Upcoming Features</b>" +
		"<br> Speech Recognition to control play, pause, forward, rewind of audio files " +
		"</p>" +
		"<P ALIGN=CENTER>"+
		"<b>Developers</b>" +
		"<br>  Aatral Arasu <br>  Nareshprasad <br> Ganesan Narayanan" +
		
		"</p>" +
		"</p>" +
		"<P ALIGN=CENTER>"+
		"<b>UI/UX Design</b>" +
		"<br>  Prabhu Beeman" +
		"<br>  Ragupathy" +
		"</p>" +
		"<P ALIGN=CENTER>"+
		"<b>Reviewers</b>" +
		"<br>  Balaji Sivasubramanian" +
		"<br><br><b>Translations</b><br>  French - Elankathir<br>  German - Elankathir<br>  Italian - Elankathir<br>  Japanese - Ramasamy<br>  Chinese - Ramasamy<br>" +
		"<br><br> Copyright &copy; Broov Information Services Private Limited <br> <a style=\"color:grey\" href=\"http://broov.in\">http://broov.in/</a>"+
		"</P>";

	public static String helpContent =
		"<font color='white'>" +
		"<P ALIGN=CENTER>  "+ Globals.ApplicationName+" "+Globals.VersionName+" is an audio and video player for Android 2.1 and above." +
		"</p>"+
		"<P ALIGN=CENTER>" +
		"<b>Play Media</b>" +
		"</p>"+
		"<P ALIGN=JUSTIFY>" +
		"To play an audio or video file, just click on the file from the file list." +
		"</p>"+
		
		"<b> Subtitles</b>" +
		"<P ALIGN=JUSTIFY>" +
		"Subtitle files with the following extensions are supported(srt, sub, smi, rt, txt, ssa, aqt, jss, js, ass, utf, utf8, utf-8). " +
		"</p>"+
		"<P ALIGN=JUSTIFY>" +
		"Unicode languages are supported in subtitles. Select the ttf file present in your SDcard in file browser. " +
		"</p>"+
		
		"<P ALIGN=CENTER>" +
		"<b>File List Configuration</b> " +
		"</p>"+
		"<P ALIGN=JUSTIFY>" +
		"The file list color, sorting type, listing of hidden files are configurable in settings menu." +
		" User can set their own Home directory for convenience in settings menu." +
		"</p>"+
		"<b> Loop option</b></br>  There are four options to play an audio/video file i.e " +
		"<P ALIGN=JUSTIFY>" +
		"<b> Play Once</b> - will play the selected file once." +
		"</p>"+
		"<P ALIGN=JUSTIFY>" +
		"<b> Play All</b> - will play all audio/video files in the directory once." +
		"</p>"+
		"<P ALIGN=JUSTIFY>" +
		"<b> Repeat One</b> - will repeat playing the same file."+
		"</p>"+
		"<P ALIGN=JUSTIFY>" +
		"<b> Repeat All</b> - will repeat playing all the audio/video files in directory." +
		"</p>"+
		"<P ALIGN=CENTER>" +
		"<b> Subtitle Configuration</b>" +
		"</p>"+
		"<P ALIGN=JUSTIFY>" +
		"User can show/hide subtitles for video and also configure size for subtitle fonts. Users can select a font file" +
		" from the file list or can configure the file in settings menu."+
		"</p>"+
		"<P ALIGN=CENTER>" +
		"<b> Supported Subtitle files</b>" +
		"</p>"+
		"<P ALIGN=JUSTIFY>" +
		"\"utf\", \"utf8\", \"utf-8\", \"sub\", \"srt\", " +
		"\"smi\", \"rt\", \"txt\", \"ssa\", \"aqt\", \"jss\", \"js\", \"ass\""+
		"</p>"+
		"<P ALIGN=CENTER>" +
		"<b> Languages Supported</b>" +
		"</p>"+
		"<P ALIGN=CENTER>" +
		"English</br>Chinese(Simplified)</br>French</br>German</br>Japanese</br>Italian"+
		"</p>"+
		"<P ALIGN=CENTER>" +
		"<b> Contact us </b>" +
		"</p>"+
		"<P ALIGN=JUSTIFY>" +
		"Use the feedback option to contact us."+
		"</p>"+
		"</p>";

	public static boolean dbHide         = false; // Show/Hide hidden folders
	public static boolean dbSubtitle     = true;  // Show/Hide subtitles
	public static int     dbColor        = Color.WHITE;
	public static int     dbSort         = 2;
	public static int     dbAudioLoop    = Globals.PLAY_ALL;
	public static int     dbVideoLoop    = Globals.REPEAT_ONE;
	public static int     dbSubtitleSize = 1;
	public static int     dbSubtitleEncoding = 0;
	public static String  dbDefaultHome	 = getSdcardPath();
	public static String  dbSubtitleFont = getSdcardPath()+"/broov.ttf";
	public static String  dbLastOpenDir  = getSdcardPath(); 
	public static boolean dbSkipframes     = true;  // Skip frames
	/**
	 * This will determine if hidden files and folders will be visible to the
	 * user.
	 * @param choice	true if user is viewing hidden files, false otherwise
	 */
	public static void setShowHiddenFiles(boolean choice) {
		Globals.dbHide = choice;
	}

	public static void setSortType(int type) {
		Globals.dbSort = type;
	}

	public static void setAudioLoop(int type){
		Globals.dbAudioLoop = type;
	}

	public static void setVideoLoop(int type){
		Globals.dbVideoLoop = type;
	}

	public static void setShowSubTitle(boolean type){
		Globals.dbSubtitle = type;	
	}

	public static void setSubTitleSize(int value){
		Globals.dbSubtitleSize = value;	
	}

	public static void setSubTitleEncoding(int value){
		Globals.dbSubtitleEncoding = value;	
	}

	public static void setDefaultHome(String value){
		System.out.println("setDefault Home value:"+value);
		if(value != null) {
			File file = new File(value);

			//if (value.contains("/")){
			if ( (file.isDirectory()&& file.exists()) ){
				Globals.dbDefaultHome = value;	
			} else {
				Globals.dbDefaultHome = defaultDir; //"/sdcard";
			}
		}
	}
	
	public static void setSubTitleFont(String value){
		System.out.println("setDefault Subtitle font value:"+value);
		if(value != null) {
			File file = new File(value);

			//if (value.contains("/")){
			if (  file.exists() ){
				Globals.dbSubtitleFont = value;	
			} else {
				Globals.dbSubtitleFont = defaultSubtitleFont; //"/sdcard";
			}
		}
	}
	
	public static void setLastOpenDir(String value){
		System.out.println("setLastOpendir value:"+value);
		if (value.contains("/")){
			Globals.dbLastOpenDir = value;	
		} else {
			Globals.dbLastOpenDir = defaultDir; //"/sdcard";
		}
	}
	
	public static void setSkipFrames(boolean choice) {
		Globals.dbSkipframes = choice;
	}

	public static void setColor(int value) {
		Globals.dbColor = value;
	}
	
	public static String getSdcardPath(){
		String sdcardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
		System.out.println("sdcardPath:"+sdcardPath);
		return sdcardPath;
	}
	static boolean isNativeLibrariesLoaded = false;
	public static void LoadNativeLibraries() {
		if (!isNativeLibrariesLoaded) {
			
			System.loadLibrary("ffmpeg");
			//System.loadLibrary("ffmpeg_7n");
			
			//System.loadLibrary("andprof");
			
			System.loadLibrary("sdl");
			System.loadLibrary("sdl_ttf");
			System.loadLibrary("sdl_image");

			System.loadLibrary("iconv");
			System.loadLibrary("universalchardet");
			
			System.loadLibrary("yuv2rgb");

			System.loadLibrary("application");
					
			isNativeLibrariesLoaded = true;
		}
	}
	
//	static {
//		System.loadLibrary("ffmpeg");
//		
//		System.loadLibrary("andprof");
//		System.loadLibrary("sdl");
//		System.loadLibrary("sdl_ttf");
//		System.loadLibrary("sdl_image");
//
//		System.loadLibrary("iconv");
//		System.loadLibrary("universalchardet");
//
//		System.loadLibrary("application");
//
//	}
}

//<!--   android:theme="@android:style/Theme.Dialog" 
//android:screenOrientation="portrait"/>-->

//<activity android:name=".Player"
//    android:label="@string/app_name" android:screenOrientation="landscape">


//<activity android:name=".Player"
//    android:label="@string/app_name" android:configChanges="orientation|keyboardHidden" 
//    android:screenOrientation="landscape" >

/*
<!-- The application's publisher ID assigned by AdMob -->
<meta-data android:value="a14d675ebf005c0" android:name="ADMOB_PUBLISHER_ID" />
<!-- AdMobActivity definition -->
<activity android:name="com.admob.android.ads.AdMobActivity"
	android:theme="@android:style/Theme.NoTitleBar.Fullscreen"
	android:configChanges="orientation|keyboard|keyboardHidden" />
<!-- Track Market installs -->
<receiver android:name="com.admob.android.ads.analytics.InstallReceiver"
	android:exported="true">
	<intent-filter>
		<action android:name="com.android.vending.INSTALL_REFERRER" />
	</intent-filter>
</receiver>
	<meta-data android:value="true" android:name="ADMOB_ALLOW_LOCATION_FOR_ADS" />
<!-- use a separate publisher id here to aid in tracking intersitial statistics -->
	<meta-data android:value="a14d675ebf005c0" android:name="ADMOB_INTERSTITIAL_PUBLISHER_ID" />


<!-- android:installLocation="auto" -->


	<com.admob.android.ads.AdView android:id="@+id/ad"
		android:layout_width="fill_parent" 
		android:layout_height="wrap_content"
		app:backgroundColor="#000000" 
		app:primaryTextColor="#FFFFFF"
		app:secondaryTextColor="#CCCCCC" />
res/main.xml, AVPlayerMain.java, AndroidManifest.xml for ADMob 

*/

/* "<b> Skip Frames</b>" +
"<P ALIGN=JUSTIFY>" +
"Slow mobile processors and highly compressed video files like mkv require more processing time. Due to this video lag will be observed for some of the files. For comfortable viewing experience, users can select this option to skip irrelevant video frames." +
"</p>"+

*/