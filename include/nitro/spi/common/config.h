#ifndef NITRO_SPI_COMMON_CONFIG_H_
#define NITRO_SPI_COMMON_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>
#include <nitro/memorymap.h>
#include <nitro/spec.h>

#include <nitro/spi/common/userInfo_ts_300.h>

#undef NVRAM_CONFIG_CONST_ADDRESS // User info position is obtained from header
                                  // offset

#define NVRAM_CONFIG_DATA_OFFSET_ADDRESS 0x0020
#define NVRAM_CONFIG_DATA_OFFSET_SIZE 2
#define NVRAM_CONFIG_DATA_OFFSET_SHIFT 3

#define NVRAM_CONFIG_SAVE_COUNT_MAX                                            \
  0x0080 // Max value of NitroConfigData.saveCount
#define NVRAM_CONFIG_SAVE_COUNT_MASK                                           \
  0x007f // Masks the range of NitroConfigData.saveCount values (0x00-0x7f)

#define NVRAM_CONFIG_MACADDRESS_ADDRESS 0x00036
#define NVRAM_CONFIG_ENABLECHANNEL_ADDRESS 0x0003c

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_SPI_COMMON_CONFIG_H_ */
