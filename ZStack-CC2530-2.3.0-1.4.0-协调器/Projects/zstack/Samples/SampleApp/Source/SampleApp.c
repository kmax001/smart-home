/**************************************************************************************************
  Filename:       SampleApp.c
  Revised:        $Date: 2009-03-18 15:56:27 -0700 (Wed, 18 Mar 2009) $
  Revision:       $Revision: 19453 $

  Description:    Sample Application (no Profile).


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

/*********************************************************************
  This application isn't intended to do anything useful, it is
  intended to be a simple example of an application's structure.

  This application sends it's messages either as broadcast or
  broadcast filtered group messages.  The other (more normal)
  message addressing is unicast.  Most of the other sample
  applications are written to support the unicast message model.

  Key control:
    SW1:  Sends a flash command to all devices in Group 1.
    SW2:  Adds/Removes (toggles) this device in and out
          of Group 1.  This will enable and disable the
          reception of the flash command.
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "OSAL.h"
#include "ZGlobals.h"
#include "AF.h"
#include "aps_groups.h"
#include "ZDApp.h"

#include "SampleApp.h"
#include "SampleAppHw.h"

#include "OnBoard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "MT_UART.h"
#include "MT_APP.h"
#include "MT.h"
#include "stdio.h"

/*********************************************************************
 * MACROS
 */


#if !defined( SERIAL_APP_PORT )
#define SERIAL_APP_PORT  0
#endif



//接叫串口数据缓冲
#define SERIAL_APP_RX_MAX  80
static uint8 SerialApp_RX_Buff[SERIAL_APP_RX_MAX+1];
static uint8 SerialApp_RX_Len;

//解析串口数据的缓冲
#define SERIAL_APP_PARSE_MAX  80
static uint8 SerialApp_PARSE_Buff[SERIAL_APP_PARSE_MAX+1];
static uint8 SerialApp_PARSE_Len;

//发送的数据长度
#define SERIAL_APP_TX_MAX  80
static uint8 SerialApp_TX_Buff[SERIAL_APP_TX_MAX+1];
static uint8 SerialApp_TX_Len;


/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// This list should be filled with Application specific Cluster IDs.
const cId_t SampleApp_ClusterList[SAMPLEAPP_MAX_CLUSTERS] =
{
  SERIALAPP_CONNECTREQ_CLUSTER,//用于上传终端短地址
  SAMPLEAPP_END1,
  SAMPLEAPP_END2,
  SAMPLEAPP_END3,
  SAMPLEAPP_END4,
  SAMPLEAPP_END5
};

const SimpleDescriptionFormat_t SampleApp_SimpleDesc =
{
  SAMPLEAPP_ENDPOINT,              //  int Endpoint;
  SAMPLEAPP_PROFID,                //  uint16 AppProfId[2];
  SAMPLEAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  SAMPLEAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  SAMPLEAPP_FLAGS,                 //  int   AppFlags:4;
  SAMPLEAPP_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
  (cId_t *)SampleApp_ClusterList,  //  uint8 *pAppInClusterList;
  SAMPLEAPP_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
  (cId_t *)SampleApp_ClusterList   //  uint8 *pAppInClusterList;
};

// This is the Endpoint/Interface description.  It is defined here, but
// filled-in in SampleApp_Init().  Another way to go would be to fill
// in the structure here and make it a "const" (in code space).  The
// way it's defined in this sample app it is define in RAM.
endPointDesc_t SampleApp_epDesc;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
uint8 SampleApp_TaskID;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // SampleApp_Init() is called.
devStates_t SampleApp_NwkState;

uint8 SampleApp_TransID;  // This is the unique message ID (counter)

afAddrType_t SampleApp_Periodic_DstAddr; //广播
afAddrType_t SampleApp_Flash_DstAddr;    //组播
afAddrType_t SampleApp_P2P_DstAddr;      //点播

aps_Group_t SampleApp_Group;

uint8 SampleAppPeriodicCounter = 0;
uint8 SampleAppFlashCounter = 0;


//协调器保存所有终端的数据
DeviceDataInfo  endDevInfo;
uint16 endDevAddr[MAX_DEVICE]={0};



/*********************************************************************
 * LOCAL FUNCTIONS
 */
void SampleApp_HandleKeys( uint8 shift, uint8 keys );
void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void SampleApp_SendPeriodicMessage( void );
void SerialApp_CallBack(uint8 port, uint8 event);
void unpackData(uint8* data, uint8 len);
void ParseframeData(uint8* data, uint8 len);
void packDataAndSend(uint8 fc, uint8* data, uint8 len);
void sendDataToEnd(uint16 shortAddr, uint16 clusterId, uint8* data, uint8 len);

