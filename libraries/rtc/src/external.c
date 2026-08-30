#include <nitro/os.h>
#include <nitro/rtc.h>

#ifdef SDK_TWL
#include <twl/rtc/ARM9/api_ex.h>
#include "private.h"
#endif

#ifndef SDK_TWL

typedef enum RTCLock {
  RTC_LOCK_OFF = 0, // Unlock status
  RTC_LOCK_ON,      // Lock status
  RTC_LOCK_MAX
} RTCLock;

typedef enum RTCSequence {
  RTC_SEQ_GET_DATE = 0,      // Sequence for getting dates
  RTC_SEQ_GET_TIME,          // Sequence for getting times
  RTC_SEQ_GET_DATETIME,      // Sequence for getting dates and times
  RTC_SEQ_SET_DATE,          // Sequence for setting dates
  RTC_SEQ_SET_TIME,          // Sequence for setting times
  RTC_SEQ_SET_DATETIME,      // Sequence for setting dates and times
  RTC_SEQ_GET_ALARM1_STATUS, // Sequence for getting alarm 1's status
  RTC_SEQ_GET_ALARM2_STATUS, // Sequence for getting alarm 2's status
  RTC_SEQ_GET_ALARM_PARAM,   // Sequence for getting alarm setting values
  RTC_SEQ_SET_ALARM1_STATUS, // Sequence for changing alarm 1's status
  RTC_SEQ_SET_ALARM2_STATUS, // Sequence for changing alarm 2's status
  RTC_SEQ_SET_ALARM1_PARAM,  // Sequence for changing alarm 1's setting values
  RTC_SEQ_SET_ALARM2_PARAM,  // Sequence for changing alarm 2's setting values
  RTC_SEQ_SET_HOUR_FORMAT,   // Sequence for changing the format of the time
                             // notation
  RTC_SEQ_SET_REG_STATUS2,   // Sequence for status 2 register writes
  RTC_SEQ_SET_REG_ADJUST,    // Sequence for adjust register writes
  RTC_SEQ_MAX
} RTCSequence;

typedef struct RTCWork {
  u32 lock;               // Exclusive lock
  RTCCallback callback;   // For saving an asynchronous callback function
  void *buffer[2];        // For storing asynchronous function parameters
  void *callbackArg;      // For saving arguments to the callback function
  u32 sequence;           // For controlling continuous processing mode
  u32 index;              // For controlling continuous processing status
  RTCInterrupt interrupt; // For saving the call function(s) when an alarm
                          // notification occurs
  RTCResult commonResult; // For saving asynchronous function processing results

} RTCWork;

#endif

static u16 rtcInitialized; // Initialized verify flag
static RTCWork rtcWork;    // Structure that combines work variables

static void RtcCommonCallback(PXIFifoTag tag, u32 data, BOOL err);
static u32 RtcBCD2HEX(u32 bcd);
static u32 RtcHEX2BCD(u32 hex);
static BOOL RtcCheckAlarmParam(const RTCAlarmParam *param);
static RTCRawAlarm RtcMakeAlarmParam(const RTCAlarmParam *param);
static BOOL RtcCheckDate(const RTCDate *date, RTCRawDate *raw);
static BOOL RtcCheckTime(const RTCTime *time, RTCRawTime *raw);
static void RtcGetResultCallback(RTCResult result, void *arg);
static void RtcWaitBusy(void);

void RTC_Init(void) {

  if (rtcInitialized) {
    return;
  }
  rtcInitialized = 1;

  rtcWork.lock = RTC_LOCK_OFF;
  rtcWork.callback = NULL;
  rtcWork.interrupt = NULL;
  rtcWork.buffer[0] = NULL;
  rtcWork.buffer[1] = NULL;

  PXI_Init();
  while (!PXI_IsCallbackReady(PXI_FIFO_TAG_RTC, PXI_PROC_ARM7)) {
  }

  PXI_SetFifoRecvCallback(PXI_FIFO_TAG_RTC, RtcCommonCallback);
}

RTCResult RTC_GetDateAsync(RTCDate *date, RTCCallback callback, void *arg) {
  OSIntrMode enabled;

  SDK_ASSERT(date != NULL);
  SDK_ASSERT(callback != NULL);

  enabled = OS_DisableInterrupts();
  if (rtcWork.lock != RTC_LOCK_OFF) {
    (void)OS_RestoreInterrupts(enabled);
    return RTC_RESULT_BUSY;
  }
  rtcWork.lock = RTC_LOCK_ON;
  (void)OS_RestoreInterrupts(enabled);

  rtcWork.sequence = RTC_SEQ_GET_DATE;
  rtcWork.index = 0;
  rtcWork.buffer[0] = (void *)date;
  rtcWork.callback = callback;
  rtcWork.callbackArg = arg;
  if (RTCi_ReadRawDateAsync()) {
    return RTC_RESULT_SUCCESS;
  } else {
    rtcWork.lock = RTC_LOCK_OFF;
    return RTC_RESULT_SEND_ERROR;
  }
}

