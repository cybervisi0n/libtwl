#include <nitro/card/rom.h>
#include <nitro/card/pullOut.h>

#include "card_rom.h"

static CARDPulledOutCallback CARD_UserCallback;

static u32 CARDiSlotResetCount;
static BOOL CARDi_IsPulledOutFlag = FALSE;

static void CARDi_PulledOutCallback(PXIFifoTag tag, u32 data, BOOL err);
static void CARDi_SendtoPxi(u32 data, u32 wait);

void CARD_InitPulledOutCallback(void) {
  PXI_Init();

  CARDiSlotResetCount = 0;
  CARDi_IsPulledOutFlag = FALSE;

  PXI_SetFifoRecvCallback(PXI_FIFO_TAG_CARD, CARDi_PulledOutCallback);

  CARD_UserCallback = NULL;
}

static void CARDi_PulledOutCallback(PXIFifoTag tag, u32 data, BOOL err) {
#pragma unused(tag, err)

  u32 command = data & CARD_PXI_COMMAND_MASK;

  if (command == CARD_PXI_COMMAND_PULLED_OUT) {
    if (CARDi_IsPulledOutFlag == FALSE) {
      BOOL isTerminateImm = TRUE;

      CARDi_IsPulledOutFlag = TRUE;
      CARDi_NotifyEvent(CARD_EVENT_PULLEDOUT, NULL);

      if (CARD_UserCallback) {
        isTerminateImm = CARD_UserCallback();
      }

      if (isTerminateImm) {
        CARD_TerminateForPulledOut();
      }
    }
  } else if (command == CARD_PXI_COMMAND_RESET_SLOT) {
    CARDiSlotResetCount += 1;
    CARDi_IsPulledOutFlag = FALSE;
    CARDi_NotifyEvent(CARD_EVENT_SLOTRESET, NULL);
  } else {
#ifndef SDK_FINALROM
    OS_TPanic("illegal card pxi command.");
#else
    OS_TPanic("");
#endif
  }
}

void CARD_SetPulledOutCallback(CARDPulledOutCallback callback) {
  CARD_UserCallback = callback;
}

BOOL CARD_IsPulledOut(void) { return CARDi_IsPulledOutFlag; }

void CARD_TerminateForPulledOut(void) {

  if (PAD_DetectFold()) {
    (void)PM_ForceToPowerOff();
  }

#ifdef SDK_TWL
  if (OS_IsRunOnTwl()) {

    PMi_ExecutePostExitCallbackList();
  }
#endif // SDK_TWL

  CARDi_SendtoPxi(CARD_PXI_COMMAND_TERMINATE, 1);

  MI_StopAllDma();
#ifdef SDK_TWL
  if (OS_IsRunOnTwl()) {
    MI_StopAllNDma();
  }
#endif

  OS_Terminate();
}

void CARDi_CheckPulledOutCore(u32 id) {

  vu32 iplCardID = *(vu32 *)(HW_BOOT_CHECK_INFO_BUF);

  if (id != (u32)iplCardID) {
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    CARDi_PulledOutCallback(PXI_FIFO_TAG_CARD, CARD_PXI_COMMAND_PULLED_OUT,
                            FALSE);
    (void)OS_RestoreInterrupts(bak_cpsr);
  }
}

void CARD_CheckPulledOut(void) { CARDi_CheckPulledOutCore(CARDi_ReadRomID()); }

static void CARDi_SendtoPxi(u32 data, u32 wait) {
  while (PXI_SendWordByFifo(PXI_FIFO_TAG_CARD, data, FALSE) !=
         PXI_FIFO_SUCCESS) {
    SVC_WaitByLoop((s32)wait);
  }
}

u32 CARDi_GetSlotResetCount(void) { return CARDiSlotResetCount; }

BOOL CARDi_IsPulledOutEx(u32 count) {
  BOOL result = FALSE;
  OSIntrMode bak = OS_DisableInterrupts();
  {
    result = ((count == CARDi_GetSlotResetCount()) && !CARD_IsPulledOut());
  }
  (void)OS_RestoreInterrupts(bak);
  return result;
}
