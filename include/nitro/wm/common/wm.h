#ifndef NITRO_WM_COMMON_WM_H_
#define NITRO_WM_COMMON_WM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>
#include <nitro/os.h>
#include <nitro_wl/common/version_wl.h>

#undef WM_GAMEINFO_TYPE_OLD // Removed support for old version of WMGameInfo

#if (SDK_VERSION_WL >= 15600)
#ifdef SDK_TEG
#undef WM_PRECALC_ALLOWEDCHANNEL
#else
#define WM_PRECALC_ALLOWEDCHANNEL
#endif
#else
#undef WM_PRECALC_ALLOWEDCHANNEL
#endif

#define WM_SSID_MASK_CUSTOMIZE 1

typedef enum WMApiid {
  WM_APIID_INITIALIZE = 0, //  0 : WM_Initialize()
  WM_APIID_RESET,          //  1 : WM_Reset()
  WM_APIID_END,            //  2 : WM_End()

  WM_APIID_ENABLE,    //  3 : WM_Enable()
  WM_APIID_DISABLE,   //  4 : WM_Disable()
  WM_APIID_POWER_ON,  //  5 : WM_PowerOn()
  WM_APIID_POWER_OFF, //  6 : WM_PowerOff()

  WM_APIID_SET_P_PARAM,   //  7 : WM_SetParentParameter()
  WM_APIID_START_PARENT,  //  8 : WM_StartParent()
  WM_APIID_END_PARENT,    //  9 : WM_EndParent()
  WM_APIID_START_SCAN,    // 10 : WM_StartScan()
  WM_APIID_END_SCAN,      // 11 : WM_EndScan()
  WM_APIID_START_CONNECT, // 12 : WM_StartConnect()
  WM_APIID_DISCONNECT,    // 13 : WM_Disconnect()
  WM_APIID_START_MP,      // 14 : WM_StartMP()
  WM_APIID_SET_MP_DATA,   // 15 : WM_SetMPData()
  WM_APIID_END_MP,        // 16 : WM_EndMP()
  WM_APIID_START_DCF,     // 17 : WM_StartDCF()
  WM_APIID_SET_DCF_DATA,  // 18 : WM_SetDCFData()
  WM_APIID_END_DCF,       // 19 : WM_EndDCF()
  WM_APIID_SET_WEPKEY,    // 20 : WM_SetWEPKey()
  WM_APIID_START_KS,      // 21 : WM_StartKeySharing() Kept for backward
                          // compatibility
  WM_APIID_END_KS, // 22 : WM_EndKeySharing()   Kept for backward compatibility
  WM_APIID_GET_KEYSET,     // 23 : WM_GetKeySet()
  WM_APIID_SET_GAMEINFO,   // 24 : WM_SetGameInfo()
  WM_APIID_SET_BEACON_IND, // 25 : WM_SetBeaconIndication()
  WM_APIID_START_TESTMODE, // 26 : WM_StartTestMode()
  WM_APIID_STOP_TESTMODE,  // 27 : WM_StopTestMode()
  WM_APIID_VALARM_MP, // 28 : For starting up MP from VAlarm in ARM7 (special)
  WM_APIID_SET_LIFETIME,    // 29 : WM_SetLifeTime()
  WM_APIID_MEASURE_CHANNEL, // 30 : WM_MeasureChannel()
  WM_APIID_INIT_W_COUNTER,  // 31 : WM_InitWirelessCounter()
  WM_APIID_GET_W_COUNTER,   // 32 : WM_GetWirelessCounter()
  WM_APIID_SET_ENTRY,       // 33 : WM_SetEntry()
  WM_APIID_AUTO_DEAUTH,     // 34 : For starting up automatic disconnection from
                            // indicate in ARM7. (special)
  WM_APIID_SET_MP_PARAMETER,  // 35 : WM_SetMPParameter()
  WM_APIID_SET_BEACON_PERIOD, // 36 : WM_SetBeaconPeriod()
  WM_APIID_AUTO_DISCONNECT, // 37 : For starting up automatic disconnection from
                            // indicate in ARM7. (special)
  WM_APIID_START_SCAN_EX,   // 38 : WM_StartScanEx()
  WM_APIID_SET_WEPKEY_EX,   // 39 : WM_SetWEPKeyEx()
  WM_APIID_SET_PS_MODE,     // 40 : WM_SetPowerSaveMode()
  WM_APIID_START_TESTRXMODE, // 41 : WM_StartTestRxMode()
  WM_APIID_STOP_TESTRXMODE,  // 42 : WM_StopTestRxMode()

  WM_APIID_KICK_MP_PARENT, // 43 : For starting up MP (ARM7 internal use)
  WM_APIID_KICK_MP_CHILD,  // 44 : For starting up MP (ARM7 internal use)
  WM_APIID_KICK_MP_RESUME, // 45 : For starting up MP (ARM7 internal use)

  WM_APIID_ASYNC_KIND_MAX, // 46 : Type of asynchronous process

  WM_APIID_INDICATION = 128, // 128 : For indication callback
  WM_APIID_PORT_SEND,        // 129 : Notification to port of data being sent
  WM_APIID_PORT_RECV,   // 130 : Notification to port of data being received
  WM_APIID_READ_STATUS, // 131 : WM_ReadStatus()

  WM_APIID_UNKNOWN =
      255 // 255 : Value returned from ARM7 at unknown command number
} WMApiid;

#define WM_API_REQUEST_ACCEPTED 0x8000

#define WM_WEPMODE_NO 0
#define WM_WEPMODE_40BIT 1
#define WM_WEPMODE_104BIT 2
#define WM_WEPMODE_128BIT 3

#define WM_AUTHMODE_OPEN_SYSTEM 0
#define WM_AUTHMODE_SHARED_KEY 1

#define WM_SCANTYPE_ACTIVE 0
#define WM_SCANTYPE_PASSIVE 1

#if WM_SSID_MASK_CUSTOMIZE
#define WM_SCANTYPE_ACTIVE_CUSTOM 2
#define WM_SCANTYPE_PASSIVE_CUSTOM 3
#endif

#define WM_NUM_MAX_CHILD                                                       \
  15 // The maximum value possible for the AssociationID provided by the NITRO
     // parent
#define WM_NUM_MAX_AP_AID                                                      \
  2007 // The maximum value possible for the AssociationID provided by the
       // wireless router

#define WM_NUM_OF_PORT 16 // Number of ports
#define WM_NUM_OF_SEQ_PORT                                                     \
  8 // Number of ports that perform sequential processing
typedef enum WMPort {

  WM_PORT_RAWDATA = 0,    // Port for backward compatibility
  WM_PORT_BT = 1,         // Port for block transfer
  WM_PORT_RESERVE_02 = 2, // Reserved
  WM_PORT_RESERVE_03 = 3, // Reserved

  WM_PORT_RESERVE_10 = 8,  // Reserved
  WM_PORT_RESERVE_11 = 9,  // Reserved
  WM_PORT_RESERVE_12 = 10, // Reserved
  WM_PORT_RESERVE_13 = 11  // Reserved
} WMPort;

#define WM_SEND_QUEUE_NUM 32            // Total 32 level queue list
#define WM_SEND_QUEUE_END ((u16)0xffff) // Value that indicates tail
#define WM_PRIORITY_LEVEL 4             // 4 priority levels
typedef enum WMPriorityLevel {
  WM_PRIORITY_URGENT = 0,
  WM_PRIORITY_HIGH,
  WM_PRIORITY_NORMAL,
  WM_PRIORITY_LOW
} WMPriorityLevel;

