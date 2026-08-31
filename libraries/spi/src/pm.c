#include <nitro/spi/ARM9/pm.h>
#include <nitro/pxi.h>
#include <nitro/gx.h>
#include <nitro/spi/common/config.h>
#include <nitro/ctrdg.h>
#include <nitro/mb.h>
#ifdef SDK_TWL
#include "../../os/include/application_jump_private.h"
#endif

#ifdef SDK_LINK_ISTD
#pragma warn_extracomma off
#include <istdbglib.h> // Has extra comma in enum
#pragma warn_extracomma reset
#endif

typedef struct {

  BOOL lock;

  PMCallback callback;
  void *callbackArg;

  void *work;
} PMiWork;

#define PMi_LCD_WAIT_SYS_CYCLES 0x360000
#define PMi_PXI_WAIT_TICK 10

#define PMi_COMPARE_GT 0 // Compare by '>'
#define PMi_COMPARE_GE 1 // Compare by '>='

#ifdef SDK_PORT
static
#endif
inline u32 PMi_MakeData1(u32 bit, u32 seq, u32 command, u32 data) {
  return (bit) | ((seq) << SPI_PXI_INDEX_SHIFT) | ((command) << 8) |
         ((data) & 0xff);
}

#ifdef SDK_PORT
static
#endif
inline u32 PMi_MakeData2(u32 bit, u32 seq, u32 data) {
  return (bit) | ((seq) << SPI_PXI_INDEX_SHIFT) | ((data) & 0xffff);
}

static u32 PMi_TryToSendPxiData(u32 *sendData, int num, u16 *retValue,
                                PMCallback callback, void *arg);
static void PMi_TryToSendPxiDataTillSuccess(u32 *sendData, int num);

static u32 PMi_ForceToPowerOff(void);
static void PMi_CallPostExitCallbackAndReset(BOOL isExit);

static void PMi_WaitBusy(void);

#ifdef SDK_PORT
static
#endif
    void PMi_DummyCallback(u32 result, void *arg);

static void PMi_InsertList(PMGenCallbackInfo **listp, PMGenCallbackInfo *info,
                           int priority, int method);
static void PMi_DeleteList(PMGenCallbackInfo **listp, PMGenCallbackInfo *info);
static void PMi_ClearList(PMGenCallbackInfo **listp);
static void PMi_ExecuteList(PMGenCallbackInfo *listp);

#ifdef SDK_TWL
static void PMi_FinalizeDebugger(void);
#include <twl/ltdmain_begin.h>
static void PMi_ProceedToExit(PMExitFactor factor);
static void PMi_ClearPreExitCallback(void);
static void PMi_ClearPostExitCallback(void);
#include <twl/ltdmain_end.h>
#endif

static void PMi_LCDOnAvoidReset(void);

static void PMi_WaitVBlank(void);

static PMiWork PMi_Work;

static volatile BOOL PMi_SleepEndFlag;

static u32 PMi_LCDCount;
static u32 PMi_DispOffCount;

static PMSleepCallbackInfo *PMi_PreSleepCallbackList = NULL;
static PMSleepCallbackInfo *PMi_PostSleepCallbackList = NULL;
#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
static PMExitCallbackInfo *PMi_PreExitCallbackList = NULL;
static PMExitCallbackInfo *PMi_PostExitCallbackList = NULL;
#ifdef SDK_PORT
static PMBatteryLowCallbackInfo PMi_BatteryLowCallbackInfo;
#else
static PMBatteryLowCallbackInfo PMi_BatteryLowCallbackInfo = {NULL, NULL, NULL};
#endif
#include <twl/ltdmain_end.h>
#endif

static u32 PMi_SetAmp(PMAmpSwitch status);
static PMAmpSwitch sAmpSwitch = PM_AMP_OFF;

#ifdef SDK_TWL
static BOOL PMi_AutoExitFlag = TRUE;
#ifndef SDK_FINALROM
static BOOL PMi_ExitSequenceFlag = FALSE;
#endif
static PMExitFactor PMi_ExitFactor = PM_EXIT_FACTOR_NONE;
#endif
static u32 PMi_PreDmaCnt[4];

#define PMi_WAITBUSY_METHOD_CPUMODE (1 << 1)
#define PMi_WAITBUSY_METHOD_CPSR (1 << 2)
#define PMi_WAITBUSY_METHOD_IME (1 << 3)
static BOOL PMi_WaitBusyMethod = PMi_WAITBUSY_METHOD_CPUMODE;

extern void PXIi_HandlerRecvFifoNotEmpty(void);

static void PMi_WaitBusy(void) {
  volatile BOOL *p = &PMi_Work.lock;

  while (*p) {
    if ((PMi_WaitBusyMethod & PMi_WAITBUSY_METHOD_CPUMODE &&
         OS_GetProcMode() == OS_PROCMODE_IRQ) ||
        (PMi_WaitBusyMethod & PMi_WAITBUSY_METHOD_CPSR &&
         OS_GetCpsrIrq() == OS_INTRMODE_IRQ_DISABLE) ||
        (PMi_WaitBusyMethod & PMi_WAITBUSY_METHOD_IME &&
         OS_GetIrq() == OS_IME_DISABLE)) {
      PXIi_HandlerRecvFifoNotEmpty();
    }
  }
}

static void PMi_DummyCallback(u32 result, void *arg) {
  if (arg) {
    *(u32 *)arg = result;
  }
}

static void PMi_CallCallbackAndUnlock(u32 result) {
  PMCallback callback = PMi_Work.callback;
  void *arg = PMi_Work.callbackArg;

  PMi_Work.lock = FALSE;

  if (callback) {
    PMi_Work.callback = NULL;
    (callback)(result, arg);
  }
}

static void PMi_WaitVBlank(void) {
  vu32 vcount = OS_GetVBlankCount();
  while (vcount == OS_GetVBlankCount()) {
  }
}

void PM_Init(void) {
  static u16 PMi_IsInit = FALSE;

  if (PMi_IsInit) {
    return;
  }
  PMi_IsInit = TRUE;

  PMi_Work.lock = FALSE;
  PMi_Work.callback = NULL;

#ifdef SDK_TWL
  *(u32 *)HW_RESET_LOCK_FLAG_BUF = PM_RESET_FLAG_NONE;
#endif

  PXI_Init();
  while (!PXI_IsCallbackReady(PXI_FIFO_TAG_PM, PXI_PROC_ARM7)) {
    SVC_WaitByLoop(100);
  }

  PXI_SetFifoRecvCallback(PXI_FIFO_TAG_PM, PMi_CommonCallback);

  PMi_LCDCount = PMi_DispOffCount = OS_GetVBlankCount();
}

