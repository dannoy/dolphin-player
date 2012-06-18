/*
    Open Manager, an open source file manager for the Android system
    Copyright (C) 2009, 2010, 2011  Joe Berria <nexesdevelopment@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

package com.broov.playerx86;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;


import android.os.AsyncTask;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Color;
import android.view.View.OnClickListener;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

public class EventHandler implements OnClickListener {
	/*
	 * Unique types to control which file operation gets
	 * performed in the background
	 */
	private static final int SEARCH_TYPE =		0x00;

	private final Context 		context;
	private final FileManager 	file_mg;
	private TableRow 			delegate;
	private int 				color = Color.WHITE;

	//the list used to feed info into the array adapter and when multi-select is on
	private ArrayList<String> data_source, multiselect_data;
	private TextView path_label;

	/**
	 * Creates an EventHandler object. This object is used to communicate
	 * most work from the Main activity to the FileManager class.
	 * 
	 * @param context	The context of the main activity e.g  Main
	 * @param manager	The FileManager object that was instantiated from Main
	 */
	public EventHandler(Context context, final FileManager manager) {
		this.context = context;
		file_mg 	 = manager;
		data_source  = new ArrayList<String>(file_mg.getLastOpenedDir());
	}

	/**
	 * This method is called from the Main activity and this has the same
	 * reference to the same object so when changes are made here or there
	 * they will display in the same way.
	 * 
	 * @param adapter	The TableRow object
	 */
	public void setListAdapter(TableRow adapter) {
		delegate = adapter;
	}

	/**
	 * This method is called from the Main activity and is passed
	 * the TextView that should be updated as the directory changes
	 * so the user knows which folder they are in.
	 * 
	 * @param path	The label to update as the directory changes
	 * @param label	the label to update information
	 */
	public void setUpdateLabel(TextView path, TextView label) {
		path_label = path;
	}

	/**
	 * 
	 * @param color
	 */
	public void setTextColor(int color) {
		this.color = color;
	}

	/**
	 * Will search for a file then display all files with the 
	 * search parameter in its name
	 * 
	 * @param name	the name to search for
	 */
	public void searchForFile(String name) {
		new BackgroundWork(SEARCH_TYPE).execute(name);
	}

	/**
	 *  This method, handles the button presses of the top buttons found
	 *  in the Main activity. 
	 */
	@Override
	public void onClick(View v) {

		switch(v.getId()) {

		case R.id.back_button:
			String currentDir = file_mg.getCurrentDir();
			if (currentDir != "/") {
				updateDirectory(file_mg.getPreviousDir());
				if(path_label != null)
					path_label.setText(currentDir);
			}
		}
	}

	/**
	 * will return the data in the ArrayList that holds the dir contents. 
	 * 
	 * @param position	the indext of the arraylist holding the dir content
	 * @return the data in the arraylist at position (position)
	 */
	public String getData(int position) {

		if(position > data_source.size() - 1 || position < 0)
			return null;

		return data_source.get(position);
	}

	/**
	 * called to update the file contents as the user navigates there
	 * phones file system. 
	 * 
	 * @param content	an ArrayList of the file/folders in the current directory.
	 */
	public void updateDirectory(ArrayList<String> content) {	
		if(!data_source.isEmpty())
			data_source.clear();

		for(String data : content)
			data_source.add(data);

		delegate.notifyDataSetChanged();
	}

	private static class ViewHolder {
		TextView topView;
		TextView bottomView;
		ImageView icon;
		ImageView mSelect;	//multi-select check mark icon
	}

	/**
	 * A nested class to handle displaying a custom view in the ListView that
	 * is used in the Main activity. If any icons are to be added, they must
	 * be implemented in the getView method. This class is instantiated once in Main
	 * and has no reason to be instantiated again. 
	 * 
	 * @author Joe Berria
	 */
	public class TableRow extends ArrayAdapter<String> {
		private final int KB = 1024;
		private final int MB = KB * KB;
		private final int GB = MB * KB;
		private String display_size;
		private ArrayList<Integer> positions;

		public TableRow() {
			super(context, R.layout.tablerow, data_source);
		}

		public void addMultiPosition(int index, String path) {
			if(positions == null)
				positions = new ArrayList<Integer>();

			if(multiselect_data == null) {
				positions.add(index);
				add_multiSelect_file(path);

			} else if(multiselect_data.contains(path)) {
				if(positions.contains(index))
					positions.remove(new Integer(index));

				multiselect_data.remove(path);

			} else {
				positions.add(index);
				add_multiSelect_file(path);
			}

			notifyDataSetChanged();
		}

		public String getFilePermissions(File file) {
			String per = "-";

			if(file.isDirectory())
				per += "d";
			if(file.canRead())
				per += "r";
			if(file.canWrite())
				per += "w";

			return per;
		}


		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			ViewHolder holder;
			int num_items = 0;
			String temp = file_mg.getCurrentDir();
			File file = new File(temp + "/" + data_source.get(position));
			String[] list = file.list();

			if(list != null)
				num_items = list.length;

			if(convertView == null) {
				LayoutInflater inflater = (LayoutInflater) context.
				getSystemService(Context.LAYOUT_INFLATER_SERVICE);

				convertView = inflater.inflate(R.layout.tablerow, parent, false);

				holder = new ViewHolder();
				holder.topView = (TextView)convertView.findViewById(R.id.top_view);
				holder.bottomView = (TextView)convertView.findViewById(R.id.bottom_view);
				holder.icon = (ImageView)convertView.findViewById(R.id.row_image);
				holder.mSelect = (ImageView)convertView.findViewById(R.id.multiselect_icon);

				convertView.setTag(holder);

			} else {
				holder = (ViewHolder)convertView.getTag();
			}

			if (positions != null && positions.contains(position))
				holder.mSelect.setVisibility(ImageView.VISIBLE);
			else
				holder.mSelect.setVisibility(ImageView.GONE);

			holder.topView.setTextColor(color);
			holder.bottomView.setTextColor(color);

			if(file.isDirectory()) {
				holder.icon.setImageResource(R.drawable.folder);

			} else if(file.isFile()  ) {
				String ext = file.toString();
				String sub_ext = ext.substring(ext.lastIndexOf(".") + 1);
				/*This series of else if statements will determine which icon is displayed*/
				if (Arrays.asList(Globals.supportedAudioFileFormats).contains(sub_ext.toLowerCase()))
				{
					holder.icon.setImageResource(R.drawable.music);
				}  else if(Arrays.asList(Globals.supportedFontFileType).contains(sub_ext.toLowerCase()))
				{
					holder.icon.setImageResource(R.drawable.ttf);
				}  else if(Arrays.asList(Globals.supportedVideoFileFormats).contains(sub_ext.toLowerCase()))
				{
					holder.icon.setImageResource(R.drawable.movies);
				} 
			}

			if(file.isFile()) {
				double size = file.length();
				if (size > GB)
					display_size = String.format("%.2f Gb ", (double)size / GB);
				else if (size < GB && size > MB)
					display_size = String.format("%.2f Mb ", (double)size / MB);
				else if (size < MB && size > KB)
					display_size = String.format("%.2f Kb ", (double)size/ KB);
				else
					display_size = String.format("%.2f bytes ", (double)size);

				if(file.isHidden())
					holder.bottomView.setText("(hidden) | " + display_size );
				else
					holder.bottomView.setText(display_size );
			} else {
				if(file.isHidden())
					holder.bottomView.setText("(hidden) | " + num_items + " items ");
				else
					holder.bottomView.setText(num_items + " items ");
			}

			holder.topView.setText(file.getName());

			return convertView;

		}

		private void add_multiSelect_file(String src) {
			if(multiselect_data == null)
				multiselect_data = new ArrayList<String>();

			multiselect_data.add(src);
		}
	}

	/**
	 * A private inner class of EventHandler used to perform time extensive 
	 * operations. So the user does not think the the application has hung, 
	 * operations such as copy/past, search, unzip and zip will all be performed 
	 * in the background. This class extends AsyncTask in order to give the user
	 * a progress dialog to show that the app is working properly.
	 * 
	 * (note): this class will eventually be changed from using AsyncTask to using
	 * Handlers and messages to perform background operations. 
	 * 
	 * @author Joe Berria
	 */
	private class BackgroundWork extends AsyncTask<String, Void, ArrayList<String>> {
		private String file_name;
		private ProgressDialog pr_dialog;
		private int type;

		private BackgroundWork(int type) {
			this.type = type;
		}

		/**
		 * This is done on the EDT thread. this is called before 
		 * doInBackground is called
		 */
		@Override
		protected void onPreExecute() {

			switch(type) {
			case SEARCH_TYPE:
				pr_dialog = ProgressDialog.show(context, context.getString( R.string.searching), 
						context.getString(R.string.searchingcurrentfilesystem),
						true, true);
				break;
			}
		}

		/**
		 * background thread here
		 */
		@Override
		protected ArrayList<String> doInBackground(String... params) {
			switch(type) {
			case SEARCH_TYPE:
				file_name = params[0];
				ArrayList<String> found = file_mg.searchInDirectory(file_mg.getCurrentDir(), 
						file_name);
				return found;
			}
			return null;
		}

		/**
		 * This is called when the background thread is finished. Like onPreExecute, anything
		 * here will be done on the EDT thread. 
		 */
		@Override
		protected void onPostExecute(final ArrayList<String> file) {			
			final CharSequence[] names;
			int len = file != null ? file.size() : 0;

			switch(type) {
			case SEARCH_TYPE:
				pr_dialog.dismiss();
				if(len == 0) {
					Toast.makeText(context, "Couldn't find " + file_name, 
							Toast.LENGTH_SHORT).show();

				} else {
					names = new CharSequence[len];

					for (int i = 0; i < len; i++) {
						String entry = file.get(i);
						names[i] = entry.substring(entry.lastIndexOf("/") + 1, entry.length());
					}

					AlertDialog.Builder builder = new AlertDialog.Builder(context);
					builder.setTitle(context.getString(R.string.found) + " " + len + " " + context.getString(R.string.files));
					builder.setItems(names, new DialogInterface.OnClickListener() {

						public void onClick(DialogInterface dialog, int position) {
							String path = file.get(position);
							System.out.println("path:"+path);
							updateDirectory(file_mg.getNextDir(path.
									substring(0, path.lastIndexOf("/")), true));
							path_label.setText(file_mg.getCurrentDir());
							File FILEpath = new File(path);
							System.out.println("Current Dir:"+file_mg.getCurrentDir()); 
							/*
							 * If the user has multi-select on, we just need to record the file
							 * not make an intent for it.
							 */
							if (FileManager.supportedFile(FILEpath.getPath())) {
								if(FileManager.isSubtitleFontFile(FILEpath.getPath())){
									String fontFilename = FILEpath.getPath();
									//	setsubtitlefile(fontFilename);
									AVPlayerMain.saveAndSetSubtitleFontFile(fontFilename);	
									//Toast.makeText(context, R.string.setasdefaultsubtitlefont, 
									//		Toast.LENGTH_LONG).show();
									System.out.print("Font file selected");
								}else					 //// Set files to play from this directory to play next.
									if(FILEpath.exists()){
										String filename =   FILEpath.getPath();
										Intent intent = new Intent(context, VideoPlayer.class);
										intent.putExtra("videofilename", filename);
										context.startActivity(intent);
									}		
							}

						}
					});

					AlertDialog dialog = builder.create();
					dialog.show();
				}
				break;
			}
		}
	}
}
