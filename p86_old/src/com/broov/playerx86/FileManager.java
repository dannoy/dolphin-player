package com.broov.playerx86;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.Stack;
import java.io.File;

public class FileManager {
	private static final int SORT_NONE = 	0;
	private static final int SORT_ALPHA = 	1;
	private static final int SORT_TYPE = 	2;

	private static final int KB = 1024;
	private static final int MB = KB * KB;
	private static final int GB = MB * KB;

	public static int subtitleFontSize = Globals.SUBTITLE_FONT_MEDIUM ;
    
	private double dir_size = 0;
	public Stack<String> path_stack;
	private ArrayList<String> dir_content;

	public static ArrayList<String> alreadyPlayed = new ArrayList<String>();

	/**
	 * Constructs an object of the class
	 * <br>
	 * this class uses a stack to handle the navigation of directories.
	 */
	public FileManager() {
		dir_content = new ArrayList<String>();
		path_stack = new Stack<String>();

		path_stack.push("/");
		path_stack.push(path_stack.peek() + "sdcard");
		System.out.println("FILE MANAGER CONSTRUCTOR");
	}

	/**
	 * This will return a string of the current directory path
	 * @return the current directory
	 */
	public String getCurrentDir() {
		System.out.println("FILE MANAGER getCurrentDir=" + path_stack.peek());
		return path_stack.peek();
	}

	/**
	 * This will return a string of the current home path.
	 * @return	the home directory
	 */
	public ArrayList<String> getHomeDir() {
		//This will eventually be placed as a settings item
		path_stack.clear();
		path_stack.push("/");

		path_stack.push(path_stack.peek() + "sdcard");
		return populate_list();
	}

	/**
	 * This will return a string of the lastopened directory strings.
	 * @return	the lastopened directory
	 */
	public ArrayList<String> getLastOpenedDir() {
		//This will eventually be placed as a settings item
		path_stack.clear();
		path_stack.push("/");
		
		System.out.println("getLastOpenedDir lastopendir:"+Globals.dbLastOpenDir);
		
		//path_stack.push(path_stack.peek() + "/");
		
		try{
			ArrayList<String> subpathsForStack =PathtoSubPaths(Globals.dbLastOpenDir);
		//	System.out.println("getLastOpenedDir() subpathsForStack:"+subpathsForStack);
			int subPathsStackSize = subpathsForStack.size();
			for(int i=1;i< subPathsStackSize;i++){
				path_stack.push(subpathsForStack.get(i));
			}
			path_stack.push(Globals.dbLastOpenDir);
			System.out.println(path_stack);
		}catch(Exception e){
			path_stack.clear();
			path_stack.push("/");
			path_stack.push(path_stack.peek() + "sdcard");
		}    
		return populate_list();
	}

	/**
	 * 
	 * @param lastopendir
	 * input directory string
	 * @return
	 * ArrayList<String> containing subpaths.
	 */
	public ArrayList<String> PathtoSubPaths(String lastopendir){
		ArrayList<String> subpaths = new ArrayList<String>();
		int lastOpenDirLength = lastopendir.length();
		for(int i =0; i<lastOpenDirLength;i++){
			if(lastopendir.charAt(i)=='/'){
				//System.out.println(lastopendir.substring(0, i));
				subpaths.add(lastopendir.substring(0, i));
			}
		}
		return subpaths;
	}

	public static int loopOptionForFile(String file){
		if(isAudioFile(file)){
			return	Globals.dbAudioLoop;
		}else{
			return Globals.dbVideoLoop;
		}
	}

	/**
	 * @return
	 * subtitleFont size with 9, 11, 13 values
	 */
	public static int getSubTitleSize(){
		switch(Globals.dbSubtitleSize){
		case 0:
			subtitleFontSize=Globals.SUBTITLE_FONT_SMALL;
			break;
		case 1:
			subtitleFontSize=Globals.SUBTITLE_FONT_MEDIUM;
			break;
		case 2:
			subtitleFontSize=Globals.SUBTITLE_FONT_LARGE;
			break;
		default:
			subtitleFontSize=Globals.SUBTITLE_FONT_MEDIUM;
		}
		return subtitleFontSize;
	}


	public static int getshow_subtitle(){
		if (Globals.dbSubtitle == false){
			return 0;
		}
		return 1;
	}

	/**
	 * 
	 * @param fileName
	 * String fileName to get filetype
	 * @return
	 */
	public static String getFileType(String fileName){	
		return "File type: "+ fileName.substring(fileName.lastIndexOf(".") + 1);
	}

