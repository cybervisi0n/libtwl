#include "../include/card_common.h"
#include "../include/card_event.h"
#include "../include/card_spi.h"
#include <nitro/card/rom.h>

CARDiCommon cardi_common ATTRIBUTE_ALIGN(32);
static CARDiCommandArg cardi_arg ATTRIBUTE_ALIGN(32);

void CARDi_LockResource(CARDiOwner owner, CARDTargetMode target) {
  CARDiCommon *const p = &cardi_common;
  OSIntrMode bak_psr = OS_DisableInterrupts();
  if (p->lock_owner == owner) {
    if (p->lock_target != target) {
      OS_TPanic(
          "card-lock : can not reuse same ID for locking without unlocking!");
    }
  } else {
    while (p->lock_owner != OS_LOCK_ID_ERROR) {
      OS_SleepThread(p->lock_queue);
    }
    p->lock_owner = owner;
    p->lock_target = target;
  }
  ++p->lock_ref;
  (void)OS_RestoreInterrupts(bak_psr);
}

void CARDi_UnlockResource(CARDiOwner owner, CARDTargetMode target) {
  CARDiCommon *p = &cardi_common;
  OSIntrMode bak_psr = OS_DisableInterrupts();
  if ((p->lock_owner != owner) || !p->lock_ref) {
    OS_TPanic("card-unlock : specified ID for unlocking is not locking one!");
  } else {
    if (p->lock_target != target) {
      OS_TPanic(
          "card-unlock : locking target and unlocking one are different!");
    }
    if (!--p->lock_ref) {
      p->lock_owner = OS_LOCK_ID_ERROR;
      p->lock_target = CARD_TARGET_NONE;
      OS_WakeupThread(p->lock_queue);
    }
  }
  (void)OS_RestoreInterrupts(bak_psr);
}

CARDAccessLevel CARDi_GetAccessLevel(void) {
  CARDAccessLevel level = CARD_ACCESS_LEVEL_NONE;
  if (OS_GetBootType() == OS_BOOTTYPE_ROM) {
    level = CARD_ACCESS_LEVEL_FULL;
  } else if (!OS_IsRunOnTwl()) {
    level = CARD_ACCESS_LEVEL_BACKUP;
  }
#ifdef SDK_TWL
  else {
    const CARDRomHeaderTWL *header = CARD_GetOwnRomHeaderTWL();
    BOOL backupPowerOn = FALSE;
    if (header->access_control.game_card_nitro_mode) {
      level |= CARD_ACCESS_LEVEL_ROM;
      backupPowerOn = TRUE;
    } else if (header->access_control.game_card_on) {
      backupPowerOn = TRUE;
    }
    if (backupPowerOn) {
      if (header->access_control.backup_access_read) {
        level |= CARD_ACCESS_LEVEL_BACKUP_R;
      }
      if (header->access_control.backup_access_write) {
        level |= CARD_ACCESS_LEVEL_BACKUP_W;
      }
    }
  }
#endif
  return level;
}

BOOL CARDi_WaitAsync(void) {
  SDK_ASSERT(CARD_IsAvailable());
  return CARDi_WaitForTask(&cardi_common, FALSE, NULL, NULL);
}

BOOL CARDi_TryWaitAsync(void) {
  CARDiCommon *const p = &cardi_common;
  SDK_ASSERT(CARD_IsAvailable());

  return !(p->flag & CARD_STAT_BUSY);
}

void CARDi_InitResourceLock(void) {
  CARDiCommon *p = &cardi_common;
  p->lock_owner = OS_LOCK_ID_ERROR;
  p->lock_ref = 0;
  p->lock_target = CARD_TARGET_NONE;
  OS_InitThreadQueue(p->lock_queue);
}

void CARDi_InitCommand(void) {
  CARDiCommon *p = &cardi_common;

#if defined(SDK_ARM9) || defined(SDK_PORT)
  p->cmd = &cardi_arg;
  MI_CpuFillFast(&cardi_arg, 0x00, sizeof(cardi_arg));
  DC_FlushRange(&cardi_arg, sizeof(cardi_arg));
#else
  p->cmd = CARD_UNSYNCHRONIZED_BUFFER;
#endif

  PXI_SetFifoRecvCallback(PXI_FIFO_TAG_FS, CARDi_OnFifoRecv);
}
