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

	//advanced options
	private boolean advskipframes_changed=false;
	private boolean advbidirectional_changed=false;
	private boolean advffmpeg_changed=false; //fast decoding
	private boolean advyuv2rgb_changed=false;
	private boolean advpixelformat_changed=false;	

	private boolean advminvideoq_changed=false;
	private boolean advmaxvideoq_changed=false;
	private boolean advmaxaudioq_changed=false;
	private boolean advstreamminvideoq_changed=false;
	private boolean advstreammaxvideoq_changed=false;
	private boolean advstreammaxaudioq_changed=false;
	private boolean advswsscaler_changed=false;
	private boolean advdebug_changed=false;	
	private boolean advavsyncmode_changed=false;
	
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

	private boolean advskipframes_state;
	private boolean advbidirectional_state;
	private boolean advffmpeg_state; //fast decoding
	private int advyuv2rgb_state;
	private int advpixelformat_state;	
	
	private int advminvideoq_state; 
	private int advmaxvideoq_state;
	private int advmaxaudioq_state;
	private int advstreamminvideoq_state;
	private int advstreammaxvideoq_state;
	private int advstreammaxaudioq_state;
	private boolean  advdebug_state;
	private int  advavsyncmode_state;
	private int advswsscaler_state;

	private String defaulthome_state;

	private Intent is = new Intent();
	Context context;
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
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

		//advanced
		advskipframes_state=i.getExtras().getBoolean("ADVSKIPFRAMES");
		advbidirectional_state=i.getExtras().getBoolean("BIDIRECTIONAL");
		advffmpeg_state=i.getExtras().getBoolean("ADVFFMPEG");
		advyuv2rgb_state=i.getExtras().getInt("ADVYUV2RGB");
		advpixelformat_state=i.getExtras().getInt("ADVPIXELFORMAT");		
		advminvideoq_state=i.getExtras().getInt("ADVMINVIDEOQ");
		advmaxvideoq_state=i.getExtras().getInt("ADVMAXVIDEOQ");
		advmaxaudioq_state=i.getExtras().getInt("ADVMAXAUDIOQ");
		advstreamminvideoq_state=i.getExtras().getInt("ADVSTREAMMINVIDEOQ");
		advstreammaxvideoq_state=i.getExtras().getInt("ADVSTREAMMAXVIDEOQ");
		advstreammaxaudioq_state=i.getExtras().getInt("ADVSTREAMMAXAUDIOQ");
		advdebug_state=i.getExtras().getBoolean("ADVDEBUG");
		advavsyncmode_state = i.getExtras().getInt("ADVAVSYNCMODE");
		advswsscaler_state=i.getExtras().getInt("ADVSWSSCALER");

		//System.out.println("Inside Settings Activity: skipframes:" + advskipframes_state+" bidirectional:"+advbidirectional_state);
		//System.out.println("ffmpeg_state:"+advffmpeg_state);
		//System.out.println("yuv2rgb state:"+advyuv2rgb_state);
		//System.out.println("Min VideoQ:"+advminvideoq_state);
		//System.out.println("Max VideoQ:"+advmaxvideoq_state);
		//System.out.println("Max AudioQ:"+advmaxaudioq_state);
		//System.out.println("Stream Min VideoQ:"+advstreamminvideoq_state);
		//System.out.println("Stream Max VideoQ:"+advstreammaxvideoq_state);
		//System.out.println("Stream Max AudioQ:"+advstreammaxaudioq_state);
		//System.out.println("Pixel Format:"+advpixelformat_state);
		//System.out.println("Debug Mode:"+advdebug_state);
		//System.out.println("A/V Sync Type:"+advavsyncmode_state);
		//System.out.println("SWS Scaler Type:"+advswsscaler_state);

		final CheckBox hidden_bx = (CheckBox)findViewById(R.id.setting_hidden_box);
		final CheckBox subtitle_bx = (CheckBox)findViewById(R.id.setting_subtitlehide_box);

		
		final String colors[]= getResources().getStringArray(R.array.Colors);       
		final String subtitleencoding[]= getResources().getStringArray(R.array.SubtitleEncoding);       
		final TextView colorSelected = (TextView)findViewById(R.id.colorSelected);
		final TextView subtitleEncodingTypeSelected = (TextView)findViewById(R.id.subtitleEncodingTypeSelected);

		//advanced
		final CheckBox skipframes_bx = (CheckBox)findViewById(R.id.setting_skipframes_hidden_box);
		final CheckBox skipbidirectionalframes_bx = (CheckBox)findViewById(R.id.setting_skipbidirectionalframes_hidden_box);
		final CheckBox advffmpeg_bx = (CheckBox)findViewById(R.id.setting_ffmpegfastdecoding_hidden_box);
		final CheckBox advdebug_bx = (CheckBox)findViewById(R.id.setting_debugmode_hidden_box);

		final TextView yuv2rgbloopSelected = (TextView)findViewById(R.id.yuv2rgbloopSelected);
		final TextView minvideoqloopSelected=(TextView)findViewById(R.id.minvideoqsizeloopSelected);
		final TextView maxvideoqloopSelected=(TextView)findViewById(R.id.maxvideoqsizeloopSelected);
		final TextView maxaudioqloopSelected=(TextView)findViewById(R.id.maxaudioqsizeloopSelected);
		final TextView streamminvideoqloopSelected=(TextView)findViewById(R.id.streamminvideooqsizeloopSelected);
		final TextView streammaxvideoqloopSelected=(TextView)findViewById(R.id.streammaxvideoqsizeloopSelected);
		final TextView streammaxaudioqloopSelected=(TextView)findViewById(R.id.streammaxaudioqsizeloopSelected);
		final TextView pixelformatloopSelected=(TextView)findViewById(R.id.displaypixelformatloopSelected);
		final TextView advavsyncmodeloopSelected=(TextView)findViewById(R.id.avsyncmodeloopSelected);
		final TextView advswsscalerloopSelected = (TextView)findViewById(R.id.swsscalerloopSelected);

		final TextView subtitleFontFileSelected = (TextView)findViewById(R.id.subtitlefontfileselected);

		final String subtitlesize[] = getResources().getStringArray(R.array.SubtitleSize);
		
		final String advminvideoq[]=getResources().getStringArray(R.array.advminvideoq);
		final String advmaxvideoq[]=getResources().getStringArray(R.array.advmaxvideoq);
		final String advmaxaudioq[]=getResources().getStringArray(R.array.advmaxaudioq);
		final String streamminvideoq[]=getResources().getStringArray(R.array.advstreamminvideoq);
		final String streammaxvideoq[]=getResources().getStringArray(R.array.advstreammaxvideoq);
		final String streammaxaudioq[]=getResources().getStringArray(R.array.advstreammaxaudioq);
		final String yuv2rgb[]=getResources().getStringArray(R.array.advyuv2rgb);		
		final String pixelformat[]=getResources().getStringArray(R.array.advpixelformat);
		final String avsyncmode[]=getResources().getStringArray(R.array.advavsyncmode);
		final String advswsscaler[]= getResources().getStringArray(R.array.advswsscaler);		
		
		hidden_bx.setChecked(hidden_state); 
		subtitle_bx.setChecked(subtitle_state);
		
		//advanced
		skipframes_bx.setChecked(advskipframes_state);
		skipbidirectionalframes_bx.setChecked(advbidirectional_state);
		advffmpeg_bx.setChecked(advffmpeg_state);
		advdebug_bx.setChecked(advdebug_state);		
		
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
		final TableRow tblrowsubtitleFontFile = (TableRow)findViewById(R.id.tblrowsubtitlefontfileselected);
		final TableRow tblrowSubtitleEncodingType = (TableRow)findViewById(R.id.tblrowsubtitleencodingtype);
	
		//advanced
		final TableRow tblrowadvancedskipframes=(TableRow)findViewById(R.id.tblrowadvancedskipframes);
		final TableRow tblrowadvancedskipbidirectionalframes=(TableRow)findViewById(R.id.tblrowadvancedskipbidirectionalframes);
		final TableRow tblrowadvancedffmpeg=(TableRow)findViewById(R.id.tblrowadvancedffmpegfastdecoding);
		final TableRow tblrowadvancedyuv2rgb=(TableRow)findViewById(R.id.tblrowadvancedyuv2rgb);
		final TableRow tblrowadvancedminvideoqsize=(TableRow)findViewById(R.id.tblrowadvancedminvideoqsize);
		final TableRow tblrowadvancedmaxvideoqsize=(TableRow)findViewById(R.id.tblrowadvancedmaxvideoqsize);
		final TableRow tblrowadvancedmaxaudioqsize=(TableRow)findViewById(R.id.tblrowadvancedmaxaudioqsize);
		
		final TableRow tblrowadvancedstreamminvideoqsize=(TableRow)findViewById(R.id.tblrowadvancedstreamminvideoqsize);
		final TableRow tblrowadvancedstreammaxvideoqsize=(TableRow)findViewById(R.id.tblrowadvancedstreammaxvideoqsize);
		final TableRow tblrowadvancedstreammaxaudioqsize=(TableRow)findViewById(R.id.tblrowadvancedstreammaxaudioqsize);
		
		final TableRow tblrowadvancedpixelformat=(TableRow)findViewById(R.id.tblrowadvanceddisplaypixelformat);
		final TableRow tblrowadvancedavsyncmode=(TableRow)findViewById(R.id.tblrowadvancedavsyncmode);
		final TableRow tblrowadvanceddebugmode=(TableRow)findViewById(R.id.tblrowadvanceddebugmode);
		final TableRow tblrowadvancedadvswsscaler=(TableRow)findViewById(R.id.tblrowadvancedswsscaler);

		ImageButton setting_color_btn = (ImageButton)this.findViewById(R.id.setting_color_button);
		ImageButton setting_subtitleenconding_btn = (ImageButton)this.findViewById(R.id.setting_subtitleencoding_button);
		ImageButton setting_subtitlefontfile_btn = (ImageButton)this.findViewById(R.id.setting_subtitlefontfile_button);
		ImageButton setting_subtitlesize_button = (ImageButton)this.findViewById(R.id.setting_subtitlesize_button);
		ImageButton setting_audioloop_btn = (ImageButton)this.findViewById(R.id.setting_audioloop_button);
		ImageButton setting_videoloop_button = (ImageButton)this.findViewById(R.id.setting_videoloop_button);
		ImageButton setting_sort_button = (ImageButton)this.findViewById(R.id.setting_sort_button);

		ImageButton setting_yuv2rgb_button = (ImageButton)this.findViewById(R.id.setting_yuv2rgb_button);
		ImageButton setting_minvideoqsize_button = (ImageButton)this.findViewById(R.id.setting_minvideoqsize_button);
		ImageButton setting_maxvideoqsize_button = (ImageButton)this.findViewById(R.id.setting_maxvideoqsize_button);
		ImageButton setting_maxaudioqsize_button = (ImageButton)this.findViewById(R.id.setting_maxaudioqsize_button);
		ImageButton setting_streamminvideoqsize_button = (ImageButton)this.findViewById(R.id.setting_streamminvideoqsize_button);
		ImageButton setting_streammaxvideoqsize_button = (ImageButton)this.findViewById(R.id.setting_streammaxvideoqsize_button);
		ImageButton setting_streammaxaudioqsize_button = (ImageButton)this.findViewById(R.id.setting_streammaxaudioqsize_button);
		ImageButton setting_avsyncmode_button = (ImageButton)this.findViewById(R.id.setting_avsyncmode_button);

		ImageButton setting_displaypixelformat_button = (ImageButton)this.findViewById(R.id.setting_displaypixelformat_button);
		ImageButton setting_swsscaler_button = (ImageButton)this.findViewById(R.id.setting_swsscaler_button);

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
		//drop down
		switch(advswsscaler_state){
		case 0:
			advswsscalerloopSelected.setText(advswsscaler[0]);
			break;
		case 1:
			advswsscalerloopSelected.setText(advswsscaler[1]);
			break;
		case 2:
			advswsscalerloopSelected.setText(advswsscaler[2]);
			break;
		}
		switch(advyuv2rgb_state){
		case 0:
			yuv2rgbloopSelected.setText(yuv2rgb[0]);
			break;
		case 1:
			yuv2rgbloopSelected.setText(yuv2rgb[1]);
			break;
		}

		//display pixel format
		switch(advpixelformat_state){
		case 0:
			pixelformatloopSelected.setText(pixelformat[0]);
			break;
		case 1:
			pixelformatloopSelected.setText(pixelformat[1]);
			break;
		}
		//pixelformat
		switch(advavsyncmode_state){
		case 0:
			advavsyncmodeloopSelected.setText(avsyncmode[0]);
			break;
		case 1:
			advavsyncmodeloopSelected.setText(avsyncmode[1]);
			break;
		case 2:
			advavsyncmodeloopSelected.setText(avsyncmode[2]);
			break;
		}

		//advminvideoq
		switch(advminvideoq_state){
		case 0:
			minvideoqloopSelected.setText(advminvideoq[0]);
			break;
		case 1:
			minvideoqloopSelected.setText(advminvideoq[1]);
			break;
		case 2:
			minvideoqloopSelected.setText(advminvideoq[2]);
			break;
		case 3:
			minvideoqloopSelected.setText(advminvideoq[3]);
			break;
		case 4:
			minvideoqloopSelected.setText(advminvideoq[4]);
			break;
		case 5:
			minvideoqloopSelected.setText(advminvideoq[5]);
			break;
		case 6:
			minvideoqloopSelected.setText(advminvideoq[6]);
			break;
		case 7:
			minvideoqloopSelected.setText(advminvideoq[7]);
			break;
		case 8:
			minvideoqloopSelected.setText(advminvideoq[8]);
			break;
		case 9:
			minvideoqloopSelected.setText(advminvideoq[9]);
			break;
		case 10:
			minvideoqloopSelected.setText(advminvideoq[10]);
			break;
		case 11:
			minvideoqloopSelected.setText(advminvideoq[11]);
			break;
		case 12:
			minvideoqloopSelected.setText(advminvideoq[12]);
			break;
		}
		
		//advmaxvideoq
		switch(advmaxvideoq_state){
		case 0:
			maxvideoqloopSelected.setText(advmaxvideoq[0]);
			break;
		case 1:
			maxvideoqloopSelected.setText(advmaxvideoq[1]);
			break;
		case 2:
			maxvideoqloopSelected.setText(advmaxvideoq[2]);
			break;
		case 3:
			maxvideoqloopSelected.setText(advmaxvideoq[3]);
			break;
		case 4:
			maxvideoqloopSelected.setText(advmaxvideoq[4]);
			break;
		case 5:
			maxvideoqloopSelected.setText(advmaxvideoq[5]);
			break;
		case 6:
			maxvideoqloopSelected.setText(advmaxvideoq[6]);
			break;
		case 7:
			maxvideoqloopSelected.setText(advmaxvideoq[7]);
			break;
		case 8:
			maxvideoqloopSelected.setText(advmaxvideoq[8]);
			break;
		case 9:
			maxvideoqloopSelected.setText(advmaxvideoq[9]);
			break;
		case 10:
			maxvideoqloopSelected.setText(advmaxvideoq[10]);
			break;
		case 11:
			maxvideoqloopSelected.setText(advmaxvideoq[11]);
			break;
		case 12:
			maxvideoqloopSelected.setText(advmaxvideoq[12]);
			break;
		case 13:
			maxvideoqloopSelected.setText(advmaxvideoq[13]);
			break;
		case 14:
			maxvideoqloopSelected.setText(advmaxvideoq[14]);
			break;
		case 15:
			maxvideoqloopSelected.setText(advmaxvideoq[15]);
			break;
		case 16:
			maxvideoqloopSelected.setText(advmaxvideoq[16]);
			break;
		case 17:
			maxvideoqloopSelected.setText(advmaxvideoq[17]);
			break;
		case 18:
			maxvideoqloopSelected.setText(advmaxvideoq[18]);
			break;
		case 19:
			maxvideoqloopSelected.setText(advmaxvideoq[19]);
			break;
		case 20:
			maxvideoqloopSelected.setText(advmaxvideoq[20]);
			break;
		case 21:
			maxvideoqloopSelected.setText(advmaxvideoq[21]);
			break;

		}
		//advmaxaudioq

		switch(advmaxaudioq_state){
		case 0:
			maxaudioqloopSelected.setText(advmaxaudioq[0]);
			break;
		case 1:
			maxaudioqloopSelected.setText(advmaxaudioq[1]);
			break;
		case 2:
			maxaudioqloopSelected.setText(advmaxaudioq[2]);
			break;
		case 3:
			maxaudioqloopSelected.setText(advmaxaudioq[3]);
			break;
		case 4:
			maxaudioqloopSelected.setText(advmaxaudioq[4]);
			break;
		case 5:
			maxaudioqloopSelected.setText(advmaxaudioq[5]);
			break;
		case 6:
			maxaudioqloopSelected.setText(advmaxaudioq[6]);
			break;
		case 7:
			maxaudioqloopSelected.setText(advmaxaudioq[7]);
			break;
		case 8:
			maxaudioqloopSelected.setText(advmaxaudioq[8]);
			break;
		case 9:
			maxaudioqloopSelected.setText(advmaxaudioq[9]);
			break;
		case 10:
			maxaudioqloopSelected.setText(advmaxaudioq[10]);
			break;
		case 11:
			maxaudioqloopSelected.setText(advmaxaudioq[11]);
			break;
		case 12:
			maxaudioqloopSelected.setText(advmaxaudioq[12]);
			break;
		case 13:
			maxaudioqloopSelected.setText(advmaxaudioq[13]);
			break;
		case 14:
			maxaudioqloopSelected.setText(advmaxaudioq[14]);
			break;
		case 15:
			maxaudioqloopSelected.setText(advmaxaudioq[15]);
			break;

		}
		//stream
		//advminvideoq
		switch(advstreamminvideoq_state){
		case 0:
			streamminvideoqloopSelected.setText(streamminvideoq[0]);
			break;
		case 1:
			streamminvideoqloopSelected.setText(streamminvideoq[1]);
			break;
		case 2:
			streamminvideoqloopSelected.setText(streamminvideoq[2]);
			break;
		case 3:
			streamminvideoqloopSelected.setText(streamminvideoq[3]);
			break;
		case 4:
			streamminvideoqloopSelected.setText(streamminvideoq[4]);
			break;
		case 5:
			streamminvideoqloopSelected.setText(streamminvideoq[5]);
			break;
		case 6:
			streamminvideoqloopSelected.setText(streamminvideoq[6]);
			break;
		case 7:
			streamminvideoqloopSelected.setText(streamminvideoq[7]);
			break;
		case 8:
			streamminvideoqloopSelected.setText(streamminvideoq[8]);
			break;
		case 9:
			streamminvideoqloopSelected.setText(streamminvideoq[9]);
			break;
		case 10:
			streamminvideoqloopSelected.setText(streamminvideoq[10]);
			break;
		case 11:
			streamminvideoqloopSelected.setText(streamminvideoq[11]);
			break;
		case 12:
			streamminvideoqloopSelected.setText(streamminvideoq[12]);
			break;


		}
		//advstreammaxvideoq
		switch(advstreammaxvideoq_state){
		case 0:
			streammaxvideoqloopSelected.setText(streammaxvideoq[0]);
			break;
		case 1:
			streammaxvideoqloopSelected.setText(streammaxvideoq[1]);
			break;
		case 2:
			streammaxvideoqloopSelected.setText(streammaxvideoq[2]);
			break;
		case 3:
			streammaxvideoqloopSelected.setText(streammaxvideoq[3]);
			break;
		case 4:
			streammaxvideoqloopSelected.setText(streammaxvideoq[4]);
			break;
		case 5:
			streammaxvideoqloopSelected.setText(streammaxvideoq[5]);
			break;
		case 6:
			streammaxvideoqloopSelected.setText(streammaxvideoq[6]);
			break;
		case 7:
			streammaxvideoqloopSelected.setText(streammaxvideoq[7]);
			break;
		case 8:
			streammaxvideoqloopSelected.setText(streammaxvideoq[8]);
			break;
		case 9:
			streammaxvideoqloopSelected.setText(streammaxvideoq[9]);
			break;
		case 10:
			streammaxvideoqloopSelected.setText(streammaxvideoq[10]);
			break;
		case 11:
			streammaxvideoqloopSelected.setText(streammaxvideoq[11]);
			break;
		case 12:
			streammaxvideoqloopSelected.setText(streammaxvideoq[12]);
			break;
		case 13:
			streammaxvideoqloopSelected.setText(streammaxvideoq[13]);
			break;
		case 14:
			streammaxvideoqloopSelected.setText(streammaxvideoq[14]);
			break;
		case 15:
			streammaxvideoqloopSelected.setText(streammaxvideoq[15]);
			break;
		case 16:
			streammaxvideoqloopSelected.setText(streammaxvideoq[16]);
			break;
		case 17:
			streammaxvideoqloopSelected.setText(streammaxvideoq[17]);
			break;
		case 18:
			streammaxvideoqloopSelected.setText(streammaxvideoq[18]);
			break;
		case 19:
			streammaxvideoqloopSelected.setText(streammaxvideoq[19]);
			break;
		case 20:
			streammaxvideoqloopSelected.setText(streammaxvideoq[20]);
			break;
		case 21:
			streammaxvideoqloopSelected.setText(streammaxvideoq[21]);
			break;

		}
		//advstreammaxaudioq

		switch(advstreammaxaudioq_state){
		case 0:
			streammaxaudioqloopSelected.setText(streammaxaudioq[0]);
			break;
		case 1:
			streammaxaudioqloopSelected.setText(streammaxaudioq[1]);
			break;
		case 2:
			streammaxaudioqloopSelected.setText(streammaxaudioq[2]);
			break;
		case 3:
			streammaxaudioqloopSelected.setText(streammaxaudioq[3]);
			break;
		case 4:
			streammaxaudioqloopSelected.setText(streammaxaudioq[4]);
			break;
		case 5:
			streammaxaudioqloopSelected.setText(streammaxaudioq[5]);
			break;
		case 6:
			streammaxaudioqloopSelected.setText(streammaxaudioq[6]);
			break;
		case 7:
			streammaxaudioqloopSelected.setText(streammaxaudioq[7]);
			break;
		case 8:
			streammaxaudioqloopSelected.setText(streammaxaudioq[8]);
			break;
		case 9:
			streammaxaudioqloopSelected.setText(streammaxaudioq[9]);
			break;
		case 10:
			streammaxaudioqloopSelected.setText(streammaxaudioq[10]);
			break;
		case 11:
			streammaxaudioqloopSelected.setText(streammaxaudioq[11]);
			break;
		case 12:
			streammaxaudioqloopSelected.setText(streammaxaudioq[12]);
			break;
		case 13:
			streammaxaudioqloopSelected.setText(streammaxaudioq[13]);
			break;
		case 14:
			streammaxaudioqloopSelected.setText(streammaxaudioq[14]);
			break;
		case 15:
			streammaxaudioqloopSelected.setText(streammaxaudioq[15]);
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

		//advanced

		tblrowadvancedskipframes.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View view) {
				if (skipframes_bx.isChecked()){
					skipframes_bx.setChecked(false);
				}else{
					skipframes_bx.setChecked(true);
				}
			}});		

		skipframes_bx.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
			@Override
			public void onCheckedChanged(CompoundButton arg0, boolean arg1) {

				advskipframes_state = skipframes_bx.isChecked();
				is.putExtra("ADVSKIPFRAMES", advskipframes_state);
				advskipframes_changed = true;
			}
		});

		//skip bidirectional frames
		tblrowadvancedskipbidirectionalframes.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View view) {
				if (skipbidirectionalframes_bx.isChecked()){
					skipbidirectionalframes_bx.setChecked(false);
				}else{
					skipbidirectionalframes_bx.setChecked(true);
				}
			}});		

		skipbidirectionalframes_bx.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
			@Override
			public void onCheckedChanged(CompoundButton arg0, boolean arg1) {
				advbidirectional_state = skipbidirectionalframes_bx.isChecked();
				is.putExtra("BIDIRECTIONAL", advbidirectional_state);
				advbidirectional_changed = true;
			}
		});

		//yuv2rgb
		tblrowadvancedffmpeg.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View view) {
				if (advffmpeg_bx.isChecked()){
					advffmpeg_bx.setChecked(false);
				}else{
					advffmpeg_bx.setChecked(true);
				}
			}});		

		advffmpeg_bx.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
			@Override
			public void onCheckedChanged(CompoundButton arg0, boolean arg1) {
				advffmpeg_state = advffmpeg_bx.isChecked();
				is.putExtra("ADVFFMPEG", advffmpeg_state);
				advffmpeg_changed = true;
			}
		});

		tblrowadvanceddebugmode.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View view) {
				if (advdebug_bx.isChecked()){
					advdebug_bx.setChecked(false);
				}else{
					advdebug_bx.setChecked(true);
				}
			}});		

		advdebug_bx.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
			@Override
			public void onCheckedChanged(CompoundButton arg0, boolean arg1) {
				advdebug_state = advdebug_bx.isChecked();
				is.putExtra("ADVDEBUG", advdebug_state);
				advdebug_changed = true;
			}
		});
	
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
		//dropdown	
		OnClickListener loopswsscalerOnClickListener =new OnClickListener()
		{
			@Override
			public void onClick(View view) {
				AlertDialog.Builder builder = new AlertDialog.Builder(Settings.this);

				builder.setTitle(R.string.swsscaler);
				builder.setItems(R.array.advswsscaler, new DialogInterface.OnClickListener() {					
					@Override
					public void onClick(DialogInterface dialog, int index) {
						switch(index) {
						case 0:
							advswsscaler_state = 0;
							advswsscaler_changed = true;
							is.putExtra("ADVSWSSCALER", advswsscaler_state);
							advswsscalerloopSelected.setText(advswsscaler[0]);
							break;

						case 1:
							advswsscaler_state = 1;
							advswsscaler_changed = true;
							is.putExtra("ADVSWSSCALER", advswsscaler_state);
							advswsscalerloopSelected.setText(advswsscaler[1]);
							break;
						case 2:
							advswsscaler_state = 2;
							advswsscaler_changed = true;
							is.putExtra("ADVSWSSCALER", advswsscaler_state);
							advswsscalerloopSelected.setText(advswsscaler[2]);
							break;

						}
					}
				});

				builder.create().show();
			}
		}; 
		tblrowadvancedadvswsscaler.setOnClickListener(loopswsscalerOnClickListener);
		setting_swsscaler_button.setOnClickListener(loopswsscalerOnClickListener);

		OnClickListener loopyuv2rgbOnClickListener =new OnClickListener()
		{
			@Override
			public void onClick(View view) {
				AlertDialog.Builder builder = new AlertDialog.Builder(Settings.this);

				builder.setTitle(R.string.advyuv2rgb);
				builder.setItems(R.array.advyuv2rgb, new DialogInterface.OnClickListener() {					
					@Override
					public void onClick(DialogInterface dialog, int index) {
						switch(index) {
						case 0:
							advyuv2rgb_state = 0;
							advyuv2rgb_changed = true;
							is.putExtra("ADVYUV2RGB", advyuv2rgb_state);
							yuv2rgbloopSelected.setText(yuv2rgb[0]);
							break;

						case 1:
							advyuv2rgb_state = 1;
							advyuv2rgb_changed = true;
							is.putExtra("ADVYUV2RGB", advyuv2rgb_state);
							yuv2rgbloopSelected.setText(yuv2rgb[1]);
							break;


						}
					}
				});

				builder.create().show();
			}
		}; 
		tblrowadvancedyuv2rgb.setOnClickListener(loopyuv2rgbOnClickListener);
		setting_yuv2rgb_button.setOnClickListener(loopyuv2rgbOnClickListener);

		OnClickListener looppixelformatOnClickListener =new OnClickListener()
		{
			@Override
			public void onClick(View view) {
				AlertDialog.Builder builder = new AlertDialog.Builder(Settings.this);

				builder.setTitle(R.string.advdisplaypixelformat);
				builder.setItems(R.array.advpixelformat, new DialogInterface.OnClickListener() {					
					@Override
					public void onClick(DialogInterface dialog, int index) {
						switch(index) {
						case 0:
							advpixelformat_state = 0;
							advpixelformat_changed = true;
							is.putExtra("ADVPIXELFORMAT", advpixelformat_state);
							pixelformatloopSelected.setText(pixelformat[0]);
							break;

						case 1:
							advpixelformat_state = 1;
							advpixelformat_changed = true;
							is.putExtra("ADVPIXELFORMAT", advpixelformat_state);
							pixelformatloopSelected.setText(pixelformat[1]);
							break;


						}
					}
				});

				builder.create().show();
			}
		}; 
		tblrowadvancedpixelformat.setOnClickListener(looppixelformatOnClickListener);
		setting_displaypixelformat_button.setOnClickListener(looppixelformatOnClickListener);

		OnClickListener loopavsyncmodeOnClickListener =new OnClickListener()
		{
			@Override
			public void onClick(View view) {
				AlertDialog.Builder builder = new AlertDialog.Builder(Settings.this);

				builder.setTitle(R.string.avsyncmode);
				builder.setItems(R.array.advavsyncmode, new DialogInterface.OnClickListener() {					
					@Override
					public void onClick(DialogInterface dialog, int index) {
						switch(index) {
						case 0:
							advavsyncmode_state = 0;
							advavsyncmode_changed = true;
							is.putExtra("ADVAVSYNCMODE", advavsyncmode_state);
							advavsyncmodeloopSelected.setText(avsyncmode[0]);
							break;

						case 1:
							advavsyncmode_state = 1;
							advavsyncmode_changed = true;
							is.putExtra("ADVAVSYNCMODE", advavsyncmode_state);
							advavsyncmodeloopSelected.setText(avsyncmode[1]);
							break;
						case 2:
							advavsyncmode_state = 2;
							advavsyncmode_changed = true;
							is.putExtra("ADVAVSYNCMODE", advavsyncmode_state);
							advavsyncmodeloopSelected.setText(avsyncmode[2]);
							break;


						}
					}
				});

				builder.create().show();
			}
		}; 
		tblrowadvancedavsyncmode.setOnClickListener(loopavsyncmodeOnClickListener);
		setting_avsyncmode_button.setOnClickListener(loopavsyncmodeOnClickListener);

		OnClickListener loopadvminvideoqOnClickListener =new OnClickListener()
		{
			@Override
			public void onClick(View view) {
				AlertDialog.Builder builder = new AlertDialog.Builder(Settings.this);

				builder.setTitle(R.string.advminvideoq);
				builder.setItems(R.array.advminvideoq, new DialogInterface.OnClickListener() {					
					@Override
					public void onClick(DialogInterface dialog, int index) {
						switch(index) {
						case 0:
							advminvideoq_state = 0;
							advminvideoq_changed = true;
							is.putExtra("ADVMINVIDEOQ", advminvideoq_state);
							minvideoqloopSelected.setText(advminvideoq[0]);
							break;

						case 1:
							advminvideoq_state = 1;
							advminvideoq_changed = true;
							is.putExtra("ADVMINVIDEOQ", advminvideoq_state);
							minvideoqloopSelected.setText(advminvideoq[1]);
							break;
						case 2:
							advminvideoq_state = 2;
							advminvideoq_changed = true;
							is.putExtra("ADVMINVIDEOQ", advminvideoq_state);
							minvideoqloopSelected.setText(advminvideoq[2]);
							break;

						case 3:
							advminvideoq_state = 3;
							advminvideoq_changed = true;
							is.putExtra("ADVMINVIDEOQ", advminvideoq_state);
							minvideoqloopSelected.setText(advminvideoq[3]);
							break;
						case 4:
							advminvideoq_state = 4;
							advminvideoq_changed = true;
							is.putExtra("ADVMINVIDEOQ", advminvideoq_state);
							minvideoqloopSelected.setText(advminvideoq[4]);
							break;

						case 5:
							advminvideoq_state = 5;
							advminvideoq_changed = true;
							is.putExtra("ADVMINVIDEOQ", advminvideoq_state);
							minvideoqloopSelected.setText(advminvideoq[5]);
							break;
						case 6:
							advminvideoq_state = 6;
							advminvideoq_changed = true;
							is.putExtra("ADVMINVIDEOQ", advminvideoq_state);
							minvideoqloopSelected.setText(advminvideoq[6]);
							break;

						case 7:
							advminvideoq_state = 7;
							advminvideoq_changed = true;
							is.putExtra("ADVMINVIDEOQ", advminvideoq_state);
							minvideoqloopSelected.setText(advminvideoq[7]);
							break;
						case 8:
							advminvideoq_state = 8;
							advminvideoq_changed = true;
							is.putExtra("ADVMINVIDEOQ", advminvideoq_state);
							minvideoqloopSelected.setText(advminvideoq[8]);
							break;

						case 9:
							advminvideoq_state = 9;
							advminvideoq_changed = true;
							is.putExtra("ADVMINVIDEOQ", advminvideoq_state);
							minvideoqloopSelected.setText(advminvideoq[9]);
							break;
						case 10:
							advminvideoq_state = 10;
							advminvideoq_changed = true;
							is.putExtra("ADVMINVIDEOQ", advminvideoq_state);
							minvideoqloopSelected.setText(advminvideoq[10]);
							break;

						case 11:
							advminvideoq_state = 11;
							advminvideoq_changed = true;
							is.putExtra("ADVMINVIDEOQ", advminvideoq_state);
							minvideoqloopSelected.setText(advminvideoq[11]);
							break;
						case 12:
							advminvideoq_state = 12;
							advminvideoq_changed = true;
							is.putExtra("ADVMINVIDEOQ", advminvideoq_state);
							minvideoqloopSelected.setText(advminvideoq[12]);
							break;


						}
					}
				});

				builder.create().show();
			}
		}; 
		tblrowadvancedminvideoqsize.setOnClickListener(loopadvminvideoqOnClickListener);
		setting_minvideoqsize_button.setOnClickListener(loopadvminvideoqOnClickListener);

		//advmaxvideoq
		OnClickListener loopadvmaxvideoqOnClickListener =new OnClickListener()
		{
			@Override
			public void onClick(View view) {
				AlertDialog.Builder builder = new AlertDialog.Builder(Settings.this);

				builder.setTitle(R.string.advmaxvideoq);
				builder.setItems(R.array.advmaxvideoq, new DialogInterface.OnClickListener() {					
					@Override
					public void onClick(DialogInterface dialog, int index) {
						switch(index) {
						case 0:
							advmaxvideoq_state = 0;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[0]);
							break;

						case 1:
							advmaxvideoq_state = 1;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[1]);
							break;
						case 2:
							advmaxvideoq_state = 2;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[2]);
							break;

						case 3:
							advmaxvideoq_state = 3;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[3]);
							break;
						case 4:
							advmaxvideoq_state = 4;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[4]);
							break;

						case 5:
							advmaxvideoq_state = 5;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[5]);
							break;
						case 6:
							advmaxvideoq_state = 6;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[6]);
							break;

						case 7:
							advmaxvideoq_state = 7;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[7]);
							break;
						case 8:
							advmaxvideoq_state = 8;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[8]);
							break;

						case 9:
							advmaxvideoq_state = 9;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[9]);
							break;
						case 10:
							advmaxvideoq_state = 10;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[10]);
							break;

						case 11:
							advmaxvideoq_state = 11;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[11]);
							break;
						case 12:
							advmaxvideoq_state = 12;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[12]);
							break;

						case 13:
							advmaxvideoq_state = 13;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[13]);
							break;
						case 14:
							advmaxvideoq_state = 14;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[14]);
							break;

						case 15:
							advmaxvideoq_state = 15;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[15]);
							break;
						case 16:
							advmaxvideoq_state = 16;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[16]);
							break;

						case 17:
							advmaxvideoq_state = 17;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[17]);
							break;
						case 18:
							advmaxvideoq_state = 18;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[18]);
							break;

						case 19:
							advmaxvideoq_state = 19;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[19]);
							break;
						case 20:
							advmaxvideoq_state = 20;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[20]);
							break;

						case 21:
							advmaxvideoq_state = 21;
							advmaxvideoq_changed = true;
							is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
							maxvideoqloopSelected.setText(advmaxvideoq[21]);
							break;

						}
					}
				});

				builder.create().show();
			}
		}; 
		tblrowadvancedmaxvideoqsize.setOnClickListener(loopadvmaxvideoqOnClickListener);
		setting_maxvideoqsize_button.setOnClickListener(loopadvmaxvideoqOnClickListener);
		//advmaxaudioq
		OnClickListener loopadvmaxaudioqOnClickListener =new OnClickListener()
		{
			@Override
			public void onClick(View view) {
				AlertDialog.Builder builder = new AlertDialog.Builder(Settings.this);

				builder.setTitle(R.string.advmaxaudioq);
				builder.setItems(R.array.advmaxaudioq, new DialogInterface.OnClickListener() {					
					@Override
					public void onClick(DialogInterface dialog, int index) {
						switch(index) {
						case 0:
							advmaxaudioq_state = 0;
							advmaxaudioq_changed = true;
							is.putExtra("ADVMAXAUDIOQ", advmaxaudioq_state);
							maxaudioqloopSelected.setText(advmaxaudioq[0]);
							break;

						case 1:
							advmaxaudioq_state = 1;
							advmaxaudioq_changed = true;
							is.putExtra("ADVMAXAUDIOQ", advmaxaudioq_state);
							maxaudioqloopSelected.setText(advmaxaudioq[1]);
							break;
						case 2:
							advmaxaudioq_state = 2;
							advmaxaudioq_changed = true;
							is.putExtra("ADVMAXAUDIOQ", advmaxaudioq_state);
							maxaudioqloopSelected.setText(advmaxaudioq[2]);
							break;

						case 3:
							advmaxaudioq_state = 3;
							advmaxaudioq_changed = true;
							is.putExtra("ADVMAXAUDIOQ", advmaxaudioq_state);
							maxaudioqloopSelected.setText(advmaxaudioq[3]);
							break;
						case 4:
							advmaxaudioq_state = 4;
							advmaxaudioq_changed = true;
							is.putExtra("ADVMAXAUDIOQ", advmaxaudioq_state);
							maxaudioqloopSelected.setText(advmaxaudioq[4]);
							break;

						case 5:
							advmaxaudioq_state = 5;
							advmaxaudioq_changed = true;
							is.putExtra("ADVMAXAUDIOQ", advmaxaudioq_state);
							maxaudioqloopSelected.setText(advmaxaudioq[5]);
							break;
						case 6:
							advmaxaudioq_state = 6;
							advmaxaudioq_changed = true;
							is.putExtra("ADVMAXAUDIOQ", advmaxaudioq_state);
							maxaudioqloopSelected.setText(advmaxaudioq[6]);
							break;

						case 7:
							advmaxaudioq_state = 7;
							advmaxaudioq_changed = true;
							is.putExtra("ADVMAXAUDIOQ", advmaxaudioq_state);
							maxaudioqloopSelected.setText(advmaxaudioq[7]);
							break;
						case 8:
							advmaxaudioq_state = 8;
							advmaxaudioq_changed = true;
							is.putExtra("ADVMAXAUDIOQ", advmaxaudioq_state);
							maxaudioqloopSelected.setText(advmaxaudioq[8]);
							break;

						case 9:
							advmaxaudioq_state = 9;
							advmaxaudioq_changed = true;
							is.putExtra("ADVMAXAUDIOQ", advmaxaudioq_state);
							maxaudioqloopSelected.setText(advmaxaudioq[9]);
							break;
						case 10:
							advmaxaudioq_state = 10;
							advmaxaudioq_changed = true;
							is.putExtra("ADVMAXAUDIOQ", advmaxaudioq_state);
							maxaudioqloopSelected.setText(advmaxaudioq[10]);
							break;

						case 11:
							advmaxaudioq_state = 11;
							advmaxaudioq_changed = true;
							is.putExtra("ADVMAXAUDIOQ", advmaxaudioq_state);
							maxaudioqloopSelected.setText(advmaxaudioq[11]);
							break;
						case 12:
							advmaxaudioq_state = 12;
							advmaxaudioq_changed = true;
							is.putExtra("ADVMAXAUDIOQ", advmaxaudioq_state);
							maxaudioqloopSelected.setText(advmaxaudioq[12]);
							break;

						case 13:
							advmaxaudioq_state = 13;
							advmaxaudioq_changed = true;
							is.putExtra("ADVMAXAUDIOQ", advmaxaudioq_state);
							maxaudioqloopSelected.setText(advmaxaudioq[13]);
							break;
						case 14:
							advmaxaudioq_state = 14;
							advmaxaudioq_changed = true;
							is.putExtra("ADVMAXAUDIOQ", advmaxaudioq_state);
							maxaudioqloopSelected.setText(advmaxaudioq[14]);
							break;

						case 15:
							advmaxaudioq_state = 15;
							advmaxaudioq_changed = true;
							is.putExtra("ADVMAXAUDIOQ", advmaxaudioq_state);
							maxaudioqloopSelected.setText(advmaxaudioq[15]);
							break;


						}
					}
				});

				builder.create().show();
			}
		}; 
		tblrowadvancedmaxaudioqsize.setOnClickListener(loopadvmaxaudioqOnClickListener);
		setting_maxaudioqsize_button.setOnClickListener(loopadvmaxaudioqOnClickListener);
		//stream
		OnClickListener loopadvstreamminvideoqOnClickListener =new OnClickListener()
		{
			@Override
			public void onClick(View view) {
				AlertDialog.Builder builder = new AlertDialog.Builder(Settings.this);

				builder.setTitle(R.string.streamminvideoq);
				builder.setItems(R.array.advstreamminvideoq, new DialogInterface.OnClickListener() {					
					@Override
					public void onClick(DialogInterface dialog, int index) {
						switch(index) {
						case 0:
							advstreamminvideoq_state = 0;
							advstreamminvideoq_changed = true;
							is.putExtra("ADVSTREAMMINVIDEOQ", advstreamminvideoq_state);
							streamminvideoqloopSelected.setText(streamminvideoq[0]);
							break;

						case 1:
							advstreamminvideoq_state = 1;
							advstreamminvideoq_changed = true;
							is.putExtra("ADVSTREAMMINVIDEOQ", advstreamminvideoq_state);
							streamminvideoqloopSelected.setText(streamminvideoq[1]);
							break;
						case 2:
							advstreamminvideoq_state = 2;
							advstreamminvideoq_changed = true;
							is.putExtra("ADVSTREAMMINVIDEOQ", advstreamminvideoq_state);
							streamminvideoqloopSelected.setText(streamminvideoq[2]);
							break;

						case 3:
							advstreamminvideoq_state = 3;
							advstreamminvideoq_changed = true;
							is.putExtra("ADVSTREAMMINVIDEOQ", advstreamminvideoq_state);
							streamminvideoqloopSelected.setText(streamminvideoq[3]);
							break;
						case 4:
							advstreamminvideoq_state = 4;
							advstreamminvideoq_changed = true;
							is.putExtra("ADVSTREAMMINVIDEOQ", advstreamminvideoq_state);
							streamminvideoqloopSelected.setText(streamminvideoq[4]);
							break;

						case 5:
							advstreamminvideoq_state = 5;
							advstreamminvideoq_changed = true;
							is.putExtra("ADVSTREAMMINVIDEOQ", advstreamminvideoq_state);
							streamminvideoqloopSelected.setText(streamminvideoq[5]);
							break;
						case 6:
							advstreamminvideoq_state = 6;
							advstreamminvideoq_changed = true;
							is.putExtra("ADVSTREAMMINVIDEOQ", advstreamminvideoq_state);
							streamminvideoqloopSelected.setText(streamminvideoq[6]);
							break;

						case 7:
							advstreamminvideoq_state = 7;
							advstreamminvideoq_changed = true;
							is.putExtra("ADVSTREAMMINVIDEOQ", advstreamminvideoq_state);
							streamminvideoqloopSelected.setText(streamminvideoq[7]);
							break;
						case 8:
							advstreamminvideoq_state = 8;
							advstreamminvideoq_changed = true;
							is.putExtra("ADVSTREAMMINVIDEOQ", advstreamminvideoq_state);
							streamminvideoqloopSelected.setText(streamminvideoq[8]);
							break;

						case 9:
							advstreamminvideoq_state = 9;
							advstreamminvideoq_changed = true;
							is.putExtra("ADVSTREAMMINVIDEOQ", advstreamminvideoq_state);
							streamminvideoqloopSelected.setText(streamminvideoq[9]);
							break;
						case 10:
							advstreamminvideoq_state = 10;
							advstreamminvideoq_changed = true;
							is.putExtra("ADVSTREAMMINVIDEOQ", advstreamminvideoq_state);
							streamminvideoqloopSelected.setText(streamminvideoq[10]);
							break;

						case 11:
							advstreamminvideoq_state = 11;
							advstreamminvideoq_changed = true;
							is.putExtra("ADVSTREAMMINVIDEOQ", advstreamminvideoq_state);
							streamminvideoqloopSelected.setText(streamminvideoq[11]);
							break;
						case 12:
							advstreamminvideoq_state = 12;
							advstreamminvideoq_changed = true;
							is.putExtra("ADVSTREAMMINVIDEOQ", advstreamminvideoq_state);
							streamminvideoqloopSelected.setText(streamminvideoq[12]);
							break;

						}
					}
				});

				builder.create().show();
			}
		}; 
		tblrowadvancedstreamminvideoqsize.setOnClickListener(loopadvstreamminvideoqOnClickListener);
		setting_streamminvideoqsize_button.setOnClickListener(loopadvstreamminvideoqOnClickListener);

		//advmaxvideoq
		OnClickListener loopadvstreammaxvideoqOnClickListener =new OnClickListener()
		{
			@Override
			public void onClick(View view) {
				AlertDialog.Builder builder = new AlertDialog.Builder(Settings.this);

				builder.setTitle(R.string.streammaxvideoq);
				builder.setItems(R.array.advmaxvideoq, new DialogInterface.OnClickListener() {					
					@Override
					public void onClick(DialogInterface dialog, int index) {
						switch(index) {
						case 0:
							advstreammaxvideoq_state = 0;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[0]);
							break;

						case 1:
							advstreammaxvideoq_state = 1;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[1]);
							break;
						case 2:
							advstreammaxvideoq_state = 2;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[2]);
							break;

						case 3:
							advstreammaxvideoq_state = 3;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[3]);
							break;
						case 4:
							advstreammaxvideoq_state = 4;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[4]);
							break;

						case 5:
							advstreammaxvideoq_state = 5;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[5]);
							break;
						case 6:
							advstreammaxvideoq_state = 6;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[6]);
							break;

						case 7:
							advstreammaxvideoq_state = 7;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[7]);
							break;
						case 8:
							advstreammaxvideoq_state = 8;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[8]);
							break;

						case 9:
							advstreammaxvideoq_state = 9;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[9]);
							break;
						case 10:
							advstreammaxvideoq_state = 10;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[10]);
							break;

						case 11:
							advstreammaxvideoq_state = 11;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[11]);
							break;
						case 12:
							advstreammaxvideoq_state = 12;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[12]);
							break;

						case 13:
							advstreammaxvideoq_state = 13;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[13]);
							break;
						case 14:
							advstreammaxvideoq_state = 14;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[14]);
							break;

						case 15:
							advstreammaxvideoq_state = 15;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[15]);
							break;
						case 16:
							advstreammaxvideoq_state = 16;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[16]);
							break;

						case 17:
							advstreammaxvideoq_state = 17;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[17]);
							break;
						case 18:
							advstreammaxvideoq_state = 18;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[18]);
							break;

						case 19:
							advstreammaxvideoq_state = 19;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[19]);
							break;
						case 20:
							advstreammaxvideoq_state = 20;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[20]);
							break;

						case 21:
							advstreammaxvideoq_state = 21;
							advstreammaxvideoq_changed = true;
							is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
							streammaxvideoqloopSelected.setText(streammaxvideoq[21]);
							break;

						}
					}
				});

				builder.create().show();
			}
		}; 
		tblrowadvancedstreammaxvideoqsize.setOnClickListener(loopadvstreammaxvideoqOnClickListener);
		setting_streammaxvideoqsize_button.setOnClickListener(loopadvstreammaxvideoqOnClickListener);
		//advmaxaudioq
		OnClickListener loopadvstreammaxaudioqOnClickListener =new OnClickListener()
		{
			@Override
			public void onClick(View view) {
				AlertDialog.Builder builder = new AlertDialog.Builder(Settings.this);

				builder.setTitle(R.string.streammaxaudioq);
				builder.setItems(R.array.advstreammaxaudioq, new DialogInterface.OnClickListener() {					
					@Override
					public void onClick(DialogInterface dialog, int index) {
						switch(index) {
						case 0:
							advstreammaxaudioq_state = 0;
							advstreammaxaudioq_changed = true;
							is.putExtra("ADVSTREAMMAXAUDIOQ", advstreammaxaudioq_state);
							streammaxaudioqloopSelected.setText(advmaxaudioq[0]);
							break;

						case 1:
							advstreammaxaudioq_state = 1;
							advstreammaxaudioq_changed = true;
							is.putExtra("ADVSTREAMMAXAUDIOQ", advstreammaxaudioq_state);
							streammaxaudioqloopSelected.setText(advmaxaudioq[1]);
							break;
						case 2:
							advstreammaxaudioq_state = 2;
							advstreammaxaudioq_changed = true;
							is.putExtra("ADVSTREAMMAXAUDIOQ", advstreammaxaudioq_state);
							streammaxaudioqloopSelected.setText(advmaxaudioq[2]);
							break;

						case 3:
							advstreammaxaudioq_state = 3;
							advstreammaxaudioq_changed = true;
							is.putExtra("ADVSTREAMMAXAUDIOQ", advstreammaxaudioq_state);
							streammaxaudioqloopSelected.setText(advmaxaudioq[3]);
							break;
						case 4:
							advstreammaxaudioq_state = 4;
							advstreammaxaudioq_changed = true;
							is.putExtra("ADVSTREAMMAXAUDIOQ", advstreammaxaudioq_state);
							streammaxaudioqloopSelected.setText(advmaxaudioq[4]);
							break;

						case 5:
							advstreammaxaudioq_state = 5;
							advstreammaxaudioq_changed = true;
							is.putExtra("ADVSTREAMMAXAUDIOQ", advstreammaxaudioq_state);
							streammaxaudioqloopSelected.setText(advmaxaudioq[5]);
							break;
						case 6:
							advstreammaxaudioq_state = 6;
							advstreammaxaudioq_changed = true;
							is.putExtra("ADVSTREAMMAXAUDIOQ", advstreammaxaudioq_state);
							streammaxaudioqloopSelected.setText(advmaxaudioq[6]);
							break;

						case 7:
							advstreammaxaudioq_state = 7;
							advstreammaxaudioq_changed = true;
							is.putExtra("ADVSTREAMMAXAUDIOQ", advstreammaxaudioq_state);
							streammaxaudioqloopSelected.setText(advmaxaudioq[7]);
							break;
						case 8:
							advstreammaxaudioq_state = 8;
							advstreammaxaudioq_changed = true;
							is.putExtra("ADVSTREAMMAXAUDIOQ", advstreammaxaudioq_state);
							streammaxaudioqloopSelected.setText(advmaxaudioq[8]);
							break;

						case 9:
							advstreammaxaudioq_state = 9;
							advstreammaxaudioq_changed = true;
							is.putExtra("ADVSTREAMMAXAUDIOQ", advstreammaxaudioq_state);
							streammaxaudioqloopSelected.setText(advmaxaudioq[9]);
							break;
						case 10:
							advstreammaxaudioq_state = 10;
							advstreammaxaudioq_changed = true;
							is.putExtra("ADVSTREAMMAXAUDIOQ", advstreammaxaudioq_state);
							streammaxaudioqloopSelected.setText(advmaxaudioq[10]);
							break;

						case 11:
							advstreammaxaudioq_state = 11;
							advstreammaxaudioq_changed = true;
							is.putExtra("ADVSTREAMMAXAUDIOQ", advstreammaxaudioq_state);
							streammaxaudioqloopSelected.setText(advmaxaudioq[11]);
							break;
						case 12:
							advstreammaxaudioq_state = 12;
							advstreammaxaudioq_changed = true;
							is.putExtra("ADVMAXSTREAMAUDIOQ", advstreammaxaudioq_state);
							streammaxaudioqloopSelected.setText(advmaxaudioq[12]);
							break;

						case 13:
							advstreammaxaudioq_state = 13;
							advstreammaxaudioq_changed = true;
							is.putExtra("ADVSTREAMMAXAUDIOQ", advstreammaxaudioq_state);
							streammaxaudioqloopSelected.setText(advmaxaudioq[13]);
							break;
						case 14:
							advstreammaxaudioq_state = 14;
							advstreammaxaudioq_changed = true;
							is.putExtra("ADVSTREAMMAXAUDIOQ", advstreammaxaudioq_state);
							streammaxaudioqloopSelected.setText(advmaxaudioq[14]);
							break;

						case 15:
							advstreammaxaudioq_state = 15;
							advstreammaxaudioq_changed = true;
							is.putExtra("ADVSTREAMMAXAUDIOQ", advstreammaxaudioq_state);
							streammaxaudioqloopSelected.setText(advmaxaudioq[15]);
							break;

						}
					}
				});

				builder.create().show();
			}
		}; 
		tblrowadvancedstreammaxaudioqsize.setOnClickListener(loopadvstreammaxaudioqOnClickListener);
		setting_streammaxaudioqsize_button.setOnClickListener(loopadvstreammaxaudioqOnClickListener);	
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
		
		//advanced
		if(!advskipframes_changed){
			is.putExtra("ADVSKIPFRAMES", advskipframes_state);
		}

		if(!advbidirectional_changed){
			is.putExtra("BIDIRECTIONAL", advbidirectional_state);
		}
		if(!advffmpeg_changed){
			is.putExtra("ADVFFMPEG", advffmpeg_state);
		}
		if(!advyuv2rgb_changed){
			is.putExtra("ADVYUV2RGB", advyuv2rgb_state);
		}
		if(!advminvideoq_changed){
			is.putExtra("ADVMINVIDEOQ", advminvideoq_state);
		}
		if(!advmaxvideoq_changed){
			is.putExtra("ADVMAXVIDEOQ", advmaxvideoq_state);
		}
		if(!advmaxaudioq_changed){
			is.putExtra("ADVMAXAUDIOQ", advmaxaudioq_state);
		}
		if(!advstreamminvideoq_changed){
			is.putExtra("ADVSTREAMMINVIDEOQ", advstreamminvideoq_state);
		}
		if(!advstreammaxvideoq_changed){
			is.putExtra("ADVSTREAMMAXVIDEOQ", advstreammaxvideoq_state);
		}
		if(!advstreammaxaudioq_changed){
			is.putExtra("ADVSTREAMMAXAUDIOQ", advstreammaxaudioq_state);
		}
		if(!advpixelformat_changed){
			is.putExtra("ADVPIXELFORMAT", advpixelformat_state);
		}
		if(!advavsyncmode_changed){
			is.putExtra("ADVAVSYNCMODE", advavsyncmode_state);
		}
		if(!advdebug_changed){
			is.putExtra("ADVDEBUG", advdebug_state);
		}
		if(!advswsscaler_changed) {
			is.putExtra("ADVSWSSCALER", advswsscaler_state);
		}
		setResult(RESULT_CANCELED, is);
	}
}
