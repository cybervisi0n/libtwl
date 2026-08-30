#ifndef NITRO_WVR_COMMON_WVR_COMMON_H_
#define NITRO_WVR_COMMON_WVR_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum WVRResult {
  WVR_RESULT_SUCCESS = 0,    // Success
  WVR_RESULT_OPERATING,      // Successful start of asynchronous process
  WVR_RESULT_DISABLE,        // Component does not support WVR library
  WVR_RESULT_INVALID_PARAM,  // Invalid parameter specified
  WVR_RESULT_FIFO_ERROR,     // PXI request transmission to ARM7 has failed
  WVR_RESULT_ILLEGAL_STATUS, // Requests cannot be executed in this state
  WVR_RESULT_VRAM_LOCKED,    // VRAM locked. Cannot use.
  WVR_RESULT_FATAL_ERROR,    // Unexpected fatal error

  WVR_RESULT_MAX
} WVRResult;

#define WVR_PXI_COMMAND_STARTUP 0x00010000
#define WVR_PXI_COMMAND_TERMINATE 0x00020000

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_WVR_COMMON_WVR_COMMON_H_ */