#define WM_VALARM_COUNT_CHILD_MP 240  // Child MP operation
#define WM_VALARM_COUNT_PARENT_MP 260 // Parent MP operation
#define WM_VALARM_COUNT_RANGE_TOP                                              \
  220 // Highest limit on possible settings for parentVCount and childVCount
      // (220 - 262, 0 - 190)
#define WM_VALARM_COUNT_RANGE_BOTTOM                                           \
  190 // Lowest limit on possible settings for parentVCount and childVCount.

#define WM_MP_FREQ_CONT 16 // Send frequency that becomes continuous send
#define WM_MP_COUNT_LIMIT                                                      \
  256 // Upper limit of the counter for the number of sends remaining.
#define WM_DEFAULT_MP_FREQ_LIMIT 6 // Default upper limit for MP frequency
#define WM_DEFAULT_MP_PARENT_INTERVAL                                          \
  1000 // Default parent MP interval (in microseconds)
#define WM_DEFAULT_MP_CHILD_INTERVAL                                           \
  0 // Default child MP interval (in microseconds)

#define WM_SIZE_BSSID 6
#define WM_SIZE_SSID 32

#define WM_SIZE_GAMEINFO 128 // Maximum size

#define WM_SIZE_SCAN_EX_BUF 1024 // Buffer size for StartScanEx

#define WM_GAMEINFO_LENGTH_MIN 16 // Minimum size of valid GameInfo
#define WM_GAMEINFO_MAGIC_NUMBER                                               \
  0x0001 // DS GameInfo magic number for identification purposes
#define WM_GAMEINFO_VERSION_NUMBER 1         // GameInfo version number
#define WM_GAMEINFO_PLATFORM_ID_NITRO 0      // Parent device is a DS
#define WM_GAMEINFO_PLATFORM_ID_REVOLUTION 8 // Parent device is a Wii
#define WM_SIZE_SYSTEM_GAMEINFO 16           // size up to userGameInfoLength
#define WM_SIZE_USER_GAMEINFO 112 // Maximum user area size for GameInfo

#define WM_SIZE_SCAN_PARAM 32

#define WM_PARENT_PARAM_SIZE 64
#define WM_SIZE_USERNAME 8
#define WM_SIZE_GAMENAME 16
#define WM_SIZE_GGID 4

#define WM_SIZE_WL_VERSION 8
#define WM_SIZE_BBP_VERSION 4
#define WM_SIZE_MACADDR 6
#define WM_SIZE_WEPKEY 80

#define WM_SIZE_CHILD_SSID 24

#if (SDK_VERSION_WL >= 21100)
#define WM_SIZE_MP_DATA_MAX 512
#else
#define WM_SIZE_MP_DATA_MAX 508
#endif

#define WM_SIZE_MADATA_HEADER 44

#define WM_SIZE_MP_PARENT_PADDING (WM_HEADER_SIZE + WM_HEADER_PARENT_MAX_SIZE)

#define WM_SIZE_MP_CHILD_PADDING (WM_HEADER_SIZE + WM_HEADER_CHILD_MAX_SIZE)

#define WM_SIZE_DS_PARENT_HEADER 4
#define WM_SIZE_KS_PARENT_DATA (2 * 16 + WM_SIZE_DS_PARENT_HEADER)
#define WM_SIZE_KS_CHILD_DATA 2

#define WM_ATTR_ENTRY_SHIFT 0
#define WM_ATTR_MB_SHIFT 1
#define WM_ATTR_KS_SHIFT 2
#define WM_ATTR_CS_SHIFT 3

#define WM_ATTR_FLAG_ENTRY (1 << WM_ATTR_ENTRY_SHIFT)
#define WM_ATTR_FLAG_MB (1 << WM_ATTR_MB_SHIFT)
#define WM_ATTR_FLAG_KS (1 << WM_ATTR_KS_SHIFT)
#define WM_ATTR_FLAG_CS (1 << WM_ATTR_CS_SHIFT)

#define WM_EXCEPTION_CB_MASK 0x0001

#define WM_HEADER_SIZE 2 // Number of bytes of shared parent/child wmHeader

#define WM_HEADER_KS 0x4000
#define WM_HEADER_VSYNC 0x8000

#define WM_HEADER_PARENT_MAX_SIZE                                              \
  4 // Maximum number of bytes that can be added as header (excluding the 2
    // bytes of wmHeader)

#define WM_HEADER_CHILD_MAX_SIZE                                               \
  2 // Maximum number of bytes that can be added as header (excluding the 2
    // bytes of wmHeader)

#define WM_HEADER_PORT_MASK 0x0f00
#define WM_HEADER_PORT_SHIFT 8
#define WM_HEADER_SEQ_FLAG 0x0800
#define WM_HEADER_DEST_BITMAP 0x1000
#define WM_HEADER_LENGTH_MASK 0x00ff
#define WM_HEADER_LENGTH_SCALE 2

#define WM_SEQ_PORT_FLAG 0x0008
#define WM_SEQ_PORT_MASK 0x0007

#define WM_HEADER_SEQ_RETRY 0x8000
#define WM_HEADER_SEQ_NUM_MASK 0x7fff

#define WM_PACKED_PACKET_MAX 128

typedef enum WMErrCode {
  WM_ERRCODE_SUCCESS = 0,
  WM_ERRCODE_FAILED = 1,
  WM_ERRCODE_OPERATING = 2,
  WM_ERRCODE_ILLEGAL_STATE = 3,
  WM_ERRCODE_WM_DISABLE = 4,
  WM_ERRCODE_NO_KEYSET = 5,
  WM_ERRCODE_NO_DATASET = 5, // Same value as NO_KEYSET
  WM_ERRCODE_INVALID_PARAM = 6,
  WM_ERRCODE_NO_CHILD = 7,
  WM_ERRCODE_FIFO_ERROR = 8,
  WM_ERRCODE_TIMEOUT = 9,
  WM_ERRCODE_SEND_QUEUE_FULL = 10,
  WM_ERRCODE_NO_ENTRY = 11,
  WM_ERRCODE_OVER_MAX_ENTRY = 12,
  WM_ERRCODE_INVALID_POLLBITMAP = 13,
  WM_ERRCODE_NO_DATA = 14,
  WM_ERRCODE_SEND_FAILED = 15,

  WM_ERRCODE_DCF_TEST,         // For debugging
  WM_ERRCODE_WL_INVALID_PARAM, // For debugging (can delete)
  WM_ERRCODE_WL_LENGTH_ERR,    // For debugging (can delete)

  WM_ERRCODE_FLASH_ERROR, // WL flash parameter error (freezes because it is
                          // fatal)
  WM_ERRCODE_MAX
} WMErrCode;

