package com.broov.player;

import java.lang.reflect.Method;

import android.view.View;

public class Utils {
	/** 
     * Returns a formated time HoursH MinutesM SecondsS
     * 
     * @param millis
     * @return
     */
    public static String formatTime(long seconds) {
              String output = "";
              //long seconds = millis / 1000;
              long minutes = seconds / 60;
              long hours = minutes / 60;
              long days = hours / 24;
              seconds = seconds % 60;
              minutes = minutes % 60;
              hours = hours % 24;

              String secondsD = String.valueOf(seconds);
              String minutesD = String.valueOf(minutes);
              String hoursD = String.valueOf(hours); 

              if (seconds < 10)
                secondsD = "0" + seconds;
              if (minutes < 10)
                minutesD = "0" + minutes;
              if (hours < 10){
                hoursD = "0" + hours;
              }

              if( days > 0 ){
                      output = days +"d ";
              } 
              if(hours > 0) {
            	  output += hoursD + ":";
              }
                      //output += hoursD + ":" + minutesD + ":" + secondsD;
              			output += minutesD + ":" + secondsD;
             
              return output;
    }
    

    //Code reference: http://code.google.com/p/libgdx

	private static Method setSystemUiVisibilityMethod;
	
	static {
		initCompatibility();
	};
	
	public static void initCompatibility() {
		try {
			Class classView = Class.forName("android.view.View");
			setSystemUiVisibilityMethod = classView.getDeclaredMethod("setSystemUiVisibility", int.class);
		} catch(NoSuchMethodException exception) {
			debug("AndroidUtils", "Could not get setSystemUiVisibility method", exception);
		} catch (ClassNotFoundException e) {
			//e.printStackTrace();
		}
	}
	
	public static void hideSystemUi(View view) {
		int apiVersion = android.os.Build.VERSION.SDK_INT;
		view.getVisibility();
		try {
			if(apiVersion >= android.os.Build.VERSION_CODES.ICE_CREAM_SANDWICH) {
				setSystemUiVisibilityMethod.invoke(view, View.SYSTEM_UI_FLAG_LOW_PROFILE);

				//setSystemUiVisibilityMethod.invoke(view, View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);
			} else {
				setSystemUiVisibilityMethod.invoke(view, View.STATUS_BAR_HIDDEN);
			}
		} catch(Exception exception) {
			debug("AndroidUtils", "Could not invoke setSystemUiVisibility method", exception);
		}
	}
	
	public static void debug (String tag, String message, Throwable exception) {
			System.out.println(tag + ": " + message);
			//exception.printStackTrace(System.out);
	}
	
//	OnSystemUiVisibilityChangeListener visibilityChangeListener = new OnSystemUiVisibilityChangeListener() {
//		@Override
//		public void  onSystemUiVisibilityChange(int state) {
////			int SDK_INT = android.os.Build.VERSION.SDK_INT;
////			System.out.println("SDK Version:"+SDK_INT);
////			if(SDK_INT >= 11 && SDK_INT < 14) {
////				getWindow().getDecorView().setSystemUiVisibility(View.STATUS_BAR_HIDDEN);
////			}else if(SDK_INT >= 14){
////				getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LOW_PROFILE);
////
////				//getWindow().getDecorView().setSystemUiVisibility(View.STATUS_BAR_HIDDEN);
////				getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);
////
////			}
//
//		}
//	};
//
//	getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(visibilityChangeListener);

//	{
//		int SDK_INT = android.os.Build.VERSION.SDK_INT;
//		System.out.println("SDK Version:"+SDK_INT);
//		if (SDK_INT >=11) {
//
//			if(SDK_INT >= 11 && SDK_INT < 14) {
//				getWindow().getDecorView().setSystemUiVisibility(View.STATUS_BAR_HIDDEN);
//			}else if(SDK_INT >= 14){
//				getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LOW_PROFILE);
//				getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);
//			}
//		}
//	}


}
