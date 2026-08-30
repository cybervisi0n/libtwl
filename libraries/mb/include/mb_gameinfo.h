#ifndef MB_GAME_INFO_H_
#define MB_GAME_INFO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>
#include <nitro/wm.h>
#include <nitro/mb/mb_gameinfo.h>

#define MB_GAMEINFO_PARENT_FLAG (0x0001)
#define MB_GAMEINFO_CHILD_FLAG(__child_aid__) (1 << (__child_aid__))

typedef struct MBGameInfoRecvList {
  MBGameInfo gameInfo; // Parent game information
  WMBssDesc bssDesc;   // Information for a parent connection.
  u32 getFragmentFlag; // Indicates in bits the fragmentary beacon that, at
                       // present, was received
  u32 allFragmentFlag; // Value of fragmentMaxNum converted into bits
  u16 getPlayerFlag;   // Indicates in bits the player flags that, at present,
                       // have already been received
  s16 lifetimeCount; // Lifetime counter for this information (if this parent's
                     // beacon is received, lifetime will be prolonged)
  u16 linkLevel; /* Value (expressed in four stages) that indicates the beacon
                    reception strength from the parent */
  u8 beaconNo;   // Number of the beacon that was last received
  u8 sameBeaconRecvCount; // The number of times the same beacon number was
                          // consecutively received
} MBGameInfoRecvList, MbGameInfoRecvList;

typedef struct MbBeaconRecvStatus {
  u16 usingGameInfoFlag; // Shows in bits the gameInfo array elements being used
                         // to receive game information
  u16 usefulGameInfoFlag; // Receives all beacons at once and shows game
                          // information for which validGameInfoFlag is on

  u16 validGameInfoFlag; // Shows in bits the gameInfo array elements that have
                         // completely received game information
  u16 nowScanTargetFlag; // Shows in bits the current scan target
  s16 nowLockTimeCount;  // Remaining lock time of the current Scan target
  s16 notFoundLockTargetCount; // Number of times the current ScanTarget was
                               // consecutively not found
  u16 scanCountUnit; // Value of the current scan time converted into a count
                     // number
  u8 pad[2];
  MBGameInfoRecvList
      list[MB_GAME_INFO_RECV_LIST_NUM]; // List for receiving game information
} MbBeaconRecvStatus;

typedef enum MbBeaconMsg {
  MB_BC_MSG_GINFO_VALIDATED = 1,
  MB_BC_MSG_GINFO_INVALIDATED,
  MB_BC_MSG_GINFO_LOST,
  MB_BC_MSG_GINFO_LIST_FULL,
  MB_BC_MSG_GINFO_BEACON
} MbBeaconMsg;

typedef void (*MBBeaconMsgCallback)(MbBeaconMsg msg, MBGameInfoRecvList *gInfop,
                                    int index);

typedef void (*MbScanLockFunc)(u8 *macAddress);
typedef void (*MbScanUnlockFunc)(void);

void MB_InitSendGameInfoStatus(void);

void MBi_MakeGameInfo(MBGameInfo *gameInfop, const MBGameRegistry *mbGameRegp,
                      const MBUserInfo *parent);

void MB_UpdateGameInfoMember(MBGameInfo *gameInfop, const MBUserInfo *member,
                             u16 nowPlayerFlag, u16 changePlayerFlag);

void MB_AddGameInfo(MBGameInfo *newGameInfop);

BOOL MB_DeleteGameInfo(MBGameInfo *gameInfop);

void MB_SendGameInfoBeacon(u32 ggid, u16 tgid, u8 attribute);

void MBi_SetBeaconRecvStatusBufferDefault(void);

void MBi_SetBeaconRecvStatusBuffer(MbBeaconRecvStatus *buf);

BOOL MBi_CheckMBParent(WMBssDesc *bssDescp);

void MB_InitRecvGameInfoStatus(void);

BOOL MB_RecvGameInfoBeacon(MBBeaconMsgCallback Callbackp, u16 linkLevel,
                           WMBssDesc *bssDescp);

void MB_CountGameInfoLifetime(MBBeaconMsgCallback Callbackp, BOOL found_parent);

void MBi_SetScanLockFunc(MbScanLockFunc lockFunc, MbScanUnlockFunc unlockFunc);

MBGameInfoRecvList *MB_GetGameInfoRecvList(int index);

const MbBeaconRecvStatus *MB_GetBeaconRecvStatus(void);

void MB_DeleteRecvGameInfo(int index);
void MB_DeleteRecvGameInfoWithoutBssdesc(int index);

#ifdef __cplusplus
}
#endif

#endif // MB_GAME_INFO_H_