typedef enum WMStateCode {
  WM_STATECODE_PARENT_START = 0, // For StartParent. Started parent operation
  WM_STATECODE_BEACON_SENT = 2,  // For StartParent. Beacon send completed

  WM_STATECODE_SCAN_START = 3,       // For StartScan. Started Scan operation
  WM_STATECODE_PARENT_NOT_FOUND = 4, // For StartScan. Cannot find parent
  WM_STATECODE_PARENT_FOUND = 5,     // For StartScan. Found parent

  WM_STATECODE_CONNECT_START = 6, // For StartConnect. Started connect process
  WM_STATECODE_BEACON_LOST = 8,   // For StartConnect. Lost parent's beacon

  WM_STATECODE_CONNECTED =
      7, // For StartParent and StartConnect, connection complete for port.
  WM_STATECODE_CHILD_CONNECTED = 7, // Same value as WM_STATECODE_CONNECTED
  WM_STATECODE_DISCONNECTED =
      9, // Notification of disconnection for StartParent and StartConnect port.
  WM_STATECODE_DISCONNECTED_FROM_MYSELF =
      26, // Notification of disconnection from self for StartParent and
          // StartConnect port.

  WM_STATECODE_MP_START = 10, // For StartMP. Started MP communication mode.
  WM_STATECODE_MPEND_IND =
      11, // For StartMP. Parent MP sequence complete (receive)
  WM_STATECODE_MP_IND = 12,    // For StartMP. Child MP receive
  WM_STATECODE_MPACK_IND = 13, // For StartMP. Child MPACK receive

  WM_STATECODE_DCF_START = 14, // For StartDCF. Started DCF communication mode.
  WM_STATECODE_DCF_IND = 15,   // For StartDCF. Received DCF data

  WM_STATECODE_BEACON_RECV = 16,  // Indicate beacon received
  WM_STATECODE_DISASSOCIATE = 17, // Indicate disconnection
  WM_STATECODE_REASSOCIATE = 18,  // Indicate reconnection
  WM_STATECODE_AUTHENTICATE = 19, // Indicate confirmed authentication

  WM_STATECODE_PORT_INIT = 25, // Initialize port
  WM_STATECODE_PORT_SEND = 20, // Send data to port
  WM_STATECODE_PORT_RECV = 21, // Receive data from port

  WM_STATECODE_FIFO_ERROR = 22,  // FIFO error occurrence notification in ARM7
  WM_STATECODE_INFORMATION = 23, // Notification of information
  WM_STATECODE_UNKNOWN = 24,     // Unspecified notification

  WM_STATECODE_MAX = 27
} WMStateCode;

typedef enum WMState {
  WM_STATE_READY = 0,   // READY state (before Init, LED normal lighting state)
  WM_STATE_STOP,        // STOP state (Before Initialize, LED blinking state)
  WM_STATE_IDLE,        // IDLE state (After Initialize. After Reset, etc)
  WM_STATE_CLASS1,      // CLASS1 state
  WM_STATE_TESTMODE,    // TESTMODE (for applying)
  WM_STATE_SCAN,        // During StartScan
  WM_STATE_CONNECT,     // During StartConnect
  WM_STATE_PARENT,      // During StartParent
  WM_STATE_CHILD,       // Child after connect complete
  WM_STATE_MP_PARENT,   // Parent after StartMP
  WM_STATE_MP_CHILD,    // Child after StartMP
  WM_STATE_DCF_CHILD,   // Child after StartDCF
  WM_STATE_TESTMODE_RX, // TESTMODE_RX

  WM_STATE_MAX
} WMState;

typedef enum WMLinkLevel {
  WM_LINK_LEVEL_0 = 0,
  WM_LINK_LEVEL_1,
  WM_LINK_LEVEL_2,
  WM_LINK_LEVEL_3,
  WM_LINK_LEVEL_MAX
} WMLinkLevel;

typedef enum WMDataSharingState {
  WM_DS_STATE_READY = 0, // Before WM_StartDataSharing
  WM_DS_STATE_START,     // During WM_StepDataSharing
  WM_DS_STATE_PAUSING,   // While transitioning to the pause state
  WM_DS_STATE_PAUSED, // Immediately after WM_StartDataSharing or during pause
  WM_DS_STATE_RETRY_SEND, // Resend required because of SEND_QUEUE_FULL
  WM_DS_STATE_ERROR       // Error generated
} WMDataSharingState;

typedef enum WMDisconnectReason {

  WM_DISCONNECT_REASON_RESERVED = 0, // Reserved
  WM_DISCONNECT_REASON_UNSPECIFIED =
      1, // Error that cannot be specifically identified
  WM_DISCONNECT_REASON_PREV_AUTH_INVALID =
      2, // The immediately previous authentication is no longer valid.
  WM_DISCONNECT_REASON_DEAUTH_LEAVING =
      3, // Deauthenticated because unit has left the BSS.
  WM_DISCONNECT_REASON_INACTIVE = 4, // Released connection due to inactivity.
  WM_DISCONNECT_REASON_UNABLE_HANDLE =
      5, // Released connection due to lack of sufficient resources for AP.
  WM_DISCONNECT_REASON_RX_CLASS2_FROM_NONAUTH_STA =
      6, // A Class2 frame was received from an unauthenticated STA.
  WM_DISCONNECT_REASON_RX_CLASS3_FROM_NONASSOC_STA =
      7, // A Class3 frame was received from an unconnected STA.
  WM_DISCONNECT_REASON_DISASSOC_LEAVING =
      8, // Released association because unit has left the BSS.
  WM_DISCONNECT_REASON_ASSOC_STA_NOTAUTHED =
      9, // An STA which has requested connection has not yet undergone
         // authentication.

  WM_DISCONNECT_REASON_NO_ENTRY =
      19, // DS parent is not currently accepting entries.

  WM_DISCONNECT_REASON_MP_LIFETIME =
      0x8001, // MP communication lifetime has run out.
  WM_DISCONNECT_REASON_TGID_CHANGED =
      0x8002, // TGID changed during receive beacon.
  WM_DISCONNECT_REASON_FATAL_ERROR =
      0x8003, // An unrecoverable error has occurred in the library.

  WM_DISCONNECT_REASON_FROM_MYSELF =
      0xf001 // Self-initiated disconnect using the WM API has been performed.
} WMDisconnectReason;

typedef enum WMInfoCode {
  WM_INFOCODE_NONE = 0,
  WM_INFOCODE_FATAL_ERROR // Notification regarding occurrence of fatal error
} WMInfoCode;

#define WM_MP_PARAM_MIN_FREQUENCY 0x0001
#define WM_MP_PARAM_FREQUENCY 0x0002
#define WM_MP_PARAM_MAX_FREQUENCY 0x0004
#define WM_MP_PARAM_PARENT_SIZE 0x0008
#define WM_MP_PARAM_CHILD_SIZE 0x0010
#define WM_MP_PARAM_PARENT_INTERVAL 0x0020
#define WM_MP_PARAM_CHILD_INTERVAL 0x0040
#define WM_MP_PARAM_PARENT_VCOUNT 0x0080
#define WM_MP_PARAM_CHILD_VCOUNT 0x0100
#define WM_MP_PARAM_DEFAULT_RETRY_COUNT 0x0200
#define WM_MP_PARAM_MIN_POLL_BMP_MODE 0x0400
#define WM_MP_PARAM_SINGLE_PACKET_MODE 0x0800
#define WM_MP_PARAM_IGNORE_FATAL_ERROR_MODE 0x1000
#define WM_MP_PARAM_IGNORE_SIZE_PRECHECK_MODE 0x2000

#define WM_MP_PARAM_MUST_SET_BEFORE_MP                                         \
  (WM_MP_PARAM_MIN_POLL_BMP_MODE | WM_MP_PARAM_SINGLE_PACKET_MODE |            \
   WM_MP_PARAM_IGNORE_SIZE_PRECHECK_MODE)

