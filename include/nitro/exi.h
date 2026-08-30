#ifndef NITRO_EXI_H_
#define NITRO_EXI_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(SDK_ARM9) || defined(SDK_PORT)

#else
#include <nitro/exi/ARM7/genPort.h>
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
