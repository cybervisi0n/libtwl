#ifndef NITRO_CARD_ROM_H_
#define NITRO_CARD_ROM_H_

#include <nitro/card/types.h>

#include <nitro/mi/dma.h>
#include <nitro/mi/exMemory.h>

#ifdef __cplusplus
extern "C" {
#endif

const u8 *CARD_GetRomHeader(void);

const CARDRomHeader *CARD_GetOwnRomHeader(void);

#ifdef SDK_TWL

const CARDRomHeaderTWL *CARD_GetOwnRomHeaderTWL(void);

#endif // SDK_TWL

SDK_INLINE const CARDRomRegion *CARD_GetRomRegionFNT(void) {
  const CARDRomHeader *header = CARD_GetOwnRomHeader();
  return &header->fnt;
}

SDK_INLINE const CARDRomRegion *CARD_GetRomRegionFAT(void) {
  const CARDRomHeader *header = CARD_GetOwnRomHeader();
  return &header->fat;
}

SDK_INLINE const CARDRomRegion *CARD_GetRomRegionOVT(MIProcessor target) {
  const CARDRomHeader *header = CARD_GetOwnRomHeader();
  return (target == MI_PROCESSOR_ARM9) ? &header->main_ovt : &header->sub_ovt;
}

void CARD_LockRom(u16 lock_id);

void CARD_UnlockRom(u16 lock_id);

BOOL CARD_TryWaitRomAsync(void);

void CARD_WaitRomAsync(void);

void CARDi_ReadRom(u32 dma, const void *src, void *dst, u32 len,
                   MIDmaCallback callback, void *arg, BOOL is_async);

SDK_INLINE void CARD_ReadRomAsync(u32 dma, const void *src, void *dst, u32 len,
                                  MIDmaCallback callback, void *arg) {
  CARDi_ReadRom(dma, src, dst, len, callback, arg, TRUE);
}

SDK_INLINE void CARD_ReadRom(u32 dma, const void *src, void *dst, u32 len) {
  CARDi_ReadRom(dma, src, dst, len, NULL, NULL, FALSE);
}

void CARD_GetCacheFlushThreshold(u32 *icache, u32 *dcache);

void CARD_SetCacheFlushThreshold(u32 icache, u32 dcache);

void CARD_GetCacheFlushFlag(BOOL *icache, BOOL *dcache);

void CARD_SetCacheFlushFlag(BOOL icache, BOOL dcache);

u32 CARDi_ReadRomID(void);
void CARDi_RefreshRom(u32 warn_mask);
BOOL CARDi_IsTwlRom(void);

const u8 *CARDi_GetOwnSignature(void);

void CARDi_SetOwnSignature(const void *signature);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
