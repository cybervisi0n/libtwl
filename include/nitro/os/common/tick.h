#ifndef NITRO_OS_TICK_H_
#define NITRO_OS_TICK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>
#ifndef SDK_TWL
#include <nitro/ioreg.h>
#else
#include <twl/ioreg.h>
#endif

typedef u64 OSTick;

#define OS_SYSTEM_CLOCK HW_SYSTEM_CLOCK

#define OSi_BOUND_SEC1 128154    // 2^8 / (OS_SYSTEM_CLOCK/1000) = 128154.4
#define OSi_BOUND_SEC2 128       // 2^8 / OS_SYSTEM_CLOCK = 128.2
#define OSi_BOUND_TICK1 67108    // 2^8 / (64*1000) = 67108.9
#define OSi_BOUND_TICK2 67108863 // 2^8 / 64 = 67108864

#define OS_MicroSecondsToTicks(usec)                                           \
  ((OSTick)(((OS_SYSTEM_CLOCK / 1000) * (u64)(usec)) / 64 / 1000))
#define OS_MicroSecondsToTicks32(usec)                                         \
  ((OSTick)(((OS_SYSTEM_CLOCK / 1000) * (u32)(usec)) / 64 / 1000))

#define OS_MilliSecondsToTicks(msec)                                           \
  ((OSTick)(((OS_SYSTEM_CLOCK / 1000) * (u64)(msec)) / 64))
#define OS_MilliSecondsToTicks32(msec)                                         \
  ((OSTick)(((OS_SYSTEM_CLOCK / 1000) * (u32)(msec)) / 64))

#define OS_SecondsToTicks(sec) ((OSTick)((OS_SYSTEM_CLOCK * (u64)(sec)) / 64))
#define OS_SecondsToTicks32(sec) ((OSTick)((OS_SYSTEM_CLOCK * (u32)(sec)) / 64))

#define OS_TicksToMicroSeconds(tick)                                           \
  (((u64)(tick) * 64 * 1000) / (OS_SYSTEM_CLOCK / 1000))
#define OS_TicksToMicroSeconds32(tick)                                         \
  (((u32)(tick) * 64 * 1000) / (OS_SYSTEM_CLOCK / 1000))

#define OS_TicksToMilliSeconds(tick)                                           \
  (((u64)(tick) * 64) / (OS_SYSTEM_CLOCK / 1000))
#define OS_TicksToMilliSeconds32(tick)                                         \
  (((u32)(tick) * 64) / (OS_SYSTEM_CLOCK / 1000))

#define OS_TicksToSeconds(tick) (((u64)(tick) * 64) / OS_SYSTEM_CLOCK)
#define OS_TicksToSeconds32(tick) (((u32)(tick) * 64) / OS_SYSTEM_CLOCK)

void OS_InitTick(void);

BOOL OS_IsTickAvailable(void);

OSTick OS_GetTick(void);

u16 OS_GetTickLo(void);

void OS_SetTick(OSTick count);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
