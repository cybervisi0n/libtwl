#ifndef NITRO_SPI_ARM9_TP_H_
#define NITRO_SPI_ARM9_TP_H_

#include <nitro/misc.h>
#include <nitro/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TP_SAMPLING_FREQUENCY_MAX 16 // Sampling frequency limit

#define TP_TOUCH_OFF 0 // Not being touched
#define TP_TOUCH_ON 1  // Being touched

#define TP_VALIDITY_VALID 0     // Enabled
#define TP_VALIDITY_INVALID_X 1 // Data with invalid x coordinate
#define TP_VALIDITY_INVALID_Y 2 // Data with invalid y coordinate
#define TP_VALIDITY_INVALID_XY                                                 \
  (TP_VALIDITY_INVALID_X |                                                     \
   TP_VALIDITY_INVALID_Y) // Data with invalid both x and y coordinates

typedef enum {
  TP_REQUEST_COMMAND_SAMPLING = 0x0,      // Performs sampling once
  TP_REQUEST_COMMAND_AUTO_ON = 0x1,       // Starts auto sampling
  TP_REQUEST_COMMAND_AUTO_OFF = 0x2,      // Stops auto sampling
  TP_REQUEST_COMMAND_SET_STABILITY = 0x3, // Sets the chattering value
  TP_REQUEST_COMMAND_AUTO_SAMPLING = 0x10 // Receive the result of auto sampling
} TPRequestCommand;

typedef enum {
  TP_REQUEST_COMMAND_FLAG_SAMPLING =
      1 << TP_REQUEST_COMMAND_SAMPLING, // Performs sampling once
  TP_REQUEST_COMMAND_FLAG_AUTO_ON =
      1 << TP_REQUEST_COMMAND_AUTO_ON, // Starts auto sampling
  TP_REQUEST_COMMAND_FLAG_AUTO_OFF =
      1 << TP_REQUEST_COMMAND_AUTO_OFF, // Stops auto sampling
  TP_REQUEST_COMMAND_FLAG_SET_STABILITY =
      1 << TP_REQUEST_COMMAND_SET_STABILITY // Sets the chattering value
} TPRequestCommandFlag;

typedef enum {
  TP_RESULT_SUCCESS = 0,       // Success
  TP_RESULT_INVALID_PARAMETER, // Parameter is abnormal
  TP_RESULT_ILLEGAL_STATUS,    // State that cannot receive instructions
  TP_RESULT_EXCLUSIVE,         // SPI device is busy
  TP_RESULT_PXI_BUSY           // PXI communication with ARM7 is busy
} TPRequestResult;

typedef enum {
  TP_STATE_READY = 0,     // Ready status
  TP_STATE_SAMPLING,      // Currently requesting sampling
  TP_STATE_AUTO_SAMPLING, // Currently auto sampling
  TP_STATE_AUTO_WAIT_END  // Waiting for auto sampling to end
} TPState;

typedef struct {
  u16 x;        // x coordinate ( 0 - 4095 )
  u16 y;        // y coordinate ( 0 - 4095 )
  u16 touch;    // Touch yes/no
  u16 validity; // Valid yes/no
} TPData;

#define TP_CALIBRATE_DOT_SCALE_SHIFT 8    // x dot size precision
#define TP_CALIBRATE_ORIGIN_SCALE_SHIFT 2 // Origin coordinate precision

typedef struct NvTpData {
  s16 x0;           // x origin coordinate
  s16 y0;           // y origin coordinate
  s16 xDotSize;     // x dot size
  s16 yDotSize;     // y dot size
} TPCalibrateParam; // 8 bytes

typedef void (*TPRecvCallback)(TPRequestCommand command, TPRequestResult result,
                               u16 index);

void TP_Init(void);
void TP_SetCallback(TPRecvCallback callback);
void TP_SetCalibrateParam(const TPCalibrateParam *param);

void TP_RequestSamplingAsync(void);
u32 TP_WaitRawResult(TPData *result);
u32 TP_WaitCalibratedResult(TPData *result);
u32 TP_GetCalibratedResult(TPData *result);
void TP_RequestAutoSamplingStartAsync(u16 vcount, u16 frequence,
                                      TPData samplingBufs[], u16 bufSize);
void TP_RequestAutoSamplingStopAsync(void);
void TP_RequestSetStabilityAsync(u8 retry, u16 range);
void TP_WaitBusy(TPRequestCommandFlag command_flgs);
void TP_WaitAllBusy(void);
u32 TP_CheckBusy(TPRequestCommandFlag command_flgs);
u32 TP_CheckError(TPRequestCommandFlag command);

void TP_GetLatestRawPointInAuto(TPData *result);
void TP_GetLatestCalibratedPointInAuto(TPData *result);
u16 TP_GetLatestIndexInAuto(void);
u32 TP_CalcCalibrateParam(TPCalibrateParam *calibrate, u16 raw_x1, u16 raw_y1,
                          u16 dx1, u16 dy1, u16 raw_x2, u16 raw_y2, u16 dx2,
                          u16 dy2);
BOOL TP_GetUserInfo(TPCalibrateParam *calibrate);
void TP_GetCalibratedPoint(TPData *disp, const TPData *raw);
void TP_GetUnCalibratedPoint(u16 *raw_x, u16 *raw_y, u16 dx, u16 dy);

static inline u32 TP_RequestSetStability(u8 retry, u16 range) {
  TP_RequestSetStabilityAsync(retry, range);
  TP_WaitBusy(TP_REQUEST_COMMAND_FLAG_SET_STABILITY);
  return TP_CheckError(TP_REQUEST_COMMAND_FLAG_SET_STABILITY);
}

static inline u32 TP_RequestAutoSamplingStart(u16 vcount, u16 frequence,
                                              TPData samplingBufs[],
                                              u16 bufSize) {
  TP_RequestAutoSamplingStartAsync(vcount, frequence, samplingBufs, bufSize);
  TP_WaitBusy(TP_REQUEST_COMMAND_FLAG_AUTO_ON);
  return TP_CheckError(TP_REQUEST_COMMAND_FLAG_AUTO_ON);
}

static inline u32 TP_RequestAutoSamplingStop(void) {
  TP_RequestAutoSamplingStopAsync();
  TP_WaitBusy(TP_REQUEST_COMMAND_FLAG_AUTO_OFF);
  return TP_CheckError(TP_REQUEST_COMMAND_FLAG_AUTO_OFF);
}

static inline u32 TP_RequestRawSampling(TPData *result) {
  TP_RequestSamplingAsync();
  return TP_WaitRawResult(result);
}

static inline u32 TP_RequestCalibratedSampling(TPData *result) {
  TP_RequestSamplingAsync();
  return TP_WaitCalibratedResult(result);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
