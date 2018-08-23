package com.android_wifi;

public class EndDeviceDataInfo{

//1、终端1：光敏、温湿度。阀值：光敏--开天窗晒太阳，温度过高--通风，湿度过低----浇水。
    byte end1_light; //光线
    byte end1_temp;  //温度
    byte end1_hum;  //湿度

	//终端1的阀值数据
    byte wenduLimit;//温度阀值
    byte shiduLimit;//湿度阀值
    byte lightLimit;//光照阀值


//2、终端2:接人体(门磁)、气体、温湿度和继电器。
    byte end2_people; //人体
    byte end2_mq2; //气体
    byte end2_temp;  //温度
    byte end2_hum;  //湿度
    byte end2_lamp;  //继电器，即灯的状态

//3、终端3：RFID刷卡系统,统计车库车辆进出情况。按5张卡算吧。
    //RFID不用记录数据
    //刷卡后直接上传
    
//4、终端4：步进电机,正转、反转、停止、加减速。
    //没有数据存储

//5、终端5: 接GPS，上传经纬度,时间，速度,自动上传，3秒1次。
    byte gpsData[];


	public EndDeviceDataInfo(){
		gpsData=new byte[22];
	}

	public int getSize()
	{
		//C++用sizeof，不知道这里怎么做
		return 30;
	}
}
