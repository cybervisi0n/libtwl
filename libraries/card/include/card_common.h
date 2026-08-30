#ifndef NITRO_LIBRARIES_CARD_COMMON_H__
#define NITRO_LIBRARIES_CARD_COMMON_H__

#include <nitro/card/common.h>
#include <nitro/card/backup.h>

#include "../include/card_utility.h"
#include "../include/card_task.h"
#include "../include/card_command.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {

  CARD_STAT_INIT = (1 << 0),

  CARD_STAT_INIT_CMD = (1 << 1),

  CARD_STAT_BUSY = (1 << 2),

  CARD_STAT_TASK = (1 << 3),

  CARD_STAT_WAITFOR7ACK = (1 << 5),

  CARD_STAT_CANCEL = (1 << 6)
};

#define CARD_UNSYNCHRONIZED_BUFFER (void *)0x80000000

typedef enum {
  CARD_TARGET_NONE,
  CARD_TARGET_ROM,
  CARD_TARGET_BACKUP,
  CARD_TARGET_RW
} CARDTargetMode;

typedef u32 CARDAccessLevel;
#define CARD_ACCESS_LEVEL_NONE 0x0000UL
#define CARD_ACCESS_LEVEL_BACKUP_R 0x0001UL
#define CARD_ACCESS_LEVEL_BACKUP_W 0x0002UL
#define CARD_ACCESS_LEVEL_BACKUP                                               \
  (u32)(CARD_ACCESS_LEVEL_BACKUP_R | CARD_ACCESS_LEVEL_BACKUP_W)
#define CARD_ACCESS_LEVEL_ROM 0x0004UL
#define CARD_ACCESS_LEVEL_FULL                                                 \
  (u32)(CARD_ACCESS_LEVEL_BACKUP | CARD_ACCESS_LEVEL_ROM)

typedef s32 CARDiOwner;

typedef struct CARDiCommon {

  CARDiCommandArg *cmd;

  volatile u32 flag;

  u32 priority;

#if defined(SDK_ARM9)

  u32 flush_threshold_ic;
  u32 flush_threshold_dc;
#endif

  volatile CARDiOwner lock_owner; // ==s32 with Error status
  volatile int lock_ref;
  OSThreadQueue lock_queue[1];
  CARDTargetMode lock_target;

  struct {
    OSThread context[1];
    u8 stack[0x400];
  } thread;

#if defined(SDK_ARM7)

  CARDTask task[1];
  CARDTaskQueue task_q[1];

  int command;
  u8 padding[20];
#else

  void (*task_func)(struct CARDiCommon *);

  MIDmaCallback callback;
  void *callback_arg;

  OSThreadQueue busy_q[1];

  u32 src;
  u32 dst;
  u32 len;
  u32 dma;
  const CARDDmaInterface *DmaCall;

  CARDRequest req_type;
  int req_retry;
  CARDRequestMode req_mode;
  OSThread *current_thread_9;

#endif

} CARDiCommon;

SDK_COMPILER_ASSERT(sizeof(CARDiCommon) % 32 == 0);

extern CARDiCommon cardi_common;
extern u32 cardi_rom_base;

BOOL CARDi_ExecuteOldTypeTask(void (*task)(CARDiCommon *), BOOL async);

BOOL CARDi_WaitForTask(CARDiCommon *p, BOOL restart, MIDmaCallback callback,
                       void *callback_arg);

void CARDi_EndTask(CARDiCommon *p);

void CARDi_OldTypeTaskThread(void *arg);

SDK_INLINE CARDTargetMode CARDi_GetTargetMode(void) {
  return cardi_common.lock_target;
}

void CARDi_LockResource(CARDiOwner owner, CARDTargetMode target);

void CARDi_UnlockResource(CARDiOwner owner, CARDTargetMode target);

CARDAccessLevel CARDi_GetAccessLevel(void);

BOOL CARDi_WaitAsync(void);

BOOL CARDi_TryWaitAsync(void);

void CARDi_InitResourceLock(void);

void CARDi_InitCommand(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NITRO_LIBRARIES_CARD_COMMON_H__