void PMi_CommonCallback(PXIFifoTag tag, u32 data, BOOL err) {
#pragma unused(tag)

  u16 command;
  u16 pxiResult;
  BOOL callCallback = TRUE;

  command = (u16)((data & SPI_PXI_RESULT_COMMAND_MASK) >>
                  SPI_PXI_RESULT_COMMAND_SHIFT);
  pxiResult =
      (u16)((data & SPI_PXI_RESULT_DATA_MASK) >> SPI_PXI_RESULT_DATA_SHIFT);

  if (err) {
    switch (command) {
    case SPI_PXI_COMMAND_PM_SLEEP_START:
    case SPI_PXI_COMMAND_PM_UTILITY:
      pxiResult = PM_RESULT_BUSY;
      break;

    default:
      pxiResult = PM_RESULT_ERROR;
    }

    PMi_CallCallbackAndUnlock(pxiResult);
    return;
  }

  switch (command) {
  case SPI_PXI_COMMAND_PM_SLEEP_START:

    break;

  case SPI_PXI_COMMAND_PM_UTILITY:
    if (PMi_Work.work) {
      *(u16 *)PMi_Work.work = (u16)pxiResult;
    }
    pxiResult = (u16)PM_RESULT_SUCCESS;
    break;

  case SPI_PXI_COMMAND_PM_SYNC:
    pxiResult = (u16)PM_RESULT_SUCCESS;
    break;

  case SPI_PXI_COMMAND_PM_SLEEP_END:
    PMi_SleepEndFlag = TRUE;
    break;

#ifdef SDK_TWL

  case SPI_PXI_COMMAND_PM_NOTIFY:
    switch (pxiResult) {
    case PM_NOTIFY_POWER_SWITCH:
      OS_TPrintf("[ARM9] Pushed power button.\n");
      PMi_ProceedToExit(PM_EXIT_FACTOR_PWSW);
      *(u32 *)HW_RESET_LOCK_FLAG_BUF = PM_RESET_FLAG_FORCED;
      break;

    case PM_NOTIFY_SHUTDOWN:
      OS_TPrintf("[ARM9] Shutdown\n");

      break;

    case PM_NOTIFY_RESET_HARDWARE:
      OS_TPrintf("[ARM9] Reset Hardware\n");

      break;
    case PM_NOTIFY_BATTERY_LOW:
      OS_TPrintf("[ARM9] Battery low\n");
      if (PMi_BatteryLowCallbackInfo.callback) {
        (PMi_BatteryLowCallbackInfo.callback)(PMi_BatteryLowCallbackInfo.arg);
      }
      break;
    case PM_NOTIFY_BATTERY_EMPTY:
      OS_TPrintf("[ARM9] Battery empty\n");
      PMi_ProceedToExit(PM_EXIT_FACTOR_BATTERY);
      *(u32 *)HW_RESET_LOCK_FLAG_BUF = PM_RESET_FLAG_FORCED;
      break;
    default:
      OS_TPrintf("[ARM9] unknown %x\n", pxiResult);
      break;
    }

    callCallback = FALSE;
    break;

#endif /* SDK_TWL */
  }

  if (callCallback) {
    PMi_CallCallbackAndUnlock(pxiResult);
  }
}

static u32 PMi_TryToSendPxiData(u32 *sendData, int num, u16 *retValue,
                                PMCallback callback, void *arg) {
  int n;
  OSIntrMode enabled = OS_DisableInterrupts();

  if (PMi_Work.lock) {
    (void)OS_RestoreInterrupts(enabled);
    return PM_BUSY;
  }
  PMi_Work.lock = TRUE;

  PMi_Work.work = (void *)retValue;
  PMi_Work.callback = callback;
  PMi_Work.callbackArg = arg;

  for (n = 0; n < num; n++) {
    PMi_SendPxiData(sendData[n]);
  }

  (void)OS_RestoreInterrupts(enabled);
  return PM_SUCCESS;
}

#define PMi_UNUSED_RESULT 0xffff0000 // Value that should never be returned
void PMi_TryToSendPxiDataTillSuccess(u32 *sendData, int num) {
  volatile u32 result;
  while (1) {
    result = PMi_UNUSED_RESULT;
    while (PMi_TryToSendPxiData(sendData, num, NULL, PMi_DummyCallback,
                                (void *)&result) != PM_SUCCESS) {
      OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
    }

    while (result == PMi_UNUSED_RESULT) {
      OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
    }
    if (result == SPI_PXI_RESULT_SUCCESS) {
      break;
    }

    OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
  }
}

u32 PMi_SendSleepStart(u16 trigger, u16 keyIntrData) {
  u32 sendData[2];

  sendData[0] = PMi_MakeData1(SPI_PXI_START_BIT | SPI_PXI_END_BIT, 0,
                              SPI_PXI_COMMAND_PM_SYNC, 0);
  PMi_TryToSendPxiDataTillSuccess(sendData, 1);

  while (PMi_SetLCDPower(PM_LCD_POWER_OFF, PM_LED_BLINK_LOW, FALSE, TRUE) !=
         TRUE) {
  }

  sendData[0] = PMi_MakeData1(SPI_PXI_START_BIT, 0,
                              SPI_PXI_COMMAND_PM_SLEEP_START, trigger);
  sendData[1] = PMi_MakeData2(SPI_PXI_END_BIT, 1, keyIntrData);
  PMi_TryToSendPxiDataTillSuccess(sendData, 2);

  return PM_SUCCESS;
}

u32 PM_SendUtilityCommandAsync(u32 number, u16 parameter, u16 *retValue,
                               PMCallback callback, void *arg) {
  u32 sendData[2];

  sendData[0] =
      PMi_MakeData1(SPI_PXI_START_BIT, 0, SPI_PXI_COMMAND_PM_UTILITY, number);
  sendData[1] = PMi_MakeData2(SPI_PXI_END_BIT, 1, parameter);

  return PMi_TryToSendPxiData(sendData, 2, retValue, callback, arg);
}

u32 PM_SendUtilityCommand(u32 number, u16 parameter, u16 *retValue) {
  u32 commandResult;
  u32 sendResult = PM_SendUtilityCommandAsync(
      number, parameter, retValue, PMi_DummyCallback, &commandResult);
  if (sendResult == PM_SUCCESS) {
    PMi_WaitBusy();
    return commandResult;
  }
  return sendResult;
}

u32 PMi_SetLEDAsync(PMLEDStatus status, PMCallback callback, void *arg) {
  u32 command;

  switch (status) {
  case PM_LED_ON:
    command = PM_UTIL_LED_ON;
    break;
  case PM_LED_BLINK_HIGH:
    command = PM_UTIL_LED_BLINK_HIGH_SPEED;
    break;
  case PM_LED_BLINK_LOW:
    command = PM_UTIL_LED_BLINK_LOW_SPEED;
    break;
  default:
    command = 0;
  }

  return (command) ? PM_SendUtilityCommandAsync(command, 0, NULL, callback, arg)
                   : PM_INVALID_COMMAND;
}

