package com.android_wifi;


import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;

import android.util.Log;
import android.widget.TextView;
import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;

@SuppressLint("HandlerLeak")
public class ClientThread extends Thread {
	private String TAG = "zigbee_demo";
	
	private OutputStream outputStream = null;
	private InputStream inputStream = null;
	private Socket socket;
	private SocketAddress socketAddress;
	public static Handler childHandler;
	private boolean RxFlag = true;
	public boolean bClientThreadStart = false;
	private RxThread rxThread;
	final int TEXT_INFO = 12;
	static final int RX_EXIT = 11;
	static final int TX_DATA = 10;
	Context mainContext;
	Message msg;
	private String strIP;
	private int iPort;
 

	//功能码定义
	static final int FUN_CODE_CHECK_ALL_DATA=0x01;	//手机/PC-->zigbee 查询所有传感器数据
	static final int FUN_CODE_UPDATA_ALL_DATA=0x02;	//zigbee-->手机/PC 上传所有传感器数据
	static final int FUN_CODE_UPDATA_RFID=0x03;	//zigbee-->手机/PC  上传RFID数据
	static final int ZIGBEE_FUN_CODE_CTRL_LAMP=0x04;	//控制单个终端上灯的状态
	static final int ZIGBEE_FUN_CODE_STEP=0x05;	//控制终端上的步进电机
	static final int ZIGBEE_FUN_CODE_END1=0x06;  //终端1的终端码，设置终端的阀值和开关电机 

	
	public ClientThread(String ip, int port) {
		strIP = ip;
		iPort =port;
	}	

	boolean socketConnect(){
		
		
		if(socket.isConnected()) return true;
		
		return false;
	}
	//连接网络
	void connect() {
		RxFlag = true;
		socketAddress = new InetSocketAddress(strIP, iPort);
		socket = new Socket();
		bClientThreadStart = false;

		try {
			socket.connect(socketAddress, iPort);
			inputStream = socket.getInputStream();
			outputStream = socket.getOutputStream();

			msg = MainActivity.mainHandler.obtainMessage(MainActivity.TIPS_UPDATE_UI, "连接成功");
			MainActivity.mainHandler.sendMessage(msg);

			msg = MainActivity.mainHandler.obtainMessage(MainActivity.Start_timer, "连接成功");
			MainActivity.mainHandler.sendMessageDelayed(msg, 2000);
			
			
			rxThread = new RxThread();
			rxThread.start();
			bClientThreadStart = true;
		} catch (IOException e) {	
			try {
				sleep(10);
			} catch (InterruptedException e1) {
				e1.printStackTrace();
			}
			msg = MainActivity.mainHandler.obtainMessage(MainActivity.TIPS_UPDATE_UI, "无法连接到服务器");
			MainActivity.mainHandler.sendMessage(msg);
			e.printStackTrace();
			bClientThreadStart = false;
		} catch (NumberFormatException e) {

		}
	}

	void initChildHandler() {
		
		Looper.prepare();  //在子线程中创建Handler必须初始化Looper

		childHandler = new Handler() {
			//子线程消息处理中心
			public void handleMessage(Message msg) {

				//接收主线程及其他线程的消息并处理...
				switch (msg.what) {
				case TX_DATA:
					int len = msg.arg1;				

					try {
						outputStream.write((byte [])msg.obj, 0, len);
						outputStream.flush();
					} catch (IOException e) {
						e.printStackTrace();
					}
					break;

				case RX_EXIT:
					RxFlag = false;
					try {
						if (socket.isConnected()) {
							inputStream.close();
							outputStream.close();
							socket.close();
						}
						
					} catch (IOException e1) {
						e1.printStackTrace();
					}

					childHandler.getLooper().quit();// 结束消息队列

					break;

				default:
					break;
				}

			}
		};

		// 启动该线程的消息队列
		Looper.loop();

	}

	public void run() {
		connect();
		initChildHandler();
		msg = MainActivity.mainHandler.obtainMessage(MainActivity.TIPS_UPDATE_UI, "与服务器断开连接");
		MainActivity.mainHandler.sendMessage(msg);
	}
		
