package com.broov.player;

import android.content.Context;
import android.graphics.PixelFormat;
import android.util.Log;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.widget.ImageView;
import com.broov.player.MyApp;


public class ImageFloatView extends ImageView {
    private float mTouchStartX;
    private float mTouchStartY;
    private float x;
    private float y;
    
    private WindowManager wm=(WindowManager)getContext().getApplicationContext().getSystemService("window");
    //此wmParams变量为获取的全局变量，用以保存悬浮窗口的属性
    private WindowManager.LayoutParams wmParams = ((MyApp)getContext().getApplicationContext()).getMywmParams2();

	public ImageFloatView(Context context) {
		super(context);		
		// TODO Auto-generated constructor stub
	}
	
	 @Override
	 public boolean onTouchEvent(MotionEvent event) {
	     //获取相对屏幕的坐标，即以屏幕左上角为原点		 
	     x = event.getRawX();   
	     y = event.getRawY()-25;   //25是系统状态栏的高度
	     Log.i("currP", "currX"+x+"====currY"+y);
	     switch (event.getAction()) {
	        case MotionEvent.ACTION_DOWN:    //捕获手指触摸按下动作
	            //获取相对View的坐标，即以此View左上角为原点
	            mTouchStartX =  event.getX();  
                    mTouchStartY =  event.getY();                
	            Log.i("startP", "startX"+mTouchStartX+"====startY"+mTouchStartY);
	            break;

	        case MotionEvent.ACTION_MOVE:	//捕获手指触摸移动动作            
	            updateViewPosition();
	            break;

	        case MotionEvent.ACTION_UP:    //捕获手指触摸离开动作
	            updateViewPosition();
	            mTouchStartX=mTouchStartY=0;
	            break;
	        }
	        return true;
		}
	 
	 private void updateViewPosition(){
	    //更新浮动窗口位置参数
	    wmParams.x=(int)( x-mTouchStartX);
	    wmParams.y=(int) (y-mTouchStartY);
	    wm.updateViewLayout(this, wmParams);  //刷新显示 
	 }

}