u32 PMi_SetLED(PMLEDStatus status) {
  u32 commandResult;
  u32 sendResult = PMi_SetLEDAsync(status, PMi_DummyCallback, &commandResult);
  if (sendResult == PM_SUCCESS) {
    PMi_WaitBusy();
    return commandResult;
  }
  return sendResult;
}

u32 PM_SetBackLightAsync(PMLCDTarget target, PMBackLightSwitch sw,
                         PMCallback callback, void *arg) {
  u32 command = 0;

  if (target == PM_LCD_TOP) {
    if (sw == PM_BACKLIGHT_ON) {
      command = PM_UTIL_LCD2_BACKLIGHT_ON;
    }
    if (sw == PM_BACKLIGHT_OFF) {
      command = PM_UTIL_LCD2_BACKLIGHT_OFF;
    }
  } else if (target == PM_LCD_BOTTOM) {
    if (sw == PM_BACKLIGHT_ON) {
      command = PM_UTIL_LCD1_BACKLIGHT_ON;
    }
    if (sw == PM_BACKLIGHT_OFF) {
      command = PM_UTIL_LCD1_BACKLIGHT_OFF;
    }
  } else if (target == PM_LCD_ALL) {
    if (sw == PM_BACKLIGHT_ON) {
      command = PM_UTIL_LCD12_BACKLIGHT_ON;
    }
    if (sw == PM_BACKLIGHT_OFF) {
      command = PM_UTIL_LCD12_BACKLIGHT_OFF;
    }
  }

  return (command) ? PM_SendUtilityCommandAsync(command, 0, NULL, callback, arg)
                   : PM_INVALID_COMMAND;
}

u32 PM_SetBackLight(PMLCDTarget target, PMBackLightSwitch sw) {
  u32 commandResult;
  u32 sendResult =
      PM_SetBackLightAsync(target, sw, PMi_DummyCallback, &commandResult);
  if (sendResult == PM_SUCCESS) {
    PMi_WaitBusy();
    return commandResult;
  }
  return sendResult;
}

u32 PMi_SetSoundPowerAsync(PMSoundPowerSwitch sw, PMCallback callback,
                           void *arg) {
  u32 command;

  switch (sw) {
  case PM_SOUND_POWER_ON:
    command = PM_UTIL_SOUND_POWER_ON;
    break;
  case PM_SOUND_POWER_OFF:
    command = PM_UTIL_SOUND_POWER_OFF;
    break;
  default:
    command = 0;
  }

  return (command) ? PM_SendUtilityCommandAsync(command, 0, NULL, callback, arg)
                   : PM_INVALID_COMMAND;
}

u32 PMi_SetSoundPower(PMSoundPowerSwitch sw) {
  u32 commandResult;
  u32 sendResult =
      PMi_SetSoundPowerAsync(sw, PMi_DummyCallback, &commandResult);
  if (sendResult == PM_SUCCESS) {
    PMi_WaitBusy();
    return commandResult;
  }
  return sendResult;
}

u32 PMi_SetSoundVolumeAsync(PMSoundVolumeSwitch sw, PMCallback callback,
                            void *arg) {
  u32 command;

  switch (sw) {
  case PM_SOUND_VOLUME_ON:
    command = PM_UTIL_SOUND_VOL_CTRL_ON;
    break;
  case PM_SOUND_VOLUME_OFF:
    command = PM_UTIL_SOUND_VOL_CTRL_OFF;
    break;
  default:
    command = 0;
  }

  return (command) ? PM_SendUtilityCommandAsync(command, 0, NULL, callback, arg)
                   : PM_INVALID_COMMAND;
}

u32 PMi_SetSoundVolume(PMSoundVolumeSwitch sw) {
  u32 commandResult;
  u32 sendResult =
      PMi_SetSoundVolumeAsync(sw, PMi_DummyCallback, &commandResult);
  if (sendResult == PM_SUCCESS) {
    PMi_WaitBusy();
    return commandResult;
  }
  return sendResult;
}

u32 PM_ForceToPowerOffAsync(PMCallback callback, void *arg) {
#ifdef SDK_TWL

  PMi_ExitFactor = PM_EXIT_FACTOR_USER;
#endif

  PMi_LCDOnAvoidReset();

#ifdef SDK_TWL
  if (OS_IsRunOnTwl()) {

    PMi_ExecuteList(PMi_PostExitCallbackList);
  }
#endif

  return PM_SendUtilityCommandAsync(PM_UTIL_FORCE_POWER_OFF, 0, NULL, callback,
                                    arg);
}

static u32 PMi_ForceToPowerOff(void) {
  u32 commandResult;
  u32 sendResult = PM_ForceToPowerOffAsync(PMi_DummyCallback, &commandResult);
  if (sendResult == PM_SUCCESS) {
    PMi_WaitBusyMethod = PMi_WAITBUSY_METHOD_CPSR | PMi_WAITBUSY_METHOD_IME;
    PMi_WaitBusy();
    PMi_WaitBusyMethod = PMi_WAITBUSY_METHOD_CPUMODE;
    return commandResult;
  }
  return sendResult;
}

u32 PM_ForceToPowerOff(void) {
  while (PMi_ForceToPowerOff() != PM_RESULT_SUCCESS) {
    OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
  }

  (void)OS_DisableInterrupts();

  MI_StopAllDma();
#ifdef SDK_TWL
  if (OS_IsRunOnTwl()) {
    MI_StopAllNDma();
  }
#endif

  while (1) {
    OS_Halt();
  }

  return PM_RESULT_SUCCESS;
}

u32 PM_SetAmpAsync(PMAmpSwitch status, PMCallback callback, void *arg) {
  return PM_SendUtilityCommandAsync(PM_UTIL_SET_AMP, (u16)status, NULL,
                                    callback, arg);
}

u32 PM_SetAmp(PMAmpSwitch status) {

  sAmpSwitch = status;
  return PMi_SetAmp(status);
}

static u32 PMi_SetAmp(PMAmpSwitch status) {
  if (PM_GetLCDPower()) {

    return PM_SendUtilityCommand(PM_UTIL_SET_AMP, (u16)status, NULL);
  } else {
    return PM_RESULT_SUCCESS;
  }
}

u32 PM_SetAmpGainAsync(PMAmpGain gain, PMCallback callback, void *arg) {
  return PM_SendUtilityCommandAsync(PM_UTIL_SET_AMPGAIN, (u16)gain, NULL,
                                    callback, arg);
}

u32 PM_SetAmpGain(PMAmpGain gain) {
  return PM_SendUtilityCommand(PM_UTIL_SET_AMPGAIN, (u16)gain, NULL);
}