RTCResult RTC_GetDate(RTCDate *date) {
  rtcWork.commonResult = RTC_GetDateAsync(date, RtcGetResultCallback, NULL);
  if (rtcWork.commonResult == RTC_RESULT_SUCCESS) {
    RtcWaitBusy();
  }
  return rtcWork.commonResult;
}

RTCResult RTC_GetTimeAsync(RTCTime *time, RTCCallback callback, void *arg) {
  OSIntrMode enabled;

  SDK_NULL_ASSERT(time);
  SDK_NULL_ASSERT(callback);

  enabled = OS_DisableInterrupts();
  if (rtcWork.lock != RTC_LOCK_OFF) {
    (void)OS_RestoreInterrupts(enabled);
    return RTC_RESULT_BUSY;
  }
  rtcWork.lock = RTC_LOCK_ON;
  (void)OS_RestoreInterrupts(enabled);

  rtcWork.sequence = RTC_SEQ_GET_TIME;
  rtcWork.index = 0;
  rtcWork.buffer[0] = (void *)time;
  rtcWork.callback = callback;
  rtcWork.callbackArg = arg;
  if (RTCi_ReadRawTimeAsync()) {
    return RTC_RESULT_SUCCESS;
  } else {
    rtcWork.lock = RTC_LOCK_OFF;
    return RTC_RESULT_SEND_ERROR;
  }
}

RTCResult RTC_GetTime(RTCTime *time) {
  rtcWork.commonResult = RTC_GetTimeAsync(time, RtcGetResultCallback, NULL);
  if (rtcWork.commonResult == RTC_RESULT_SUCCESS) {
    RtcWaitBusy();
  }
  return rtcWork.commonResult;
}

RTCResult RTC_GetDateTimeAsync(RTCDate *date, RTCTime *time,
                               RTCCallback callback, void *arg) {
  OSIntrMode enabled;

  SDK_NULL_ASSERT(date);
  SDK_NULL_ASSERT(time);
  SDK_NULL_ASSERT(callback);

  enabled = OS_DisableInterrupts();
  if (rtcWork.lock != RTC_LOCK_OFF) {
    (void)OS_RestoreInterrupts(enabled);
    return RTC_RESULT_BUSY;
  }
  rtcWork.lock = RTC_LOCK_ON;
  (void)OS_RestoreInterrupts(enabled);

  rtcWork.sequence = RTC_SEQ_GET_DATETIME;
  rtcWork.index = 0;
  rtcWork.buffer[0] = (void *)date;
  rtcWork.buffer[1] = (void *)time;
  rtcWork.callback = callback;
  rtcWork.callbackArg = arg;
  if (RTCi_ReadRawDateTimeAsync()) {
    return RTC_RESULT_SUCCESS;
  } else {
    rtcWork.lock = RTC_LOCK_OFF;
    return RTC_RESULT_SEND_ERROR;
  }
}

RTCResult RTC_GetDateTime(RTCDate *date, RTCTime *time) {
  rtcWork.commonResult =
      RTC_GetDateTimeAsync(date, time, RtcGetResultCallback, NULL);
  if (rtcWork.commonResult == RTC_RESULT_SUCCESS) {
    RtcWaitBusy();
  }
  return rtcWork.commonResult;
}

RTCResult RTC_SetDateAsync(const RTCDate *date, RTCCallback callback,
                           void *arg) {
  OSIntrMode enabled;

  SDK_NULL_ASSERT(date);
  SDK_NULL_ASSERT(callback);

  if (!RtcCheckDate(
          date,
          &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->t.date))) {
    return RTC_RESULT_ILLEGAL_PARAMETER;
  }

  enabled = OS_DisableInterrupts();
  if (rtcWork.lock != RTC_LOCK_OFF) {
    (void)OS_RestoreInterrupts(enabled);
    return RTC_RESULT_BUSY;
  }
  rtcWork.lock = RTC_LOCK_ON;
  (void)OS_RestoreInterrupts(enabled);

  rtcWork.sequence = RTC_SEQ_SET_DATE;
  rtcWork.index = 0;
  rtcWork.callback = callback;
  rtcWork.callbackArg = arg;
  if (RTCi_WriteRawDateAsync()) {
    return RTC_RESULT_SUCCESS;
  } else {
    rtcWork.lock = RTC_LOCK_OFF;
    return RTC_RESULT_SEND_ERROR;
  }
}

RTCResult RTC_SetDate(const RTCDate *date) {
  rtcWork.commonResult = RTC_SetDateAsync(date, RtcGetResultCallback, NULL);
  if (rtcWork.commonResult == RTC_RESULT_SUCCESS) {
    RtcWaitBusy();
  }
  return rtcWork.commonResult;
}

