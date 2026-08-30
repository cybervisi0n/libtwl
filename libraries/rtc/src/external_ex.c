#include <nitro/os.h>
#include <nitro/rtc.h>
#include <twl/rtc/common/type_ex.h>
#include <twl/rtc/ARM9/api_ex.h>
#include "private.h"

static void RtcexGetResultCallback(RTCResult result, void *arg);
static void RtcexWaitBusy(u32 *lock);

RTCResult RTCEXi_GetCounterAsync(u32 *count, RTCCallback callback, void *arg) {
  OSIntrMode enabled;
  RTCWork *w = RTCi_GetSysWork();

  SDK_NULL_ASSERT(count);
  SDK_NULL_ASSERT(callback);

  enabled = OS_DisableInterrupts();
  if (w->lock != RTC_LOCK_OFF) {
    (void)OS_RestoreInterrupts(enabled);
    return RTC_RESULT_BUSY;
  }
  w->lock = RTC_LOCK_ON;
  (void)OS_RestoreInterrupts(enabled);

  w->sequence = RTC_SEQ_GET_COUNTER;
  w->index = 0;
  w->buffer[0] = (void *)count;
  w->callback = callback;
  w->callbackArg = arg;
  if (RTCEXi_ReadRawCounterAsync() == TRUE) {
    return RTC_RESULT_SUCCESS;
  } else {
    w->lock = RTC_LOCK_OFF;
    return RTC_RESULT_SEND_ERROR;
  }
}

RTCResult RTCEXi_GetCounter(u32 *count) {
  RTCWork *w = RTCi_GetSysWork();

  w->commonResult = RTCEXi_GetCounterAsync(count, RtcexGetResultCallback, NULL);
  if (w->commonResult == RTC_RESULT_SUCCESS) {
    RtcexWaitBusy(&(w->lock));
  }
  return w->commonResult;
}

RTCResult RTCEXi_GetFoutAsync(u16 *fout, RTCCallback callback, void *arg) {
  OSIntrMode enabled;
  RTCWork *w = RTCi_GetSysWork();

  SDK_NULL_ASSERT(fout);
  SDK_NULL_ASSERT(callback);

  enabled = OS_DisableInterrupts();
  if (w->lock != RTC_LOCK_OFF) {
    (void)OS_RestoreInterrupts(enabled);
    return RTC_RESULT_BUSY;
  }
  w->lock = RTC_LOCK_ON;
  (void)OS_RestoreInterrupts(enabled);

  w->sequence = RTC_SEQ_GET_FOUT;
  w->index = 0;
  w->buffer[0] = (void *)fout;
  w->callback = callback;
  w->callbackArg = arg;
  if (RTCEXi_ReadRawFoutAsync() == TRUE) {
    return RTC_RESULT_SUCCESS;
  } else {
    w->lock = RTC_LOCK_OFF;
    return RTC_RESULT_SEND_ERROR;
  }
}

RTCResult RTCEXi_GetFout(u16 *fout) {
  RTCWork *w = RTCi_GetSysWork();

  w->commonResult = RTCEXi_GetFoutAsync(fout, RtcexGetResultCallback, NULL);
  if (w->commonResult == RTC_RESULT_SUCCESS) {
    RtcexWaitBusy(&(w->lock));
  }
  return w->commonResult;
}

RTCResult RTCEXi_SetFoutAsync(const u16 *fout, RTCCallback callback,
                              void *arg) {
  OSIntrMode enabled;
  RTCWork *w = RTCi_GetSysWork();

  SDK_NULL_ASSERT(fout);
  SDK_NULL_ASSERT(callback);

  enabled = OS_DisableInterrupts();
  if (w->lock != RTC_LOCK_OFF) {
    (void)OS_RestoreInterrupts(enabled);
    return RTC_RESULT_BUSY;
  }
  w->lock = RTC_LOCK_ON;
  (void)OS_RestoreInterrupts(enabled);

  ((RTCRawDataEx *)(OS_GetSystemWork()->real_time_clock))->a.fout.fout = *fout;

  w->sequence = RTC_SEQ_SET_FOUT;
  w->index = 0;
  w->callback = callback;
  w->callbackArg = arg;
  if (RTCEXi_WriteRawFoutAsync() == TRUE) {
    return RTC_RESULT_SUCCESS;
  } else {
    w->lock = RTC_LOCK_OFF;
    return RTC_RESULT_SEND_ERROR;
  }
}

RTCResult RTCEXi_SetFout(const u16 *fout) {
  RTCWork *w = RTCi_GetSysWork();

  w->commonResult = RTCEXi_SetFoutAsync(fout, RtcexGetResultCallback, NULL);
  if (w->commonResult == RTC_RESULT_SUCCESS) {
    RtcexWaitBusy(&(w->lock));
  }
  return w->commonResult;
}

RTCResult RTCEXi_CommonCallback(void) {
  RTCWork *w = RTCi_GetSysWork();
  RTCResult result = RTC_RESULT_SUCCESS;

  switch (w->sequence) {

  case RTC_SEQ_GET_COUNTER: {
    u32 *pDst = (u32 *)(w->buffer[0]);
    RTCRawCounter *pSrc =
        &(((RTCRawDataEx *)(OS_GetSystemWork()->real_time_clock))->a.counter);

    *pDst =
        (u32)(pSrc->bytes[0] | (pSrc->bytes[1] << 8) | (pSrc->bytes[2] << 16));
  } break;

  case RTC_SEQ_GET_FOUT: {
    u16 *pDst = (u16 *)(w->buffer[0]);
    RTCRawFout *pSrc =
        &(((RTCRawDataEx *)(OS_GetSystemWork()->real_time_clock))->a.fout);
    *pDst = (u16)(pSrc->fout);
  } break;

  case RTC_SEQ_SET_FOUT:

    break;
  default:

    w->index = 0;
    result = RTC_RESULT_INVALID_COMMAND;
  }
  return result;
}

static void RtcexGetResultCallback(RTCResult result, void *arg) {
#pragma unused(arg)

  RTCi_GetSysWork()->commonResult = result;
}

#include <nitro/code32.h>

#ifdef SDK_PORT
static void RtcexWaitBusy(u32 *lock) {}
#else
static asm void RtcexWaitBusy(u32 *lock) {
loop:
  ldr r1, [r0] cmp r1, #RTC_LOCK_ON beq loop bx lr
}
#include <nitro/codereset.h>
#endif