#ifdef SDK_TWL

u32 PM_SetAmpGainLevelAsync(u8 level, PMCallback callback, void *arg) {
  SDK_ASSERT(level <= PM_AMPGAIN_LEVEL_MAX);
  return PM_SendUtilityCommandAsync(PM_UTIL_SET_AMPGAIN_LEVEL, (u16)level, NULL,
                                    callback, arg);
}

u32 PM_SetAmpGainLevel(u8 level) {
  SDK_ASSERT(level <= PM_AMPGAIN_LEVEL_MAX);
  return PM_SendUtilityCommand(PM_UTIL_SET_AMPGAIN_LEVEL, (u16)level, NULL);
}
#endif

u32 PM_GetBattery(PMBattery *batteryBuf) {
  u16 status;
  u32 result =
      PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_BATTERY, &status);

  if (result == PM_RESULT_SUCCESS) {
    if (batteryBuf) {
      *batteryBuf = status ? PM_BATTERY_LOW : PM_BATTERY_HIGH;
    }
  }
  return result;
}

#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
static u32 PMi_GetBatteryLevelCore(PMBatteryLevel *levelBuf) {
  u16 status;
  u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS,
                                     PM_UTIL_PARAM_BATTERY_LEVEL, &status);

  if (result == PM_RESULT_SUCCESS) {
    if (levelBuf) {
      *levelBuf = (PMBatteryLevel)status;
    }
  }
  return result;
}
#include <twl/ltdmain_end.h>

u32 PM_GetBatteryLevel(PMBatteryLevel *levelBuf) {
  if (OS_IsRunOnTwl()) {
    return PMi_GetBatteryLevelCore(levelBuf);
  } else {
    return PM_RESULT_ERROR;
  }
}

#endif

#ifdef SDK_TWL

#include <twl/ltdmain_begin.h>
static u32 PMi_GetACAdapterCore(BOOL *isConnectedBuf) {
  u16 status;
  u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS,
                                     PM_UTIL_PARAM_AC_ADAPTER, &status);

  if (result == PM_RESULT_SUCCESS) {
    if (isConnectedBuf) {
      *isConnectedBuf = status ? TRUE : FALSE;
    }
  }
  return result;
}
#include <twl/ltdmain_end.h>

u32 PM_GetACAdapter(BOOL *isConnectedBuf) {
  if (OS_IsRunOnTwl()) {
    return PMi_GetACAdapterCore(isConnectedBuf);
  } else {
    return PM_RESULT_ERROR;
  }
}
#endif

u32 PM_GetBackLight(PMBackLightSwitch *top, PMBackLightSwitch *bottom) {
  u16 status;
  u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS,
                                     PM_UTIL_PARAM_BACKLIGHT, &status);

  if (result == PM_RESULT_SUCCESS) {
    if (top) {
      *top = (status & PMIC_CTL_BKLT2) ? PM_BACKLIGHT_ON : PM_BACKLIGHT_OFF;
    }
    if (bottom) {
      *bottom = (status & PMIC_CTL_BKLT1) ? PM_BACKLIGHT_ON : PM_BACKLIGHT_OFF;
    }
  }
  return result;
}

u32 PMi_GetSoundPower(PMSoundPowerSwitch *swBuf) {
  u16 status;
  u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS,
                                     PM_UTIL_PARAM_SOUND_POWER, &status);

  if (result == PM_RESULT_SUCCESS) {
    if (swBuf) {
      *swBuf = status ? PM_SOUND_POWER_ON : PM_SOUND_POWER_OFF;
    }
  }
  return result;
}

u32 PMi_GetSoundVolume(PMSoundVolumeSwitch *swBuf) {
  u16 status;
  u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS,
                                     PM_UTIL_PARAM_SOUND_VOLUME, &status);

  if (result == PM_RESULT_SUCCESS) {
    if (swBuf) {
      *swBuf = status ? PM_SOUND_VOLUME_ON : PM_SOUND_VOLUME_OFF;
    }
  }
  return result;
}

u32 PM_GetAmp(PMAmpSwitch *swBuf) {
  u16 status;
  u32 result =
      PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_AMP, &status);

  if (result == PM_RESULT_SUCCESS) {
    if (swBuf) {
      *swBuf = status ? PM_AMP_ON : PM_AMP_OFF;
    }
  }
  return result;
}

u32 PM_GetAmpGain(PMAmpGain *gainBuf) {
  u16 status;
  u32 result =
      PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_AMPGAIN, &status);

  if (result == PM_RESULT_SUCCESS) {
    if (gainBuf) {
      *gainBuf = (PMAmpGain)status;
    }
  }
  return result;
}

#ifdef SDK_TWL

u32 PM_GetAmpGainLevel(u8 *levelBuf) {
  u16 status;
  u32 result = PM_SendUtilityCommand(PM_UTIL_GET_STATUS,
                                     PM_UTIL_PARAM_AMPGAIN_LEVEL, &status);

  if (result == PM_RESULT_SUCCESS) {
    if (levelBuf) {
      *levelBuf = (u8)status;
    }
  }
  return result;
}
#endif

#ifdef SDK_TWL

u32 PMi_SetWirelessLED(PMWirelessLEDStatus sw) {
  return PM_SendUtilityCommand(PM_UTIL_WIRELESS_LED, (u16)sw, NULL);
}
#endif // ifdef SDK_TWL

void PMi_SendPxiData(u32 data) {
  while (PXI_SendWordByFifo(PXI_FIFO_TAG_PM, data, FALSE) != PXI_FIFO_SUCCESS) {
  }
}

