#ifndef NITRO_OS_MUTEX_H_
#define NITRO_OS_MUTEX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/os/common/thread.h>

#define OSi_MUTEX_TYPE_SHIFT 24
#define OSi_MUTEX_TYPE_MASK (0xff << OSi_MUTEX_TYPE_SHIFT)
#define OS_MUTEX_TYPE_NONE (0x00 << OSi_MUTEX_TYPE_SHIFT)
#define OS_MUTEX_TYPE_STD (0x10 << OSi_MUTEX_TYPE_SHIFT)
#define OS_MUTEX_TYPE_R (0x20 << OSi_MUTEX_TYPE_SHIFT)
#define OS_MUTEX_TYPE_W (0x30 << OSi_MUTEX_TYPE_SHIFT)

#define OSi_MUTEX_COUNT_MASK 0xffffff

#ifndef SDK_THREAD_INFINITY
typedef struct OSMutex OSMutex;
#endif

#pragma warn_padding off
struct OSMutex {
  OSThreadQueue queue;
  OSThread *thread; // current owner
  s32 count;        // lock count (notice: use upper 1byte as mutex type)

#ifndef SDK_THREAD_INFINITY
  OSMutex *prev; // link for OSThread.queueMutex
  OSMutex *next; // link for OSThread.queueMutex
#else
  OSMutexLink link;
#endif
};
#pragma warn_padding reset

static inline void OS_SetMutexCount(OSMutex *mutex, s32 count) {
  mutex->count = (s32)((mutex->count & OSi_MUTEX_TYPE_MASK) |
                       (count & OSi_MUTEX_COUNT_MASK));
}
static inline s32 OS_GetMutexCount(OSMutex *mutex) {
  return (s32)(mutex->count & OSi_MUTEX_COUNT_MASK);
}
static inline void OS_IncreaseMutexCount(OSMutex *mutex) {
  u32 type = (u32)(mutex->count & OSi_MUTEX_TYPE_MASK);
  mutex->count++;
  mutex->count = (s32)(type | (mutex->count & OSi_MUTEX_COUNT_MASK));
}
static inline void OS_DecreaseMutexCount(OSMutex *mutex) {
  u32 type = (u32)(mutex->count & OSi_MUTEX_TYPE_MASK);
  mutex->count--;
  mutex->count = (s32)(type | (mutex->count & OSi_MUTEX_COUNT_MASK));
}
static inline void OS_SetMutexType(OSMutex *mutex, u32 type) {
  mutex->count = (s32)(type | (mutex->count & OSi_MUTEX_COUNT_MASK));
}
static inline u32 OS_GetMutexType(OSMutex *mutex) {
  return (u32)(mutex->count & OSi_MUTEX_TYPE_MASK);
}

void OS_InitMutex(OSMutex *mutex);

void OS_LockMutex(OSMutex *mutex);

void OS_UnlockMutex(OSMutex *mutex);

BOOL OS_TryLockMutex(OSMutex *mutex);

void OSi_UnlockAllMutex(OSThread *thread);

void OS_LockMutexR(OSMutex *mutex);

void OS_LockMutexW(OSMutex *mutex);

BOOL OS_TryLockMutexR(OSMutex *mutex);

BOOL OS_TryLockMutexW(OSMutex *mutex);

void OS_UnlockMutexR(OSMutex *mutex);

void OS_UnlockMutexW(OSMutex *mutex);

void OS_UnlockMutexRW(OSMutex *mutex);

void OS_LockMutexFromRToW(OSMutex *mutex);

BOOL OS_TryLockMutexFromRToW(OSMutex *mutex);

void OS_LockMutexFromWToR(OSMutex *mutex);

BOOL OS_TryLockMutexFromWToR(OSMutex *mutex);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
