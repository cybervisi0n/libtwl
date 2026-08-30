#ifndef NITRO_CARD_PULLOUT_H_
#define NITRO_CARD_PULLOUT_H_

#include <nitro/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef BOOL (*CARDPulledOutCallback)(void);

void CARD_InitPulledOutCallback(void);

BOOL CARD_IsPulledOut(void);

#if defined(SDK_ARM9) || defined(SDK_PORT)

void CARD_SetPulledOutCallback(CARDPulledOutCallback callback);

void CARD_TerminateForPulledOut(void);

void CARD_PulledOutCallbackProc(void);

void CARD_CheckPulledOut(void);

#endif

#ifdef SDK_ARM7

void CARD_CheckPullOut_Polling(void);

#endif

void CARDi_ResetSlotStatus(void);

u32 CARDi_GetSlotResetCount(void);

BOOL CARDi_IsPulledOutEx(u32 count);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NITRO_CARD_PULLOUT_H_