#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
static void PMi_PreSleepForNdma(u32 i) {
  vu32 *ndmaCntp = (vu32 *)MI_NDMA_REGADDR(i, MI_NDMA_REG_CNT_WOFFSET);
  if (*ndmaCntp & MI_NDMA_IMM_MODE_ON) {

    MI_WaitNDma(i);
    return;
  }
  switch (*ndmaCntp & MI_NDMA_TIMING_MASK) {
  case MI_NDMA_TIMING_CARD:

    while (reg_MI_MCCNT1 & REG_MI_MCCNT1_START_MASK) {
    }
    break;
  case MI_NDMA_TIMING_DISP_MMEM:
  case MI_NDMA_TIMING_DISP:

    MI_WaitNDma(i);
  case MI_NDMA_TIMING_CAMERA:

    MI_StopNDma(i);
    break;
  default:

    if (!(*ndmaCntp & MI_NDMA_CONTINUOUS_ON)) {
      MI_WaitNDma(i);
    }

    else {
      MI_StopNDma(i);
      SDK_WARNING(FALSE, "[ARM9] Force to stop NDMA%d before sleep.", i);
    }
    break;
  }
}
#include <twl/ltdmain_end.h>
#endif
static void PMi_PreSleepForDma(void) {
  u32 i;

  for (i = 0; i <= MI_DMA_MAX_NUM; i++) {

    {
      vu32 *dmaCntp = (vu32 *)MI_DMA_REGADDR(i, MI_DMA_REG_CNT_WOFFSET);

      PMi_PreDmaCnt[i] = *dmaCntp;

      switch (*dmaCntp & MI_DMA_TIMING_MASK) {
      case MI_DMA_TIMING_CARD:

        while (reg_MI_MCCNT1 & REG_MI_MCCNT1_START_MASK) {
        }
        break;
      case MI_DMA_TIMING_DISP_MMEM:
      case MI_DMA_TIMING_DISP:

        MI_WaitDma(i);
        break;
      default:

        if (!(*dmaCntp & MI_DMA_CONTINUOUS_ON)) {
          MI_WaitDma(i);
        }

        else {
          MI_StopDma(i);
          SDK_WARNING(FALSE, "[ARM9] Force to stop DMA%d before sleep.", i);
        }
        break;
      }
    }
#ifdef SDK_TWL

    if (OS_IsRunOnTwl()) {
      PMi_PreSleepForNdma(i);
    }
#endif // SDK_TWL
  }
}

#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
static void PMi_PostSleepForNdma(u32 i) {
  vu32 *ndmaCntp = (vu32 *)MI_NDMA_REGADDR(i, MI_NDMA_REG_CNT_WOFFSET);
  if (*ndmaCntp & MI_NDMA_IMM_MODE_ON) {

    return;
  }
  switch (*ndmaCntp & MI_NDMA_TIMING_MASK) {
  case MI_NDMA_TIMING_V_BLANK:
  case MI_NDMA_TIMING_H_BLANK:

    if (*ndmaCntp & MI_NDMA_CONTINUOUS_ON) {
      *ndmaCntp |= MI_NDMA_ENABLE;
    }
    break;
  }
}
#include <twl/ltdmain_end.h>
#endif
static void PMi_PostSleepForDma(void) {
  u32 i;

  for (i = 0; i <= MI_DMA_MAX_NUM; i++) {

    {
      vu32 *dmaCntp = (vu32 *)MI_DMA_REGADDR(i, MI_DMA_REG_CNT_WOFFSET);
      u32 preCnt = PMi_PreDmaCnt[i];

      switch (preCnt & MI_DMA_TIMING_MASK) {
      case MI_DMA_TIMING_V_BLANK:
      case MI_DMA_TIMING_H_BLANK:
      case MI_DMA_TIMING_CARTRIDGE:

        if (preCnt & MI_DMA_CONTINUOUS_ON) {
          *dmaCntp = preCnt;
        }
        break;
      }
    }
#ifdef SDK_TWL

    if (OS_IsRunOnTwl()) {
      PMi_PostSleepForNdma(i);
    }
#endif // SDK_TWL
  }
}

void PM_GoSleepMode(PMWakeUpTrigger trigger, PMLogic logic, u16 keyPattern) {
  BOOL prepIrq;            // IME
  OSIntrMode prepIntrMode; // CPSR-IRQ
  OSIrqMask prepIntrMask;  // IE
  BOOL powerOffFlag = FALSE;

  PMBackLightSwitch preTop;
  PMBackLightSwitch preBottom;

  u32 preGX;
  u32 preGXS;
  PMLCDPower preLCDPower;

  PMi_ExecuteList(PMi_PreSleepCallbackList);

  prepIrq = OS_DisableIrq();
  prepIntrMode = OS_DisableInterrupts();
  prepIntrMask = OS_DisableIrqMask(OS_IE_MASK_ALL);

  (void)OS_SetIrqMask(OS_IE_FIFO_RECV |
                      (OS_IsTickAvailable() ? OS_IE_TIMER0 : 0));
  (void)OS_RestoreInterrupts(prepIntrMode);
  (void)OS_EnableIrq();

  if (trigger & PM_TRIGGER_CARD) {
    OSBootType type = OS_GetBootType();

    if (type == OS_BOOTTYPE_DOWNLOAD_MB || type == OS_BOOTTYPE_NAND) {
      trigger &= ~PM_TRIGGER_CARD;
    }
  }

  if (trigger & PM_TRIGGER_CARTRIDGE) {

    if (OS_IsRunOnTwl() || !CTRDG_IsExisting()) {
      trigger &= ~PM_TRIGGER_CARTRIDGE;
    }
  }

  preGX = reg_GX_DISPCNT;
  preGXS = reg_GXS_DB_DISPCNT;
  preLCDPower = PM_GetLCDPower();

  while (PM_GetBackLight(&preTop, &preBottom) != PM_RESULT_SUCCESS) {
    OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
  }
  while (PM_SetBackLight(PM_LCD_ALL, PM_BACKLIGHT_OFF) != PM_RESULT_SUCCESS) {
    OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
  }

  PMi_WaitVBlank();

  reg_GX_DISPCNT =
      reg_GX_DISPCNT & ~REG_GX_DISPCNT_MODE_MASK; // Main screen off
  GXS_DispOff();

  PMi_WaitVBlank();
  PMi_WaitVBlank(); // Twice

  PMi_PreSleepForDma();

  PMi_SleepEndFlag = FALSE;
  {
    u16 param = (u16)(trigger |
                      (preTop ? PM_BACKLIGHT_RECOVER_TOP_ON
                              : PM_BACKLIGHT_RECOVER_TOP_OFF) |
                      (preBottom ? PM_BACKLIGHT_RECOVER_BOTTOM_ON
                                 : PM_BACKLIGHT_RECOVER_BOTTOM_OFF));

    (void)OS_SetIrqMask(OS_IE_FIFO_RECV);
    (void)PMi_SendSleepStart(param, (u16)(logic | keyPattern));
  }

  while (!PMi_SleepEndFlag) {
    OS_Halt();
  }

  (void)OS_SetIrqMask(OS_IE_FIFO_RECV |
                      (OS_IsTickAvailable() ? OS_IE_TIMER0 : 0));

  if ((trigger & PM_TRIGGER_CARD) &&
      (OS_GetRequestIrqMask() & OS_IE_CARD_IREQ)) {
    powerOffFlag = TRUE;
  }

  if (!powerOffFlag) {
    if (preLCDPower == PM_LCD_POWER_ON) {
      while (PMi_SetLCDPower(PM_LCD_POWER_ON, PM_LED_ON, TRUE, TRUE) != TRUE) {
      }
    } else {
      while (PMi_SetLED(PM_LED_ON) != PM_RESULT_SUCCESS) {
        OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
      }
    }

    reg_GX_DISPCNT = preGX;
    reg_GXS_DB_DISPCNT = preGXS;
  }

  OS_SpinWaitSysCycles(PMi_LCD_WAIT_SYS_CYCLES);

  (void)OS_DisableIrq();
  (void)OS_SetIrqMask(prepIntrMask);
  (void)OS_RestoreInterrupts(prepIntrMode);
  (void)OS_RestoreIrq(prepIrq);

  if (powerOffFlag) {
    (void)PM_ForceToPowerOff();
  }

  PMi_ExecuteList(PMi_PostSleepCallbackList);
}