#define WM_MP_TMP_PARAM_MIN_FREQUENCY 0x0001
#define WM_MP_TMP_PARAM_FREQUENCY 0x0002
#define WM_MP_TMP_PARAM_MAX_FREQUENCY 0x0004
#define WM_MP_TMP_PARAM_DEFAULT_RETRY_COUNT 0x0200
#define WM_MP_TMP_PARAM_MIN_POLL_BMP_MODE 0x0400
#define WM_MP_TMP_PARAM_SINGLE_PACKET_MODE 0x0800
#define WM_MP_TMP_PARAM_IGNORE_FATAL_ERROR_MODE 0x1000

#define WM_MP_PARAM_INTERVAL_MAX                                               \
  10000 // The value specified for interval must be 10000us or lower.

#define WM_MISC_FLAG_LISTEN_ONLY 0x0001
#define WM_MISC_FLAG_NO_BLINK 0x0002

typedef void (*WMcallbackFunc)(void *arg); // WM API callback type
typedef void (*WMCallbackFunc)(void *arg); // WM API callback type

#define WM_ARM9WM_BUF_SIZE 512 // Size of the WM variable area on ARM9 side
#define WM_ARM7WM_BUF_SIZE                                                     \
  (256 + 512)                   // Size of the WM variable area on ARM7 side
#define WM_STATUS_BUF_SIZE 2048 // Size of the WM status area
#define WM_FIFO_BUF_SIZE                                                       \
  256 // Size of the FIFO command buffer between ARM9 and ARM7

#define WM_SYSTEM_BUF_SIZE                                                     \
  (WM_ARM9WM_BUF_SIZE + WM_ARM7WM_BUF_SIZE + WM_STATUS_BUF_SIZE +              \
   WM_FIFO_BUF_SIZE + WM_FIFO_BUF_SIZE)
#define WM_BSS_DESC_SIZE                                                       \
  192 // Size of the buffer for storing parent information passed by
      // WM_StartScan()

#define WM_DCF_MAX_SIZE 1508 // Maximum size that can be received with DCF
#define WM_KEYSET_SIZE 36    // Size of KeySet
#define WM_KEYDATA_SIZE 2    // Size of KeyData

#define WM_DS_HEADER_SIZE 4

#ifdef WM_DS_DATA_SIZE_252 // When it is OK to set 252 for WM_DS_DATA_SIZE
#define WM_DS_DATA_SIZE 252
#else                       // When WM_DS_DATA_SIZE == 508 (default)
#define WM_DS_DATA_SIZE 508 // Maximum data size of one WMDataSet (512-4)

#endif

#define WM_DS_DATASET_NUM                                                      \
  4 // How many WMDataSet to be kept in WMDataSetBuf (fixed value)
#define WM_DS_INFO_SIZE                                                        \
  (sizeof(WMDataSharingInfo)) // Buffer size for DataSharing

#define WM_SCAN_EX_PARENT_MAX                                                  \
  16 // Maximum number of parent devices that can be found with one ScanEx
#define WM_SCAN_OTHER_ELEMENT_MAX                                              \
  16 // Maximum number of otherElement that can be acknowledged

#ifdef SDK_TWL
#define WM_WIRELESS_COMM_FLAG_OFF 0 // Mode with wireless communications on
#define WM_WIRELESS_COMM_FLAG_ON 1  // Mode with wireless communications off
#define WM_WIRELESS_COMM_FLAG_UNKNOWN                                          \
  2 // This cannot be determined because the program is running on a DS
#endif

typedef struct WMDataSet {

  u16 aidBitmap;
  u16 receivedBitmap;                      // Bit map of receive status
  u16 data[WM_DS_DATA_SIZE / sizeof(u16)]; // Shared data

} WMDataSet;

typedef struct WMDataSharingInfo {
  WMDataSet ds[WM_DS_DATASET_NUM];
  u16 seqNum[WM_DS_DATASET_NUM]; // Sequential Number
  u16 writeIndex;                // Next Index to write
  u16 sendIndex;                 // Index currently being sent
  u16 readIndex;                 // Next Index to read
  u16 aidBitmap;     // bitmap of data sharing group (including self)
  u16 dataLength;    // Amount of shared data per unit
  u16 stationNumber; // Number of units sharing data (number of bits set to 1 in
                     // aidBitmap)
  u16 dataSetLength; // dataLength * stationNumber
  u16 port;          // Port being used (number 8 or above)
  u16 doubleMode;    // In mode for alternately sending/receiving each frame?
  u16 currentSeqNum; // SeqNum of the DataSet read most recently
  u16 state;         // Current DataSharing state (WMDataSharingState)
  u16 reserved[1];   // for 32-byte alignment
} WMDataSharingInfo;

typedef struct WMKeySet {
  u16 seqNum; // Key set's sequential number
  u16 rsv;
  u16 key[16]; // Key data

} WMKeySet, WMkeySet;

typedef WMDataSharingInfo WMKeySetBuf, WMkeySetBuf;

typedef struct {
  u16 next;
  u16 port;
  u16 destBitmap;
  u16 restBitmap;
  u16 sentBitmap;
  u16 sendingBitmap;
  u16 padding;
  u16 size;
  u16 seqNo;
  u16 retryCount;
  const u16 *data;
  WMCallbackFunc callback;
  void *arg;
} WMPortSendQueueData;

typedef struct {
  u16 head;
  u16 tail;
} WMPortSendQueue;

typedef struct WMMpRecvBuf {
  u16 rsv1[3];
  u16 length;

  u16 rsv2[1];
  u16 ackTimeStamp;
  u16 timeStamp;
  u16 rate_rssi;
  u16 rsv3[2];

  u16 rsv4[2];
  u8 destAdrs[6];
  u8 srcAdrs[6];
  u16 rsv5[3];
  u16 seqCtrl;

  u16 txop;
  u16 bitmap;
  u16 wmHeader;
  u16 data[2];

} WMMpRecvBuf, WMmpRecvBuf;

typedef struct WMMpRecvData {
  u16 length;
  u16 rate_rssi;
  u16 aid;
  u16 noResponse;
  u16 wmHeader;
  u16 cdata[1];

} WMMpRecvData, WMmpRecvData;

typedef struct WMMpRecvHeader {
  u16 bitmap;
#if SDK_VERSION_WL >= 20500
  u16 errBitmap;
#endif
  u16 count;
  u16 length;
  u16 txCount;
  WMMpRecvData data[1];

} WMMpRecvHeader, WMmpRecvHeader;

typedef struct WMDcfRecvBuf {
  u16 frameID;
  u16 rsv1[2];
  u16 length;

  u16 rsv2[3];
  u16 rate_rssi;
  u16 rsv3[4];
  u8 destAdrs[6];
  u8 srcAdrs[6];
  u16 rsv4[4];

  u16 data[2];

} WMDcfRecvBuf, WMdcfRecvBuf;

typedef struct WMParentParam {
  u16 *userGameInfo;      // Pointer to data to set in user area
  u16 userGameInfoLength; // Length of user area
  u16 padding;

  u32 ggid;
  u16 tgid; // Temporary GameID (generated each time a user becomes a parent)
  u16 entryFlag;     // 1: Connect possible, 0: Connect not possible
  u16 maxEntry;      // Number of child devices that can connect
  u16 multiBootFlag; // 1: Multiboot possible, 0: Multiboot not possible
  u16 KS_Flag;       // 1: KeySharing OK, 0: No KeySharing
  u16 CS_Flag;       // 1: Continuous send, 0: Send in each frame
  u16 beaconPeriod;  // Beacon interval (ms)
  u16 rsv1[WM_SIZE_USERNAME / sizeof(u16)]; //
  u16 rsv2[WM_SIZE_GAMENAME / sizeof(u16)]; //
  u16 channel;                              // Channel
  u16 parentMaxSize;                        // Maximum parent send size
  u16 childMaxSize;                         // Maximum child send size

  u16 rsv[4]; // For 32-byte alignment

} WMParentParam, WMpparam;