/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SampleApp_Init
 *
 * @brief   Initialization function for the Generic App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void SampleApp_Init( uint8 task_id )
{ 
  SampleApp_TaskID = task_id;
  SampleApp_NwkState = DEV_INIT;
  SampleApp_TransID = 0;
  
  MT_UartInit();                  //串口初始化
  MT_UartRegisterTaskID(task_id); //注册串口任务
    HalUARTWrite(0,"SampleApp_Init", 14);

  
  // Device hardware initialization can be added here or in main() (Zmain.c).
  // If the hardware is application specific - add it here.
  // If the hardware is other parts of the device add it in main().

 #if defined ( BUILD_ALL_DEVICES )
  // The "Demo" target is setup to have BUILD_ALL_DEVICES and HOLD_AUTO_START
  // We are looking at a jumper (defined in SampleAppHw.c) to be jumpered
  // together - if they are - we will start up a coordinator. Otherwise,
  // the device will start as a router.
  if ( readCoordinatorJumper() )
    zgDeviceLogicalType = ZG_DEVICETYPE_COORDINATOR;dd
  else
    zgDeviceLogicalType = ZG_DEVICETYPE_ROUTER;
#endif // BUILD_ALL_DEVICES

#if defined ( HOLD_AUTO_START )
  // HOLD_AUTO_START is a compile option that will surpress ZDApp
  //  from starting the device and wait for the application to
  //  start the device.
  ZDOInitDevice(0);ee
#endif

  // Setup for the periodic message's destination address
  // Broadcast to everyone
  SampleApp_Periodic_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast;
  SampleApp_Periodic_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_Periodic_DstAddr.addr.shortAddr = 0xFFFF;

  // Setup for the flash command's destination address - Group 1
  SampleApp_Flash_DstAddr.addrMode = (afAddrMode_t)afAddrGroup;
  SampleApp_Flash_DstAddr.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_Flash_DstAddr.addr.shortAddr = SAMPLEAPP_FLASH_GROUP;
  
  SampleApp_P2P_DstAddr.addrMode = (afAddrMode_t)Addr16Bit; //点播 
  SampleApp_P2P_DstAddr.endPoint = SAMPLEAPP_ENDPOINT; 
  SampleApp_P2P_DstAddr.addr.shortAddr = 0x0000;            //发给协调器

  // Fill out the endpoint description.
  SampleApp_epDesc.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_epDesc.task_id = &SampleApp_TaskID;
  SampleApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&SampleApp_SimpleDesc;
  SampleApp_epDesc.latencyReq = noLatencyReqs;

  // Register the endpoint description with the AF
  afRegister( &SampleApp_epDesc );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( SampleApp_TaskID );

  // By default, all devices start out in Group 1
  SampleApp_Group.ID = 0x0001;
  osal_memcpy( SampleApp_Group.name, "Group 1", 7 );
  aps_AddGroup( SAMPLEAPP_ENDPOINT, &SampleApp_Group );

#if defined ( LCD_SUPPORTED )
  HalLcdWriteString( "SampleApp", HAL_LCD_LINE_1 );
#endif
}

/*********************************************************************
 * @fn      SampleApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */
uint16 SampleApp_ProcessEvent( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SampleApp_TaskID );
    while ( MSGpkt )
    {
      switch ( MSGpkt->hdr.event )
      {
        // Received when a key is pressed
        case KEY_CHANGE:
          SampleApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        // Received when a messages is received (OTA) for this endpoint
        case AF_INCOMING_MSG_CMD:
          SampleApp_MessageMSGCB( MSGpkt );
          break;

        // Received whenever the device changes state in the network
        case ZDO_STATE_CHANGE:
          SampleApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          if ( //(SampleApp_NwkState == DEV_ZB_COORD) ||
                 (SampleApp_NwkState == DEV_ROUTER)
              || (SampleApp_NwkState == DEV_END_DEVICE) )
          {
          }
          else
          {
            // Device is no longer in the network
          }
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );

      // Next - if one is available
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SampleApp_TaskID );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  // Send a message out - This event is generated by a timer
  //  (setup in SampleApp_Init()).
  if ( events & SAMPLEAPP_SEND_PERIODIC_MSG_EVT )
  {

    packDataAndSend(FUN_CODE_UPDATA_ALL_DATA, (uint8*)&endDevInfo, sizeof(DeviceDataInfo));
  
    osal_start_timerEx( SampleApp_TaskID,
                  SAMPLEAPP_SEND_PERIODIC_MSG_EVT,
                  SAMPLEAPP_SEND_PERIODIC_MSG_TIMEOUT );

    
    // return unprocessed events
    return (events ^ SAMPLEAPP_SEND_PERIODIC_MSG_EVT);
  }

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * Event Generation Functions
 */