#define PMi_WAIT_FRAME_AFTER_LCDOFF 7
#define PMi_WAIT_FRAME_AFTER_GXDISP 2

BOOL PMi_SetLCDPower(PMLCDPower sw, PMLEDStatus led, BOOL skip, BOOL isSync) {
  switch (sw) {
  case PM_LCD_POWER_ON:

    if (!skip &&
        OS_GetVBlankCount() - PMi_LCDCount <= PMi_WAIT_FRAME_AFTER_LCDOFF) {
      return FALSE;
    }

    if (led != PM_LED_NONE) {
      if (isSync) {
        while (PMi_SetLED(led) != PM_RESULT_SUCCESS) {
          OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
        }
      } else {
        while (PMi_SetLEDAsync(led, NULL, NULL) != PM_RESULT_SUCCESS) {
          OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
        }
      }
    }

    (void)GXi_PowerLCD(TRUE);

    while (PMi_SetAmp(sAmpSwitch) != PM_RESULT_SUCCESS) {
      OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
    }
    break;

  case PM_LCD_POWER_OFF:

    while (PMi_SetAmp(PM_AMP_OFF) != PM_RESULT_SUCCESS) {
      OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
    }

    if (OS_GetVBlankCount() - PMi_DispOffCount <= PMi_WAIT_FRAME_AFTER_GXDISP) {
      PMi_WaitVBlank();
      PMi_WaitVBlank(); // Twice
    }

    (void)GXi_PowerLCD(FALSE);

    PMi_LCDCount = OS_GetVBlankCount();

    if (led != PM_LED_NONE) {
      if (isSync) {
        while (PMi_SetLED(led) != PM_RESULT_SUCCESS) {
          OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
        }
      } else {
        while (PMi_SetLEDAsync(led, NULL, NULL) != PM_RESULT_SUCCESS) {
          OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
        }
      }
    }
    break;

  default:

    break;
  }

  return TRUE;
}

BOOL PM_SetLCDPower(PMLCDPower sw) {
  if (sw != PM_LCD_POWER_ON) {
    sw = PM_LCD_POWER_OFF;
    if (GX_IsDispOn()) // To turn the LCD OFF, first make very sure to set the
                       // GX_DispOff status
    {
      GX_DispOff();
    }
  }
  return PMi_SetLCDPower(sw, PM_LED_NONE /* No touch */, FALSE, TRUE);
}

PMLCDPower PM_GetLCDPower(void) {
  return (reg_GX_POWCNT & REG_GX_POWCNT_LCD_MASK) ? PM_LCD_POWER_ON
                                                  : PM_LCD_POWER_OFF;
}

u32 PMi_GetLCDOffCount(void) { return PMi_LCDCount; }

u32 PMi_SendLEDPatternCommandAsync(PMLEDPattern pattern, PMCallback callback,
                                   void *arg) {
  return PM_SendUtilityCommandAsync(PM_UTIL_SET_BLINK, pattern, NULL, callback,
                                    arg);
}

u32 PMi_SendLEDPatternCommand(PMLEDPattern pattern) {
  return PM_SendUtilityCommand(PM_UTIL_SET_BLINK, pattern, NULL);
}

u32 PM_GetLEDPatternAsync(PMLEDPattern *patternBuf, PMCallback callback,
                          void *arg) {
  return PM_SendUtilityCommandAsync(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_BLINK,
                                    (u16 *)&patternBuf, callback, arg);
}

u32 PM_GetLEDPattern(PMLEDPattern *patternBuf) {
  u16 status;
  u32 result =
      PM_SendUtilityCommand(PM_UTIL_GET_STATUS, PM_UTIL_PARAM_BLINK, &status);

  if (result == PM_RESULT_SUCCESS) {
    if (patternBuf) {
      *patternBuf = (PMLEDPattern)status;
    }
  }
  return result;
}

static void PMi_InsertList(PMGenCallbackInfo **listp, PMGenCallbackInfo *info,
                           int priority, int method) {
  OSIntrMode intr;
  PMGenCallbackInfo *p;
  PMGenCallbackInfo *pre;

  if (!listp) {
    return;
  }

  info->priority = priority;

  intr = OS_DisableInterrupts();
  p = *listp;
  pre = NULL;

  while (p) {

    if (method == PMi_COMPARE_GT && p->priority > priority) {
      break;
    }
    if (method == PMi_COMPARE_GE && p->priority >= priority) {
      break;
    }

    pre = p;
    p = p->next;
  }

  if (p) {
    info->next = p;
  } else {
    info->next = NULL;
  }

  if (pre) {
    pre->next = info;
  } else {

    *listp = info;
  }

  (void)OS_RestoreInterrupts(intr);
}

static void PMi_DeleteList(PMGenCallbackInfo **listp, PMGenCallbackInfo *info) {
  OSIntrMode intr;
  PMGenCallbackInfo *p = *listp;
  PMGenCallbackInfo *pre;

  if (!listp) {
    return;
  }

  intr = OS_DisableInterrupts();
  pre = p = *listp;
  while (p) {

    if (p == info) {
      if (p == pre) {
        *listp = p->next;
      } else {
        pre->next = p->next;
      }
      break;
    }

    pre = p;
    p = p->next;
  }
  (void)OS_RestoreInterrupts(intr);
}

static void PMi_ClearList(PMGenCallbackInfo **listp) { listp = NULL; }

static void PMi_ExecuteList(PMGenCallbackInfo *listp) {
  while (listp) {
    (listp->callback)(listp->arg);

    listp = listp->next;
  }
}

void PM_AppendPreSleepCallback(PMSleepCallbackInfo *info) {
  PMi_InsertList(&PMi_PreSleepCallbackList, info, PM_CALLBACK_PRIORITY_MAX,
                 PMi_COMPARE_GT);
}

void PM_PrependPreSleepCallback(PMSleepCallbackInfo *info) {
  PMi_InsertList(&PMi_PreSleepCallbackList, info, PM_CALLBACK_PRIORITY_MIN,
                 PMi_COMPARE_GE);
}

void PM_AppendPostSleepCallback(PMSleepCallbackInfo *info) {
  PMi_InsertList(&PMi_PostSleepCallbackList, info, PM_CALLBACK_PRIORITY_MAX,
                 PMi_COMPARE_GT);
}

