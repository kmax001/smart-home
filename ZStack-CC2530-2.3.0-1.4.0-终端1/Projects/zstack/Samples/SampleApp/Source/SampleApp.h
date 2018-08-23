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


//功能码定义
#define ZIGBEE_FUN_CODE_CHECK_LAMP				0x01	//查询/响应单个终端上灯的状态
#define ZIGBEE_FUN_CODE_CTRL_LAMP				0x02	//控制/响应单个终端上灯的状态
#define ZIGBEE_FUN_CODE_CHECK_Smoke				0x03	//查询/响应单个终端上烟雾传感器
#define ZIGBEE_FUN_CODE_CHECK_HUMAN				0x04	//查询/响应单个终端上人体感应模块
#define ZIGBEE_FUN_CODE_CHECK_LIGHT				0x05	//查询/响应单个终端上光敏传感器
#define ZIGBEE_FUN_CODE_CHECK_WindowCurtains		0x06      //查询/响应单个终端上电动窗帘
#define ZIGBEE_FUN_CODE_CTRL_WindowCurtains		0x07	//控制/响应单个终端上电动窗帘
#define ZIGBEE_FUN_CODE_CHECK_RFID				0x08	//查询/响应RFID上报
#define ZIGBEE_FUN_CODE_CHECK_TEMP_HUM			0x09	//查询/响应单个终端上温度和湿度
#define ZIGBEE_FUN_CODE_CHECK_Flame				0x0a	//查询单个终端上火焰传感器状态
#define ZIGBEE_FUN_CODE_STEP				0x0b	//控制/响应终端上的步进电机
#define ZIGBEE_FUN_CODE_RFID_FINDCARD			0x0c   //寻卡/响应终端上的RFID
#define ZIGBEE_FUN_CODE_RFID_Conflict			0x0d   //防冲突/响应终端上的RFID



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
