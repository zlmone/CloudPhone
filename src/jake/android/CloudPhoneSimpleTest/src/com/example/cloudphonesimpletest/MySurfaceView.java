package com.example.cloudphonesimpletest;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.util.List;

import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.Size;
import android.os.AsyncTask;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class MySurfaceView extends SurfaceView implements SurfaceHolder.Callback {
	SurfaceHolder mHolder;
	Camera mCamera;
	private String ip = "211.189.20.137";
	private int port = 3737;
	private Socket mSocket;
	private OutputStream outStream;
	
	private boolean mForever = true;
	
	public MySurfaceView(Context context, AttributeSet attrs){
		super(context, attrs);
		mHolder = getHolder();
		mHolder.addCallback(this);
		mHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
	}

	// 표면의 크기가 결정될 때 최적의 미리보기 크기를 구해 설정한다.
	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
		// TODO Auto-generated method stub
		Camera.Parameters parameters = mCamera.getParameters();
		List<Size> arSize = parameters.getSupportedPreviewSizes();
		if(arSize==null){
			parameters.setPreviewSize(width, height);
			String msg;
			msg= "width:"+width+"height:"+height;
			Log.i("tag", msg);
		} else {
			int diff = 10000;
			Size opti = null;
			for(Size s : arSize){
				if(Math.abs(s.height - height) < diff) {
					diff = Math.abs(s.height - height);
					opti = s;
				}
			}
			parameters.setPreviewSize(opti.width, opti.height);
		}
		mCamera.setParameters(parameters);
		mCamera.startPreview();
		
	}
	
	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		// TODO Auto-generated method stub
		mCamera = Camera.open();
		try {
			mCamera.setPreviewCallback(mPreviewCallBack);
			mCamera.setPreviewDisplay(mHolder);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			mCamera.release();
			mCamera = null;
		}
		
		new NetworkTask(ip, port).execute();
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		// TODO Auto-generated method stub
		
	}
	
	private Camera.PreviewCallback mPreviewCallBack = new PreviewCallback() {
		@Override
		public void onPreviewFrame(byte[] data, Camera camera) {
			
			Log.w("LOG", "onPreviewFrame");
			/**
			 * Yuv to JPEG
			 */
			Camera.Parameters parameters = camera.getParameters();
			Size size = parameters.getPreviewSize();
			YuvImage image = new YuvImage(data, ImageFormat.NV21,
					size.width, size.height, null);
			Rect rectangle = new Rect();
			rectangle.bottom = size.height;
			rectangle.top = 0;
			rectangle.left = 0;
			rectangle.right = size.width;
			ByteArrayOutputStream out = new ByteArrayOutputStream();
			image.compressToJpeg(rectangle, 50, out);
			
			try {
				byte[] len = ByteBuffer.allocate(4).putInt(out.toByteArray().length).array();
				outStream.write(len, 0, len.length);
				outStream.write(out.toByteArray(), 0, out.toByteArray().length);
			} catch (Exception e) {
				e.printStackTrace();
			}
			//out.toByteArray();
			
		}
	};
	
	public class NetworkTask extends AsyncTask<Void, Void, Void> {

		String dstAddress;
		int dstPort;
		String response;

		NetworkTask(String addr, int port) {
			dstAddress = addr;
			dstPort = port;
		}

		@Override
		protected Void doInBackground(Void... arg0) {

			try {
				SocketAddress addr = new InetSocketAddress(ip, port);
				mSocket = new Socket();
				mSocket.connect(addr, 15000);
				
				if(mSocket.isConnected()) {
					outStream = mSocket.getOutputStream();
					
					while(mForever) {
					
					}
				}
			} catch (UnknownHostException e) {
				e.printStackTrace();
			} catch (IOException e) {
				e.printStackTrace();
			}
			return null;
		}

		@Override
		protected void onPostExecute(Void result) {
			super.onPostExecute(result);
		}
	}
}