/*********************************************************************
 * @fn      SampleApp_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
void SampleApp_HandleKeys( uint8 shift, uint8 keys )
{
  (void)shift;  // Intentionally unreferenced parameter
  
  if ( keys & HAL_KEY_SW_1 )
  {
    /* This key sends the Flash Command is sent to Group 1.
     * This device will not receive the Flash Command from this
     * device (even if it belongs to group 1).
     */
        packDataAndSend(FUN_CODE_UPDATA_RFID, "7CCC8204", 8);
  }

  if ( keys & HAL_KEY_SW_6 )
  {
    /* The Flashr Command is sent to Group 1.
     * This key toggles this device in and out of group 1.
     * If this device doesn't belong to group 1, this application
     * will not receive the Flash command sent to group 1.
     */
    int i;
    
    endDevInfo.end1_light=1;
    endDevInfo.end1_temp=2;
    endDevInfo.end1_hum=3;


    endDevInfo.end2_people=4;
    endDevInfo.end2_mq2=5;
    endDevInfo.end2_temp=6;
    endDevInfo.end2_hum=7;
    endDevInfo.end2_lamp=8;

    for(i=0; i<22; i++)
    {
        endDevInfo.gpsData[i]=9+i;
    }

    packDataAndSend(FUN_CODE_UPDATA_ALL_DATA, (uint8*)&endDevInfo, 30);

  }
}



/*********************************************************************
 * @fn      SampleApp_MessageMSGCB
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   none
 *
 * @return  none
 */
void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
    uint16 flashTime;

    switch ( pkt->clusterId )
    {
        case SERIALAPP_CONNECTREQ_CLUSTER://保存终端的短地址
            uint8 addr=pkt->cmd.Data[0]-1;
            if(addr>=MAX_DEVICE) break;
            endDevAddr[addr]=BUILD_UINT16(pkt->cmd.Data[2], pkt->cmd.Data[1]);            
        break;
        case SAMPLEAPP_END1:
            endDevInfo.end1_temp=pkt->cmd.Data[1];//温度
            endDevInfo.end1_hum=pkt->cmd.Data[2];//湿度
            endDevInfo.end1_light=pkt->cmd.Data[3];  //光照              

            endDevInfo.wenduLimit=pkt->cmd.Data[4];//温度阀值
            endDevInfo.shiduLimit=pkt->cmd.Data[5];//湿度阀值
            endDevInfo.lightLimit=pkt->cmd.Data[6];//光照阀值            
        break;
        case SAMPLEAPP_END2:
            endDevInfo.end2_temp=pkt->cmd.Data[1]; //温度
            endDevInfo.end2_hum=pkt->cmd.Data[2]; //湿度
            endDevInfo.end2_mq2=pkt->cmd.Data[3];//气体
            endDevInfo.end2_people=pkt->cmd.Data[4]; //人体
            endDevInfo.end2_lamp=pkt->cmd.Data[5];//继电器，即灯的状态
        break;
        case SAMPLEAPP_END3:
            //RFID上传不保存，直接上传手机或者PC
            packDataAndSend(FUN_CODE_UPDATA_RFID, pkt->cmd.Data, pkt->cmd.DataLength);
        break;
        case SAMPLEAPP_END4:
            //步进电机没有数据上传
        break;
        case SAMPLEAPP_END5:
            osal_memcpy(endDevInfo.gpsData, pkt->cmd.Data,22);//终端上传的GPS共22个字节
        break;
    }
}

uint8 CheckSum(uint8 *pdata, uint8 len)
{
	uint8 i;
	uint8 check_sum=0;

	for(i=0; i<len; i++)
	{
		check_sum += pdata[i];
	}
	return check_sum;
}

//数据打包发送
/**
*fc:功能码
*data:上传的数据
*len:数据长度
*/
void packDataAndSend(uint8 fc, uint8* data, uint8 len)
{
    osal_memset(SerialApp_TX_Buff, 0, SERIAL_APP_TX_MAX+1);

    //数据包长度
    SerialApp_TX_Buff[0]=3+len;

    //功能码
    SerialApp_TX_Buff[2]=fc;

    //发送的数据
    if(len>0)
    {
        osal_memcpy(SerialApp_TX_Buff+3, data, len);
    }

    //校验和
    SerialApp_TX_Buff[1]=CheckSum(SerialApp_TX_Buff+2, len+1);

    //发送长度
    SerialApp_TX_Len=3+len;
    HalUARTWrite(0,SerialApp_TX_Buff, SerialApp_TX_Len);
 //   HalUARTWrite(0,"\r\n", 2);//数据包结尾
}

