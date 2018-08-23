package com.android_wifi;

import java.util.Timer;
import java.util.TimerTask;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.android_wifi.R;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity {
	private String TAG = "zigbee_demo";
	
	//终端1
	private TextView textNode1, textLight1,textTemp1,textHumi1,tvPrompt;
	private EditText editTempLimit, editHumiLimit, editLightLimit;
	private Button btnLimit;
		
	//终端2
	private TextView  textNode2,textTemp2,textHumi2;
	private ImageView ivGas2,ivHuman2;
	private ImageButton btnLamp2;
	
	//终端3
	private TextView textNode3, textRfidCur,textRfidAll;
	
	//终端4
	private Button btnStep1, btnStep2,btnStep3,btnStep4,btnStep5;
	private TextView  textNode4;
	
	//终端5
	private TextView  textNode5, textGps1,textGps2,textGps3;
	
	//rfid信息
	final static int MAX_car=5;//总人数
	static CarInfo car[];

	//提示	
	static TextView textTips;


	
	private TextView  textTemp3, textTemp4,  textHumi3,
			textHumi4;
	private ImageButton btnLamp1, btnLamp3, btnLamp4;
	private ImageView ivGas1, ivGas3, ivGas4;
	private ImageView ivHuman1,ivHuman3, ivHuman4;
	private Button btnNetwork, btnExit, btnLampAll, btnScene;
	private Button btnFindCard, btnReadCard;
	static TextView textRfidCardType;
	
	//消息定义
	static final int RX_DATA_UPDATE_UI = 1;
	static final int RX_RFID_UPDATE_UI = 2;
	final int TX_DATA_UPDATE_UI = 3;
	static final int TIPS_UPDATE_UI = 4;

	final int READ_ALL_INFO = 5;
	final int WRITE_LAMP = 6;
	final int WRITE_Step1 = 7;
	final int WRITE_Step2 = 8;
	final int WRITE_Step3 = 9;
	final int WRITE_Step4 = 10;
	final int WRITE_Step5 = 11;
	final int WRITE_Limit = 12;
	static final int Start_timer = 13;
	
	public static Handler mainHandler;
	private ClientThread clientThread = null;
	private Timer mainTimer;

	byte SendBuf[] = { 0x3A, 0x00, 0x01, 0x0A, 0x00, 0x00, 0x23, 0x00 };
	private String strTemp, strHumi, strRfid;
	private Message MainMsg;
	final byte LAMP_ON = 1;
	final byte LAMP_OFF = 0;
	private byte LampAllState = LAMP_ON;

	static EndDeviceDataInfo  endDevInfo=null;
	
	final String RFID_ID1		="8645371F";
	final String RFID_NAME1		="苏A00001";
	final String RFID_ID2		="FD9553D3";
	final String RFID_NAME2		="苏A00002";
	final String RFID_ID3		="1E798821";
	final String RFID_NAME3		="苏A00003";
	final String RFID_ID4		="B13E8045";
	final String RFID_NAME4		="苏A00004";
	final String RFID_ID5		="65849B2C";
	final String RFID_NAME5		="苏A00005";
	
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		
		Log.i(TAG, "create");

		endDevInfo = new EndDeviceDataInfo();
		endDevInfo.wenduLimit=30;
		endDevInfo.shiduLimit=50;
		endDevInfo.lightLimit=40;

		
		car = new CarInfo[MAX_car];
		
		for(int i=0; i<MAX_car; i++){
			car[i]=new CarInfo();
		}

		if(true)
		{
			car[0].rfid=RFID_ID1;
			car[0].name=RFID_NAME1;

			car[1].rfid=RFID_ID2;
			car[1].name=RFID_NAME2;

			car[2].rfid=RFID_ID3;
			car[2].name=RFID_NAME3;

			car[3].rfid=RFID_ID4;
			car[3].name=RFID_NAME4;

			car[4].rfid=RFID_ID5;
			car[4].name=RFID_NAME5;
		}

		
		initControl();
		initMainHandler();
	}

	class ButtonClick implements OnClickListener {
		@Override
		public void onClick(View v) {
			if (clientThread == null
					&& (v.getId() != R.id.btn_exit) && (v.getId() != R.id.btn_network)) {
				textTips.setText("提示信息：请先连接网络");
				return;
			}
			
			switch (v.getId()) {
			case R.id.btn_network: //连接网络
				showDialog(MainActivity.this);
				break;

			case R.id.image_lamp2: //开终端2的灯
				MainMsg = mainHandler.obtainMessage(TX_DATA_UPDATE_UI,
						WRITE_LAMP, 2);
				mainHandler.sendMessage(MainMsg);
				break;

			case R.id.btn_exit: //退出系统
				if (clientThread != null) {
					MainMsg = ClientThread.childHandler
							.obtainMessage(ClientThread.RX_EXIT);
					ClientThread.childHandler.sendMessage(MainMsg);
				}

				finish();
				break;


			case R.id.btn_set_limit://设置阀值
				int limit=0;

				limit = Integer.parseInt(editTempLimit.getText().toString());//温度阀值

				if(!(limit>0 && limit<100))
				{
					Toast.makeText(MainActivity.this, "温度阀值为0~100!", Toast.LENGTH_SHORT).show();  
					break;
				}

				
				limit = Integer.parseInt(editHumiLimit.getText().toString());//湿度阀值
				if(!(limit>0 && limit<100))
				{
					Toast.makeText(MainActivity.this, "湿度阀值为0~100!", Toast.LENGTH_SHORT).show();  
					break;
				}
				
				limit = Integer.parseInt(editLightLimit.getText().toString());//光照阀值
				if(!(limit>0 && limit<100))
				{
					Toast.makeText(MainActivity.this, "光照阀值为0~100!", Toast.LENGTH_SHORT).show();  
					break;
				}
			
				MainMsg = mainHandler.obtainMessage(TX_DATA_UPDATE_UI,
						WRITE_Limit, 4);
				mainHandler.sendMessage(MainMsg);
				break;
				
			case R.id.btn_step1:
				MainMsg = mainHandler.obtainMessage(TX_DATA_UPDATE_UI,
						WRITE_Step1, 4);
				mainHandler.sendMessage(MainMsg);
				break;

			case R.id.btn_step2:
				MainMsg = mainHandler.obtainMessage(TX_DATA_UPDATE_UI,
						WRITE_Step2, 4);
				mainHandler.sendMessage(MainMsg);
				break;

			case R.id.btn_step3:
				MainMsg = mainHandler.obtainMessage(TX_DATA_UPDATE_UI,
						WRITE_Step3, 4);
				mainHandler.sendMessage(MainMsg);
				break;
			case R.id.btn_step4:
				MainMsg = mainHandler.obtainMessage(TX_DATA_UPDATE_UI,
						WRITE_Step4, 4);
				mainHandler.sendMessage(MainMsg);
				break;
			case R.id.btn_step5:
				MainMsg = mainHandler.obtainMessage(TX_DATA_UPDATE_UI,
						WRITE_Step5, 4);
				mainHandler.sendMessage(MainMsg);
				break;
			default:
				break;
			}
		}
	}

	//显示连接对话框
	private void showDialog(Context context) {
		LayoutInflater mInflater = (LayoutInflater) context  
		    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);  
		View view = mInflater.inflate(R.layout.recordlayout, null);  
		LinearLayout layout = (LinearLayout) view  
		    .findViewById(R.id.id_recordlayout);


	    TextView tv1 = new TextView(context);  
	    tv1.setText("IP:");  
	    final EditText editIP = new EditText(context); 
	    editIP.setText("192.168.1.99");  
	    layout.addView(tv1);  
	    layout.addView(editIP);  

	    TextView tv2 = new TextView(context);  
	    tv2.setText("端口:");
	    final EditText editPort = new EditText(context);  
		editPort.setText("15000");  
	    layout.addView(tv2);  
	    layout.addView(editPort);  	


		AlertDialog.Builder builder = new AlertDialog.Builder(context);
		builder.setTitle("服务器设置");
		builder.setView(view);
		builder.setPositiveButton("连接", new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int whichButton) {
				String strIpAddr = editIP.getText().toString();
				int iPort=Integer.parseInt(editPort.getText().toString());
				boolean ret = isIPAddress(strIpAddr);

				if (ret) {
					textTips.setText("IP地址:" + strIpAddr + ",端口:"+iPort);
				} else {
					textTips.setText("地址不合法，请重新设置");
					return;
				}

				clientThread = new ClientThread(strIpAddr, iPort);//建立客户端线程
				clientThread.start();
								
//				if(clientThread != null && clientThread.socketConnect()){
//					textTips.setText("start timer");
//					mainTimer = new Timer();//定时查询所有终端信息
//					setTimerTask();
//				}
			}
		});
		builder.setNeutralButton("取消", new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int whichButton) {
				if (clientThread != null) {
					MainMsg = ClientThread.childHandler
							.obtainMessage(ClientThread.RX_EXIT);
					ClientThread.childHandler.sendMessage(MainMsg);
					textTips.setText("与服务器断开连接");
				}
			}
		});

		builder.show();
	}
	
	private void setTimerTask() {
        mainTimer.schedule(new TimerTask() {
            @Override
            public void run() {
				if (clientThread != null) {
					MainMsg = mainHandler.obtainMessage(TX_DATA_UPDATE_UI,
							READ_ALL_INFO, 0xFF);
					mainHandler.sendMessage(MainMsg);
				}
            }
        }, 500, 1000);//表示500毫秒之后，每隔1000毫秒执行一次 
    }

	byte CheckSum(byte pdata[], byte len){
		byte i;
		byte check_sum=0;

		for(i=0; i<len; i++)
		{
			check_sum += pdata[2+i];
		}
		return check_sum;
	}

	void packDataAndSend(int fc, byte data[], int len){
		byte buff[]=new byte[250];
		int dataLen=0;
		int i=0;

	    //数据包长度
	    buff[0]=(byte) (3+len);

	    //功能码
	    buff[2]=(byte)fc;

	    //发送的数据
		for(i=0; i<len; i++){
			buff[3+i]=data[i];
		}
		
	    //校验和
	    buff[1]=CheckSum(buff, (byte)(len+1));

	    //发送长度
	    dataLen=3+len;

		//增加"\r\n"
	//	buff[dataLen]='\r';
	//	buff[dataLen++]='\n';
	    SendData(buff, dataLen);
	}
	
	//通知客户端线程 发送消息
	void SendData(byte buffer[], int len) {
		MainMsg = ClientThread.childHandler.obtainMessage(ClientThread.TX_DATA,
				len, 0, (Object) buffer);
		ClientThread.childHandler.sendMessage(MainMsg);
	}

	void initMainHandler() {
		mainHandler = new Handler() {

			//主线程消息处理中心
			public void handleMessage(Message msg) {
				switch (msg.what) {
				case Start_timer:
					mainTimer = new Timer();//定时查询所有终端信息
					setTimerTask();
					break;
				case RX_DATA_UPDATE_UI:

					
					System.out.println("RX_DATA_UPDATE_UI");
					//终端1				
					strTemp = "温度：" + endDevInfo.end1_temp + "℃";
					textTemp1.setText(strTemp);
					strTemp = "湿度：" + endDevInfo.end1_hum + "%";
					textHumi1.setText(strTemp);
					strTemp = "光照：" + endDevInfo.end1_light + "%";
					textLight1.setText(strTemp);

					//阀值设置
					strTemp = "阀值,温：" + endDevInfo.wenduLimit + "℃,湿："+endDevInfo.shiduLimit+"%,光："+endDevInfo.lightLimit+"%";
					tvPrompt.setText(strTemp);
					
					//editTempLimit.setText(Integer.toString(endDevInfo.wenduLimit));
					//editHumiLimit.setText(Integer.toString(endDevInfo.shiduLimit));
					//editLightLimit.setText(Integer.toString(endDevInfo.lightLimit));

					//终端2
					strTemp = "温度：" + endDevInfo.end2_temp + "℃";
					textTemp2.setText(strTemp);
					strTemp = "湿度：" + endDevInfo.end2_hum + "%";
					textHumi2.setText(strTemp);
					
					if (endDevInfo.end2_mq2>0)
						ivGas2.setImageResource(R.drawable.gas_off);   //气体高电平时正常
					else
						ivGas2.setImageResource(R.drawable.gas_on);    //气体低电平时异常

					if (endDevInfo.end2_people>0)
						ivHuman2.setImageResource(R.drawable.image_human_on);   //有人
					else
						ivHuman2.setImageResource(R.drawable.image_human_off);    //无人

					if (endDevInfo.end2_lamp>0)
						btnLamp2.setImageResource(R.drawable.lamp_on); //灯亮
					else
						btnLamp2.setImageResource(R.drawable.lamp_off); //灯灭

					//终端3
					//rfid


					//终端4
					//步进电机

					//终端5
					//GPS信息
					if(endDevInfo.gpsData[0]>0)
					{
						textGps1.setText("GPS已定位");    //是否定位

						strTemp = endDevInfo.gpsData[1]+"年"+
							endDevInfo.gpsData[2]+"月"+
							endDevInfo.gpsData[3]+"日   "+
							endDevInfo.gpsData[4]+"时"+
							endDevInfo.gpsData[5]+"分"+
							endDevInfo.gpsData[6]+"秒";    //时间

						textGps2.setText(strTemp); //时间

						//经度
						long longitude=endDevInfo.gpsData[11]*0xffffff+endDevInfo.gpsData[10]*0xffff+endDevInfo.gpsData[9]*0xff+endDevInfo.gpsData[8];
						if((endDevInfo.gpsData[7]&0x02)==0x02)
						{
							strTemp="东经:"+((float)longitude/10000000.0); //经度	
						}
						else
						{
							strTemp="西经:"+((float)longitude/10000000.0); //经度	
						}


						//纬度
						long latitude=endDevInfo.gpsData[15]*0xffffff+endDevInfo.gpsData[14]*0xffff+endDevInfo.gpsData[13]*0xff+endDevInfo.gpsData[12];
						if((endDevInfo.gpsData[7]&0x01)==0x01)
						{
							strTemp=strTemp+"   北纬:"+((float)latitude/10000000.0); //纬度
						}
						else
						{
							strTemp=strTemp+"   南纬:"+((float)latitude/10000000.0); //纬度
						}
						textGps3.setText(strTemp);

						//速度

					}
					else
					{
						textGps1.setText("gps 未定位!");    //是否定位
						textGps2.setText("GPS时间：--------"); //时间
						textGps3.setText("经度:--------   纬度:-------- ");
					}					
					
					break;

				case RX_RFID_UPDATE_UI:
					{
						String id = (String) msg.obj;
						String strBuff;
						int count=0;
						int i=0;
						boolean isFound=false;

					//	textTips.setText(id+","+car[0].rfid+","+car[1].rfid+","+car[2].rfid+","+car[3].rfid+","+car[4].rfid);
												
						for( i=0; i<MAX_car; i++)
						{
							//if(stringCompate(id, car[i].rfid))
							if(id.equals(car[i].rfid))
							{
								isFound=true;
								
								if(car[i].isUse)
								{
									car[i].isUse=false;
									strBuff=car[i].name+"离开车库，一路顺风!";
									textRfidCur.setText(strBuff);
								}
								else
								{
									car[i].isUse=true;
									strBuff=car[i].name+"进入车库，安全着陆!";
									textRfidCur.setText(strBuff);
								}
							}							
						}						
						
						//不识别的卡。
						if(!isFound){
							textRfidCur.setText("找不到这辆车!");
						}

						for( i=0; i<MAX_car; i++)
						{
							if(car[i].isUse) count++;
						}

						if(count>0)
						{
							int iCount=0;

							strBuff="车库现在有"+count+"辆车，分别为：";
						
							for(i=0; i<MAX_car; i++)
							{
								if(car[i].isUse)
								{	
									//格式化日志的数据
									if(i==MAX_car-1)
									{
										strBuff=strBuff+car[i].name+".";
									}
									else
									{
										strBuff=strBuff+car[i].name+",";
									}			
								}
							}
						}
						else
						{
							strBuff="当前没有车停在车库。";
							
						}

						textRfidAll.setText(strBuff);
					}
					break;
				case TX_DATA_UPDATE_UI: //msg.arg1保存功能码 arg2保存终端地址
					switch (msg.arg1) {
					case READ_ALL_INFO:  //读所有信息
						packDataAndSend(ClientThread.FUN_CODE_CHECK_ALL_DATA, SendBuf, 0);
						break;
						
					case WRITE_LAMP: //控制灯
						SendBuf[0] = (byte) msg.arg2; //几号终端的灯，在这里是2号

						if (endDevInfo.end2_lamp>0){
							SendBuf[1] = 0; //灯亮
						}else{
							SendBuf[1] = 1;
						}

						packDataAndSend(ClientThread.ZIGBEE_FUN_CODE_CTRL_LAMP, SendBuf, 2);
						break;
						
					case WRITE_Limit://阀值
						SendBuf[0] = 1;
						SendBuf[1] = (byte) Integer.parseInt(editTempLimit.getText().toString());//温度阀值
						SendBuf[2] = (byte) Integer.parseInt(editHumiLimit.getText().toString());//湿度阀值
						SendBuf[3] = (byte) Integer.parseInt(editLightLimit.getText().toString());//光照阀值
						packDataAndSend(ClientThread.ZIGBEE_FUN_CODE_END1, SendBuf, 4);
						break;

					case WRITE_Step1://正转
						SendBuf[0] = 4;
						SendBuf[1] = 1;
						packDataAndSend(ClientThread.ZIGBEE_FUN_CODE_STEP, SendBuf, 2);
						break;

					case WRITE_Step2://反转
						SendBuf[0] = 4;
						SendBuf[1] = 2;
						packDataAndSend(ClientThread.ZIGBEE_FUN_CODE_STEP, SendBuf, 2);
						break;

					case WRITE_Step3://加速
						SendBuf[0] = 4;
						SendBuf[1] = 3;
						packDataAndSend(ClientThread.ZIGBEE_FUN_CODE_STEP, SendBuf, 2);
						break;

					case WRITE_Step4://减速
						SendBuf[0] = 4;
						SendBuf[1] = 4;
						packDataAndSend(ClientThread.ZIGBEE_FUN_CODE_STEP, SendBuf, 2);
						break;

					case WRITE_Step5://停止
						SendBuf[0] = 4;
						SendBuf[1] = 5;
						packDataAndSend(ClientThread.ZIGBEE_FUN_CODE_STEP, SendBuf, 2);
						break;
						
					default:
						break;
					}
					break;
				case TIPS_UPDATE_UI:
					String str = (String) msg.obj;
					textTips.setText(str);
					break;
				}
				super.handleMessage(msg);
			}
		};
	}

	void initControl() {
		
		//----------------------node 1----------------------
		textNode1 = (TextView) findViewById(R.id.node_title1);
		textNode1.setBackgroundResource(R.drawable.node1);
		btnLimit=(Button) findViewById(R.id.btn_set_limit);
		btnLimit.setOnClickListener(new ButtonClick());

		textTemp1 = (TextView) findViewById(R.id.temp1);
		textTemp1.setText(R.string.init_temp);
		textHumi1 = (TextView) findViewById(R.id.humi1);
		textHumi1.setText(R.string.init_humi);
		textLight1 = (TextView) findViewById(R.id.light1);
		textLight1.setText(R.string.init_light);

		editTempLimit = (EditText) findViewById(R.id.edit_temp_limit);
		editTempLimit.setText("30");
		
		editHumiLimit = (EditText) findViewById(R.id.edit_humi_limit);
		editHumiLimit.setText("50");
		
		editLightLimit = (EditText) findViewById(R.id.edit_light_limit);
		editLightLimit.setText("40");
		
		//阀值
		tvPrompt = (TextView) findViewById(R.id.textview_prompt);
		strTemp = "阀值,温：" + endDevInfo.wenduLimit + "℃,湿："+endDevInfo.shiduLimit+"%,光："+endDevInfo.lightLimit+"%";
		tvPrompt.setText(strTemp);
		
		
		
		
		
		//----------------------node 2----------------------
		textNode2 = (TextView) findViewById(R.id.node_title2);
		textNode2.setBackgroundResource(R.drawable.node2);
		textTemp2 = (TextView) findViewById(R.id.temperature2);
		textTemp2.setText(R.string.init_temp);
		textHumi2 = (TextView) findViewById(R.id.humidity2);
		textHumi2.setText(R.string.init_humi);
		ivHuman2 = (ImageView) findViewById(R.id.image_human2);
		ivHuman2.setImageResource(R.drawable.image_human_off);
		ivGas2 = (ImageView) findViewById(R.id.image_gas2);
		ivGas2.setImageResource(R.drawable.gas_off);
		btnLamp2 = (ImageButton) findViewById(R.id.image_lamp2);
		btnLamp2.setImageResource(R.drawable.lamp_off);
		btnLamp2.setOnClickListener(new ButtonClick());
		
		//----------------------终端3 rfid----------------------
		textNode3 = (TextView) findViewById(R.id.node_title3);
		textNode3.setBackgroundResource(R.drawable.node3);		
		textRfidCur = (TextView) findViewById(R.id.end3_rfid_cur);
		textRfidCur.setText("当前没有车进出车库");

		textRfidAll = (TextView) findViewById(R.id.end3_rfid_all);
		textRfidAll.setText("当前没有车停在车库。");
		
		
		//----------------------终端4 步进电机----------------------
		textNode4 = (TextView) findViewById(R.id.node_title4);
		textNode4.setBackgroundResource(R.drawable.node4);
		btnStep1 = (Button) findViewById(R.id.btn_step1);
		btnStep2 = (Button) findViewById(R.id.btn_step2);
		btnStep3 = (Button) findViewById(R.id.btn_step3);
		btnStep4 = (Button) findViewById(R.id.btn_step4);		
		btnStep5 = (Button) findViewById(R.id.btn_step5);		

		btnStep1.setOnClickListener(new ButtonClick());
		btnStep2.setOnClickListener(new ButtonClick());
		btnStep3.setOnClickListener(new ButtonClick());
		btnStep4.setOnClickListener(new ButtonClick());		
		btnStep5.setOnClickListener(new ButtonClick());
		
		
		//----------------------终端5 GPS----------------------
/*		textNode5 = (TextView) findViewById(R.id.node_title5);
		textNode5.setBackgroundResource(R.drawable.node5);
		
		textGps1 = (TextView) findViewById(R.id.text_gps1);
		textGps1.setText("gps 未定位!");

		textGps2 = (TextView) findViewById(R.id.text_gps2);
		textGps2.setText("GPS时间：");

		textGps3 = (TextView) findViewById(R.id.text_gps3);
		textGps3.setText("经度:  纬度:");*/

		
		//提示
		textTips = (TextView) findViewById(R.id.Tips);
		textTips.setText(R.string.init_tips);


		//网络设置
		btnNetwork = (Button) findViewById(R.id.btn_network);
		btnNetwork.setOnClickListener(new ButtonClick());

		//退出
		btnExit = (Button) findViewById(R.id.btn_exit);
		btnExit.setOnClickListener(new ButtonClick());

		
		/*
		//----------------------node 3----------------------
		textNode3 = (TextView) findViewById(R.id.node_title3);
		textNode3.setBackgroundResource(R.drawable.node3);
		textTemp3 = (TextView) findViewById(R.id.temperature3);
		textTemp3.setText(R.string.init_temp);
		textHumi3 = (TextView) findViewById(R.id.humidity3);
		textHumi3.setText(R.string.init_humi);
		ivHuman3 = (ImageView) findViewById(R.id.image_human3);
		ivHuman3.setImageResource(R.drawable.image_human_off);
		ivGas3 = (ImageView) findViewById(R.id.image_gas3);
		ivGas3.setImageResource(R.drawable.gas_off);
		btnLamp3 = (ImageButton) findViewById(R.id.image_lamp3);
		btnLamp3.setImageResource(R.drawable.lamp_off);

		
		//----------------------node 4----------------------
		textNode4 = (TextView) findViewById(R.id.node_title4);
		textNode4.setBackgroundResource(R.drawable.node4);

		textTemp4 = (TextView) findViewById(R.id.temperature4);
		textTemp4.setText(R.string.init_temp);
		textHumi4 = (TextView) findViewById(R.id.humidity4);
		textHumi4.setText(R.string.init_humi);
		ivHuman4 = (ImageView) findViewById(R.id.image_human4);
		ivHuman4.setImageResource(R.drawable.image_human_off);
		ivGas4 = (ImageView) findViewById(R.id.image_gas4);
		ivGas4.setImageResource(R.drawable.gas_off);
		btnLamp4 = (ImageButton) findViewById(R.id.image_lamp4);
		btnLamp4.setImageResource(R.drawable.lamp_off);

		btnLampAll = (Button) findViewById(R.id.btn_lamp_all);
		btnLampAll.setOnClickListener(new ButtonClick());

//		btnScene = (Button) findViewById(R.id.btn_scenes);
//		btnScene.setOnClickListener(new ButtonClick());

//		btnLamp1.setOnClickListener(new ButtonClick());
		btnLamp2.setOnClickListener(new ButtonClick());
		btnLamp3.setOnClickListener(new ButtonClick());
		btnLamp4.setOnClickListener(new ButtonClick());
		
		btnFindCard = (Button) findViewById(R.id.btn_fine_card);
		btnReadCard = (Button) findViewById(R.id.btn_read_card);
		btnStep1 = (Button) findViewById(R.id.btn_step1);
		btnStep2 = (Button) findViewById(R.id.btn_step2);
		btnStep3 = (Button) findViewById(R.id.btn_step3);
		btnStep4 = (Button) findViewById(R.id.btn_step4);		

		btnFindCard.setOnClickListener(new ButtonClick());
		btnReadCard.setOnClickListener(new ButtonClick());
		btnStep1.setOnClickListener(new ButtonClick());
		btnStep2.setOnClickListener(new ButtonClick());
		btnStep3.setOnClickListener(new ButtonClick());
		btnStep4.setOnClickListener(new ButtonClick());

		textRfidCardType = (TextView) findViewById(R.id.text_card_type);
		textRfidCardType.setText(R.string.rfid_init_prompt);
*/
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
		mainTimer.cancel();
	}

	//判断输入IP是否合法
	private boolean isIPAddress(String ipaddr) {
		boolean flag = false;
		Pattern pattern = Pattern
				.compile("\\b((?!\\d\\d\\d)\\d+|1\\d\\d|2[0-4]\\d|25[0-5])\\.((?!\\d\\d\\d)\\d+|1\\d\\d|2[0-4]\\d|25[0-5])\\.((?!\\d\\d\\d)\\d+|1\\d\\d|2[0-4]\\d|25[0-5])\\.((?!\\d\\d\\d)\\d+|1\\d\\d|2[0-4]\\d|25[0-5])\\b");
		Matcher m = pattern.matcher(ipaddr);
		flag = m.matches();
		return flag;
	}

	byte XorCheckSum(byte[] pBuf, int len) {
		int i;
		byte byRet = 0;

		if (len == 0)
			return byRet;
		else
			byRet = pBuf[0];

		for (i = 1; i < len; i++)
			byRet = (byte) (byRet ^ pBuf[i]);

		return byRet;
	}

	boolean stringCompate(String str1, String str2)
	{
		if(TextUtils.isEmpty(str1)){
			textTips.setText("aa");
			return false;
		}
		
		if(TextUtils.isEmpty(str2)){
			textTips.setText("bb");
			return false;
		}

		int str1len=str1.length();
		int str2len=str2.length();
	
//		textTips.setText("str1len="+str1len+",str2len="+str2len);
		if(str1len!=str2len){
			textTips.setText("cc");
			return false;
		}
		
		byte[] str1byte = str1.getBytes(); 
		byte[] str2byte = str2.getBytes(); 
		
		for(int i=0; i<str1len; i++)
		{
			if(str1byte[i]!=str2byte[i]){
				textTips.setText("dd");
				return false;
			}
		}
		
		return true;
	}

}
