#ifndef NITRO_CARD_COMMON_H_
#define NITRO_CARD_COMMON_H_

#include <nitro/card/types.h>

#include <nitro/memorymap.h>
#include <nitro/mi/dma.h>
#include <nitro/os.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CARD_THREAD_PRIORITY_DEFAULT 4

void CARD_Init(void);

BOOL CARD_IsAvailable(void);

BOOL CARD_IsEnabled(void);

void CARD_CheckEnabled(void);

void CARD_Enable(BOOL enable);

u32 CARD_GetThreadPriority(void);

u32 CARD_SetThreadPriority(u32 prior);

CARDResult CARD_GetResultCode(void);

typedef u32 CARDEvent;
#define CARD_EVENT_PULLEDOUT 0x00000001
#define CARD_EVENT_SLOTRESET 0x00000002

typedef void (*CARDHookFunction)(void *, CARDEvent, void *);

typedef struct CARDHookContext {
  struct CARDHookContext *next;
  void *userdata;
  CARDHookFunction callback;
} CARDHookContext;

void CARDi_RegisterHook(CARDHookContext *hook, CARDHookFunction callback,
                        void *arg);

void CARDi_UnregisterHook(CARDHookContext *hook);

void CARDi_NotifyEvent(CARDEvent event, void *arg);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NITRO_CARD_COMMON_H_
