#ifndef NITRO_LIBRARIES_CARD_UTILITY_H__
#define NITRO_LIBRARIES_CARD_UTILITY_H__

#include <nitro/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CARDDmaInterface {
  void (*Recv)(u32 channel, const void *src, void *dst, u32 len);
  void (*Stop)(u32 channel);
} CARDDmaInterface;

const CARDDmaInterface *CARDi_GetDmaInterface(u32 channel);

void CARDi_ICInvalidateSmart(void *buffer, u32 length, u32 threshold);

void CARDi_DCInvalidateSmart(void *buffer, u32 length, u32 threshold);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NITRO_LIBRARIES_CARD_UTILITY_H__
