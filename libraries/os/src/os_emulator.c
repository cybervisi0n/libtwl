#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#ifdef SDK_ARM7
#include <twl/hw/common/mmap_wramEnv.h>
#endif

#ifdef SDK_ARM9
#define OSi_CURPROC_LOCKED_FLAG OS_MAINP_LOCKED_FLAG
#else
#define OSi_CURPROC_LOCKED_FLAG OS_SUBP_LOCKED_FLAG
#endif

#define memorySize() (*(u16 *)HW_MMEMCHECKER_SUB & OS_CONSOLE_SIZE_MASK)
#define soundMixFlag() (*(u16 *)OS_CHIPTYPE_SMX_ADDR & OS_CHIPTYPE_SMX_MASK)

static u32 OSi_DetectDeviceType(void);
static u32 OSi_DetectPlatform(void);

u32 OS_GetConsoleType(void) {
#if defined(SDK_FINALROM) || defined(SDK_SMALL_BUILD)
  static u32 OSi_ConsoleTypeCache = OSi_CONSOLE_NOT_DETECT;

  if (OSi_ConsoleTypeCache == OSi_CONSOLE_NOT_DETECT) {
    if (OS_IsRunOnTwl()) {
      OSi_ConsoleTypeCache =
          OS_CONSOLE_TWL | OS_CONSOLE_TWLTYPE_RETAIL | OS_CONSOLE_SIZE_16MB;
    } else {
      OSi_ConsoleTypeCache = OS_CONSOLE_NITRO | OS_CONSOLE_SIZE_4MB;
    }
    OSi_ConsoleTypeCache |= OSi_DetectDeviceType();
  }
  return OSi_ConsoleTypeCache;
#else
  return OS_GetRunningConsoleType();
#endif
}

static u32 OSi_RunningConsoleTypeCache = OSi_CONSOLE_NOT_DETECT;
u32 OS_GetRunningConsoleType(void) {
#ifdef SDK_ARM9
  while (*(vu16 *)HW_MMEMCHECKER_SUB == 0) {
    SVC_WaitByLoop(0x100 / 4);
  }
#endif

  if (OSi_RunningConsoleTypeCache == OSi_CONSOLE_NOT_DETECT) {
    u32 emulator = OSi_DetectEmulator();

    OSi_RunningConsoleTypeCache =
        ((emulator) ? emulator : OSi_DetectPlatform()) |
        OSi_DetectDeviceType() | memorySize();
  }
  return OSi_RunningConsoleTypeCache;
}

#ifndef SDK_FINALROM
void OS_SetConsoleType(u32 type) { OSi_RunningConsoleTypeCache = type; }
#endif

static u32 OSi_DetectDeviceType(void) {
  static const u32 table[] = {
      0,                       // for OS_BOOTTYPE_ILLEGAL
      OS_CONSOLE_DEV_CARD,     // for OS_BOOTTYPE_ROM
      OS_CONSOLE_DEV_DOWNLOAD, // for OS_BOOTTYPE_DOWNLOAD_MB
      OS_CONSOLE_DEV_NAND,     // for OS_BOOTTYPE_NAND
      OS_CONSOLE_DEV_MEMORY,   // for OS_BOOTTYPE_MEMORY
  };

  return table[OS_GetBootType()];
}

u32 OSi_DetectEmulator(void) {
  static u32 OSi_IsDetectedEmulator = FALSE;
  static u32 OSi_Emulator;

  if (!OSi_IsDetectedEmulator) {
#ifdef SDK_ARM9
#ifndef SDK_FINALROM

    u32 val;
    OSIntrMode intr = OS_DisableInterrupts();

    (*(REGType32v *)REG_CLIPMTX_RESULT_0_ADDR) = 0x2468ace0;

    val = *(vu16 *)REG_VCOUNT_ADDR & 0x1ffU;

    if (val == 270) {

      *(vu32 *)0x4fff010 = 0x13579bdf; // ACK Signal 1
      *(vu32 *)0x4fff010 = 0xfdb97531; // ACK Signal 2
      OSi_Emulator = OS_CONSOLE_ENSATA;
    }

    (void)OS_RestoreInterrupts(intr);
#else

    OSi_Emulator = 0;
#endif

#else

    OSi_Emulator = 0;
#endif

    OSi_IsDetectedEmulator = TRUE;
  }
  return OSi_Emulator;
}