RTCResult RTC_SetTimeAsync(const RTCTime *time, RTCCallback callback,
                           void *arg) {
  OSIntrMode enabled;

  SDK_NULL_ASSERT(time);
  SDK_NULL_ASSERT(callback);

  if (!RtcCheckTime(
          time,
          &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->t.time))) {
    return RTC_RESULT_ILLEGAL_PARAMETER;
  }

  enabled = OS_DisableInterrupts();
  if (rtcWork.lock != RTC_LOCK_OFF) {
    (void)OS_RestoreInterrupts(enabled);
    return RTC_RESULT_BUSY;
  }
  rtcWork.lock = RTC_LOCK_ON;
  (void)OS_RestoreInterrupts(enabled);

  rtcWork.sequence = RTC_SEQ_SET_TIME;
  rtcWork.index = 0;
  rtcWork.callback = callback;
  rtcWork.callbackArg = arg;
  if (RTCi_WriteRawTimeAsync()) {
    return RTC_RESULT_SUCCESS;
  } else {
    rtcWork.lock = RTC_LOCK_OFF;
    return RTC_RESULT_SEND_ERROR;
  }
}

RTCResult RTC_SetTime(const RTCTime *time) {
  rtcWork.commonResult = RTC_SetTimeAsync(time, RtcGetResultCallback, NULL);
  if (rtcWork.commonResult == RTC_RESULT_SUCCESS) {
    RtcWaitBusy();
  }
  return rtcWork.commonResult;
}

RTCResult RTC_SetDateTimeAsync(const RTCDate *date, const RTCTime *time,
                               RTCCallback callback, void *arg) {
  OSIntrMode enabled;

  SDK_NULL_ASSERT(date);
  SDK_NULL_ASSERT(time);
  SDK_NULL_ASSERT(callback);

  if (!RtcCheckDate(
          date,
          &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->t.date))) {
    return RTC_RESULT_ILLEGAL_PARAMETER;
  }
  if (!RtcCheckTime(
          time,
          &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->t.time))) {
    return RTC_RESULT_ILLEGAL_PARAMETER;
  }

  enabled = OS_DisableInterrupts();
  if (rtcWork.lock != RTC_LOCK_OFF) {
    (void)OS_RestoreInterrupts(enabled);
    return RTC_RESULT_BUSY;
  }
  rtcWork.lock = RTC_LOCK_ON;
  (void)OS_RestoreInterrupts(enabled);

  rtcWork.sequence = RTC_SEQ_SET_DATETIME;
  rtcWork.index = 0;
  rtcWork.callback = callback;
  rtcWork.callbackArg = arg;
  if (RTCi_WriteRawDateTimeAsync()) {
    return RTC_RESULT_SUCCESS;
  } else {
    rtcWork.lock = RTC_LOCK_OFF;
    return RTC_RESULT_SEND_ERROR;
  }
}

RTCResult RTC_SetDateTime(const RTCDate *date, const RTCTime *time) {
  rtcWork.commonResult =
      RTC_SetDateTimeAsync(date, time, RtcGetResultCallback, NULL);
  if (rtcWork.commonResult == RTC_RESULT_SUCCESS) {
    RtcWaitBusy();
  }
  return rtcWork.commonResult;
}