	public static String getFileName(String fileName){	
		//return "Playing: "+ fileName.substring(fileName.lastIndexOf("/") + 1);
		return fileName.substring(fileName.lastIndexOf("/") + 1);
	}

	public static String getFileSize(String filename){
		File file = new File(filename);

		String display_size ="";
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
		}
		return "File size: " + display_size;
	}

	/**
	 * This will return a string that represents the path of the previous path
	 * @return	returns the previous path
	 */
	public ArrayList<String> getPreviousDir() {
		int size = path_stack.size();

		if (size >= 2)
			path_stack.pop();

		else if(size == 0)
			path_stack.push("/");

		return populate_list();
	}

	/**
	 * @param path
	 * @param isFullPath
	 * @return
	 */
	public ArrayList<String> getNextDir(String path, boolean isFullPath) {
		int size = path_stack.size();

		if(!path.equals(path_stack.peek()) && !isFullPath) {
			if(size == 1)
				path_stack.push("/" + path);
			else
				path_stack.push(path_stack.peek() + "/" + path);
		}

		else if(!path.equals(path_stack.peek()) && isFullPath) {
			path_stack.push(path);
		}

		return populate_list();
	}



	/**
	 * 
	 * @param name
	 * @return
	 */
	public boolean isDirectory(String name) {
		return new File(path_stack.peek() + "/" + name).isDirectory();
	}
	/**
	 * 
	 * @param dir
	 * @param pathName
	 * @return
	 */
	public ArrayList<String> searchInDirectory(String dir, String pathName) {
		ArrayList<String> names = new ArrayList<String>();
		search_file(dir, pathName, names);

		return names;
	}

	/**
	 * 
	 * @param path
	 * @return
	 */
	public double getDirSize(String path) {
		get_dir_size(new File(path));

		return dir_size;
	}


	private static final Comparator alph = new Comparator<String>() {
		@Override
		public int compare(String arg0, String arg1) {
			return arg0.toLowerCase().compareTo(arg1.toLowerCase());
		}
	};

	private static final Comparator type = new Comparator<String>() {
		@Override
		public int compare(String arg0, String arg1) {
			String ext = null;
			String ext2 = null;

			try {
				ext = arg0.substring(arg0.lastIndexOf(".") + 1, arg0.length());
				ext2 = arg1.substring(arg1.lastIndexOf(".") + 1, arg1.length());

			} catch (IndexOutOfBoundsException e) {
				return 0;
			}

			return ext.compareTo(ext2);
		}
	};

	/** (non-Javadoc)
	 * this function will check the file is supported or not
	 * the string is taken and the filetype is checked with supported file type
	 * @param file
	 * file to be checked. 
	 * @return
	 * true if supported i.e the file extension matches supported filetype list
	 * false if not supported i.e the file extension does not matches the supported filetype list
	 */
	public static Boolean supportedFile(String file){
		String ext = file.toString();
		String sub_ext = ext.substring(ext.lastIndexOf(".") + 1);
		if(Arrays.asList(Globals.supportedFileFormats).contains(sub_ext.toLowerCase())){
			return true; 
		}
		return false;
	}
	
	public static Boolean isSubtitleFontFile(String file){
		String ext = file.toString();
		String sub_ext = ext.substring(ext.lastIndexOf(".") + 1);
		if(Arrays.asList(Globals.supportedFontFileType).contains(sub_ext.toLowerCase())){
			return true; 
		}
		return false;
	}

	public static Boolean isAudioFile(String file){
		if(file==null || file =="" ){
			return false;
		}		
		String ext = file.toString();
		String sub_ext = ext.substring(ext.lastIndexOf(".") + 1);
		if(Arrays.asList(Globals.supportedAudioFileFormats).contains(sub_ext.toLowerCase())){
			return true; 
		}
		return false;
	}

	public static Boolean isVideoFile(String file){
		if(file==null || file ==""){
			return false;
		}
		String ext = file.toString();
		String sub_ext = ext.substring(ext.lastIndexOf(".") + 1);
		if (Arrays.asList(Globals.supportedVideoFileFormats).contains(sub_ext.toLowerCase())){
			return true; 
		}
		return false;
	}
	
	public static Boolean isImageFile(String file){
		String ext = file.toString();
		String sub_ext = ext.substring(ext.lastIndexOf(".") + 1);
		if (Arrays.asList(Globals.supportedImageFileFormats).contains(sub_ext.toLowerCase())){
			return true; 
		}
		return false;
	}
	

	public static String getPrevFileInDirectory(String filename){
		String prevFile="";
		System.out.println("fileName: "+filename);
		File FILEfilename = new File(filename);

		//	System.out.println("alreadyPlayed: "+alreadyPlayed);
		//	System.out.println("audioloop_type:"+audioloop_type);
		System.out.println("getPrevFile in Directory input filename:"+filename);
		if (isAudioFile(filename)){
			ArrayList<String> listOfAudioFiles =	listofAudioFiles(FILEfilename.getParent());
			int audioFilesSize = listOfAudioFiles.size();
			for(int i=0; i<audioFilesSize;i++)
			{  
				System.out.println("filename:"+filename);
				System.out.println("currentFile:"+listOfAudioFiles.get(i));

				if (filename.equalsIgnoreCase(listOfAudioFiles.get(i))){
					if (i<=0){i=audioFilesSize;}
					i = i-1;

					prevFile = listOfAudioFiles.get(i);
					System.out.println("got previous file:"+ prevFile);
					return prevFile;
				}
			}
		}else {
			ArrayList<String> listOfVideoFiles =	listofVideoFiles(FILEfilename.getParent());
			int videoFilesSize = listOfVideoFiles.size();
			for(int i=0; i<videoFilesSize;i++)
			{  
				System.out.println("filename:"+filename);
				System.out.println("currentFile:"+listOfVideoFiles.get(i));

				if (filename.equalsIgnoreCase(listOfVideoFiles.get(i))){
					if (i<=0){i=videoFilesSize;}
					i = i-1;

					prevFile = listOfVideoFiles.get(i);
					System.out.println("got previous file:"+ prevFile);
					return prevFile;
				}
			}
		}
		
		//alreadyPlayed.clear();
		return "";
	}

	public static String getNextFileInDirectory(String filename){
		String nextFile="";
		System.out.println("fileName: "+filename);
		File FILEfilename = new File(filename);
		//	ArrayList<String> listOfFiles =  getFilesAlone(filename1.getParent());

		//System.out.println("listofFiles: "+listOfFiles);
		alreadyPlayed.add(FILEfilename.getAbsolutePath());

		System.out.println("alreadyPlayed: "+alreadyPlayed);
		//System.out.println("audioloop_type:"+audioloop_type);

		if (isAudioFile(filename)){
			ArrayList<String> listOfAudioFiles =	listofAudioFiles(FILEfilename.getParent());
			int audioFilesSize = listOfAudioFiles.size();
			for(int i=0; i<audioFilesSize;i++)
			{
				nextFile = listOfAudioFiles.get(i);
				if (!(alreadyPlayed.contains(nextFile))){
					if (alreadyPlayed.size() ==(audioFilesSize-1) && 
						Globals.dbAudioLoop==Globals.REPEAT_ALL){
						alreadyPlayed.clear();
						System.out.println("Already played items are cleared");
					}
					return nextFile;
				}		
			}
		}else{
			ArrayList<String> listOfVideoFiles =	listofVideoFiles(FILEfilename.getParent());
			int videoFilesSize = listOfVideoFiles.size();
			for(int i=0; i<videoFilesSize;i++)
			{
				nextFile = listOfVideoFiles.get(i);
				if (!(alreadyPlayed.contains(nextFile))) {
					if (alreadyPlayed.size() ==(videoFilesSize -1) && 
					    Globals.dbVideoLoop==Globals.REPEAT_ALL) {
						alreadyPlayed.clear();
						System.out.println("Already played items are cleared");
					}
					return nextFile;
				}		
			}	
		}
		alreadyPlayed.clear();
		return "";
	}

	public ArrayList<String> getFilesAlone(String file){
		File inFile = new File(file);
		ArrayList<String> listOfFiles = new ArrayList<String>();
		
		for (File files : inFile.listFiles()) {
			String absolutePath = files.getAbsolutePath();
			if (files.isFile() && supportedFile(absolutePath)){
				listOfFiles.add(absolutePath);
			}
		}
		return listOfFiles;
	}

	public static ArrayList<String> listofAudioFiles(String file){
		File inFile = new File(file);
		ArrayList<String> listOfFiles = new ArrayList<String>();
		for (File files : inFile.listFiles()) {
			String absolutePath = files.getAbsolutePath();
			if (files.isFile() && isAudioFile(absolutePath)){
				listOfFiles.add(absolutePath);
			}
		}
		return listOfFiles;
	}

	public static ArrayList<String> listofVideoFiles(String file){
		File inFile = new File(file);
		ArrayList<String> listOfFiles = new ArrayList<String>();
		for (File files : inFile.listFiles()) {
			String absolutePath = files.getAbsolutePath();
			if (files.isFile() && isVideoFile(absolutePath)){
				listOfFiles.add(absolutePath);
			}
		}
		return listOfFiles;
	}
	
	public static ArrayList<String> listofImageFiles(String file){
		File inFile = new File(file);
		ArrayList<String> listOfFiles = new ArrayList<String>();
		for (File files : inFile.listFiles()) {
			String absolutePath = files.getAbsolutePath();
			if (files.isFile() && isImageFile(absolutePath)){
				listOfFiles.add(absolutePath);
			}
		}
		return listOfFiles;
	}

	public static int getImageIndex(){
		int imageIndex = 0;
		int min = 0;

		try{
			int max = Globals.numberofImages ;

			imageIndex = (int) (Math.random() * (max - min + 1) ) + min;
			//	System.out.println("Random Image index number : " + imageIndex);
		}catch(Exception e){}
		return imageIndex;
	}

	/** (non-Javadoc)
	 * this function will take the string from the top of the directory stack
	 * and list all files/folders that are in it and return that list so 
	 * it can be displayed. Since this function is called every time we need
	 * to update the the list of files to be shown to the user, this is where 
	 * we do our sorting (by type, alphabetical, etc).
	 * 
	 * @return
	 */
	public ArrayList<String> populate_list() {

		if(!dir_content.isEmpty())
			dir_content.clear();

		File file = new File(path_stack.peek());

		if (file.exists() && file.canRead()) {
			String[] list = file.list();
			int len = list.length;

			/* add files/folder to arraylist depending on hidden status */
			for (int i = 0; i < len; i++) {
				if(supportedFile(list[i]) || isDirectory(list[i])){
					if (!Globals.dbHide) {
						if(list[i].toString().charAt(0) != '.')
							dir_content.add(list[i]);
					} else {
						dir_content.add(list[i]);
					}
				}
			}

			/* sort the arraylist that was made from above for loop */
			switch(Globals.dbSort) {
				case SORT_NONE:
					//no sorting needed
					break;

				case SORT_ALPHA:
					Object[] tt = dir_content.toArray();
					dir_content.clear();

					Arrays.sort(tt, alph);

					for (Object a : tt){
						dir_content.add((String)a);
					}
				break;

			case SORT_TYPE:
				Object[] t = dir_content.toArray();
				String dir = path_stack.peek();

				Arrays.sort(t, type);
				dir_content.clear();

				for (Object a : t){
					if(new File(dir + "/" + (String)a).isDirectory())
						dir_content.add(0, (String)a);
					else
						dir_content.add((String)a);
				}
				break;
			}

		} else {
			dir_content.add("Empty");
		}
		return dir_content;
	}

	/**
	 * 
	 * @param path
	 */
	private void get_dir_size(File path) {
		File[] list = path.listFiles();
		int len;

		if(list != null) {
			len = list.length;

			for (int i = 0; i < len; i++) {
				if(list[i].isFile())
					dir_size += list[i].length();

				else if(list[i].isDirectory() && list[i].canRead())
					get_dir_size(list[i]);
			}
		}
	}

	/**
	 * (non-JavaDoc)
	 * I dont like this method, it needs to be rewritten. Its hacky in that
	 * if you are searching in the root dir (/) then it is not going to be treated
	 * as a recursive method so the user dosen't have to sit forever and wait.
	 * 
	 * I will rewrite this ugly method.
	 * 
	 * @param dir		directory to search in
	 * @param fileName	filename that is being searched for
	 * @param n			ArrayList to populate results
	 */
	private void search_file(String dir, String fileName, ArrayList<String> n) {
		File root_dir = new File(dir);
		String[] list = root_dir.list();

		if(list != null && root_dir.canRead()) {
			int len = list.length;

			for (int i = 0; i < len; i++) {
				File check = new File(dir + "/" + list[i]);
				String name = check.getName();

				if(check.isFile() && name.toLowerCase().contains(fileName.toLowerCase()) && supportedFile(name)) {
					n.add(check.getPath());
				}
				else if(check.isDirectory()) {
					if(name.toLowerCase().contains(fileName.toLowerCase()))
						n.add(check.getPath());

					else if(check.canRead() && !dir.equals("/"))
						search_file(check.getAbsolutePath(), fileName, n);
				}
			}
		}
	}
}
