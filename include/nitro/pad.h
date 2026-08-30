#ifndef NITRO_PAD_H_
#define NITRO_PAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/pad/common/pad.h>

#ifdef SDK_ARM9
#else
#ifndef SDK_PORT
#include <nitro/pad/ARM7/xyButton.h>
#endif
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