RTCResult RTCi_SetRegStatus2Async(const RTCRawStatus2 *status2,
                                  RTCCallback callback, void *arg) {
  OSIntrMode enabled;

  SDK_NULL_ASSERT(status2);
  SDK_NULL_ASSERT(callback);

  ((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.status2.intr_mode =
      status2->intr_mode;
  ((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.status2.intr2_mode =
      status2->intr2_mode;
  ((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.status2.test =
      status2->test;

  enabled = OS_DisableInterrupts();
  if (rtcWork.lock != RTC_LOCK_OFF) {
    (void)OS_RestoreInterrupts(enabled);
    return RTC_RESULT_BUSY;
  }
  rtcWork.lock = RTC_LOCK_ON;
  (void)OS_RestoreInterrupts(enabled);

  rtcWork.sequence = RTC_SEQ_SET_REG_STATUS2;
  rtcWork.index = 0;
  rtcWork.callback = callback;
  rtcWork.callbackArg = arg;
  if (RTCi_WriteRawStatus2Async()) {
    return RTC_RESULT_SUCCESS;
  } else {
    rtcWork.lock = RTC_LOCK_OFF;
    return RTC_RESULT_SEND_ERROR;
  }
}

RTCResult RTCi_SetRegStatus2(const RTCRawStatus2 *status2) {
  rtcWork.commonResult =
      RTCi_SetRegStatus2Async(status2, RtcGetResultCallback, NULL);
  if (rtcWork.commonResult == RTC_RESULT_SUCCESS) {
    RtcWaitBusy();
  }
  return rtcWork.commonResult;
}

RTCResult RTCi_SetRegAdjustAsync(const RTCRawAdjust *adjust,
                                 RTCCallback callback, void *arg) {
  OSIntrMode enabled;

  SDK_NULL_ASSERT(adjust);
  SDK_NULL_ASSERT(callback);

  ((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.adjust.adjust =
      adjust->adjust;

  enabled = OS_DisableInterrupts();
  if (rtcWork.lock != RTC_LOCK_OFF) {
    (void)OS_RestoreInterrupts(enabled);
    return RTC_RESULT_BUSY;
  }
  rtcWork.lock = RTC_LOCK_ON;
  (void)OS_RestoreInterrupts(enabled);

  rtcWork.sequence = RTC_SEQ_SET_REG_ADJUST;
  rtcWork.index = 0;
  rtcWork.callback = callback;
  rtcWork.callbackArg = arg;
  if (RTCi_WriteRawAdjustAsync()) {
    return RTC_RESULT_SUCCESS;
  } else {
    rtcWork.lock = RTC_LOCK_OFF;
    return RTC_RESULT_SEND_ERROR;
  }
}

RTCResult RTCi_SetRegAdjust(const RTCRawAdjust *Adjust) {
  rtcWork.commonResult =
      RTCi_SetRegAdjustAsync(Adjust, RtcGetResultCallback, NULL);
  if (rtcWork.commonResult == RTC_RESULT_SUCCESS) {
    RtcWaitBusy();
  }
  return rtcWork.commonResult;
}

RTCResult RTC_GetAlarmStatusAsync(RTCAlarmChan chan, RTCAlarmStatus *status,
                                  RTCCallback callback, void *arg) {
  OSIntrMode enabled;

  SDK_NULL_ASSERT(status);
  SDK_NULL_ASSERT(callback);

  if (chan >= RTC_ALARM_CHAN_MAX) {
    return RTC_RESULT_ILLEGAL_PARAMETER;
  }

  enabled = OS_DisableInterrupts();
  if (rtcWork.lock != RTC_LOCK_OFF) {
    (void)OS_RestoreInterrupts(enabled);
    return RTC_RESULT_BUSY;
  }
  rtcWork.lock = RTC_LOCK_ON;
  (void)OS_RestoreInterrupts(enabled);

  switch (chan) {
  case RTC_ALARM_CHAN_1:
    rtcWork.sequence = RTC_SEQ_GET_ALARM1_STATUS;
    break;
  case RTC_ALARM_CHAN_2:
    rtcWork.sequence = RTC_SEQ_GET_ALARM2_STATUS;
    break;
  }
  rtcWork.index = 0;
  rtcWork.buffer[0] = (void *)status;
  rtcWork.callback = callback;
  rtcWork.callbackArg = arg;
  if (RTCi_ReadRawStatus2Async()) {
    return RTC_RESULT_SUCCESS;
  } else {
    rtcWork.lock = RTC_LOCK_OFF;
    return RTC_RESULT_SEND_ERROR;
  }
}

RTCResult RTC_GetAlarmStatus(RTCAlarmChan chan, RTCAlarmStatus *status) {
  rtcWork.commonResult =
      RTC_GetAlarmStatusAsync(chan, status, RtcGetResultCallback, NULL);
  if (rtcWork.commonResult == RTC_RESULT_SUCCESS) {
    RtcWaitBusy();
  }
  return rtcWork.commonResult;
}

RTCResult RTC_GetAlarmParamAsync(RTCAlarmChan chan, RTCAlarmParam *param,
                                 RTCCallback callback, void *arg) {
  OSIntrMode enabled;

  SDK_NULL_ASSERT(param);
  SDK_NULL_ASSERT(callback);

  if (chan >= RTC_ALARM_CHAN_MAX) {
    return RTC_RESULT_ILLEGAL_PARAMETER;
  }

  enabled = OS_DisableInterrupts();
  if (rtcWork.lock != RTC_LOCK_OFF) {
    (void)OS_RestoreInterrupts(enabled);
    return RTC_RESULT_BUSY;
  }
  rtcWork.lock = RTC_LOCK_ON;
  (void)OS_RestoreInterrupts(enabled);

  rtcWork.sequence = RTC_SEQ_GET_ALARM_PARAM;
  rtcWork.index = 0;
  rtcWork.buffer[0] = (void *)param;
  rtcWork.callback = callback;
  rtcWork.callbackArg = arg;
  if (chan == RTC_ALARM_CHAN_1) {
    if (RTCi_ReadRawAlarm1Async()) {
      return RTC_RESULT_SUCCESS;
    } else {
      rtcWork.lock = RTC_LOCK_OFF;
      return RTC_RESULT_SEND_ERROR;
    }
  }
  if (RTCi_ReadRawAlarm2Async()) {
    return RTC_RESULT_SUCCESS;
  } else {
    rtcWork.lock = RTC_LOCK_OFF;
    return RTC_RESULT_SEND_ERROR;
  }
}

RTCResult RTC_GetAlarmParam(RTCAlarmChan chan, RTCAlarmParam *param) {
  rtcWork.commonResult =
      RTC_GetAlarmParamAsync(chan, param, RtcGetResultCallback, NULL);
  if (rtcWork.commonResult == RTC_RESULT_SUCCESS) {
    RtcWaitBusy();
  }
  return rtcWork.commonResult;
}

void RTC_SetAlarmInterrupt(RTCInterrupt interrupt) {
  rtcWork.interrupt = interrupt;
}

RTCResult RTC_SetAlarmStatusAsync(RTCAlarmChan chan,
                                  const RTCAlarmStatus *status,
                                  RTCCallback callback, void *arg) {
  OSIntrMode enabled;

  SDK_NULL_ASSERT(status);
  SDK_NULL_ASSERT(callback);

  if (chan >= RTC_ALARM_CHAN_MAX) {
    return RTC_RESULT_ILLEGAL_PARAMETER;
  }
  if (*status > RTC_ALARM_STATUS_ON) {
    return RTC_RESULT_ILLEGAL_PARAMETER;
  }

  enabled = OS_DisableInterrupts();
  if (rtcWork.lock != RTC_LOCK_OFF) {
    (void)OS_RestoreInterrupts(enabled);
    return RTC_RESULT_BUSY;
  }
  rtcWork.lock = RTC_LOCK_ON;
  (void)OS_RestoreInterrupts(enabled);

  switch (chan) {
  case RTC_ALARM_CHAN_1:
    rtcWork.sequence = RTC_SEQ_SET_ALARM1_STATUS;
    break;
  case RTC_ALARM_CHAN_2:
    rtcWork.sequence = RTC_SEQ_SET_ALARM2_STATUS;
    break;
  }
  rtcWork.index = 0;
  rtcWork.buffer[0] = (void *)status;
  rtcWork.callback = callback;
  rtcWork.callbackArg = arg;
  if (RTCi_ReadRawStatus2Async()) {
    return RTC_RESULT_SUCCESS;
  } else {
    rtcWork.lock = RTC_LOCK_OFF;
    return RTC_RESULT_SEND_ERROR;
  }
}

RTCResult RTC_SetAlarmStatus(RTCAlarmChan chan, const RTCAlarmStatus *status) {
  rtcWork.commonResult =
      RTC_SetAlarmStatusAsync(chan, status, RtcGetResultCallback, NULL);
  if (rtcWork.commonResult == RTC_RESULT_SUCCESS) {
    RtcWaitBusy();
  }
  return rtcWork.commonResult;
}

RTCResult RTC_SetAlarmParamAsync(RTCAlarmChan chan, const RTCAlarmParam *param,
                                 RTCCallback callback, void *arg) {
  OSIntrMode enabled;
  RTCRawAlarm *pAlarm =
      &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.alarm);
  BOOL result = FALSE;

  SDK_NULL_ASSERT(param);
  SDK_NULL_ASSERT(callback);

  if (chan >= RTC_ALARM_CHAN_MAX) {
    return RTC_RESULT_ILLEGAL_PARAMETER;
  }
  if (!RtcCheckAlarmParam(param)) {
    return RTC_RESULT_ILLEGAL_PARAMETER;
  }

  enabled = OS_DisableInterrupts();
  if (rtcWork.lock != RTC_LOCK_OFF) {
    (void)OS_RestoreInterrupts(enabled);
    return RTC_RESULT_BUSY;
  }
  rtcWork.lock = RTC_LOCK_ON;
  (void)OS_RestoreInterrupts(enabled);

  rtcWork.index = 0;
  rtcWork.callback = callback;
  rtcWork.callbackArg = arg;
  ((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.alarm =
      RtcMakeAlarmParam(param);

  switch (chan) {
  case RTC_ALARM_CHAN_1:

    rtcWork.sequence = RTC_SEQ_SET_ALARM1_PARAM;
    result = RTCi_WriteRawAlarm1Async();
    break;
  case RTC_ALARM_CHAN_2:

    rtcWork.sequence = RTC_SEQ_SET_ALARM2_PARAM;
    result = RTCi_WriteRawAlarm2Async();
    break;
  }
  if (result) {
    return RTC_RESULT_SUCCESS;
  }
  rtcWork.lock = RTC_LOCK_OFF;
  return RTC_RESULT_SEND_ERROR;
}

RTCResult RTC_SetAlarmParam(RTCAlarmChan chan, const RTCAlarmParam *param) {
  rtcWork.commonResult =
      RTC_SetAlarmParamAsync(chan, param, RtcGetResultCallback, NULL);
  if (rtcWork.commonResult == RTC_RESULT_SUCCESS) {
    RtcWaitBusy();
  }
  return rtcWork.commonResult;
}

#ifdef SDK_TWL

RTCWork *RTCi_GetSysWork(void) { return &rtcWork; }

RTCResult RTCi_GetCounterAsync(u32 *count, RTCCallback callback, void *arg) {
  return ((OS_IsRunOnTwl() == FALSE)
              ? RTC_RESULT_INVALID_COMMAND
              : RTCEXi_GetCounterAsync(count, callback, arg));
}

RTCResult RTCi_GetCounter(u32 *count) {
  return ((OS_IsRunOnTwl() == FALSE) ? RTC_RESULT_INVALID_COMMAND
                                     : RTCEXi_GetCounter(count));
}

RTCResult RTCi_GetFoutAsync(u16 *fout, RTCCallback callback, void *arg) {
  return ((OS_IsRunOnTwl() == FALSE)
              ? RTC_RESULT_INVALID_COMMAND
              : RTCEXi_GetFoutAsync(fout, callback, arg));
}

RTCResult RTCi_GetFout(u16 *fout) {
  return ((OS_IsRunOnTwl() == FALSE) ? RTC_RESULT_INVALID_COMMAND
                                     : RTCEXi_GetFout(fout));
}

RTCResult RTCi_SetFoutAsync(const u16 *fout, RTCCallback callback, void *arg) {
  return ((OS_IsRunOnTwl() == FALSE)
              ? RTC_RESULT_INVALID_COMMAND
              : RTCEXi_SetFoutAsync(fout, callback, arg));
}

RTCResult RTCi_SetFout(const u16 *fout) {
  return ((OS_IsRunOnTwl() == FALSE) ? RTC_RESULT_INVALID_COMMAND
                                     : RTCEXi_SetFout(fout));
}
#endif

static void RtcCommonCallback(PXIFifoTag tag, u32 data, BOOL err) {
#pragma unused(tag)

  RTCResult result;
  RTCPxiResult pxiresult;
  u8 command;
  RTCCallback cb;

  if (err) {

    if (rtcWork.index) {
      rtcWork.index = 0;
    }
    if (rtcWork.lock != RTC_LOCK_OFF) {
      rtcWork.lock = RTC_LOCK_OFF;
    }
    if (rtcWork.callback) {
      cb = rtcWork.callback;
      rtcWork.callback = NULL;
      cb(RTC_RESULT_FATAL_ERROR, rtcWork.callbackArg);
    }
    return;
  }

  command = (u8)((data & RTC_PXI_COMMAND_MASK) >> RTC_PXI_COMMAND_SHIFT);
  pxiresult =
      (RTCPxiResult)((data & RTC_PXI_RESULT_MASK) >> RTC_PXI_RESULT_SHIFT);

  if (command == RTC_PXI_COMMAND_INTERRUPT) {

    if (rtcWork.interrupt) {

      rtcWork.interrupt();
    }
    return;
  }

  if (pxiresult == RTC_PXI_RESULT_SUCCESS) {
    result = RTC_RESULT_SUCCESS;
    switch (rtcWork.sequence) {

    case RTC_SEQ_GET_DATE: {
      RTCDate *pDst = (RTCDate *)(rtcWork.buffer[0]);
      RTCRawDate *pSrc =
          &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->t.date);

      pDst->year = RtcBCD2HEX(pSrc->year);
      pDst->month = RtcBCD2HEX(pSrc->month);
      pDst->day = RtcBCD2HEX(pSrc->day);
      pDst->week = RTC_GetDayOfWeek(pDst);
    } break;

    case RTC_SEQ_GET_TIME: {
      RTCTime *pDst = (RTCTime *)(rtcWork.buffer[0]);
      RTCRawTime *pSrc =
          &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->t.time);

      pDst->hour = RtcBCD2HEX(pSrc->hour);
      pDst->minute = RtcBCD2HEX(pSrc->minute);
      pDst->second = RtcBCD2HEX(pSrc->second);
    } break;

    case RTC_SEQ_GET_DATETIME: {
      RTCDate *pDst = (RTCDate *)(rtcWork.buffer[0]);
      RTCRawDate *pSrc =
          &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->t.date);

      pDst->year = RtcBCD2HEX(*(u32 *)pSrc & 0x000000ff);
      pDst->month = RtcBCD2HEX(pSrc->month);
      pDst->day = RtcBCD2HEX(pSrc->day);
      pDst->week = RTC_GetDayOfWeek(pDst);
    }
      {
        RTCTime *pDst = (RTCTime *)(rtcWork.buffer[1]);
        RTCRawTime *pSrc =
            &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->t.time);

        pDst->hour = RtcBCD2HEX(pSrc->hour);
        pDst->minute = RtcBCD2HEX(pSrc->minute);
        pDst->second = RtcBCD2HEX(pSrc->second);
      }
      break;

    case RTC_SEQ_SET_DATE:
    case RTC_SEQ_SET_TIME:
    case RTC_SEQ_SET_DATETIME:

      break;

    case RTC_SEQ_GET_ALARM1_STATUS: {
      RTCAlarmStatus *pDst = (RTCAlarmStatus *)(rtcWork.buffer[0]);
      RTCRawStatus2 *pSrc =
          &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.status2);

      switch (pSrc->intr_mode) {
      case RTC_INTERRUPT_MODE_ALARM:
        *pDst = RTC_ALARM_STATUS_ON;
        break;
      default:
        *pDst = RTC_ALARM_STATUS_OFF;
      }
    } break;

    case RTC_SEQ_GET_ALARM2_STATUS: {
      RTCAlarmStatus *pDst = (RTCAlarmStatus *)(rtcWork.buffer[0]);
      RTCRawStatus2 *pSrc =
          &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.status2);

      if (pSrc->intr2_mode) {
        *pDst = RTC_ALARM_STATUS_ON;
      } else {
        *pDst = RTC_ALARM_STATUS_OFF;
      }
    } break;

    case RTC_SEQ_GET_ALARM_PARAM: {
      RTCAlarmParam *pDst = (RTCAlarmParam *)(rtcWork.buffer[0]);
      RTCRawAlarm *pSrc =
          &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.alarm);

      pDst->week = (RTCWeek)(pSrc->week);
      pDst->hour = RtcBCD2HEX(pSrc->hour);
      pDst->minute = RtcBCD2HEX(pSrc->minute);
      pDst->enable = RTC_ALARM_ENABLE_NONE;
      if (pSrc->we)
        pDst->enable += RTC_ALARM_ENABLE_WEEK;
      if (pSrc->he)
        pDst->enable += RTC_ALARM_ENABLE_HOUR;
      if (pSrc->me)
        pDst->enable += RTC_ALARM_ENABLE_MINUTE;
    } break;

    case RTC_SEQ_SET_ALARM1_STATUS:
      if (rtcWork.index == 0) {
        RTCRawStatus2 *pSrc =
            &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.status2);

        if (*(RTCAlarmStatus *)(rtcWork.buffer[0]) == RTC_ALARM_STATUS_ON) {

          if (pSrc->intr_mode != RTC_INTERRUPT_MODE_ALARM) {

            rtcWork.index++; // Go to next sequence
            pSrc->intr_mode = RTC_INTERRUPT_MODE_ALARM;
            if (!RTCi_WriteRawStatus2Async()) {
              rtcWork.index = 0; // Interrupt the sequence
              result = RTC_RESULT_SEND_ERROR;
            }
          }
        } else {

          if (pSrc->intr_mode != RTC_INTERRUPT_MODE_NONE) {

            rtcWork.index++; // Go to next sequence
            pSrc->intr_mode = RTC_INTERRUPT_MODE_NONE;
            if (!RTCi_WriteRawStatus2Async()) {
              rtcWork.index = 0; // Interrupt the sequence
              result = RTC_RESULT_SEND_ERROR;
            }
          }
        }
      } else {

        rtcWork.index = 0; // End sequence
      }
      break;

    case RTC_SEQ_SET_ALARM2_STATUS:
      if (rtcWork.index == 0) {
        RTCRawStatus2 *pSrc =
            &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.status2);

        if (*(RTCAlarmStatus *)(rtcWork.buffer[0]) == RTC_ALARM_STATUS_ON) {

          if (!pSrc->intr2_mode) {

            rtcWork.index++; // Go to next sequence
            pSrc->intr2_mode = 1;
            if (!RTCi_WriteRawStatus2Async()) {
              rtcWork.index = 0; // Interrupt the sequence
              result = RTC_RESULT_SEND_ERROR;
            }
          }
        } else {

          if (pSrc->intr2_mode) {

            rtcWork.index++; // Go to next sequence
            pSrc->intr2_mode = 0;
            if (!RTCi_WriteRawStatus2Async()) {
              rtcWork.index = 0; // Interrupt the sequence
              result = RTC_RESULT_SEND_ERROR;
            }
          }
        }
      } else {

        rtcWork.index = 0; // End sequence
      }
      break;

    case RTC_SEQ_SET_ALARM1_PARAM:

    case RTC_SEQ_SET_ALARM2_PARAM:

    case RTC_SEQ_SET_HOUR_FORMAT:

    case RTC_SEQ_SET_REG_STATUS2:

    case RTC_SEQ_SET_REG_ADJUST:

      break;

    default:
#ifdef SDK_TWL
      if (OS_IsRunOnTwl() == TRUE) {
        result = RTCEXi_CommonCallback();
        break;
      }
#endif
      result = RTC_RESULT_INVALID_COMMAND;
      rtcWork.index = 0;
    }
  } else {

    rtcWork.index = 0;

    switch (pxiresult) {
    case RTC_PXI_RESULT_INVALID_COMMAND:
      result = RTC_RESULT_INVALID_COMMAND;
      break;
    case RTC_PXI_RESULT_ILLEGAL_STATUS:
      result = RTC_RESULT_ILLEGAL_STATUS;
      break;
    case RTC_PXI_RESULT_BUSY:
      result = RTC_RESULT_BUSY;
      break;
    case RTC_PXI_RESULT_FATAL_ERROR:
    default:
      result = RTC_RESULT_FATAL_ERROR;
    }
  }

  if (rtcWork.index == 0) {

    if (rtcWork.lock != RTC_LOCK_OFF) {
      rtcWork.lock = RTC_LOCK_OFF;
    }

    if (rtcWork.callback) {
      cb = rtcWork.callback;
      rtcWork.callback = NULL;
      cb(result, rtcWork.callbackArg);
    }
  }
}

