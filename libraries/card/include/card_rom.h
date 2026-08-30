#ifndef NITRO_LIBRARIES_CARD_ROM_H__
#define NITRO_LIBRARIES_CARD_ROM_H__

#include <nitro.h>
#include "../include/card_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CARDTransferInfo {
  u32 command;
  void (*callback)(void *userdata);
  void *userdata;
  u32 src;
  u32 dst;
  u32 len;
  u32 work;
} CARDTransferInfo;

typedef void (*CARDTransferCallbackFunction)(void *userdata);

int CARDi_ReadRomWithCPU(void *userdata, void *buffer, u32 offset, u32 length);

void CARDi_ReadRomWithDMA(CARDTransferInfo *info);

void CARDi_InitRom(void);

void CARDi_CheckPulledOutCore(u32 id);

u32 CARDi_ReadRomIDCore(void);

#ifdef SDK_ARM7_READROM_SUPPORT

void CARDi_ReadRomCore(const void *src, void *dst, u32 len);

#endif // SDK_ARM7_READROM_SUPPORT

u32 CARDi_ReadRomStatusCore(void);

void CARDi_RefreshRomCore(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NITRO_LIBRARIES_CARD_ROM_H__
