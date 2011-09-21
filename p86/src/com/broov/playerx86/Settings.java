package com.broov.playerx86;

import java.io.File;

import android.os.Bundle;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Color;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TableRow;
import android.widget.TextView;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.Toast;

public class Settings extends Activity {

	private boolean color_changed = false;
	private boolean sort_changed = false;
	private boolean hidden_changed = false;
	private boolean subtitle_changed = false;
	private boolean audioloop_changed = false;
	private boolean videoloop_changed = false;
	private boolean subtitlesize_changed = false;
	private boolean defaulthome_changed = false;
	private boolean subtitleencoding_changed = false;
	private boolean subtitlefontfile_changed = false;
	private boolean skipframe_changed = false;
	
	private int color_state;
	private int subtitleencoding_state;
	private int sort_state;
	private int audioloop_state;
	private int videoloop_state;
	private int subtitlesize_state;
	private boolean hidden_state;
	private boolean subtitle_state;
	private boolean skipframe_state;
	private String subtitlefontfile_state;
	
	private String defaulthome_state;

	private Intent is = new Intent();
	Context context;
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.settings);
		 context = getApplicationContext();
		Intent i = getIntent();
		hidden_state = i.getExtras().getBoolean("HIDDEN");
		subtitle_state = i.getExtras().getBoolean("SUBTITLE");
		audioloop_state = i.getExtras().getInt("AUDIOLOOP");
		videoloop_state = i.getExtras().getInt("VIDEOLOOP");

		subtitlesize_state = i.getExtras().getInt("SUBTITLESIZE");

		color_state = i.getExtras().getInt("COLOR");
		sort_state = i.getExtras().getInt("SORT");
		defaulthome_state = i.getExtras().getString("HOME");
		subtitleencoding_state = i.getExtras().getInt("SUBTITLEENCODING");
		subtitlefontfile_state = i.getExtras().getString("SUBTITLEFONT");
		skipframe_state = i.getExtras().getBoolean("SKIPFRAME");		
		System.out.println("Settings skipframe_state:"+skipframe_state);
		final CheckBox hidden_bx = (CheckBox)findViewById(R.id.setting_hidden_box);
		final CheckBox subtitle_bx = (CheckBox)findViewById(R.id.setting_subtitlehide_box);
		
		//final CheckBox skipframe_bx = (CheckBox)findViewById(R.id.setting_skipframes_box);
		
		final String colors[]= getResources().getStringArray(R.array.Colors);       
		final String subtitleencoding[]= getResources().getStringArray(R.array.SubtitleEncoding);       
		
		final TextView colorSelected = (TextView)findViewById(R.id.colorSelected);
		final TextView subtitleEncodingTypeSelected = (TextView)findViewById(R.id.subtitleEncodingTypeSelected);
		//final TextView defaulthomeSelected = (TextView)findViewById(R.id.defaulthomeselected);
		
		final TextView subtitleFontFileSelected = (TextView)findViewById(R.id.subtitlefontfileselected);
		
		final String subtitlesize[] = getResources().getStringArray(R.array.SubtitleSize);

		hidden_bx.setChecked(hidden_state); 
		subtitle_bx.setChecked(subtitle_state);
		//skipframe_bx.setChecked(skipframe_state);

		final TextView audioloopSelected = (TextView)findViewById(R.id.audioloopSelected);
		final TextView videoloopSelected = (TextView)findViewById(R.id.videoloopSelected);
		final TextView sortoptionSelected = (TextView)findViewById(R.id.sortoptionSelected);

		final TextView subtitlesizeSelected = (TextView)findViewById(R.id.subtitleSizeSelected);

		final String sortoptions[]=getResources().getStringArray(R.array.Sort);
		final String loop[]= getResources().getStringArray(R.array.Loop);

		final TableRow tblrowTextColor = (TableRow)findViewById(R.id.tblrowtextcolor);
		final TableRow tblrowloopAudio = (TableRow)findViewById(R.id.tblrowloopaudio);
		final TableRow tblrowloopVideo = (TableRow)findViewById(R.id.tblrowloopvideo);
		final TableRow tblrowShowHiddenFiles = (TableRow)findViewById(R.id.tblrowshowhiddenfiles);
		final TableRow tblrowShowSubTitles = (TableRow)findViewById(R.id.tblrowshowsubtitles);
		final TableRow tblrowSortingType = (TableRow)findViewById(R.id.tblrowsortingtype);
		final TableRow tblrowSubtitleSize = (TableRow)findViewById(R.id.tblrowsubtitlesize);
		//final TableRow tblrowdefaulthomeSelected = (TableRow)findViewById(R.id.tblrowdefaulthome);
		final TableRow tblrowsubtitleFontFile = (TableRow)findViewById(R.id.tblrowsubtitlefontfileselected);
		final TableRow tblrowSubtitleEncodingType = (TableRow)findViewById(R.id.tblrowsubtitleencodingtype);
		//final TableRow tblrowSkipframes = (TableRow)findViewById(R.id.tblrowskipframes);
		
		ImageButton setting_color_btn = (ImageButton)this.findViewById(R.id.setting_color_button);
		//ImageButton setting_defaulthome_btn = (ImageButton)this.findViewById(R.id.setting_defaulthome_button);
		ImageButton setting_subtitleenconding_btn = (ImageButton)this.findViewById(R.id.setting_subtitleencoding_button);
		ImageButton setting_subtitlefontfile_btn = (ImageButton)this.findViewById(R.id.setting_subtitlefontfile_button);
		ImageButton setting_subtitlesize_button = (ImageButton)this.findViewById(R.id.setting_subtitlesize_button);
		ImageButton setting_audioloop_btn = (ImageButton)this.findViewById(R.id.setting_audioloop_button);
		ImageButton setting_videoloop_button = (ImageButton)this.findViewById(R.id.setting_videoloop_button);
		ImageButton setting_sort_button = (ImageButton)this.findViewById(R.id.setting_sort_button);
		
		//defaulthomeSelected.setText(defaulthome_state);
		subtitleFontFileSelected.setText(subtitlefontfile_state);
		subtitleEncodingTypeSelected.setText(subtitleencoding[subtitleencoding_state]);
		
		switch(sort_state){
		case 0:
			sortoptionSelected.setText(sortoptions[0]);
			break;
		case 1:
			sortoptionSelected.setText(sortoptions[1]);
			break;
		case 2:
			sortoptionSelected.setText(sortoptions[2]);
			break;
		}

		switch(audioloop_state){
		case 0:
			audioloopSelected.setText(loop[0]);
			break;
		case 1:
			audioloopSelected.setText(loop[1]);
			break;
		case 2:
			audioloopSelected.setText(loop[2]);
			break;
		case 3:
			audioloopSelected.setText(loop[3]);
			break;
		}

		switch(videoloop_state){
		case 0:
			videoloopSelected.setText(loop[0]);
			break;
		case 1:
			videoloopSelected.setText(loop[1]);
			break;
		case 2:
			videoloopSelected.setText(loop[2]);
			break;
		case 3:
			videoloopSelected.setText(loop[3]);
			break;
		}

		switch(subtitlesize_state){
		case 0:
			subtitlesizeSelected.setText(subtitlesize[0]);
			break;
		case 1:
			subtitlesizeSelected.setText(subtitlesize[1]);
			break;
		case 2:
			subtitlesizeSelected.setText(subtitlesize[2]);
			break;
		}

		switch(color_state){
		case Color.WHITE:
			colorSelected.setText(colors[0]);
			break;
		case Color.GRAY:
			colorSelected.setText(colors[1]);
			break;
		case Color.GREEN:
			colorSelected.setText(colors[2]);
			break;
		case Color.RED:
			colorSelected.setText(colors[3]);
			break;
		case Color.BLUE:
			colorSelected.setText(colors[4]);
			break;
		case Color.YELLOW:
			colorSelected.setText(colors[5]);
			break;
		case Color.MAGENTA:
			colorSelected.setText(colors[6]);
			break;
		case Color.CYAN:
			colorSelected.setText(colors[7]);
			break;
		}
		
		OnClickListener subtitleEncodingTypeOnClickListener = new OnClickListener() {
			@Override
			public void onClick(View view) {
				AlertDialog.Builder builder = new AlertDialog.Builder(Settings.this);
				AlertDialog dialog;
				builder.setTitle(R.string.subtitleencodingtype);
				builder.setIcon(R.drawable.icon);

				builder.setItems(R.array.SubtitleEncoding, new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int index) {
						subtitleEncodingTypeSelected.setText(subtitleencoding[index]);
						subtitleencoding_state=index;
						is.putExtra("SUBTITLEENCODING", subtitleencoding_state);
						subtitleencoding_changed = true;
					}
				});

				dialog = builder.create();
				dialog.show();
			}
		};
		tblrowSubtitleEncodingType.setOnClickListener(subtitleEncodingTypeOnClickListener);
		setting_subtitleenconding_btn.setOnClickListener(subtitleEncodingTypeOnClickListener);
		
		/*OnClickListener defaulthomeselectedOnClickListener =new OnClickListener(){
			//System.out.println("inside OnClickListener");
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				System.out.println("inside OnClick");
				final Dialog dialog = new Dialog(Settings.this);
				dialog.setContentView(R.layout.home_input_dialog);
				dialog.setTitle(R.string.setyourdefaulthomepath);

				dialog.setCancelable(false);

				ImageView homeIcon = (ImageView)dialog.findViewById(R.id.input_icon);
				homeIcon.setImageResource(R.drawable.home);

				final TextView defaulthomealreadyset_label = (TextView)dialog.findViewById(R.id.input_label);
				defaulthomealreadyset_label.setText(defaulthome_state);
				final EditText homepath_input = (EditText)dialog.findViewById(R.id.home_PathText);
				
				homepath_input.setText(defaulthomeSelected.getText());
				Button ok_button = (Button)dialog.findViewById(R.id.input_ok_b);
				Button cancel_button = (Button)dialog.findViewById(R.id.input_cancel_b);

				ok_button.setOnClickListener(new OnClickListener() {
					public void onClick(View v) {
						String path_Entered = homepath_input.getText().toString();
						// Check if the input directory is valid or not
						System.out.println("path_Entered:" + path_Entered);
						File file = new File(path_Entered);
						if (file.isDirectory()&& file.exists()) {
							final String filePathString = file.getPath();
							defaulthomealreadyset_label.setText(filePathString);
							defaulthomeSelected.setText(filePathString);
							defaulthome_state = filePathString;
							System.out.print("OK button click:"+defaulthome_state);
							defaulthome_changed = true;
							is.putExtra("HOME", defaulthome_state);
							Toast.makeText(context, R.string.setasdefaulthomepath, Toast.LENGTH_LONG).show();
							dialog.dismiss();
						}else{
							Toast.makeText(context, R.string.setavaliddirectory, Toast.LENGTH_LONG).show();
						}
					}
				});

				cancel_button.setOnClickListener(new OnClickListener() {
					public void onClick(View v) { dialog.dismiss(); }
				});
				dialog.show();
			}
		};
		tblrowdefaulthomeSelected.setOnClickListener(defaulthomeselectedOnClickListener);
		setting_defaulthome_btn.setOnClickListener(defaulthomeselectedOnClickListener);
*/
		OnClickListener subtitleFontFileClickListener =	new OnClickListener(){
			//System.out.println("inside OnClickListener");
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				System.out.println("inside OnClick");
				final Dialog dialog = new Dialog(Settings.this);
				dialog.setContentView(R.layout.home_input_dialog);
				dialog.setTitle(R.string.fontfile);

				dialog.setCancelable(false);

				ImageView fontIcon = (ImageView)dialog.findViewById(R.id.input_icon);
				fontIcon.setImageResource(R.drawable.ttf);

				final TextView fontfilealreadyset_label = (TextView)dialog.findViewById(R.id.input_label);
				fontfilealreadyset_label.setText(subtitlefontfile_state);
				final EditText fontfile_input = (EditText)dialog.findViewById(R.id.home_PathText);
				fontfile_input.setText(subtitleFontFileSelected.getText());
				Button ok_button = (Button)dialog.findViewById(R.id.input_ok_b);
				Button cancel_button = (Button)dialog.findViewById(R.id.input_cancel_b);

				ok_button.setOnClickListener(new OnClickListener() {
					public void onClick(View v) {
						String path_Entered = fontfile_input.getText().toString();
						// Check if the input directory is valid or not
						System.out.println("path_Entered:" + path_Entered);
						File file = new File(path_Entered);
						if ( file.exists() ) {
							final String filePathString = file.getPath();
							fontfilealreadyset_label.setText(filePathString);
							
							subtitlefontfile_state = filePathString;
							System.out.print("OK button click:"+subtitlefontfile_state);
							subtitlefontfile_changed = true;
							is.putExtra("SUBTITLEFONT", subtitlefontfile_state);
							
							Toast.makeText(context, R.string.setasdefaultsubtitlefont, Toast.LENGTH_LONG).show();
							dialog.dismiss();
							
						}else{
							Toast.makeText(context, R.string.setavalidsubtitlefontfile, Toast.LENGTH_LONG).show();
						}
					}
				});

				cancel_button.setOnClickListener(new OnClickListener() {
					public void onClick(View v) { dialog.dismiss(); }
				});
				dialog.show();
			}
	};
		tblrowsubtitleFontFile.setOnClickListener(subtitleFontFileClickListener);
		setting_subtitlefontfile_btn.setOnClickListener(subtitleFontFileClickListener);
		
		OnClickListener colorClickListener = new OnClickListener() {
			@Override
			public void onClick(View view) {
				AlertDialog.Builder builder = new AlertDialog.Builder(Settings.this);
				AlertDialog dialog;

				builder.setTitle(R.string.changetextcolor);
				builder.setIcon(R.drawable.color);

				builder.setItems(R.array.Colors, new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int index) {
						switch(index) {
						case 0:
							color_state = Color.WHITE;
							colorSelected.setText(colors[0]);
							break;
						case 1:
							color_state = Color.GRAY;
							colorSelected.setText(colors[1]);
							break;
						case 2:
							color_state = Color.GREEN;
							colorSelected.setText(colors[2]);
							break;
						case 3:
							color_state = Color.RED;
							colorSelected.setText(colors[3]);
							break;
						case 4:
							color_state = Color.BLUE;
							colorSelected.setText(colors[4]);
							break;
						case 5:
							color_state = Color.YELLOW;
							colorSelected.setText(colors[5]);
							break;
						case 6:
							color_state = Color.MAGENTA;
							colorSelected.setText(colors[6]);
							break;
						case 7:
							color_state = Color.CYAN;
							colorSelected.setText(colors[7]);
							break;
						}

						is.putExtra("COLOR", color_state);

						color_changed = true;
					}

				});
				dialog = builder.create();
				dialog.show();
			}
		};  
		tblrowTextColor.setOnClickListener(colorClickListener);
		setting_color_btn.setOnClickListener(colorClickListener);
		
		
		tblrowShowHiddenFiles.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View view) {
				if (hidden_bx.isChecked()){
					hidden_bx.setChecked(false);
				}else{
					hidden_bx.setChecked(true);
				}
			}});		

		hidden_bx.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
			@Override
			public void onCheckedChanged(CompoundButton arg0, boolean arg1) {
				hidden_state = hidden_bx.isChecked();
				is.putExtra("HIDDEN", hidden_state);
				hidden_changed = true;
			}
		});

		tblrowShowSubTitles.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View view) {
				if (subtitle_bx.isChecked()){
					subtitle_bx.setChecked(false);
				}else{
					subtitle_bx.setChecked(true);
				}
			}});		

		subtitle_bx.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
			@Override
			public void onCheckedChanged(CompoundButton arg0, boolean arg1) {
				subtitle_state = subtitle_bx.isChecked();
				is.putExtra("SUBTITLE", subtitle_state);
				subtitle_changed = true;
				//	boolean b=	is.getExtras().getBoolean("SUBTITLE");
				//	   System.out.println("is.getBool subtitle:"+b);
			}
		});
		
		/*tblrowSkipframes.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View view) {
				if (skipframe_bx.isChecked()){
					skipframe_bx.setChecked(false);
				}else{
					skipframe_bx.setChecked(true);
				}
			}});		

		skipframe_bx.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
			@Override
			public void onCheckedChanged(CompoundButton arg0, boolean arg1) {
				skipframe_state = skipframe_bx.isChecked();
				is.putExtra("SKIPFRAME", skipframe_state);
				skipframe_changed = true;
				//	boolean b=	is.getExtras().getBoolean("SUBTITLE");
				//	   System.out.println("is.getBool subtitle:"+b);
			}
		});
*/
		OnClickListener sortingtypeOnClickListener =new OnClickListener() {
			@Override
			public void onClick(View view) {
				tblrowSortingType.setBackgroundColor(Color.YELLOW);

				AlertDialog.Builder builder = new AlertDialog.Builder(Settings.this);

				builder.setTitle(R.string.sortby);
				builder.setItems(R.array.Sort, new DialogInterface.OnClickListener() {					
					@Override
					public void onClick(DialogInterface dialog, int index) {
						switch(index) {
						case 0:
							sort_state = 0;
							sort_changed = true;
							is.putExtra("SORT", sort_state);
							sortoptionSelected.setText(sortoptions[0]);
							break;

						case 1:
							sort_state = 1;
							sort_changed = true;
							is.putExtra("SORT", sort_state);
							sortoptionSelected.setText(sortoptions[1]);
							break;

						case 2:
							sort_state = 2;
							sort_changed = true;
							is.putExtra("SORT", sort_state);
							sortoptionSelected.setText(sortoptions[2]);
							break;
						}
					}
				});
				tblrowSortingType.setBackgroundColor(Color.TRANSPARENT);

				builder.create().show();

			}
		};
		tblrowSortingType.setOnClickListener(sortingtypeOnClickListener);
		setting_sort_button.setOnClickListener(sortingtypeOnClickListener);

		OnClickListener subtitlesizeOnClickListener =new OnClickListener() {
			@Override
			public void onClick(View view) {
				AlertDialog.Builder builder = new AlertDialog.Builder(Settings.this);

				builder.setTitle(R.string.subtitlesize);
				builder.setItems(R.array.SubtitleSize, new DialogInterface.OnClickListener() {					
					@Override
					public void onClick(DialogInterface dialog, int index) {
						switch(index) {
						case 0:
							subtitlesize_state = 0;
							subtitlesize_changed = true;
							is.putExtra("SUBTITLESIZE", subtitlesize_state);
							subtitlesizeSelected.setText(subtitlesize[0]);
							break;

						case 1:
							subtitlesize_state = 1;
							subtitlesize_changed = true;
							is.putExtra("SUBTITLESIZE", subtitlesize_state);
							subtitlesizeSelected.setText(subtitlesize[1]);
							break;

						case 2:
							subtitlesize_state = 2;
							subtitlesize_changed = true;
							is.putExtra("SUBTITLESIZE", subtitlesize_state);
							subtitlesizeSelected.setText(subtitlesize[2]);
							break;
						}
					}
				});

				builder.create().show();
			}
		}; 
		tblrowSubtitleSize.setOnClickListener(subtitlesizeOnClickListener);
		setting_subtitlesize_button.setOnClickListener(subtitlesizeOnClickListener);

		OnClickListener loopaudioOnClickListener =new OnClickListener() {
			@Override
			public void onClick(View view) {
				AlertDialog.Builder builder = new AlertDialog.Builder(Settings.this);

				builder.setTitle(R.string.loop);
				builder.setItems(R.array.Loop, new DialogInterface.OnClickListener() {					
					@Override
					public void onClick(DialogInterface dialog, int index) {
						switch(index) {
						case 0:
							audioloop_state = 0;
							audioloop_changed = true;
							is.putExtra("AUDIOLOOP", audioloop_state);
							audioloopSelected.setText(loop[0]);
							break;

						case 1:
							audioloop_state = 1;
							audioloop_changed = true;
							is.putExtra("AUDIOLOOP", audioloop_state);
							audioloopSelected.setText(loop[1]);
							break;

						case 2:
							audioloop_state = 2;
							audioloop_changed = true;
							is.putExtra("AUDIOLOOP", audioloop_state);
							audioloopSelected.setText(loop[2]);
							break;

						case 3:
							audioloop_state = 3;
							audioloop_changed = true;
							is.putExtra("AUDIOLOOP", audioloop_state);
							audioloopSelected.setText(loop[3]);
							break;
						}
					}
				});

				builder.create().show();
			}
		};  
		tblrowloopAudio.setOnClickListener(loopaudioOnClickListener);
		setting_audioloop_btn.setOnClickListener(loopaudioOnClickListener);
		
		OnClickListener loopVideoOnClickListener =new OnClickListener() {
			@Override
			public void onClick(View view) {
				AlertDialog.Builder builder = new AlertDialog.Builder(Settings.this);

				builder.setTitle(R.string.loop);
				builder.setItems(R.array.Loop, new DialogInterface.OnClickListener() {					
					@Override
					public void onClick(DialogInterface dialog, int index) {
						switch(index) {
						case 0:
							videoloop_state = 0;
							videoloop_changed = true;
							is.putExtra("VIDEOLOOP", videoloop_state);
							videoloopSelected.setText(loop[0]);
							break;

						case 1:
							videoloop_state = 1;
							videoloop_changed = true;
							is.putExtra("VIDEOLOOP", videoloop_state);
							videoloopSelected.setText(loop[1]);
							break;

						case 2:
							videoloop_state = 2;
							videoloop_changed = true;
							is.putExtra("VIDEOLOOP", videoloop_state);
							videoloopSelected.setText(loop[2]);
							break;
						case 3:
							videoloop_state = 3;
							videoloop_changed = true;
							is.putExtra("VIDEOLOOP", videoloop_state);
							videoloopSelected.setText(loop[3]);
							break;

						}
					}
				});

				builder.create().show();
			}
		}; 
		tblrowloopVideo.setOnClickListener(loopVideoOnClickListener);
		setting_videoloop_button.setOnClickListener(loopVideoOnClickListener);
	}

	@Override
	protected void onResume() {
		super.onResume();

		if(!hidden_changed){
			is.putExtra("HIDDEN", hidden_state);
		}

		if(!audioloop_changed){
			is.putExtra("AUDIOLOOP", audioloop_state);
		}

		if(!videoloop_changed){
			is.putExtra("VIDEOLOOP", videoloop_state);
		}

		if(!subtitlesize_changed){
			is.putExtra("SUBTITLESIZE", subtitlesize_state);
		}

		if(!subtitle_changed){
			is.putExtra("SUBTITLE", subtitle_state);
		}

		if(!color_changed){
			is.putExtra("COLOR", color_state);
		}

		if(!sort_changed){
			is.putExtra("SORT", sort_state);
		}
		if(!defaulthome_changed){
			System.out.println("settings result return defaulthome_state:"+defaulthome_state);
			is.putExtra("HOME", defaulthome_state);
		}
		if(!subtitlefontfile_changed){
			System.out.println("settings result return defaulthome_state:"+defaulthome_state);
			is.putExtra("SUBTITLEFONT", subtitlefontfile_state);
		}

		if(!subtitleencoding_changed){
			is.putExtra("SUBTITLEENCODING", subtitleencoding_state);
		}

		if(!skipframe_changed){
			is.putExtra("SKIPFRAME", skipframe_state);
		}

		setResult(RESULT_CANCELED, is);
	}
}