static u32 RtcBCD2HEX(u32 bcd) {
  u32 hex = 0;
  s32 i;
  s32 w;

  for (i = 0; i < 8; i++) {
    if (((bcd >> (i * 4)) & 0x0000000f) >= 0x0a) {
      return hex; // Stop conversion and forcibly return 0
    }
  }

  for (i = 0, w = 1; i < 8; i++, w *= 10) {
    hex += (((bcd >> (i * 4)) & 0x0000000f) * w);
  }
  return hex;
}

static u32 RtcHEX2BCD(u32 hex) {
  u32 bcd = 0;
  s32 i;
  u32 w;

  if (hex > 99999999) {
    return 0;
  }

  for (i = 0, w = hex; i < 8; i++) {
    bcd += ((w % 10) << (i * 4));
    w /= 10;
  }
  return bcd;
}

static BOOL RtcCheckAlarmParam(const RTCAlarmParam *param) {
  if (param->week >= RTC_WEEK_MAX)
    return FALSE;
  if (param->hour >= 24)
    return FALSE;
  if (param->minute >= 60)
    return FALSE;
  if (param->enable & ~RTC_ALARM_ENABLE_ALL)
    return FALSE;
  return TRUE;
}

static RTCRawAlarm RtcMakeAlarmParam(const RTCAlarmParam *param) {
  RTCRawAlarm dst;

  *((u32 *)(&dst)) = 0;

  if (!RtcCheckAlarmParam(param)) {
    return dst;
  }

  dst.week = (u32)(param->week);

  if (param->hour >= 12) {
    dst.afternoon = 1;
  }
  dst.hour = RtcHEX2BCD(param->hour);

  dst.minute = RtcHEX2BCD(param->minute);

  if (param->enable & RTC_ALARM_ENABLE_WEEK) {
    dst.we = 1;
  }
  if (param->enable & RTC_ALARM_ENABLE_HOUR) {
    dst.he = 1;
  }
  if (param->enable & RTC_ALARM_ENABLE_MINUTE) {
    dst.me = 1;
  }

  return dst;
}

