package com.broov.playerN;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.URL;
import java.net.URLConnection;

import org.apache.http.HttpResponse;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.impl.client.DefaultHttpClient;


import android.app.Activity;
import android.content.Context;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.ArrayAdapter;
import android.widget.TextView;
import android.widget.Toast;


public class Feedback extends Activity {
	public static String feedback_url="http://bullshit.broov.in/androidquote.php?msg=";
	//public static String feedback_url="http://m.broov.com/feedback.php?msg=";
	static Context mContext;
	static InputStream 		stream = null;
	static URLConnection 	cn;
	static URL    			url;
	public static String appname;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		mContext=this;
		setContentView(R.layout.feedback);
		appname = mContext.getString(R.string.app_name);
		ArrayAdapter<CharSequence> adapter;
		
		Button submitButton =(Button) findViewById(R.id.submitbutton);
		Button cancelButton =(Button) findViewById(R.id.cancelbutton);
		
		final	TextView mDesc=  (TextView) findViewById(R.id.feedback_description);
		final	TextView mMail=(TextView) findViewById(R.id.feedback_mailid);
	
		final Spinner s1 = (Spinner) findViewById(R.id.spinner1);
		
		adapter =  ArrayAdapter.createFromResource(this, R.array.Category,
		android.R.layout.simple_spinner_item);
		adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
		s1.setAdapter(adapter);
	    		
		submitButton.setOnClickListener(new View.OnClickListener() {
        
			public void onClick(View v) {
				// Button feedOption = (Button) findViewById(R.id.spinner1); 
			
				String  description = mDesc.getText().toString();
				String mailid = mMail.getText().toString();
				
				if(description.length()<=2)
				{
					Toast.makeText(mContext,R.string.pleaseenteryourthoughtsindescription ,
		                    Toast.LENGTH_SHORT).show();
				}
				else{
					String category = s1.getSelectedItem().toString();

					new SendFeedbackTask().execute(description, mailid, category) ;
					finish();
				}} });
		
		cancelButton.setOnClickListener(new View.OnClickListener() {
	        
			public void onClick(View v) {
				finish();
			}});
		
	}
	private class SendFeedbackTask extends AsyncTask<String, Void, String> {
		protected String doInBackground(String... urls) {
			return SendFeedbackRequest(urls[0], urls[1], urls [2]);
		}
		protected void onPreExecute() {
			Toast.makeText(mContext, "Sending .." ,
					Toast.LENGTH_LONG).show();			

		}    

		protected void onPostExecute(String result) {
			// System.out.println("result:"+result);
			Toast.makeText(mContext,result ,
					Toast.LENGTH_LONG).show();			
		}
	}
	
	public static String SendFeedbackRequest(String msgfeed, String contact, String category) 
	{
		msgfeed=CleanTitle(msgfeed);
		contact=CleanTitle(contact);
		category=CleanTitle(category);
		//	System.out.println("Msg:"+msgfeed+":contact:"+contact+":category:"+category);
		
		String responseReceived="";
		try {
			String url_modified=msgfeed+"&contact="+contact+"&category="+category+"&ver="+android.os.Build.VERSION.SDK+"&app="+appname;
			//System.out.println("url1 is"+url_modified);
			responseReceived = SendHttpRequest(url_modified, feedback_url);
		
			responseReceived.replace('\n', ' ');
			//System.out.println("Response:"+responseReceived);
		} catch (Exception e) {
			//	System.out.println("Feedback:exc:"+e.getMessage());
		}
		return responseReceived;
	}
	
	public static class HttpHelper {
		public static String request(HttpResponse response){
			String result = "";
			try{
				InputStream in = response.getEntity().getContent();
				BufferedReader reader = new BufferedReader(new InputStreamReader(in));
				StringBuilder str = new StringBuilder();
				String line = null;
				while((line = reader.readLine()) != null){
					str.append(line + "\n");
				}
				in.close();
				result = str.toString();
			}catch(Exception ex){
				result = "Error";
			}
			return result;
		}
	} 
	
	//SendFeedbackRequest() to Broov website
	public static String SendHttpRequest(String msg, String url_link) throws Exception {
		final String proxyHost = android.net.Proxy.getDefaultHost(); 
		final int proxyPort = android.net.Proxy.getDefaultPort(); 
		if (proxyPort != -1) 
		{ 
		        System.setProperty("http.proxyHost", proxyHost); 
		        System.setProperty("http.proxyPort", Integer.toString(proxyPort)); 
		}
		System.out.println("proxyHost:"+android.net.Proxy.getDefaultHost());
		System.out.println("proxyPort:"+android.net.Proxy.getDefaultPort());
		
		//byte[] buffer = new byte[2048]; /* arbitrary buffer size */
		String errorMsg = null;
		String responseMsg = "";
		//int length;

	 	try {
			msg = msg.replace(' ', '+');
			//	System.out.println("msg"+msg);
			//final String urlString = url_link + msg +"&msisdn=0012" ;
			final String urlString = url_link + msg ;
			
			//System.out.println(urlString);
			try {
				//new Thread(new Runnable() {
				//    public void run() {
				try{
			 
			HttpClient httpclient = new DefaultHttpClient();

				HttpPost httppost = new HttpPost(urlString);
					HttpResponse response = httpclient.execute(httppost);
					return 	HttpHelper.request(response);
				} catch (ClientProtocolException ex) {
					//errorMsg="Unable to connect to server. Please try after sometime";
					errorMsg=mContext.getString( R.string.unabletoconnecttoserverpleasetryaftersometime);
				}


				//  }
				//}).start();

				} catch (Exception ex) {
					errorMsg=mContext.getString( R.string.unabletoconnecttoserverpleasetryaftersometime);
				} 
		} catch (Exception e) {
			errorMsg=mContext.getString( R.string.unabletoconnecttoserverpleasetryaftersometime);
		}		
		
		if (errorMsg != null) {
			responseMsg = errorMsg;
		}
		return (responseMsg);
	}

	private static String CleanTitle(String sTitle) 
	{
		sTitle = sTitle.replaceAll("\n"," ");
		sTitle = sTitle.replaceAll("'","").replaceAll("\"","");
		sTitle = sTitle.replaceAll("´","").replaceAll("`","");
		sTitle = sTitle.replaceAll("'","").replaceAll(" ","_");
		sTitle = sTitle.replace('\\', '-').replace('/', '-').replace('.', '_');
		sTitle = sTitle.replace('$', '_').replace('~','-').replace('<','(').replace('>',')');
		sTitle = sTitle.replace('?', '_').replace('*', '-').replace(':', '-').trim();
		return sTitle;
	} // End of CleanTitle() Method

	
}