typedef struct WMGameInfo {
  u16 magicNumber; // == 0x0001
  u8 ver;          // GameInfo Version 1
  u8 platform;     // DS: 0, Wii: 8
  u32 ggid;
  u16 tgid;
  u8 userGameInfoLength; // Length of user area
  union {

    u8 gameNameCount_attribute; // Old name for compatibility purposes
    u8 attribute;
  };
  u16 parentMaxSize; // Maximum parent send size
  u16 childMaxSize;  // Maximum child send size
  union {
    u16 userGameInfo[WM_SIZE_USER_GAMEINFO /
                     sizeof(u16)]; // User region (Maximum: 112 bytes
                                   // (provisional))
    struct {
      u16 userName[WM_SIZE_USERNAME / sizeof(u16)]; // WM_SIZE_USERNAME -> 8
      u16 gameName[WM_SIZE_GAMENAME / sizeof(u16)]; // WM_SIZE_GAMENAME -> 16
      u16 padd1[44];                                // pad -> 88
    } old_type;                                     // Total: 112 bytes
  };
} WMGameInfo, WMgameInfo;

#if SDK_VERSION_WL < 17100
typedef struct WMBssDesc {
  u16 length;              // 2
  u16 rssi;                // 4
  u8 bssid[WM_SIZE_BSSID]; // 10
  u16 ssidLength;          // 12
  u8 ssid[WM_SIZE_SSID];   // 44
  u16 capaInfo;            // 46
  struct {
    u16 basic;   // 48
    u16 support; // 50
  } rateSet;
  u16 beaconPeriod;    // 52
  u16 dtimPeriod;      // 54
  u16 channel;         // 56
  u16 cfpPeriod;       // 58
  u16 cfpMaxDuration;  // 60
  u16 gameInfoLength;  // 62
  WMGameInfo gameInfo; // 190
  u16 rsv;             // 192

} WMBssDesc, WMbssDesc;
#else
typedef struct WMBssDesc {
  u16 length;              // 2
  u16 rssi;                // 4
  u8 bssid[WM_SIZE_BSSID]; // 10
  u16 ssidLength;          // 12
  u8 ssid[WM_SIZE_SSID];   // 44
  u16 capaInfo;            // 46
  struct {
    u16 basic;   // 48
    u16 support; // 50
  } rateSet;
  u16 beaconPeriod;      // 52
  u16 dtimPeriod;        // 54
  u16 channel;           // 56
  u16 cfpPeriod;         // 58
  u16 cfpMaxDuration;    // 60
  u16 gameInfoLength;    // 62
  u16 otherElementCount; // 64
  WMGameInfo gameInfo;   // 192

} WMBssDesc, WMbssDesc;
#endif

typedef struct WMOtherElements {
  u8 count;
  u8 rsv[3];
  struct {
    u8 id;
    u8 length;
    u8 rsv[2];
    u8 *body;
  } element[WM_SCAN_OTHER_ELEMENT_MAX];
} WMOtherElements;

typedef struct WMScanParam {
  WMBssDesc *scanBuf;      // Buffer that stores parent information
  u16 channel;             // Channel which carries out Scan
  u16 maxChannelTime;      // Maximum Scan time
  u8 bssid[WM_SIZE_BSSID]; // MacAddress of Scan target parent (if 0xff, all
                           // parents are targets)
  u16 rsv[9];

} WMScanParam, WMscanParam;

typedef struct WMScanExParam {
  WMBssDesc *scanBuf; // Buffer that stores parent information
  u16 scanBufSize;    // Size of scanBuf
  u16 channelList;    // List of channels to do Scan for (Can specify multiple.
                   // Same format as the return value for WM_GetAllowedChannel)
  u16 maxChannelTime;      // Maximum Scan time
  u8 bssid[WM_SIZE_BSSID]; // MacAddress of Scan target parent (if 0xff, all
                           // parents are targets)
  u16 scanType;   // Normally WM_SCANTYPE_PASSIVE. WM_SCANTYPE_ACTIVE during
                  // ActiveScan
  u16 ssidLength; // SSID length of target parent (if 0, all parents are
                  // targets)
  u8 ssid[WM_SIZE_SSID]; // SSID of parent that is target of Scan
#if WM_SSID_MASK_CUSTOMIZE
  u16 ssidMatchLength;
  u16 rsv2[7];
#else
  u16 rsv2[8];
#endif

} WMScanExParam, WMscanExParam;

typedef struct WMMPParam {
  u32 mask; // Which of the fields below is valid is set by bits.

  u16 minFrequency;   // Number of MP cycles with one picture frame (minimum
                      // value)(un-implemented)
  u16 frequency;      // Number of MP cycles in one picture frame
  u16 maxFrequency;   // Number of MP cycles in one picture frame (upper limit
                      // value)
  u16 parentSize;     // Current parent send size
  u16 childSize;      // Current child send size
  u16 parentInterval; // If performed continuously, the parent MP communications
                      // interval within one frame (in microseconds)
  u16 childInterval; // If performed continuously, the interval a child takes to
                     // prepare MP communications within one frame (in
                     // microseconds)
  u16 parentVCount;  // The starting V Count for parent processing when in frame
                     // synchronous communications
  u16 childVCount;   // The starting V Count for child processing when in frame
                     // synchronous communications
  u16 defaultRetryCount; // Number of communications default retries in the Raw
                         // communications port
  u8 minPollBmpMode; // Mode that minimizes to fit the send target of the packet
                     // that sends PollBitmap
  u8 singlePacketMode;       // Mode that only sends 1 packet in 1 MP
  u8 ignoreFatalErrorMode;   // Mode that ignores fatal errors
  u8 ignoreSizePrecheckMode; // Mode that ignores the preliminary check of the
                             // send / receive size when starting communications
} WMMPParam;

typedef struct WMMPTmpParam {
  u32 mask; // Which of the fields below is valid is set by bits.

  u16 minFrequency; // Number of MP cycles with one picture frame (minimum
                    // value) (un-implemented)
  u16 frequency;    // Number of MP cycles in one picture frame
  u16 maxFrequency; // Number of MP cycles in one picture frame (upper limit
                    // value)
  u16 defaultRetryCount; // Number of communications default retries in the raw
                         // communications port
  u8 minPollBmpMode; // Mode that minimizes to fit the send target of the packet
                     // that sends PollBitmap
  u8 singlePacketMode;     // Mode that only sends 1 packet in 1 MP
  u8 ignoreFatalErrorMode; // Mode that ignores fatal errors
  u8 reserved[1];
} WMMPTmpParam;

