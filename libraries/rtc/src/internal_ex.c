#include <twl/rtc/common/fifo_ex.h>
#include <nitro/pxi.h>
#include "private.h"

static BOOL RtcexSendPxiCommand(u32 command);

BOOL RTCEXi_ReadRawCounterAsync(void) {
  return RtcexSendPxiCommand(RTC_PXI_COMMAND_READ_COUNTER);
}

BOOL RTCEXi_ReadRawFoutAsync(void) {
  return RtcexSendPxiCommand(RTC_PXI_COMMAND_READ_FOUT);
}

BOOL RTCEXi_WriteRawFoutAsync(void) {
  return RtcexSendPxiCommand(RTC_PXI_COMMAND_WRITE_FOUT);
}

BOOL RTCEXi_ReadRawAlarmEx1Async(void) {
  return RtcexSendPxiCommand(RTC_PXI_COMMAND_READ_ALARM_EX1);
}

BOOL RTCEXi_WriteRawAlarmEx1Async(void) {
  return RtcexSendPxiCommand(RTC_PXI_COMMAND_WRITE_ALARM_EX1);
}

BOOL RTCEXi_ReadRawAlarmEx2Async(void) {
  return RtcexSendPxiCommand(RTC_PXI_COMMAND_READ_ALARM_EX2);
}

BOOL RTCEXi_WriteRawAlarmEx2Async(void) {
  return RtcexSendPxiCommand(RTC_PXI_COMMAND_WRITE_ALARM_EX2);
}

static BOOL RtcexSendPxiCommand(u32 command) {
  if (0 > PXI_SendWordByFifo(
              PXI_FIFO_TAG_RTC,
              ((command << RTC_PXI_COMMAND_SHIFT) & RTC_PXI_COMMAND_MASK), 0)) {
    return FALSE;
  }
  return TRUE;
}
