package com.broov.player;

import java.io.File;

//import com.google.ads.AdRequest;
//import com.google.ads.AdSize;
//import com.google.ads.AdView;

import android.app.Dialog;
import android.app.AlertDialog;
import android.app.ListActivity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.SubMenu;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.View.OnClickListener;
import android.view.Menu;
import android.view.MenuItem;
import android.webkit.WebView;
//import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.ImageView;
import android.widget.Button;
import android.widget.Toast;

public final class AVPlayerMain extends ListActivity  {

	private static final int MENU_SETTING   = 0x01;	    //option menu id
	private static final int MENU_SEARCH    = 0x02;		//option menu id

	private static final int MENU_INFO   	= 0x07;		//option menu id
	private static final int MENU_HELP      = 0x071;		//option submenu id
	private static final int MENU_ABOUTUS   = 0x072;		//option submenu id
	private static final int MENU_FEEDBACK  = 0x073;		//option submenu id

	private static final int MENU_HOME      = 0x03;		//option menu id
	private static final int MENU_QUIT      = 0x06;		//option menu id

	private static final int SEARCH_B       = 0x09;
	private static final int SETTING_REQ    = 0x10;		//request code for intent

	private static FileManager 		 flmg;
	private static SharedPreferences settings;

	private EventHandler handler;
	private EventHandler.TableRow table;