typedef struct WMStatus {

  u16 state;     // WM state information
  u16 BusyApiid; // Busy APIID

  BOOL apiBusy;       // API executing flag (Flag prohibiting issuing API,
                      // attributable to FIFO)
  BOOL scan_continue; // Scan continue flag (As of 2004/07/19, not used)
  BOOL mp_flag;       // Flag that MP is in execution
  BOOL dcf_flag;      // Flag that DCF is in execution
  BOOL ks_flag;       // Flag that KeySharing is in execution
  BOOL dcf_sendFlag;  // 'Waiting for DCF send' flag
  BOOL VSyncFlag;     // 'VBlank sync OK' flag

  u8 wlVersion[WM_SIZE_WL_VERSION];                  // WL version
  u16 macVersion;                                    // MAC version
  u16 rfVersion;                                     // RF version
  u16 bbpVersion[WM_SIZE_BBP_VERSION / sizeof(u16)]; // BBP version

  u16 mp_parentSize;    // Send size in one MP communication transmission by the
                        // parent
  u16 mp_childSize;     // Send size in one MP communication transmission by the
                        // child
  u16 mp_parentMaxSize; // Maximum send size in one MP communication
                        // transmission by the parent
  u16 mp_childMaxSize; // Maximum send size in one MP communication transmission
                       // by the child
  u16 mp_sendSize;     // Maximum send size with current connection
  u16 mp_recvSize;     // Maximum receive size with current connection
  u16 mp_maxSendSize;  // Maximum send size with current connection
                       // (parentMaxSize+4 or childMaxSize+2)
  u16 mp_maxRecvSize;  // Maximum receive size with current connection
                       // (childMaxSize+2 or parentMaxSize+4)
  u16 mp_parentVCount; // MP operation start VCount on the parent side
  u16 mp_childVCount;  // MP operation start VCount on the child side
  u16 mp_parentInterval; // MP communications interval used by the parent (us)
  u16 mp_childInterval;  // MP communications interval used by the child (us)

  OSTick mp_parentIntervalTick; // MP communications interval used by the parent
                                // (tick)
  OSTick mp_childIntervalTick;  // MP communications interval used by the child
                                // (tick)

  u16 mp_minFreq; // Number of MP cycles in one picture frame (minimum)
                  // (un-implemented)
  u16 mp_freq;    // Number of MP cycles in one picture frame (standard value)
  u16 mp_maxFreq; // Number of MP cycles in one picture frame (maximum value)

  u16 mp_vsyncOrderedFlag; // Whether the next MP was instructed by the parent
                           // to synchronize to vsync
  u16 mp_vsyncFlag;  // Whether the next MP is performed by synchronizing to
                     // vsync
  s16 mp_count;      // How many more MP cycles for the current picture frame?
  s16 mp_limitCount; // How many more times it is okay to MP in the current
                     // picture frame, including resends?
  u16 mp_resumeFlag; // Flag that resume is in execution
  u16 mp_prevPollBitmap;      // PollBitmap in MP immediately before
  u16 mp_prevWmHeader;        // wmHeader in MP immediately before
  u16 mp_prevTxop;            // txop in immediately previous MP
  u16 mp_prevDataLength;      // dataLength in immediately previous MP
  u16 mp_recvBufSel;          // 'MP receive buffer selection' flag
  u16 mp_recvBufSize;         // Size of MP Receive buffer
  WMMpRecvBuf *mp_recvBuf[2]; // Pointer to the MP receive buffer
  u32 *mp_sendBuf;            // Pointer to the MP send buffer
  u16 mp_sendBufSize;         // Size of the MP send buffer

  u16 mp_ackTime;     // MPACK estimated time of arrival
  u16 mp_waitAckFlag; // Waiting for MPACK

  u16 mp_readyBitmap; // Bitmap of children with MP preparations complete (for
                      // parent use)

  u16 mp_newFrameFlag; // flag that becomes TRUE when a new picture frame comes
  u8 reserved_b[2];
  u16 mp_sentDataFlag;    // flag that reply data is sent (for child use)
  u16 mp_bufferEmptyFlag; // 'reply data buffer is empty' flag (for child use)
  u16 mp_isPolledFlag;    // Was self polled in the last MP? (for child)

  u16 mp_minPollBmpMode;   // Mode that minimizes to fit the send target of the
                           // packet that sends PollBitmap
  u16 mp_singlePacketMode; // Mode that only sends 1 packet in 1 MP
  u8 reserved_c[2];
  u16 mp_defaultRetryCount; // default number of retries for communication on a
                            // port that does not do Seq control
  u16 mp_ignoreFatalErrorMode;   // Whether to ignore fatal errors
  u16 mp_ignoreSizePrecheckMode; // Invalidates the preliminary check of the
                                 // send / receive size when starting
                                 // communications

  u16 mp_pingFlag;    // flag that is enabled with the same timing as ping
  u16 mp_pingCounter; // counter of number of frames remaining until ping

  u8 dcf_destAdr[WM_SIZE_MACADDR]; // DCF send destination MAC address
  u16 *dcf_sendData;               // DCF send data pointer
  u16 dcf_sendSize;                // DCF send data size
  u16 dcf_recvBufSel;              // DCF receive buffer select flag
  WMDcfRecvBuf *dcf_recvBuf[2];    // DCF receive buffer pointer
  u16 dcf_recvBufSize;             // DCF receive buffer size

  u16 curr_tgid; // TGID of the parent that is currently connected (used with
                 // BeaconRecv.Ind)

  u16 linkLevel;
  u16 minRssi;
  u16 rssiCounter;

  u16 beaconIndicateFlag; // Flag that enables beacon-related notifications
  u16 wepKeyId;           // WEP key ID (0-3)
  u16 pwrMgtMode;         // Power management mode
  u32 miscFlags;          // miscellaneous flags

  u16 VSyncBitmap;       // Bitmap of children after V-Blank synchronization
  u16 valarm_queuedFlag; // V-Alarm processes have accumulated in the run queue.

  u32 v_tsf;          // For V-Blank sync. V_TSF value
  u32 v_tsf_bak;      // For V-Blank sync. Last acquired V_TSF value
  u32 v_remain;       // For V-Blank sync. Remaining adjustment
  u16 valarm_counter; // For V-Alarm interrupt occurrence counter

  u8 reserved_e[2];

  u8 MacAddress[WM_SIZE_MACADDR];
  u16 mode; // Parent: WL_CMDLABEL_MODE_PARENT, Child: WL_CMDLABEL_MODE_CHILD

  WMParentParam pparam; // Parent parameter
  u8 childMacAddress[WM_NUM_MAX_CHILD][WM_SIZE_MACADDR];
  u16 child_bitmap; // Connection status of children (used as poll bitmap)

  WMBssDesc *pInfoBuf; // Pointer to the buffer that is used for storing scanned
                       // parent information (given as an ARM9 API argument)
  u16 aid;             // Current AID
  u8 parentMacAddress[WM_SIZE_MACADDR];
  u16 scan_channel; // Channel that is currently doing Scan

  u8 reserved_f[4];

  u16 wepMode;   // WEP mode (how many bits is WEP?)
  BOOL wep_flag; // Indicates whether other party is requesting WEP
  u16 wepKey[WM_SIZE_WEPKEY / sizeof(u16)]; // WEP key (20Byte * 4)

  u16 rate;     // Utilization rate (WL_CMDLABEL_RATE_AUTO, 1M, 2M)
  u16 preamble; // Long:0, Short:1

  u16 tmptt;
  u16 retryLimit;

  u16 enableChannel;

  u16 allowedChannel;

  u16 portSeqNo[WM_NUM_MAX_CHILD + 1][WM_NUM_OF_SEQ_PORT]; // 16*8*2 = 256 Bytes

  WMPortSendQueueData sendQueueData[WM_SEND_QUEUE_NUM]; // 32*32 = 1024 bytes
  WMPortSendQueue sendQueueFreeList;                    // 4 Bytes
  WMPortSendQueue sendQueue[WM_PRIORITY_LEVEL];         // 16 Bytes
  WMPortSendQueue readyQueue[WM_PRIORITY_LEVEL];        // 16 Bytes
  OSMutex sendQueueMutex;                               // 20 bytes
  BOOL sendQueueInUse;                                  // MP send wait flag

  OSTick mp_lastRecvTick[1 + WM_NUM_MAX_CHILD];
  OSTick mp_lifeTimeTick;

  u16 mp_current_minFreq; // Number of MP cycles in one picture frame (minimum)
                          // (un-implemented)
  u16 mp_current_freq;    // Number of MP cycles in one picture frame (standard
                          // value)
  u16 mp_current_maxFreq; // Number of MP cycles in one picture frame (maximum
                          // value)
  u16 mp_current_minPollBmpMode;   // Mode that minimizes to fit the send target
                                   // of the packet that sends PollBitmap
  u16 mp_current_singlePacketMode; // Mode that only sends 1 packet in 1 MP
  u16 mp_current_defaultRetryCount; // Default number of retries for
                                    // communication on a port that does not do
                                    // Seq control
  u16 mp_current_ignoreFatalErrorMode; // Whether to ignore fatal errors
  u8 reserved_g[2];
} WMStatus, WMstatus;