static u32 OSi_DetectPlatform(void) {
  static u32 OSi_IsDetectedPlatform = FALSE;
  static u32 OSi_Platform;

  if (!OSi_IsDetectedPlatform) {
    switch (*(u8 *)(OS_CHIPTYPE_DEBUGGER_ADDR)&OS_CHIPTYPE_DEBUGGER_MASK) {
    case OS_CHIPTYPE_TWL: // equal to 0, also means running NITRO platform.
    {
      switch (memorySize()) {
      case OS_CONSOLE_SIZE_4MB:
        OSi_Platform = OSi_IsNitroModeOnTwl()
                           ? (OS_CONSOLE_TWL | OS_CONSOLE_TWLTYPE_RETAIL)
                           : OS_CONSOLE_NITRO;
        break;
      case OS_CONSOLE_SIZE_8MB: {
        int isDebuggerFlag = *(u16 *)HW_CHECK_DEBUGGER_BUF2;
        if (isDebuggerFlag == 1) {
          OSi_Platform = OS_CONSOLE_ISDEBUGGER;
        } else {

          OSi_Platform = 0;
        }
      } break;
      case OS_CONSOLE_SIZE_16MB:
        OSi_Platform = OS_CONSOLE_TWL | OS_CONSOLE_TWLTYPE_RETAIL;
        break;
      case OS_CONSOLE_SIZE_32MB:

        OSi_Platform = 0;
        break;
      default:

        OSi_Platform = 0;
      }
    } break;
    case OS_CHIPTYPE_DEBUGGER_1:
      if (*(u8 *)(OS_CHIPTYPE_JTAG_ADDR)&OS_CHIPTYPE_JTAG_MASK) {
        OSi_Platform = OS_CONSOLE_TWLDEBUGGER | OS_CONSOLE_BOARD_A9_A7;
      } else {
        OSi_Platform = OS_CONSOLE_TWL | OS_CONSOLE_TWLTYPE_DEV;
      }
      break;
    case OS_CHIPTYPE_DEBUGGER_2:
      if (*(u8 *)(OS_CHIPTYPE_JTAG_ADDR)&OS_CHIPTYPE_JTAG_MASK) {
        OSi_Platform = OS_CONSOLE_TWLDEBUGGER | OS_CONSOLE_BOARD_A9;
      } else {
        OSi_Platform = OS_CONSOLE_TWL | OS_CONSOLE_TWLTYPE_DEV;
      }
      break;
    case OS_CHIPTYPE_EVALUATE:
      OSi_Platform =
          OS_CONSOLE_TWL | OS_CONSOLE_TWLTYPE_DEV | OS_CONSOLE_EVALUATE;
      break;
    }

    OSi_IsDetectedPlatform = TRUE;
  }
  return OSi_Platform;
}

u32 OSi_DetectDebugger(void) { return OSi_DetectPlatform(); }

BOOL OS_IsRunOnEmulator(void) {
  return (OS_GetConsoleType() & OS_CONSOLE_ENSATA) ? TRUE : FALSE;
}

BOOL OS_IsRunOnDebugger(void) {
  return (OS_GetConsoleType() &
          (OS_CONSOLE_TWLDEBUGGER | OS_CONSOLE_ISDEBUGGER))
             ? TRUE
             : FALSE;
}

#if defined(SDK_TWLHYB) || defined(SDK_PORT)
BOOL OS_IsRunOnTwl(void) {
  static BOOL OSi_IsDetectedTWL = FALSE;
  static BOOL OSi_IsRunOnTWL = FALSE;

  if (!OSi_IsDetectedTWL) {
#ifdef SDK_ARM9
    u8 rom9 = reg_SCFG_A9ROM;
#else  // SDK_ARM7
    u8 rom9 = (u8)(*(u8 *)(HW_PRV_WRAM_SYSRV + HWi_WSYS08_WRAMOFFSET) >>
                   HWi_WSYS08_ROM_ARM9SEC_SHIFT);
#endif // SDK_ARM7
    OSi_IsRunOnTWL =
        (rom9 & (REG_SCFG_A9ROM_SEC_MASK | REG_SCFG_A9ROM_RSEL_MASK)) ==
        REG_SCFG_A9ROM_SEC_MASK;

    OSi_IsDetectedTWL = TRUE;
  }

  return OSi_IsRunOnTWL;
}
#endif

extern BOOL OSi_IsRunOnTwl(void);
BOOL OSi_IsRunOnTwl(void) { return OS_IsRunOnTwl(); }

#ifndef SDK_TWLLTD
BOOL OSi_IsNitroModeOnTwl(void) {
  static BOOL OSi_IsDetected = FALSE;
  static BOOL OSi_IsNitroModeOnTwl = FALSE;

  if (!OSi_IsDetected) {
    if (!OS_IsRunOnTwl() && soundMixFlag()) {
      OSi_IsNitroModeOnTwl = TRUE;
    }

    OSi_IsDetected = TRUE;
  }

  return OSi_IsNitroModeOnTwl;
}
#endif // SDK_TWLLTD