	//socket 接收线程
	public class RxThread extends Thread {
		public void run() {
			int i=0;
			byte ParseBuf[] = new byte[256];
			byte ParseLen=0;
			
			try {
				while (socket.isConnected() && RxFlag) {
					byte RxBuf[] = new byte[256];
					int len = inputStream.read(RxBuf);

					if(false){
						continue;
					}


					msg = MainActivity.mainHandler.obtainMessage(MainActivity.TIPS_UPDATE_UI,
						"="+RxBuf[0]+","+RxBuf[1]+","+RxBuf[2]+","+RxBuf[3]+","+RxBuf[4]+","+RxBuf[5]+","+RxBuf[6]+","+RxBuf[7]+","+RxBuf[8]+","+RxBuf[9]+","+RxBuf[10]+",ParseLen="+ParseLen);
				//	MainActivity.mainHandler.sendMessage(msg);
					
					
					if(len>0)
					{
						ParseframeData(RxBuf, (byte)len);
/*						
						for(i=0; i<len; i++)
						{
							if(RxBuf[i]=='\r' || RxBuf[i]=='\n')//发送的数据是以"\r\n"结尾的
							{
								if(ParseLen>0)
								{
									msg = MainActivity.mainHandler.obtainMessage(MainActivity.TIPS_UPDATE_UI,
										"="+ParseBuf[0]+","+ParseBuf[1]+","+ParseBuf[2]+","+ParseBuf[3]+","+ParseBuf[4]+","+ParseBuf[5]+","+ParseBuf[6]+","+ParseBuf[7]+","+ParseBuf[8]+","+ParseBuf[9]+","+ParseBuf[10]+",ParseLen="+ParseLen);
									MainActivity.mainHandler.sendMessage(msg);
								
									ParseframeData(ParseBuf, ParseLen);
								}

								ParseLen=0;
							}else{
								ParseBuf[ParseLen]=RxBuf[i];
								ParseLen++;
							}
						}
*/						
					}else{
						msg = MainActivity.mainHandler.obtainMessage(MainActivity.TIPS_UPDATE_UI,
						"与服务器断开连接");
						MainActivity.mainHandler.sendMessage(msg);

						//退出接收线程
						msg = childHandler.obtainMessage(RX_EXIT);
						childHandler.sendMessage(msg);
						break;
					}					
				}
				
				if (socket.isConnected())
					socket.close();
				
			}catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

	void ParseframeData(byte data[], byte len)
	{
	    byte dataLen=0;
	    byte fc=0;
	    byte addr=0;
		byte i=0,j=0;
		byte sum=0;
		
	    if(len==0) return;

	    //长度不一致
	    if(data[0]!=len) return;

	    //校验和不一致
	    dataLen=(byte) (data[0]-2);
		sum=CheckSum(data, dataLen);
	    if(data[1]!=sum) return;

	    //功能码
	    fc=data[2];

		//msg = MainActivity.mainHandler.obtainMessage(MainActivity.TIPS_UPDATE_UI,
		//		"h"+data[0]+","+data[1]+","+data[2]+","+data[3]+","+data[4]+","+data[5]+","+data[6]+","+data[7]+","+data[8]+","+data[9]+","+data[10]+",fc="+fc+"sum="+sum+"len="+len);
		//MainActivity.mainHandler.sendMessage(msg);

		//if(true) return;
	    
	    //分发处理
	    switch(fc)
    	{
			case FUN_CODE_CHECK_ALL_DATA://手机/PC-->zigbee 查询所有传感器数据
			break;
			case FUN_CODE_UPDATA_ALL_DATA://zigbee-->手机/PC 上传所有传感器数据
			{				
				i=3;
				//终端1
			    MainActivity.endDevInfo.end1_light=data[i++]; //光线
			    MainActivity.endDevInfo.end1_temp=data[i++];  //温度
			    MainActivity.endDevInfo.end1_hum=data[i++];  //湿度

				MainActivity.endDevInfo.wenduLimit=data[i++];//温度阀值
				MainActivity.endDevInfo.shiduLimit=data[i++];//湿度阀值
				MainActivity.endDevInfo.lightLimit=data[i++];//光照阀值


			//2、终端2:接人体(门磁)、气体、温湿度和继电器。
			    MainActivity.endDevInfo.end2_people=data[i++]; //人体
			    MainActivity.endDevInfo.end2_mq2=data[i++]; //气体
			    MainActivity.endDevInfo.end2_temp=data[i++];  //温度
			    MainActivity.endDevInfo.end2_hum=data[i++];  //湿度
			    MainActivity.endDevInfo.end2_lamp=data[i++];  //继电器，即灯的状态

			//3、终端3：RFID刷卡系统,统计车库车辆进出情况。按5张卡算吧。
			    //RFID不用记录数据
			    //刷卡后直接上传
			    
			//4、终端4：步进电机,正转、反转、停止、加减速。
			    //没有数据存储

			//5、终端5: 接GPS，上传经纬度,时间，速度,自动上传，3秒1次。
				
				System.arraycopy(data, i, MainActivity.endDevInfo.gpsData, 0, 22);
				
				msg = MainActivity.mainHandler.obtainMessage(MainActivity.RX_DATA_UPDATE_UI,"Connect");
				MainActivity.mainHandler.sendMessage(msg);
			}
			break;
			case FUN_CODE_UPDATA_RFID://zigbee-->手机/PC  上传RFID数据
			{
				byte card_buff[]=new byte[8];
				String strBuff;
				
				System.arraycopy(data, 3, card_buff, 0, 8);

				msg = MainActivity.mainHandler.obtainMessage(MainActivity.RX_RFID_UPDATE_UI, new String(card_buff));
				MainActivity.mainHandler.sendMessage(msg);
			}
			break;
			case ZIGBEE_FUN_CODE_CTRL_LAMP://控制单个终端上灯的状态
			break;
			case ZIGBEE_FUN_CODE_STEP://控制终端上的步进电机
			break;
			case ZIGBEE_FUN_CODE_END1://终端1的终端码，设置终端的阀值和开关电机 
			break;
    	}
	}

	byte CheckSum(byte pdata[], byte len)
	{
		byte i;
		byte check_sum=0;

		for(i=0; i<len; i++)
		{
			check_sum += pdata[2+i];
		}
		return check_sum;
	}
	

}