typedef struct WMArm7Buf {
  WMStatus *status; // WM status buffer

  u8 reserved_a[4];
  u32 *fifo7to9; // Buffer pointer for FIFO(ARM7->9)

  u8 reserved_b[4];

  WMBssDesc connectPInfo; // Connection target parent information (190 bytes)

  u32 requestBuf[512 / sizeof(u32)];

} WMArm7Buf, WMarm7Buf;

#define WM_NUM_OF_CALLBACK (WM_APIID_ASYNC_KIND_MAX - 2)

typedef struct WMArm9Buf {

  WMArm7Buf *WM7;   // ARM7 side WM variable area (no access from ARM9)
  WMStatus *status; // WM status buffer (read only from ARM9)
  u32 *indbuf;      // Indication buffer
  u32 *fifo9to7;    // Buffer for FIFO (ARM9->7)
  u32 *fifo7to9;    // Buffer for FIFO (ARM7->9)

  u16 dmaNo;        // DMA number that WM uses
  u16 scanOnlyFlag; // Scan only mode flag

  WMCallbackFunc
      CallbackTable[WM_NUM_OF_CALLBACK]; // Callback table (42*4 = 168 bytes)
  WMCallbackFunc indCallback;

  WMCallbackFunc portCallbackTable[WM_NUM_OF_PORT]; // 16*4 = 64 bytes
  void *portCallbackArgument[WM_NUM_OF_PORT];       // 16*4 = 64 bytes
  u32 connectedAidBitmap; // List of connection targets (allocate as u32 type
                          // since bitwise operations are used frequently)
  u16 myAid;              // Current AID

  u8 reserved1[WM_ARM9WM_BUF_SIZE - (20 + 4 + 4 * WM_NUM_OF_CALLBACK + 4 +
                                     (4 + 4) * WM_NUM_OF_PORT + 6)];

} WMArm9Buf, WMarm9Buf;

typedef struct WMStartScanReq {
  u16 apiid;
  u16 channel;
  WMBssDesc *scanBuf;
  u16 maxChannelTime;
  u8 bssid[WM_SIZE_BSSID];

} WMStartScanReq, WMstartScanReq;

typedef struct WMStartScanExReq {
  u16 apiid;
  u16 channelList;
  WMBssDesc *scanBuf;
  u16 scanBufSize;
  u16 maxChannelTime;
  u8 bssid[WM_SIZE_BSSID];
  u16 scanType;
  u16 ssidLength;
  u8 ssid[WM_SIZE_SSID];
#if WM_SSID_MASK_CUSTOMIZE
  u16 ssidMatchLength;
  u16 rsv[2];
#else
  u16 rsv[3];
#endif
} WMStartScanExReq, WMstartScanExReq;

typedef struct WMStartConnectReq {
  u16 apiid;
  u16 reserved;
  WMBssDesc *pInfo;
  u8 ssid[WM_SIZE_CHILD_SSID];
  BOOL powerSave;
  u16 reserved2;
  u16 authMode;

} WMStartConnectReq, WMstartConnectReq;

typedef struct WMMeasureChannelReq {
  u16 apiid;
  u16 ccaMode;
  u16 edThreshold;
  u16 channel;
  u16 measureTime;

} WMMeasureChannelReq, WMmeasureChannelReq;

typedef struct WMSetMPParameterReq {
  u16 apiid;
  u16 reserved;

  WMMPParam param;
} WMSetMPParameterReq;

typedef struct WMStartMPReq {
  u16 apiid;
  u16 rsv1;
  u32 *recvBuf;
  u32 recvBufSize;
  u32 *sendBuf;
  u32 sendBufSize;

  WMMPParam param; // Unused
  WMMPTmpParam tmpParam;
} WMStartMPReq;

typedef struct WMStartTestModeReq {
  u16 apiid;
  u16 control;
  u16 signal;
  u16 rate;
  u16 channel;
} WMStartTestModeReq, WMstartTestModeReq;

typedef struct WMStartTestRxModeReq {
  u16 apiid;
  u16 channel;
} WMStartTestRxModeReq, WMstartTestRxModeReq;

typedef struct WMCallback {
  u16 apiid;
  u16 errcode;
  u16 wlCmdID;
  u16 wlResult;

} WMCallback;

typedef struct WMStartParentCallback {
  u16 apiid;
  u16 errcode;
  u16 wlCmdID;
  u16 wlResult;
  u16 state; // PARENT_START, BEACON_SENT, CHILD_CONNECTED, DISCONNECTED
  u8 macAddress[WM_SIZE_MACADDR]; // MAC address of each child that was
                                  // connected
  u16 aid;
  u16 reason;                  // Reason code at time of disconnect
  u8 ssid[WM_SIZE_CHILD_SSID]; // SSID (child information)
  u16 parentSize;
  u16 childSize;

} WMStartParentCallback, WMstartParentCallback;

typedef struct WMStartScanCallback {
  u16 apiid;
  u16 errcode;
  u16 wlCmdID;
  u16 wlResult;
  u16 state;                      // SCAN_START, PARENT_NOT_FOUND, PARENT_FOUND
  u8 macAddress[WM_SIZE_MACADDR]; // MAC address of the parent that was found
  u16 channel;
  u16 linkLevel;
  u16 ssidLength;
  u16 ssid[WM_SIZE_SSID / sizeof(u16)];
  u16 gameInfoLength;
  WMGameInfo gameInfo;

} WMStartScanCallback, WMstartScanCallback;

typedef struct WMStartScanExCallback {
  u16 apiid;
  u16 errcode;
  u16 wlCmdID;
  u16 wlResult;
  u16 state;        // SCAN_START, PARENT_NOT_FOUND, PARENT_FOUND
  u16 channelList;  // Scanned channel list, regardless of whether found or not.
  u8 reserved[2];   // padding
  u16 bssDescCount; // Number of parents that were found
  WMBssDesc *bssDesc[WM_SCAN_EX_PARENT_MAX]; // Beginning address of the parent
                                             // information
  u16 linkLevel[WM_SCAN_EX_PARENT_MAX];      // Reception signal strength

} WMStartScanExCallback, WMstartScanExCallback;

