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
#include "DHT11.h"
#include "Hal_adc.h"
#include "stdio.h"

/*********************************************************************
 * MACROS
 */

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


#define HUMAN_PIN	P0_4       //人体感应1:有人0：无人
#define LAMP_PIN     P0_5        //定义P0.5口为继电器输入端
#define GAS_PIN      P1_5        //定义P0.6口为烟雾传感器的输入端  
#define DHT11_DATA P0_7   //dht11
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr)[0])



//---------------------------------------------------------------------
//标准版不同的终端需要修改此ID,用于识别协调器发过来的数据，ID相同则处理
//专业版自动从Flash获得地址，所有终端固件相同，适合量产
static uint16 EndDeviceID = 0x0002; //终端ID，非常重要


/*********************************************************************
 * LOCAL FUNCTIONS
 */
void SampleApp_HandleKeys( uint8 shift, uint8 keys );
void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void SampleApp_SendPeriodicMessage( void );
void SerialApp_DeviceConnect();
static void SerialApp_SendSensorsData();
uint8 GetLamp( void );
void SetLamp(bool on);
uint8 GetGas( void );
uint8 GetHuman( void );

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

    P0SEL &= ~0x10;                  //设置P0.4口为普通IO
    P0DIR &= ~0x10;                  //设置P0.4为输入

    P0SEL &= ~0x20;                  //设置P0.5口为普通IO
    P0DIR |= 0x20;                  //设置P0.5为输出
    
    P0SEL &= ~0x40;                  //设置P0.6口为普通IO
    P0DIR &= ~0x40;                  //设置P0.6为输入

    P1SEL &= ~0x20;                  //设置P1.5口为普通IO
    P1DIR &= ~0x20;                  //设置P1.5为输入

    
    LAMP_PIN = 1;                   //低电平继电器断开;高电平继电器吸合


  
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

            SerialApp_DeviceConnect();//上传终端的短地址
            // Start sending the periodic message in a regular interval.
            osal_start_timerEx( SampleApp_TaskID,
                              SAMPLEAPP_SEND_PERIODIC_MSG_EVT,
                              SAMPLEAPP_SEND_PERIODIC_MSG_TIMEOUT );
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
    // Send the periodic message
    
    SerialApp_SendSensorsData();

    // Setup to send message again in normal period (+ a little jitter)
    osal_start_timerEx( SampleApp_TaskID, SAMPLEAPP_SEND_PERIODIC_MSG_EVT,
        1000 );


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
  }

  if ( keys & HAL_KEY_SW_2 )
  {
    /* The Flashr Command is sent to Group 1.
     * This key toggles this device in and out of group 1.
     * If this device doesn't belong to group 1, this application
     * will not receive the Flash command sent to group 1.
     */
    aps_Group_t *grp;
    grp = aps_FindGroup( SAMPLEAPP_ENDPOINT, SAMPLEAPP_FLASH_GROUP );
    if ( grp )
    {
      // Remove from the group
      aps_RemoveGroup( SAMPLEAPP_ENDPOINT, SAMPLEAPP_FLASH_GROUP );
    }
    else
    {
      // Add to the flash group
      aps_AddGroup( SAMPLEAPP_ENDPOINT, &SampleApp_Group );
    }
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
    case SAMPLEAPP_END2:
        {
            if(pkt->cmd.Data[0]>0)
            {
                SetLamp(1);//开灯
            }
            else
            {
                SetLamp(0);//关灯
            }
            SerialApp_SendSensorsData();
        }
      break;
  }
}

/*********************************************************************
 * @fn      SampleApp_SendPeriodicMessage
 *
 * @brief   Send the periodic message.
 *
 * @param   none
 *
 * @return  none
 */
void SampleApp_SendPeriodicMessage( void )
{
    
}


static void SerialApp_SendSensorsData()
{
    uint8 SendBuf[20]={0};
    uint8 SendLen=0;

    DHT11();                //获取温湿度

    SendBuf[0] =EndDeviceID;
    SendBuf[1] = wendu;  //温度
    SendBuf[2] = shidu;    //湿度
    SendBuf[3] = GetGas()>0?true:false;  //获取气体传感器的状态 
    SendBuf[4] = GetHuman()>0?true:false;  //获取人体传感器的状态 
    SendBuf[5] = GetLamp()>0?true:false;  //true:亮

    SendLen=6;//共6个字节发送

    if ( AF_DataRequest( &SampleApp_P2P_DstAddr, &SampleApp_epDesc,
                       SAMPLEAPP_END2,
                       SendLen,
                       SendBuf,
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

//获得P0_5 继电器引脚的电平
//on  1:亮  0:灭
uint8 GetLamp( void )
{
  uint8 ret;
  
  if(LAMP_PIN == 0)
  {	
    ret = 0;
  }
  else
  {
	ret = 1;
  }
  
	return ret;
}

//on  true:亮  false:灭
void SetLamp(bool on)
{
	LAMP_PIN=(on)?1:0;
}

//获得P0_6 MQ-2气体传感器的数据
//高电平时，气体正常
uint8 GetGas( void )
{
  uint8 ret;
  
  if(GAS_PIN == 0)
    ret = 0;
  else
    ret = 1;
  
  return ret;
}

//获得P0_4 人体红外传感器的数据
//返回,1:有人0：无人
uint8 GetHuman( void )
{
  uint8 ret;
  
  if(HUMAN_PIN == 0)
    ret = 0;
  else
    ret = 1;
  
  return ret;
}

//把终端地址上传服务器
void  SerialApp_DeviceConnect()
{
  uint16 nwkAddr;
  uint16 parentNwkAddr;
  char buff[5] = {0};
    afAddrType_t SerialApp_TxAddr;
  
  nwkAddr = NLME_GetShortAddr();
//  parentNwkAddr = NLME_GetCoordShortAddr();
  
  SerialApp_TxAddr.addrMode = (afAddrMode_t)Addr16Bit;
  SerialApp_TxAddr.endPoint = SAMPLEAPP_ENDPOINT;
  SerialApp_TxAddr.addr.shortAddr = 0x0;

  buff[0] = EndDeviceID;
  buff[1] = HI_UINT16( nwkAddr );
  buff[2] = LO_UINT16( nwkAddr );
  
  if ( AF_DataRequest( &SerialApp_TxAddr, &SampleApp_epDesc,
                       SERIALAPP_CONNECTREQ_CLUSTER,
                       3,
                       (uint8*)buff,
                       &SampleApp_TransID, 
                       0, 
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