	private boolean   	use_back_key = true;
	private TextView  	path_label;
	private TextView  	detail_label;
	static Context 		mContext;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_NO_TITLE);

		setContentView(R.layout.main);
		mContext = this;

		/*read settings*/
		readSettings();

		handler = new EventHandler(AVPlayerMain.this, flmg);
		handler.setTextColor(Globals.dbColor);
		table = handler.new TableRow();

		/*sets the ListAdapter for our ListActivity and
		 *gives our EventHandler class the same adapter
		 */
		handler.setListAdapter(table); 
		setListAdapter(table);

		/* register context menu for our list view */
		registerForContextMenu(getListView());

		path_label = (TextView)findViewById(R.id.path_label);
		detail_label = (TextView)findViewById(R.id.detail_label);
		//path_label.setText(mContext.getString(R.string.pathis)+ ":" +flmg.getCurrentDir());
		path_label.setText(flmg.getCurrentDir());

		handler.setUpdateLabel(path_label, detail_label);

	}

	@Override
	public void onPause(){
		super.onPause();
	}

	@Override
	public void onDestroy(){
		super.onDestroy();
	}

	/**
	 *  To add more functionality and let the user interact with more
	 *  file types, this is the function to add the ability. 
	 *  
	 *  (note): this method can be done more efficiently 
	 */
	@Override
	public void onListItemClick(ListView parent, View view, int position, long id) {

		final String item = handler.getData(position);

		File file = new File(flmg.getCurrentDir() + "/" + item);

		if (file.isDirectory()) {
			if(file.canRead()) {
				handler.updateDirectory(flmg.getNextDir(file.getAbsolutePath(), true));
				System.out.println("Current Dir:"+flmg.getCurrentDir());
				System.out.println("Path Stack:"+flmg.path_stack);
				path_label.setText(flmg.getCurrentDir());

				/*set back button switch to true (this will be better implemented later)*/
				if(!use_back_key)
					use_back_key = true;
			} else {
				Toast.makeText(this, R.string.cantreadfolderduetopermissions, 
						Toast.LENGTH_SHORT).show();
			}
		}else if(FileManager.isSubtitleFontFile(file.getPath())){
			String fontFilename = file.getPath();
			saveAndSetSubtitleFontFile(fontFilename);
			
		}else if (FileManager.supportedFile(file.getPath())) {

			if(file.exists()){
				if (FileManager.isAudioFile(file.getPath())) {
					String filename =   file.getPath();
					Intent intent = new Intent(AVPlayerMain.this, AudioPlayer.class);
					intent.putExtra("audiofilename", filename);
					startActivity(intent);

				} else {
					String filename =   file.getPath();
					Intent intent;
					if (Globals.isNativeVideoPlayerFeatureEnabled()) {
						intent = new Intent(AVPlayerMain.this, NativeVideoPlayer.class);
					} else {
						intent = new Intent(AVPlayerMain.this, VideoPlayer.class);
					}
					intent.putExtra("videofilename", filename);
					startActivity(intent);
				}
			}		
		}
	}

	@Override
	protected void onStop(){
		super.onStop();
		writeLastOpenedDirectory(flmg.getCurrentDir());
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);

		/* resultCode must equal RESULT_CANCELED because the only way
		 * out of that activity is pressing the back button on the phone
		 * this publishes a canceled result code not an ok result code
		 */
		if (requestCode == SETTING_REQ && resultCode == RESULT_CANCELED) {
			//save the information we get from settings activity
			boolean check_hidden = data.getBooleanExtra("HIDDEN", Globals.dbHide);
			boolean subtitle     = data.getBooleanExtra("SUBTITLE", Globals.dbSubtitle);
			int     color        = data.getIntExtra("COLOR", Globals.dbColor);
			int     sort         = data.getIntExtra("SORT", Globals.dbSort);
			int     audio_loop   = data.getIntExtra("AUDIOLOOP", Globals.dbAudioLoop);
			int     video_loop   = data.getIntExtra("VIDEOLOOP", Globals.dbVideoLoop);
			int     subtitlesize = data.getIntExtra("SUBTITLESIZE", Globals.dbSubtitleSize);
			int     subtitleencoding        = data.getIntExtra("SUBTITLEENCODING", Globals.dbSubtitleEncoding);
			String defaulthome = data.getStringExtra("HOME");
			String subtitlefont = data.getStringExtra("SUBTITLEFONT");
			boolean skipframes = data.getBooleanExtra("SKIPFRAME", Globals.dbSkipframes);
			//log("onActivityResult subtitlefont : " + subtitlefont);
			writeSettings(check_hidden, subtitle, color, sort, audio_loop, video_loop, subtitlesize, defaulthome, subtitleencoding, subtitlefont, skipframes);

			handler.setTextColor(Globals.dbColor);

			//Reload the directory listing based on the sorting type
			handler.updateDirectory(flmg.getNextDir(flmg.getCurrentDir(), true));
		}
	}

	/* ================Menus, options menu and context menu start here=================*/
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		super.onCreateOptionsMenu(menu);
		//menu.add(0, MENU_HOME,  0, R.string.home).setIcon(R.drawable.home);
		menu.add(0, MENU_SEARCH,  0, R.string.search).setIcon(R.drawable.search);
		menu.add(0, MENU_SETTING, 0, R.string.setting).setIcon(R.drawable.setting);
		SubMenu sub = menu.addSubMenu(0,MENU_INFO,0, R.string.info).setIcon(R.drawable.about);
		sub.add(0,MENU_HELP,0,R.string.help);
		sub.add(0,MENU_ABOUTUS,0,R.string.about);
		sub.add(0,MENU_FEEDBACK,0,R.string.feedback);

		menu.add(0, MENU_QUIT, 0, R.string.quit).setIcon(R.drawable.logout);

		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch(item.getItemId()) {
		case MENU_HELP:
			showDialog(MENU_HELP);
			return true;

		case MENU_HOME:
			File home_dir = new File(Globals.dbDefaultHome);
			if (home_dir.isDirectory()&&home_dir.exists()) {
				if(home_dir.canRead()) {
					handler.updateDirectory(flmg.getNextDir(home_dir.getAbsolutePath(), true));
					System.out.println("Current Dir:"+flmg.getCurrentDir());
					System.out.println("Path Stack:"+flmg.path_stack);
					path_label.setText(flmg.getCurrentDir());
				} else {
					Toast.makeText(this, R.string.cantreadfolderduetopermissions, 
							Toast.LENGTH_SHORT).show();
				}
			}
			return true;

		case MENU_ABOUTUS:
			showDialog(MENU_ABOUTUS);
			return true;

		case MENU_SEARCH:
			showDialog(MENU_SEARCH);
			return true;

		case MENU_FEEDBACK: 
			Intent Feedback = new Intent(this, Feedback.class);
			startActivity(Feedback);
			return true;

		case MENU_SETTING:
			Intent settings_int = new Intent(this, Settings.class);
			settings_int.putExtra("HIDDEN", Globals.dbHide);
			settings_int.putExtra("SUBTITLE", Globals.dbSubtitle);
			settings_int.putExtra("COLOR", Globals.dbColor);
			settings_int.putExtra("AUDIOLOOP", Globals.dbAudioLoop);
			settings_int.putExtra("VIDEOLOOP", Globals.dbVideoLoop);
			settings_int.putExtra("SORT", Globals.dbSort);
			settings_int.putExtra("SUBTITLESIZE", Globals.dbSubtitleSize);
			settings_int.putExtra("SUBTITLEENCODING", Globals.dbSubtitleEncoding);
			settings_int.putExtra("HOME", Globals.dbDefaultHome);
			settings_int.putExtra("SUBTITLEFONT", Globals.dbSubtitleFont);
			settings_int.putExtra("SKIPFRAME", Globals.dbSkipframes);
			startActivityForResult(settings_int, SETTING_REQ);

			return true;

		case MENU_QUIT:
			finish();
			return true;

		}
		return false;
	}

	/* ================Menus, options menu and context menu end here=================*/
	@Override
	protected Dialog onCreateDialog(int id) {
		final Dialog dialog = new Dialog(AVPlayerMain.this);

		switch(id) {
		case MENU_HOME:
		{
			File home_dir = new File(Globals.dbDefaultHome);
			if (home_dir.isDirectory()) {
				if(home_dir.canRead()) {
					handler.updateDirectory(flmg.getNextDir(home_dir.getAbsolutePath(), true));
					System.out.println("Current Dir:"+flmg.getCurrentDir());
					System.out.println("Path Stack:"+flmg.path_stack);
					path_label.setText(flmg.getCurrentDir());
				} else {
					Toast.makeText(this, R.string.cantreadfolderduetopermissions, 
							Toast.LENGTH_SHORT).show();
				}
			}
		}
		break;

		case MENU_SEARCH:
		{
			dialog.setContentView(R.layout.input_layout);
			dialog.setTitle(R.string.search);

			dialog.setCancelable(false);

			ImageView searchIcon = (ImageView)dialog.findViewById(R.id.input_icon);
			searchIcon.setImageResource(R.drawable.search);

			TextView search_label = (TextView)dialog.findViewById(R.id.input_label);
			search_label.setText(R.string.searchforafile);
			final EditText search_input = (EditText)dialog.findViewById(R.id.input_inputText);

			Button search_button = (Button)dialog.findViewById(R.id.input_create_b);
			Button cancel_button = (Button)dialog.findViewById(R.id.input_cancel_b);
			search_button.setText(R.string.search);

			search_button.setOnClickListener(new OnClickListener() {
				public void onClick(View v) {
					String temp = search_input.getText().toString();

					if (temp.length() > 0)
						handler.searchForFile(temp);
					dialog.dismiss();
				}
			});

			cancel_button.setOnClickListener(new OnClickListener() {
				public void onClick(View v) { dialog.dismiss(); }
			});
		}
			break;
			
		case MENU_ABOUTUS:
		{
			AlertDialog.Builder builder;
			AlertDialog alertDialog;

			System.out.println("About click");
			LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(LAYOUT_INFLATER_SERVICE);
			View layout = inflater.inflate(R.layout.custom_dialog,
					(ViewGroup) findViewById(R.id.layout_root));

			builder = new AlertDialog.Builder(mContext);
			builder.setView(layout);
			alertDialog = builder.create();
			alertDialog.setTitle(R.string.about);
			alertDialog.setIcon(R.drawable.icon);
			WebView contentText =(WebView) layout.findViewById(R.id.webviewcustom);
			contentText.setBackgroundColor(0);

			contentText.loadData(Globals.aboutUsContent, "text/html", "utf-8");

			alertDialog.setButton(mContext.getString(R.string.ok), new DialogInterface.OnClickListener() 
			{
				public void onClick(DialogInterface dialog, int which) 
				{
					dialog.dismiss();
				}
			});
			return	alertDialog;
		}

			//break;
		case MENU_HELP:
		{
			AlertDialog.Builder builder;
			AlertDialog alertDialog;
			System.out.println("About click");
			LayoutInflater inflaterhelp = (LayoutInflater) mContext.getSystemService(LAYOUT_INFLATER_SERVICE);
			View layouthelp = inflaterhelp.inflate(R.layout.custom_dialog,
					(ViewGroup) findViewById(R.id.layout_root));

			WebView contentTextHelp =(WebView) layouthelp.findViewById(R.id.webviewcustom);
			contentTextHelp.loadData(Globals.helpContent, "text/html", "utf-8");
			//contentTextHelp.setBackgroundColor(Color.GRAY);
			contentTextHelp.setBackgroundColor(Color.TRANSPARENT);
			
			builder = new AlertDialog.Builder(mContext);
			builder.setView(layouthelp);
			alertDialog = builder.create();
			alertDialog.setTitle(R.string.help);
			alertDialog.setIcon(R.drawable.icon);

			alertDialog.setButton(mContext.getString(R.string.ok), new DialogInterface.OnClickListener() 
			{
				public void onClick(DialogInterface dialog, int which) 
				{
					dialog.dismiss();
				}
			});
			return	alertDialog;
		}
		
		} //End of Switch case
		return dialog;
	} //End of OnCreateDialog() method


	/*
	 * (non-Javadoc)
	 * This will check if the user is at root directory. If so, if they press back
	 * again, it will close the application. 
	 * @see android.app.Activity#onKeyDown(int, android.view.KeyEvent)
	 */
	@Override
	public boolean onKeyDown(int keycode, KeyEvent event) {
		String current = flmg.getCurrentDir();
		if (keycode == KeyEvent.KEYCODE_SEARCH) {
			showDialog(SEARCH_B);
			return true;
		} else if (keycode == KeyEvent.KEYCODE_BACK && use_back_key && !current.equals("/")) {
			handler.updateDirectory(flmg.getPreviousDir());
			path_label.setText(flmg.getCurrentDir());

			return true;
		} else if (keycode == KeyEvent.KEYCODE_BACK && use_back_key && current.equals("/")) {
			Toast.makeText(AVPlayerMain.this, R.string.pressbackagaintoquit, Toast.LENGTH_SHORT).show();
			use_back_key = false;
			path_label.setText(flmg.getCurrentDir());
			return false;
		} else if (keycode == KeyEvent.KEYCODE_BACK && !use_back_key && current.equals("/")) {
			finish();
			return false;
		}
		return false;
	}

	public void log(String s){
		System.out.println(s);
	}


	public static void saveAndSetSubtitleFontFile(String value){
		SharedPreferences.Editor editor = settings.edit();
		editor.putString(Globals.PREFS_SUBTITLEFONT, value);
		editor.commit();
		Globals.setSubTitleFont(value);
		Toast.makeText(mContext, R.string.setasdefaultsubtitlefont, 
				Toast.LENGTH_LONG).show();
	}


	public void readSettings() {

		settings = getSharedPreferences(Globals.PREFS_NAME, MODE_PRIVATE);

		Globals.dbHide         = settings.getBoolean(Globals.PREFS_HIDDEN, Globals.dbHide);
		Globals.dbSubtitle     = settings.getBoolean(Globals.PREFS_SUBTITLE, Globals.dbSubtitle);
		Globals.dbColor        = settings.getInt(Globals.PREFS_COLOR, Globals.dbColor);
		Globals.dbSort         = settings.getInt(Globals.PREFS_SORT, Globals.dbSort);
		Globals.dbAudioLoop    = settings.getInt(Globals.PREFS_AUDIOLOOP, Globals.dbAudioLoop);
		Globals.dbVideoLoop    = settings.getInt(Globals.PREFS_VIDEOLOOP, Globals.dbVideoLoop);
		Globals.dbSubtitleSize = settings.getInt(Globals.PREFS_SUBTITLESIZE, Globals.dbSubtitleSize);
		Globals.dbLastOpenDir  = settings.getString(Globals.PREFS_LASTOPENDIR, Globals.dbLastOpenDir);
		Globals.dbSubtitleEncoding        = settings.getInt(Globals.PREFS_SUBTITLEENCODING, Globals.dbSubtitleEncoding);
		Globals.dbDefaultHome = settings.getString(Globals.PREFS_DEFAULTHOME, Globals.dbDefaultHome);
		Globals.dbSubtitleFont = settings.getString(Globals.PREFS_SUBTITLEFONT, Globals.dbSubtitleFont);
		Globals.dbSkipframes   = settings.getBoolean(Globals.PREFS_SKIPFRAME, Globals.dbSkipframes);
		System.out.println("On Main Start skipframes:"+Globals.dbSkipframes);

		flmg = new FileManager();
		Globals.setShowHiddenFiles(Globals.dbHide);
		Globals.setShowSubTitle(Globals.dbSubtitle);
		Globals.setSortType(Globals.dbSort);
		Globals.setAudioLoop(Globals.dbAudioLoop);
		Globals.setVideoLoop(Globals.dbVideoLoop);
		Globals.setSubTitleSize(Globals.dbSubtitleSize);
		Globals.setLastOpenDir(Globals.dbLastOpenDir);
		Globals.setSubTitleFont(Globals.dbSubtitleFont);
		Globals.setSkipFrames(Globals.dbSkipframes);
		//	log("Main audioloop:"+audioloop);
		//	log("Main videoloop:"+videoloop);
		//	log("Main sort:"+sort);
		//	log("Main color:"+color);
		//	log("Main subtitle:"+subtitle);
		//  log("Main subtitle size:"+subtitlesize);
		//  log("Main language:"+language);

		//log("Main audioloop_type:"+flmg.audioloop_type);
		//log("Main getloopSelected:"+flmg.getloopSelected());
	}

	public void writeSettings(
			boolean check_hidden, boolean subtitle,
			int color, int sort, int audio_loop,
			int video_loop,	int subtitlesize, String defaulthome, int subtitleencoding, String subtitlefont,
			boolean skipframes)
	{
		SharedPreferences.Editor editor = settings.edit();

		editor.putBoolean(Globals.PREFS_HIDDEN, check_hidden);
		editor.putBoolean(Globals.PREFS_SUBTITLE, subtitle);

		editor.putInt(Globals.PREFS_AUDIOLOOP, audio_loop);
		editor.putInt(Globals.PREFS_VIDEOLOOP, video_loop);

		editor.putInt(Globals.PREFS_SUBTITLESIZE, subtitlesize);
		editor.putInt(Globals.PREFS_COLOR, color);
		editor.putInt(Globals.PREFS_SORT, sort);
		editor.putString(Globals.PREFS_DEFAULTHOME, defaulthome);
		editor.putInt(Globals.PREFS_SUBTITLEENCODING, subtitleencoding);
		editor.putString(Globals.PREFS_SUBTITLEFONT, subtitlefont);
		editor.putBoolean(Globals.PREFS_SKIPFRAME, skipframes);
		System.out.println("Write settings skipframes:"+Globals.dbSkipframes);
		editor.commit();

		Globals.setShowHiddenFiles(check_hidden);
		Globals.setSortType(sort);
		Globals.setAudioLoop(audio_loop);
		Globals.setVideoLoop(video_loop);
		Globals.setShowSubTitle(subtitle);
		Globals.setDefaultHome(defaulthome);
		Globals.setSubTitleEncoding(subtitleencoding);
		Globals.setSubTitleFont(subtitlefont);
		Globals.setSkipFrames(skipframes);
	}

	public void writeLastOpenedDirectory(String currentOpenedDirectory) 
	{
		// We need an Editor object to make preference changes.
		// All objects are from android.context.Context
		SharedPreferences settings = getSharedPreferences(Globals.PREFS_NAME, MODE_PRIVATE);
		SharedPreferences.Editor editor = settings.edit();

		if (currentOpenedDirectory == null) {
			currentOpenedDirectory = Globals.getSdcardPath();
		}

		//log("OnStop flmg.getCurrentDir:"+currentOpenedDirectory);
		editor.putString(Globals.PREFS_LASTOPENDIR, currentOpenedDirectory); 

		// Commit the edits!
		editor.commit();
	}
}