typedef struct WMStartConnectCallback {
  u16 apiid;
  u16 errcode; // PARENT_NOT_FOUND (timeout)
  u16 wlCmdID;
  u16 wlResult;
  u16 state;  // CONNECT_START, BEACON_LOST, CONNECTED, DISCONNECTED
  u16 aid;    // Only when CONNECTED. AID assigned to self
  u16 reason; // Reason code at time of disconnect
  u16 wlStatus;
  u8 macAddress[WM_SIZE_MACADDR];
  u16 parentSize;
  u16 childSize;

} WMStartConnectCallback, WMstartConnectCallback;

typedef struct WMDisconnectCallback {
  u16 apiid;
  u16 errcode;
  u16 wlCmdID;
  u16 wlResult;
  u16 tryBitmap;
  u16 disconnectedBitmap;
} WMDisconnectCallback;

typedef struct WMSetMPParameterCallback {
  u16 apiid;
  u16 errcode;
  u32 mask;
  WMMPParam oldParam;
} WMSetMPParameterCallback;

typedef struct WMStartMPCallback {
  u16 apiid;
  u16 errcode;
  u16 state;
  u8 reserved[2];       // for padding 4-byte align
  WMMpRecvBuf *recvBuf; // Receive buffer

  u16 timeStamp;
  u16 rate_rssi;
  u8 destAdrs[6];
  u8 srcAdrs[6];
  u16 seqNum;
  u16 tmptt;
  u16 pollbmp;
  u16 reserved2;
} WMStartMPCallback, WMstartMPCallback;

typedef struct WMStartDCFCallback {
  u16 apiid;
  u16 errcode;
  u16 state;
  u8 reserved[2];        // for padding 4-byte align
  WMDcfRecvBuf *recvBuf; // Receive buffer

} WMStartDCFCallback, WMstartDCFCallback;

typedef struct WMMeasureChannelCallback {
  u16 apiid;
  u16 errcode;
  u16 wlCmdID;
  u16 wlResult;
  u16 channel;
  u16 ccaBusyRatio;
} WMMeasureChannelCallback, WMmeasureChannelCallback;

typedef struct WMGetWirelessCounterCallback {
  u16 apiid;
  u16 errcode;
  u16 wlCmdID;
  u16 wlResult;
  u32 TX_Success;   // Number of successful sends
  u32 TX_Failed;    // Number of failed sends
  u32 TX_Retry;     // Number of resends
  u32 TX_AckError;  // Number of times desired ACK frame could not be received
  u32 TX_Unicast;   // Number of successfully sent Unicast addressed frames
  u32 TX_Multicast; // Number of successfully sent Multicast addressed frames
  u32 TX_WEP;       // Number of successfully sent encrypted frames
  u32 TX_Beacon;    // Number of successfully sent Beacon frames
  u32 RX_RTS; // NOTE: Number of times of receiving RTS and then performing CTS
              // response
  u32 RX_Fragment;  // Number of times fragmented frames received
  u32 RX_Unicast;   // Number of times Unicast addressed frames received
  u32 RX_Multicast; // Number of times Multicast addressed frames received
  u32 RX_WEP;       // Note: Number of times received encrypted frames that were
                    // successfully decrypted
  u32 RX_Beacon;    // Number of times a Beacon frame was received
  u32 RX_FCSError;  // Note: Number of times an FCS error occurred
  u32 RX_DuplicateError;   // Note: Number of times redundant errors occurred
  u32 RX_MPDuplicateError; // Note: Number of times an MP frame duplicate error
                           // occurred
  u32 RX_ICVError;         // Note: Number of times ICV errors occurred
  u32 RX_FrameCtrlError;   // Note: Number of times received frames that have
                           // illegal FrameCtrl
  u32 RX_LengthError; // Note: Number of times received frames that are longer
                      // or shorter than the prescribed length
  u32 RX_PLCPError;   // Note: Number of times that PLCP CRC error occurred
  u32 RX_BufferOverflowError; // Note: Number of times the receive buffer
                              // overflowed
  u32 RX_PathError; // Note: Number of times receive block internal errors
                    // occurred
  u32 RX_RateError; // Note: Number of times received frames that have an
                    // illegal receive rate
  u32 RX_FCSOK;   // Note: Number of times received frames that have normal FCS
  u32 TX_MP;      // Number of times MP frame sent
  u32 TX_KeyData; // Number of times MP-KeyData frames sent
  u32 TX_NullKey; // Number of times MP-NullKey frames sent
  u32 RX_MP;      // Number of times proper MP frames received
  u32 RX_MPACK;   // Number of times proper MPACK frames received
  u32 MPKeyResponseError[15]; // Number of times proper response not made to Key
                              // request

} WMGetWirelessCounterCallback, WMgetWirelessCounterCallback;

typedef struct WMIndCallback {
  u16 apiid;
  u16 errcode;
  u16 state;
  u16 reason;

} WMIndCallback, WMindCallback;

typedef struct WMPortSendCallback {
  u16 apiid;
  u16 errcode;
  u16 wlCmdID;
  u16 wlResult;
  u16 state;
  u16 port;
  u16 destBitmap;
  u16 restBitmap;
  u16 sentBitmap;
  u16 rsv;
  const u16 *data;
  union {
    u16 size;
    u16 length;
  };
  u16 seqNo;
  WMCallbackFunc callback;
  void *arg;
  u16 maxSendDataSize;
  u16 maxRecvDataSize;

} WMPortSendCallback;

typedef struct WMPortRecvCallback {
  u16 apiid;
  u16 errcode;
  u16 state;
  u16 port; // Port number
  WMMpRecvBuf
      *recvBuf; // Receive buffer // For compatibility with WMStartMPCallback
  u16 *data;    // Receive data (in the case of receive data addressed to port,
                // reference this)
  u16 length;   // Length of received data
  u16 aid;      // Send source AID
  u8 macAddress[WM_SIZE_MACADDR];
  u16 seqNo;              // seqNo
  void *arg;              // Argument passed to callback (append on WM9 side)
  u16 myAid;              // One's own AID
  u16 connectedAidBitmap; // AID Bitmap during connection
  u8 ssid[WM_SIZE_CHILD_SSID]; // SSID (child information)
  u16 reason;                  // Reason code at time of disconnect
  u16 rssi;                    // RSSI
  u16 maxSendDataSize;         // Current possible send size
  u16 maxRecvDataSize;         // Current possible receive size

} WMPortRecvCallback;

typedef struct WMBeaconRecvIndCallback {
  u16 apiid;
  u16 errcode;
  u16 state;
  u16 tgid;
  u16 wmstate;
  u16 gameInfoLength;
  WMGameInfo gameInfo;
} WMBeaconRecvIndCallback;

typedef struct WMStartTestModeCallback {
  u16 apiid;
  u16 errcode;
  u32 RFadr5;
  u32 RFadr6;
  u16 PllLockCheck;
  u16 RFMDflag;

} WMStartTestModeCallback;

typedef struct WMStopTestRxModeCallback {
  u16 apiid;
  u16 errcode;
  u32 fcsOk;
  u32 fcsErr;
} WMStopTestRxModeCallback;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_WM_COMMON_WM_H_ */
