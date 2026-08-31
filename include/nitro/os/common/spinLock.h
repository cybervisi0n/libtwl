#ifndef NITRO_OS_SPINLOCK_H_
#define NITRO_OS_SPINLOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>

#ifndef SDK_TWL
#ifdef SDK_PORT
#include <nitro/hw/X86/mmap_global.h>
#else
#ifdef SDK_ARM9
#include <nitro/hw/ARM9/mmap_global.h>
#else // SDK_ARM7
#include <nitro/hw/ARM7/mmap_global.h>
#endif
#endif
#else // SDK_TWL
#include <twl/hw/common/mmap_shared.h>
#ifdef SDK_PORT
#include <twl/hw/X86/mmap_global.h>
#else
#ifdef SDK_ARM9
#include <twl/hw/ARM9/mmap_global.h>
#else // SDK_ARM7
#include <twl/hw/ARM7/mmap_global.h>
#endif
#endif
#endif // SDK_TWL

#define OS_UNLOCK_ID 0            // ID when lock variable is not locked
#define OS_MAINP_LOCKED_FLAG 0x40 // Main processor lock verification flag
#define OS_MAINP_LOCK_ID_START                                                 \
  0x40 // Starting number of lock ID assignments, for main processor use
#define OS_MAINP_LOCK_ID_END                                                   \
  0x6f //                               Assignment completion number
#define OS_MAINP_DBG_LOCK_ID                                                   \
  0x70 //                               Debugger reservation number
#define OS_MAINP_SYSTEM_LOCK_ID                                                \
  0x7f //                               System reservation number
#define OS_SUBP_LOCKED_FLAG 0x80 //   Lock verification flag by sub-processor
#define OS_SUBP_LOCK_ID_START                                                  \
  0x80 //   Starting number of lock ID assignments, for sub-processor use
#define OS_SUBP_LOCK_ID_END                                                    \
  0xaf //                               Assignment completion number
#define OS_SUBP_DBG_LOCK_ID                                                    \
  0xb0 //                               Debugger reservation number
#define OS_SUBP_SYSTEM_LOCK_ID                                                 \
  0xbf //                               System reservation number

#define OS_LOCK_SUCCESS 0  // Lock success
#define OS_LOCK_ERROR (-1) // Lock error

#define OS_UNLOCK_SUCCESS 0  // Unlock success
#define OS_UNLOCK_ERROR (-2) // Unlock error

#define OS_LOCK_FREE 0 // Unlocking

#define OS_LOCK_ID_ERROR (-3) // Lock ID error

#if(defined(SDK_PORT) && defined(__cplusplus))
typedef volatile struct OSLockWord_struct
#else
typedef volatile struct OSLockWord
#endif
{
  u32 lockFlag;
  u16 ownerID;
  u16 extension;
} OSLockWord;

void OS_InitLock(void);

s32 OS_LockByWord(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp)(void));
s32 OS_LockCartridge(u16 lockID);
s32 OS_LockCard(u16 lockID);

s32 OS_UnlockByWord(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp)(void));
s32 OS_UnlockCartridge(u16 lockID);
s32 OS_UnlockCard(u16 lockID);

s32 OS_UnLockByWord(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp)(void));
s32 OS_UnLockCartridge(u16 lockID);
s32 OS_UnLockCard(u16 lockID);

s32 OS_TryLockByWord(u16 lockID, OSLockWord *lockp, void (*crtlFuncp)(void));
s32 OS_TryLockCartridge(u16 lockID);
s32 OS_TryLockCard(u16 lockID);

u16 OS_ReadOwnerOfLockWord(OSLockWord *lockp);
#define OS_ReadOwnerOfLockCartridge()                                          \
  OS_ReadOwnerOfLockWord((OSLockWord *)HW_CTRDG_LOCK_BUF)
#define OS_ReadOwnerOfLockCard()                                               \
  OS_ReadOwnerOfLockWord((OSLockWord *)HW_CARD_LOCK_BUF)

s32 OS_GetLockID(void);

void OS_ReleaseLockID(u16 lockID);

#ifdef SDK_TWL

#define OSi_SYNCTYPE_SENDER 0
#define OSi_SYNCTYPE_RECVER 1

#define OSi_SYNCVAL_NOT_READY 0
#define OSi_SYNCVAL_READY 1

void OSi_SyncWithOtherProc(int type, void *syncBuf);

static inline void OSi_SetSyncValue(u8 n) {
  *(vu8 *)(HW_INIT_LOCK_BUF + 4) = n;
}
static inline u8 OSi_GetSyncValue(void) {
  return *(vu8 *)(HW_INIT_LOCK_BUF + 4);
}

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
