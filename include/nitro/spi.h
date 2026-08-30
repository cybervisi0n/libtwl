#ifndef NITRO_SPI_H_
#define NITRO_SPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/spi/common/config.h>
#include <nitro/spi/common/type.h>

#if defined(SDK_ARM9) || defined(SDK_PORT)

#include <nitro/spi/ARM9/tp.h>
#include <nitro/spi/ARM9/mic.h>
#include <nitro/spi/ARM9/pm.h>

#else // SDK_ARM7

#include <nitro/spi/ARM7/spi.h>
#include <nitro/spi/ARM7/pm.h>

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
