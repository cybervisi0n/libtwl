#ifndef NITRO_SPI_ARM9_MIC_H_
#define NITRO_SPI_ARM9_MIC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/spi/common/type.h>
#include <nitro/pxi.h>

typedef enum MICResult {
  MIC_RESULT_SUCCESS = 0,       // Success
  MIC_RESULT_BUSY,              // Exclusion control in effect
  MIC_RESULT_ILLEGAL_PARAMETER, // Illegal parameter
  MIC_RESULT_SEND_ERROR,        // Failed transmission by PXI
  MIC_RESULT_INVALID_COMMAND,   // Unknown command
  MIC_RESULT_ILLEGAL_STATUS,    // Status does not permit execution
  MIC_RESULT_FATAL_ERROR,       // Errors other than those above
  MIC_RESULT_MAX
} MICResult;

typedef enum MICSamplingType {
  MIC_SAMPLING_TYPE_8BIT = 0,     //  8-bit sampling
  MIC_SAMPLING_TYPE_12BIT,        // 12-bit sampling
  MIC_SAMPLING_TYPE_SIGNED_8BIT,  // Signed 8-bit sampling
  MIC_SAMPLING_TYPE_SIGNED_12BIT, // Signed 12-bit sampling
  MIC_SAMPLING_TYPE_12BIT_FILTER_OFF,
  MIC_SAMPLING_TYPE_SIGNED_12BIT_FILTER_OFF,
  MIC_SAMPLING_TYPE_MAX
} MICSamplingType;

typedef enum MICSamplingRate {
  MIC_SAMPLING_RATE_8K = (HW_CPU_CLOCK_ARM7 / 8000),   // Approx. 8.0 kHz
  MIC_SAMPLING_RATE_11K = (HW_CPU_CLOCK_ARM7 / 11025), // Approx. 11.025 kHz
  MIC_SAMPLING_RATE_16K = (HW_CPU_CLOCK_ARM7 / 16000), // Approx. 16.0 kHz
  MIC_SAMPLING_RATE_22K = (HW_CPU_CLOCK_ARM7 / 22050), // Approx. 22.05 kHz
  MIC_SAMPLING_RATE_32K = (HW_CPU_CLOCK_ARM7 / 32000), // Approx. 32.0 kHz
  MIC_SAMPLING_RATE_LIMIT = 1024
} MICSamplingRate;

typedef void (*MICCallback)(MICResult result, void *arg);

typedef struct MICAutoParam {
  MICSamplingType type;      // Sampling type
  void *buffer;              // Pointer to result storage buffer
  u32 size;                  // Buffer size
  u32 rate;                  // Sampling period (ARM7 clock count)
  BOOL loop_enable;          // Enable/disable the loop when buffer is full
  MICCallback full_callback; // Callback when buffer is full
  void *full_arg;            // Argument to specify for the above callbacks

} MICAutoParam;

void MIC_Init(void);

MICResult MIC_DoSamplingAsync(MICSamplingType type, void *buf,
                              MICCallback callback, void *arg);

MICResult MIC_StartAutoSamplingAsync(const MICAutoParam *param,
                                     MICCallback callback, void *arg);

MICResult MIC_StopAutoSamplingAsync(MICCallback callback, void *arg);

MICResult MIC_AdjustAutoSamplingAsync(u32 rate, MICCallback callback,
                                      void *arg);

void *MIC_GetLastSamplingAddress(void);

MICResult MIC_DoSampling(MICSamplingType type, void *buf);

MICResult MIC_StartAutoSampling(const MICAutoParam *param);

MICResult MIC_StopAutoSampling(void);

MICResult MIC_AdjustAutoSampling(u32 rate);

#ifdef SDK_TWL

MICResult MIC_StartLimitedSamplingAsync(const MICAutoParam *param,
                                        MICCallback callback, void *arg);

MICResult MIC_StopLimitedSamplingAsync(MICCallback callback, void *arg);

MICResult MIC_AdjustLimitedSamplingAsync(u32 rate, MICCallback callback,
                                         void *arg);

MICResult MIC_StartLimitedSampling(const MICAutoParam *param);

MICResult MIC_StopLimitedSampling(void);

MICResult MIC_AdjustLimitedSampling(u32 rate);

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_RTC_ARM9_API_H_ */
