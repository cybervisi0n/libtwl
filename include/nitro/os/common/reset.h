#ifndef NITRO_OS_RESET_H_
#define NITRO_OS_RESET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/os/common/pxi.h>

#define OS_PXI_COMMAND_RESET 0x10
#define OS_PXI_COMMAND_TERMINATE 0x20

#define OS_PXI_COMMAND_MASK 0x00007f00
#define OS_PXI_COMMAND_SHIFT 8
#define OS_PXI_DATA_MASK 0x000000ff
#define OS_PXI_DATA_SHIFT 0

void OS_InitReset(void);

#if defined(SDK_ARM9) || defined(SDK_PORT)
void OS_ResetSystem(u32 parameter);
#else
void OS_ResetSystem(void);
#endif

static inline u32 OS_GetResetParameter(void) {
  return (u32) * (u32 *)HW_RESET_PARAMETER_BUF;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