void SerialApp_CallBack(uint8 port, uint8 event)
{
    (void)port;

    if(port==SERIAL_APP_PORT)//串口0数据
    {
        osal_memset(SerialApp_RX_Buff, 0, SERIAL_APP_RX_MAX+1);
        SerialApp_RX_Len = HalUARTRead(SERIAL_APP_PORT, SerialApp_RX_Buff, 
        SERIAL_APP_RX_MAX);
        if(SerialApp_RX_Len>0)
        {
            unpackData(SerialApp_RX_Buff, SerialApp_RX_Len);
        }
    }
}

//找出一帧数据
void unpackData(uint8* data, uint8 len)
{
    int16 i=0;
    if(data==NULL) return;
    if(len==0) return;

    ParseframeData(data, len);
    return;

    
	for(i=0; i<len; i++)
	{
		if(data[i]=='\r')
		{
			if(SerialApp_PARSE_Len>0) ParseframeData(SerialApp_PARSE_Buff, SerialApp_PARSE_Len);

			memset(SerialApp_PARSE_Buff, 0, SERIAL_APP_PARSE_MAX+1);
			SerialApp_PARSE_Len=0;
		}
		else if(data[i]=='\n')
		{
			if(SerialApp_PARSE_Len>0) ParseframeData(SerialApp_PARSE_Buff, SerialApp_PARSE_Len);
			
			memset(SerialApp_PARSE_Buff, 0, SERIAL_APP_PARSE_MAX+1);
			SerialApp_PARSE_Len=0;
		}
		else
		{
			SerialApp_PARSE_Buff[SerialApp_PARSE_Len]=data[i];
			SerialApp_PARSE_Len++;	
		}
	}    
}

void ParseframeData(uint8* data, uint8 len)
{
    uint8 dataLen=0;
    uint8 fc=0;
    uint8 addr=0;
    
    if(data==NULL) return;
    if(len==0) return;

    //长度不一致
    if(data[0]!=len) return;

    //校验和不一致
    dataLen=data[0]-2;
    if(data[1]!=CheckSum(&data[2], dataLen)) return;

    //功能码
    fc=data[2];

    //分发处理
    switch(fc)
    {
        case FUN_CODE_CHECK_ALL_DATA://取所有传感器数据
            packDataAndSend(FUN_CODE_UPDATA_ALL_DATA, (uint8*)&endDevInfo, sizeof(DeviceDataInfo));
            break;

        case ZIGBEE_FUN_CODE_CTRL_LAMP://控制终端上灯,
            addr=data[3];
            if(addr>MAX_DEVICE) break;
            if(addr==0) break;
            
            sendDataToEnd(endDevAddr[addr-1],SAMPLEAPP_END2,&data[4],1);//灯接在终端2上
            break;
        case ZIGBEE_FUN_CODE_STEP://控制终端上的步进电机
            addr=data[3];
            if(addr>MAX_DEVICE) break;
            if(addr==0) break;
           
            sendDataToEnd(endDevAddr[addr-1], SAMPLEAPP_END4,&data[4],1);//步进电机接在终端4上
            break;
        case ZIGBEE_FUN_CODE_END1:
            
            addr=data[3];
            if(addr>MAX_DEVICE) break;
            if(addr==0) break;
            sendDataToEnd(endDevAddr[addr-1], SAMPLEAPP_END1,&data[4],3);
            break;
        default:
            break;
    }
}

void sendDataToEnd(uint16 shortAddr, uint16 clusterId, uint8* data, uint8 len)
{
    afAddrType_t Tx_DstAddr;

    if(shortAddr==0)
    {
        //如果没有短地址，使用广播
        Tx_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast;
        Tx_DstAddr.endPoint = SAMPLEAPP_ENDPOINT; 
        Tx_DstAddr.addr.shortAddr = 0xffff;            //广播
    }
    else
    {
        //点播
        Tx_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
        Tx_DstAddr.endPoint = SAMPLEAPP_ENDPOINT; 
        Tx_DstAddr.addr.shortAddr = shortAddr;           //点播
    }



    if ( AF_DataRequest( &Tx_DstAddr, &SampleApp_epDesc,
                       clusterId,
                       len,
                       data,
                       &SampleApp_TransID,
                       AF_DISCV_ROUTE,
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
    {
    }
    else
    {
    // Error occurred in request to send.
    }
}

/*********************************************************************
*********************************************************************/
