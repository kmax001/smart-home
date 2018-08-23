/**************************************************************************************************
  Filename:       SampleApp.h
  Revised:        $Date: 2007-10-27 17:22:23 -0700 (Sat, 27 Oct 2007) $
  Revision:       $Revision: 15795 $

  Description:    This file contains the Sample Application definitions.


  Copyright 2007 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, 
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE, 
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com. 
**************************************************************************************************/

#ifndef SAMPLEAPP_H
#define SAMPLEAPP_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"

/*********************************************************************
 * CONSTANTS
 */

// These constants are only for example and should be changed to the
// device's needs
#define SAMPLEAPP_ENDPOINT           20

#define SAMPLEAPP_PROFID             0x0F08
#define SAMPLEAPP_DEVICEID           0x0001
#define SAMPLEAPP_DEVICE_VERSION     0
#define SAMPLEAPP_FLAGS              0

#define SAMPLEAPP_MAX_CLUSTERS       6
#define SERIALAPP_CONNECTREQ_CLUSTER 1
#define SAMPLEAPP_END1 2
#define SAMPLEAPP_END2 3
#define SAMPLEAPP_END3 4
#define SAMPLEAPP_END4 5
#define SAMPLEAPP_END5 6
  
// Send Message Timeout
#define SAMPLEAPP_SEND_PERIODIC_MSG_TIMEOUT   2000     // Every 3 seconds

// Application Events (OSAL) - These are bit weighted definitions.
#define SAMPLEAPP_SEND_PERIODIC_MSG_EVT       0x0001
  
// Group ID for Flash Command
#define SAMPLEAPP_FLASH_GROUP                 0x0001
  
// Flash Command Duration - in milliseconds
#define SAMPLEAPP_FLASH_DURATION              1000


/*********************************************************************
 * MACROS
 */

//终端数
#define MAX_DEVICE  		6

//功能码定义
#define FUN_CODE_CHECK_ALL_DATA				0x01	//手机/PC-->zigbee 查询所有传感器数据
#define FUN_CODE_UPDATA_ALL_DATA				0x02	//zigbee-->手机/PC 上传所有传感器数据
#define FUN_CODE_UPDATA_RFID				0x03	//zigbee-->手机/PC  上传RFID数据
#define ZIGBEE_FUN_CODE_CTRL_LAMP				0x04	//控制单个终端上灯的状态
#define ZIGBEE_FUN_CODE_STEP				0x05	//控制终端上的步进电机
#define ZIGBEE_FUN_CODE_END1                0x06  //终端1的终端码，设置终端的阀值和开关电机 

//
typedef struct _device_data_info_
{
//1、终端1：光敏、温湿度。阀值：光敏--开天窗晒太阳，温度过高--通风，湿度过低----浇水。
    uint8 end1_light; //光线
    uint8 end1_temp;  //温度
    uint8 end1_hum;  //湿度

//终端1的阀值数据
    uint8 wenduLimit;//温度阀值
    uint8 shiduLimit;//湿度阀值
    uint8 lightLimit;//光照阀值


//2、终端2:接人体(门磁)、气体、温湿度和继电器。
    uint8 end2_people; //人体
    uint8 end2_mq2; //气体
    uint8 end2_temp;  //温度
    uint8 end2_hum;  //湿度
    uint8 end2_lamp;  //继电器，即灯的状态

//3、终端3(模块图书馆人员管理)：RFID刷卡系统,统计图书馆人员情况进出情况。按5张卡算吧。
    //RFID不用记录数据
    //刷卡后直接上传
    
//4、终端4：步进电机,正转、反转、停止、加减速。
    //没有数据存储

//5、终端5: 接GPS，上传经纬度,时间，速度,自动上传，3秒1次。
    uint8 gpsData[22];
}DeviceDataInfo;


/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the Generic Application
 */
extern void SampleApp_Init( uint8 task_id );

/*
 * Task Event Processor for the Generic Application
 */
extern UINT16 SampleApp_ProcessEvent( uint8 task_id, uint16 events );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SAMPLEAPP_H */
