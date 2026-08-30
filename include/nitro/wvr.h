#ifndef NITRO_WVR_H_
#define NITRO_WVR_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(SDK_ARM9) || defined(SDK_PORT)

#include <nitro/wvr/common/wvr_common.h>
#include <nitro/wvr/ARM9/wvr.h>

#else /* SDK_ARM7 */

#include <nitro/wvr/common/wvr_common.h>
#include <nitro/wvr/ARM7/wvr_sp.h>

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_WVR_H_ */