void PM_PrependPostSleepCallback(PMSleepCallbackInfo *info) {
  PMi_InsertList(&PMi_PostSleepCallbackList, info, PM_CALLBACK_PRIORITY_MIN,
                 PMi_COMPARE_GE);
}

void PMi_InsertPreSleepCallbackEx(PMSleepCallbackInfo *info, int priority) {
  SDK_ASSERT(PM_CALLBACK_PRIORITY_SYSMIN <= priority &&
             priority <= PM_CALLBACK_PRIORITY_SYSMAX);
  PMi_InsertList(&PMi_PreSleepCallbackList, info, priority, PMi_COMPARE_GT);
}
void PM_InsertPreSleepCallback(PMSleepCallbackInfo *info, int priority) {
  SDK_ASSERT(PM_CALLBACK_PRIORITY_MIN <= priority &&
             priority <= PM_CALLBACK_PRIORITY_MAX);
  PMi_InsertPreSleepCallbackEx(info, priority);
}

void PMi_InsertPostSleepCallbackEx(PMSleepCallbackInfo *info, int priority) {
  SDK_ASSERT(PM_CALLBACK_PRIORITY_SYSMIN <= priority &&
             priority <= PM_CALLBACK_PRIORITY_SYSMAX);
  PMi_InsertList(&PMi_PostSleepCallbackList, info, priority, PMi_COMPARE_GT);
}
void PM_InsertPostSleepCallback(PMSleepCallbackInfo *info, int priority) {
  SDK_ASSERT(PM_CALLBACK_PRIORITY_MIN <= priority &&
             priority <= PM_CALLBACK_PRIORITY_MAX);
  PMi_InsertPostSleepCallbackEx(info, priority);
}

void PM_DeletePreSleepCallback(PMSleepCallbackInfo *info) {
  PMi_DeleteList(&PMi_PreSleepCallbackList, info);
}

void PM_DeletePostSleepCallback(PMSleepCallbackInfo *info) {
  PMi_DeleteList(&PMi_PostSleepCallbackList, info);
}

void PM_ClearPreSleepCallback(void) {
  PMi_ClearList(&PMi_PreSleepCallbackList);
}

void PM_ClearPostSleepCallback(void) {
  PMi_ClearList(&PMi_PostSleepCallbackList);
}

#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>

static void PMi_ProceedToExit(PMExitFactor factor) {

  if (!PMi_TryLockForReset()) {
    return;
  }

  PMi_ExitFactor = factor;

#ifndef SDK_FINALROM

  PMi_ExitSequenceFlag = TRUE;
#endif

  PMi_ExecuteList(PMi_PreExitCallbackList);

  if (PMi_AutoExitFlag) {

    PM_ReadyToExit();
  }
}

void PM_ReadyToExit(void) {
#ifndef SDK_FINALROM
  SDK_ASSERT(PMi_ExitSequenceFlag == TRUE);
#endif

  PMi_CallPostExitCallbackAndReset(TRUE);
}

static void PMi_FinalizeDebugger(void) {

  OSi_SetSyncValue(OSi_SYNCVAL_NOT_READY);
  OSi_SyncWithOtherProc(OSi_SYNCTYPE_SENDER, (void *)HW_INIT_LOCK_BUF);
  OSi_SyncWithOtherProc(OSi_SYNCTYPE_RECVER, (void *)HW_INIT_LOCK_BUF);

  (void)OS_DisableInterrupts();

#ifndef SDK_FINALROM

  if (OSi_DetectDebugger() & OS_CONSOLE_TWLDEBUGGER) {
    _ISTDbgLib_OnBeforeResetHard();
  }
#endif

  OSi_SetSyncValue(OSi_SYNCVAL_READY);
}

PMExitFactor PM_GetExitFactor(void) { return PMi_ExitFactor; }

void PM_AppendPreExitCallback(PMExitCallbackInfo *info) {
  PMi_InsertList(&PMi_PreExitCallbackList, info, PM_CALLBACK_PRIORITY_MAX,
                 PMi_COMPARE_GT);
}

void PM_AppendPostExitCallback(PMExitCallbackInfo *info) {
  PMi_InsertList(&PMi_PostExitCallbackList, info, PM_CALLBACK_PRIORITY_MAX,
                 PMi_COMPARE_GT);
}

void PM_PrependPreExitCallback(PMExitCallbackInfo *info) {
  PMi_InsertList(&PMi_PreExitCallbackList, info, PM_CALLBACK_PRIORITY_MIN,
                 PMi_COMPARE_GE);
}

void PM_PrependPostExitCallback(PMExitCallbackInfo *info) {
  PMi_InsertList(&PMi_PostExitCallbackList, info, PM_CALLBACK_PRIORITY_MIN,
                 PMi_COMPARE_GE);
}

void PMi_InsertPreExitCallbackEx(PMExitCallbackInfo *info, int priority) {
  SDK_ASSERT(PM_CALLBACK_PRIORITY_SYSMIN <= priority &&
             priority <= PM_CALLBACK_PRIORITY_SYSMAX);
  PMi_InsertList(&PMi_PreExitCallbackList, info, priority, PMi_COMPARE_GT);
}
void PM_InsertPreExitCallback(PMExitCallbackInfo *info, int priority) {
  SDK_ASSERT(PM_CALLBACK_PRIORITY_MIN <= priority &&
             priority <= PM_CALLBACK_PRIORITY_MAX);
  PMi_InsertPreExitCallbackEx(info, priority);
}

void PMi_InsertPostExitCallbackEx(PMExitCallbackInfo *info, int priority) {
  SDK_ASSERT(PM_CALLBACK_PRIORITY_SYSMIN <= priority &&
             priority <= PM_CALLBACK_PRIORITY_SYSMAX);
  PMi_InsertList(&PMi_PostExitCallbackList, info, priority, PMi_COMPARE_GT);
}
void PM_InsertPostExitCallback(PMExitCallbackInfo *info, int priority) {
  SDK_ASSERT(PM_CALLBACK_PRIORITY_MIN <= priority &&
             priority <= PM_CALLBACK_PRIORITY_MAX);
  PMi_InsertPostExitCallbackEx(info, priority);
}

void PM_DeletePreExitCallback(PMExitCallbackInfo *info) {
  PMi_DeleteList(&PMi_PreExitCallbackList, info);
}

void PM_DeletePostExitCallback(PMExitCallbackInfo *info) {
  PMi_DeleteList(&PMi_PostExitCallbackList, info);
}

static void PMi_ClearPreExitCallback(void) {
  PMi_ClearList(&PMi_PreExitCallbackList);
}

static void PMi_ClearPostExitCallback(void) {
  PMi_ClearList(&PMi_PostExitCallbackList);
}