static BOOL RtcCheckDate(const RTCDate *date, RTCRawDate *raw) {

  if (date->year >= 100)
    return FALSE;
  if ((date->month < 1) || (date->month > 12))
    return FALSE;
  if ((date->day < 1) || (date->day > 31))
    return FALSE;
  if (date->week >= RTC_WEEK_MAX)
    return FALSE;

  *(u32 *)raw = RtcHEX2BCD(date->year);
  raw->month = RtcHEX2BCD(date->month);
  raw->day = RtcHEX2BCD(date->day);
  raw->week = (u32)RTC_GetDayOfWeek(
      (RTCDate *)date); // Always recalculate the day of week, don't use the day
                        // of week from the input data
  return TRUE;
}

static BOOL RtcCheckTime(const RTCTime *time, RTCRawTime *raw) {

  if (time->hour >= 24)
    return FALSE;
  if (time->minute >= 60)
    return FALSE;
  if (time->second >= 60)
    return FALSE;

  if (time->hour >= 12) {
    raw->afternoon = 1;
  } else {
    raw->afternoon = 0;
  }
  raw->hour = RtcHEX2BCD(time->hour);
  raw->minute = RtcHEX2BCD(time->minute);
  raw->second = RtcHEX2BCD(time->second);

  return TRUE;
}

static void RtcGetResultCallback(RTCResult result, void *arg) {
#pragma unused(arg)

  rtcWork.commonResult = result;
}

#include <nitro/code32.h>

#ifdef SDK_PORT
static void RtcWaitBusy(void) {}
#else
static asm void RtcWaitBusy(void) {
  ldr r12, = rtcWork.lock loop : ldr r0, [ r12, #0 ] cmp r0,
      #RTC_LOCK_ON beq loop bx lr
}
#include <nitro/codereset.h>
#endif
