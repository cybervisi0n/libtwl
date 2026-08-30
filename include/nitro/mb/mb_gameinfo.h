#ifndef NITRO_MB_MB_GAMEINFO_H_
#define NITRO_MB_MB_GAMEINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>
#include <nitro/mb/mb.h>

#define MB_USER_VOLAT_DATA_SIZE 8

typedef void (*MBSendVolatCallbackFunc)(u32 ggid);

typedef struct MBGameInfoFixed {
  MBIconInfo icon;   // 544B     Icon data
  MBUserInfo parent; //  22B     Parent user information
  u8 maxPlayerNum;   //   1B     Maximum number of players
  u8 pad[1];
  u16 gameName[MB_GAME_NAME_LENGTH];          //  96B     Game title
  u16 gameIntroduction[MB_GAME_INTRO_LENGTH]; // 192B     Description of game
                                              // content
} MBGameInfoFixed, MbGameInfoFixed;

typedef struct MBGameInfoVolatile {
  u8 nowPlayerNum; //   1B:     Current number of players
  u8 pad[1];
  u16 nowPlayerFlag; //   2B:     Indicates, in bits, the player numbers of all
                     //   current players.
  u16 changePlayerFlag; //   2B:     Indicates, with a flag, the player
                        //   information number that was changed in latest
                        //   change.
  MBUserInfo member[MB_MEMBER_MAX_NUM];      // 330B:     Member information
  u8 userVolatData[MB_USER_VOLAT_DATA_SIZE]; //   8B:     Data the user can set
} MBGameInfoVolatile, MbGameInfoVolatile;

typedef enum MbBeaconDataAttr {
  MB_BEACON_DATA_ATTR_FIXED_NORMAL =
      0, /* Fixed data for when there is icon data */
  MB_BEACON_DATA_ATTR_FIXED_NO_ICON, /* Fixed data for when there is no icon
                                        data */
  MB_BEACON_DATA_ATTR_VOLAT /* Member information and other volatile data */
} MBBeaconDataAttr,
    MbBeaconDataAttr;

typedef struct MBGameInfo {
  MBGameInfoFixed fixed;     // Fixed data
  MBGameInfoVolatile volat;  // Volatile data
  u16 broadcastedPlayerFlag; // Indicates in bits player information that has
                             // been broadcast in volatile data.
  u8 dataAttr;               // Data attributes
  u8 seqNoFixed;             // Fixed region's sequence number
  u8 seqNoVolat;             // Volatile region's sequence number
  u8 fileNo;                 // File number
  u8 pad[2];
  u32 ggid;                 // GGID
  struct MBGameInfo *nextp; // Pointer to next GameInfo (unidirectional list)
} MBGameInfo, MbGameInfo;

enum {
  MB_SEND_VOLAT_CALLBACK_TIMING_BEFORE,
  MB_SEND_VOLAT_CALLBACK_TIMING_AFTER,

  MB_SEND_VOLAT_CALLBACK_TIMMING_BEFORE = MB_SEND_VOLAT_CALLBACK_TIMING_BEFORE,
  MB_SEND_VOLAT_CALLBACK_TIMMING_AFTER = MB_SEND_VOLAT_CALLBACK_TIMING_AFTER
};

void MB_SetSendVolatCallback(MBSendVolatCallbackFunc callback, u32 timing);
void MB_SetUserVolatData(u32 ggid, const u8 *userData, u32 size);
void *MB_GetUserVolatData(const WMGameInfo *gameInfo);

#ifdef __cplusplus
}
#endif

#endif // NITRO_MB_MB_GAMEINFO_H_
