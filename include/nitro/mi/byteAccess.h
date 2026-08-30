#ifndef NITRO_MI_BYTEACCESS_H_
#define NITRO_MI_BYTEACCESS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SDK_TWL
#include <nitro/types.h>
#else
#include <twl/types.h>
#endif

static inline u8 MI_ReadByte(const void *address) { return *(u8 *)address; }

static inline void MI_WriteByte(void *address, u8 value) {
  *(u8 *)address = value;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