void PMi_ExecutePreExitCallbackList(void) {
  PMi_ExecuteList(PMi_PreExitCallbackList);
}

void PMi_ExecutePostExitCallbackList(void) {
  PMi_ExecuteList(PMi_PostExitCallbackList);
}

void PMi_ExecuteAllListsOfExitCallback(void) {
  PMi_ExecuteList(PMi_PreExitCallbackList);
  PMi_ExecuteList(PMi_PostExitCallbackList);
}

void PM_SetAutoExit(BOOL sw) { PMi_AutoExitFlag = sw; }

BOOL PM_GetAutoExit(void) { return PMi_AutoExitFlag; }

void PM_SetBatteryLowCallback(PMBatteryLowCallback callback, void *arg) {
  PMi_BatteryLowCallbackInfo.callback = callback;
  PMi_BatteryLowCallbackInfo.arg = arg;
}

void PM_ForceToResetHardware(void) {

  PMi_ExitFactor = PM_EXIT_FACTOR_USER;

  PMi_CallPostExitCallbackAndReset(FALSE);
}

static void PMi_CallPostExitCallbackAndReset(BOOL isExit) {

  PMi_ExecuteList(PMi_PostExitCallbackList);

  GX_DispOff();
  GXS_DispOff();

  MI_SetMainMemoryPriority(MI_PROCESSOR_ARM7);
  {
    int n;
    for (n = 0; n < 3; n++) {
      u32 count = OS_GetVBlankCount();
      while (count == OS_GetVBlankCount()) {
        OS_SpinWait(100);
      }
    }
  }

  while (1) {
    u16 result;
    u32 command = isExit ? PM_UTIL_FORCE_EXIT : PM_UTIL_FORCE_RESET_HARDWARE;

    if (*(u32 *)HW_RESET_LOCK_FLAG_BUF == PM_RESET_FLAG_FORCED) {
      command = PM_UTIL_FORCE_EXIT;

      ((LauncherParam *)HW_PARAM_LAUNCH_PARAM)->header.magicCode = 0;
    }

    PMi_WaitBusyMethod = PMi_WAITBUSY_METHOD_CPUMODE |
                         PMi_WAITBUSY_METHOD_CPSR | PMi_WAITBUSY_METHOD_IME;
    while (PM_SendUtilityCommand(command, 0, &result) != PM_SUCCESS) {

      OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
      PMi_WaitBusy();
    }
    if (result == SPI_PXI_RESULT_SUCCESS) {
      break;
    }

    OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
  }

  (void)OS_DisableInterrupts();

  MI_StopAllDma();
  MI_StopAllNDma();

  PMi_FinalizeDebugger();

  OSi_TerminateCore();
}
#include <twl/ltdmain_end.h>
#endif

static void PMi_LCDOnAvoidReset(void) {
  BOOL preMethod;

  OS_SpinWaitSysCycles(PMi_LCD_WAIT_SYS_CYCLES);

  preMethod = PMi_WaitBusyMethod;
  PMi_WaitBusyMethod = PMi_WAITBUSY_METHOD_CPUMODE | PMi_WAITBUSY_METHOD_CPSR |
                       PMi_WAITBUSY_METHOD_IME;
  if (PM_GetLCDPower() != PM_LCD_POWER_ON) {

    while (PM_SetBackLight(PM_LCD_ALL, PM_BACKLIGHT_OFF) != PM_RESULT_SUCCESS) {
      OS_SpinWait(HW_CPU_CLOCK_ARM9 / 100);
    }

    while (!PM_SetLCDPower(PM_LCD_POWER_ON)) {
      OS_SpinWait(PMi_PXI_WAIT_TICK);
    }
  }
  PMi_WaitBusyMethod = preMethod;
}

#ifndef SDK_FINALROM

u32 PMi_ReadRegisterAsync(u16 registerAddr, u16 *buffer, PMCallback callback,
                          void *arg) {
  return PM_SendUtilityCommandAsync(PMi_UTIL_READREG, registerAddr, buffer,
                                    callback, arg);
}
u32 PMi_ReadRegister(u16 registerAddr, u16 *buffer) {
  return PM_SendUtilityCommand(PMi_UTIL_READREG, registerAddr, buffer);
}

u32 PMi_WriteRegisterAsync(u16 registerAddr, u16 data, PMCallback callback,
                           void *arg) {
  return PM_SendUtilityCommandAsync(PMi_UTIL_WRITEREG,
                                    (u16)((registerAddr << 8) | (data & 0xff)),
                                    NULL, callback, arg);
}

u32 PMi_WriteRegister(u16 registerAddr, u16 data) {
  return PM_SendUtilityCommand(
      PMi_UTIL_WRITEREG, (u16)((registerAddr << 8) | (data & 0xff)), NULL);
}
#endif

void PMi_SetDispOffCount(void) { PMi_DispOffCount = OS_GetVBlankCount(); }

#ifndef SDK_FINALROM

void PM_DumpSleepCallback(void) {
  PMGenCallbackInfo *p;

  p = PMi_PreSleepCallbackList;
  OS_TPrintf("----PreSleep Callback\n");
  while (p) {
    OS_TPrintf("[%08x] (prio=%d) (arg=%x)\n", p->callback, p->priority, p->arg);
    p = p->next;
  }

  p = PMi_PostSleepCallbackList;
  OS_TPrintf("----PostSleep Callback\n");
  while (p) {
    OS_TPrintf("[%08x] (prio=%d) (arg=%x)\n", p->callback, p->priority, p->arg);
    p = p->next;
  }
}

#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>

void PM_DumpExitCallback(void) {
  PMGenCallbackInfo *p;

  p = PMi_PreExitCallbackList;
  OS_TPrintf("----PreExit Callback\n");
  while (p) {
    OS_TPrintf("[%08x] (prio=%d) (arg=%x)\n", p->callback, p->priority, p->arg);
    p = p->next;
  }

  p = PMi_PostExitCallbackList;
  OS_TPrintf("----PostExit Callback\n");
  while (p) {
    OS_TPrintf("[%08x] (prio=%d) (arg=%x)\n", p->callback, p->priority, p->arg);
    p = p->next;
  }
}
#include <twl/ltdmain_end.h>
#endif // ifdef SDK_TWL
#endif // ifndef SDK_FINALROM

#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>

static volatile BOOL isLockedReset = FALSE;
BOOL PMi_TryLockForReset(void) {
  OSIntrMode e = OS_DisableInterrupts();

  if (isLockedReset) {
    (void)OS_RestoreInterrupts(e);
    return FALSE;
  }
  isLockedReset = TRUE;

  (void)OS_RestoreInterrupts(e);
  return TRUE;
}
#include <twl/ltdmain_end.h>
#endif // ifdef SDK_TWL
