#ifndef NITRO_PXI_COMMON_COMPPARAM_H_
#define NITRO_PXI_COMMON_COMPPARAM_H_

#ifndef SDK_TWL
#include <nitro/types.h>
#include <nitro/memorymap.h>
#else
#include <twl/types.h>
#include <twl/memorymap.h>
#endif

#include <nitro/mi.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline void PXI_SetComponentParam(u32 param) {
  MI_WriteWord(HW_COMPONENT_PARAM, param);
}

static inline u32 PXI_GetComponentParam(void) {
  return MI_ReadWord(HW_COMPONENT_PARAM);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
