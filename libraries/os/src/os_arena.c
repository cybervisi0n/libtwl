#include <nitro.h>

#define OSi_TRUNC(n, a) (((u32)(n)) & ~((a) - 1))
#define OSi_ROUND(n, a) (((u32)(n) + (a) - 1) & ~((a) - 1))

#define OS_ERR_GETARENAHI_INIT "OS_GetArenaHi: init in advance"
#define OS_ERR_GETARENAHI_INVALID "OS_GetArenaHi: invalid arena (hi<lo)"
#define OS_ERR_GETARENALO_INIT "OS_GetArenaLo: init in advance"
#define OS_ERR_GETARENALO_INVALID "OS_GetArenaLo: invalid arena (hi<lo)"

#define SDK_ARENAID_ASSERT(id) SDK_ASSERT((u32)id < OS_ARENA_MAX)

#define OSi_GetArenaInfo() (*(OSArenaInfo *)HW_ARENA_INFO_BUF)

#ifdef SDK_NITRO

extern void SDK_MAIN_ARENA_LO(void);
#define OSi_MAIN_ARENA_LO_DEFAULT ((u32)SDK_MAIN_ARENA_LO) // Defined by LCF
#define OSi_MAIN_ARENA_HI_DEFAULT HW_MAIN_MEM_MAIN_END
#else

extern void SDK_MAIN_ARENA_LO(void);
#define OSi_MAIN_ARENA_LO_DEFAULT_forNITRO                                     \
  ((u32)SDK_MAIN_ARENA_LO) // Defined by LCF
#define OSi_MAIN_ARENA_HI_DEFAULT_forNITRO HW_MAIN_MEM_MAIN_END

extern void SDK_LTDMAIN_ARENA_LO(void);
#define OSi_MAIN_ARENA_LO_DEFAULT_forTWL                                       \
  ((u32)SDK_LTDMAIN_ARENA_LO) // Defined by LCF
#define OSi_MAIN_ARENA_HI_DEFAULT_forTWL HW_TWL_MAIN_MEM_MAIN_END
#endif

#ifdef SDK_ARM7
#ifdef SDK_NITRO

extern void SDK_SUBPRIV_ARENA_LO(void);
#define OSi_MAIN_SUBPRIV_ARENA_LO_DEFAULT                                      \
  ((u32)SDK_SUBPRIV_ARENA_LO) // Defined by LCF
#define OSi_MAIN_SUBPRIV_ARENA_HI_DEFAULT HW_MAIN_MEM_SUB_END
#else

extern void SDK_SUBPRIV_ARENA_LO(void);
#define OSi_MAIN_SUBPRIV_ARENA_LO_DEFAULT_forNITRO                             \
  ((u32)SDK_SUBPRIV_ARENA_LO) // Defined by LCF
#define OSi_MAIN_SUBPRIV_ARENA_HI_DEFAULT_forNITRO HW_MAIN_MEM_SUB_END

#define OSi_MAIN_SUBPRIV_ARENA_LO_DEFAULT_forTWL                               \
  ((u32)SDK_SUBPRIV_ARENA_LO) // Defined by LCF
#define OSi_MAIN_SUBPRIV_ARENA_HI_DEFAULT_forTWL HW_TWL_MAIN_MEM_SUB_END
#endif
#endif

#if defined(SDK_ARM9) || defined(SDK_PORT)
#ifdef SDK_NITRO

extern void SDK_SECTION_ARENA_EX_START(void);
#define OSi_MAINEX_ARENA_LO_DEFAULT                                            \
  ((u32)SDK_SECTION_ARENA_EX_START) // Defined by LCF
#define OSi_MAINEX_ARENA_HI_DEFAULT HW_MAIN_MEM_DEBUGGER
#else

extern void SDK_SECTION_ARENA_EX_START(void);
#define OSi_MAINEX_ARENA_LO_DEFAULT_forNITRO                                   \
  ((u32)SDK_SECTION_ARENA_EX_START) // Defined by LCF
#define OSi_MAINEX_ARENA_HI_DEFAULT_forNITRO HW_MAIN_MEM_DEBUGGER

extern void SDK_LTDMAIN_EX_ARENA_LO(void);
#define OSi_MAINEX_ARENA_LO_DEFAULT_forTWL                                     \
  ((u32)SDK_LTDMAIN_EX_ARENA_LO) // Defined by LCF
#define OSi_MAINEX_ARENA_HI_DEFAULT_forTWL HW_TWL_MAIN_MEM_DEBUGGER
#endif
#endif

#if defined(SDK_ARM9) || defined(SDK_PORT)

extern void SDK_SECTION_ARENA_DTCM_START(void);
#define OSi_DTCM_ARENA_LO_DEFAULT                                              \
  ((u32)SDK_SECTION_ARENA_DTCM_START) // Defined by LCF

extern void SDK_SECTION_ARENA_ITCM_START(void);
#define OSi_ITCM_ARENA_LO_DEFAULT                                              \
  ((u32)SDK_SECTION_ARENA_ITCM_START) // Defined by LCF
#endif

#define OSi_WRAM_MAIN_ARENA_LO_DEFAULT HW_WRAM
#define OSi_WRAM_MAIN_ARENA_HI_DEFAULT HW_WRAM

#ifdef SDK_ARM7
extern void SDK_WRAM_ARENA_LO(void);
#define OSi_WRAM_SUB_ARENA_LO_DEFAULT ((u32)SDK_WRAM_ARENA_LO) // Defined by LCF
#define OSi_WRAM_SUB_ARENA_HI_DEFAULT HW_WRAM_END
#endif

#ifdef SDK_ARM7
#define OSi_WRAM_SUBPRIV_ARENA_LO_DEFAULT HW_PRV_WRAM
#define OSi_WRAM_SUBPRIV_ARENA_HI_DEFAULT HW_PRV_WRAM_END
#endif

extern unsigned long SDK_SYS_STACKSIZE[];
#define OSi_SYS_STACKSIZE ((int)SDK_SYS_STACKSIZE)
#ifdef SDK_BUILD_ARM
extern unsigned long SDK_IRQ_STACKSIZE[];
#endif
#define OSi_IRQ_STACKSIZE ((int)SDK_IRQ_STACKSIZE)

static BOOL OSi_Initialized = FALSE;

#if defined(SDK_ARM9) || defined(SDK_PORT)
#ifdef SDK_NITRO

#ifdef SDK_4M
BOOL OSi_MainExArenaEnabled = FALSE;
#else
BOOL OSi_MainExArenaEnabled = TRUE;
#endif
#else

#ifdef SDK_16M
BOOL OSi_MainExArenaEnabled = FALSE;
#else
BOOL OSi_MainExArenaEnabled = TRUE;
#endif
#endif
#endif

#if defined(SDK_ARM9) || defined(SDK_PORT)

void OS_InitArena(void) {

  if (OSi_Initialized) {
    return;
  }
  OSi_Initialized = TRUE;

  OS_InitArenaHiAndLo(OS_ARENA_MAIN);

  OS_SetArenaLo(OS_ARENA_MAINEX, (void *)0);
  OS_SetArenaHi(OS_ARENA_MAINEX, (void *)0);

  OS_InitArenaHiAndLo(OS_ARENA_ITCM);

  OS_InitArenaHiAndLo(OS_ARENA_DTCM);

  OS_InitArenaHiAndLo(OS_ARENA_SHARED);

  OS_InitArenaHiAndLo(OS_ARENA_WRAM_MAIN);
}
#endif

#ifdef SDK_ARM7

void OS_InitArena(void) {

  if (OSi_Initialized) {
    return;
  }
  OSi_Initialized = TRUE;

  OS_InitArenaHiAndLo(OS_ARENA_MAIN_SUBPRIV);

  OS_InitArenaHiAndLo(OS_ARENA_WRAM_SUB);

  OS_InitArenaHiAndLo(OS_ARENA_WRAM_SUBPRIV);
}
#endif

#if defined(SDK_ARM9) || defined(SDK_PORT)

#ifndef SDK_FINALROM

static inline void OSi_SetForMainPR(u32 consoleType) {
  u32 size = OS_PR_SIZE_4MB;

  if (OSi_MainExArenaEnabled) {
    u32 type = consoleType & OS_CONSOLE_MASK;

    if ((type == OS_CONSOLE_ISDEBUGGER) || (type == OS_CONSOLE_TWLDEBUGGER) ||
        (type == OS_CONSOLE_TWL)) {
      size = OS_PR_SIZE_8MB;
    }
  }

  OS_SetProtectionRegionEx(1, HW_MAIN_MEM_MAIN, size);
}
#endif

#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
void OSi_InitArenaEx_ltdmain(void);
void OSi_InitArenaEx_ltdmain(void) {
  u32 consoleType = OS_GetConsoleType();
  u32 memSize = consoleType & OS_CONSOLE_SIZE_MASK;

#ifdef SDK_FINALROM

  OS_SetProtectionRegionEx(3, HW_CTRDG_ROM, OS_PR_SIZE_64MB);
#else
  if (OSi_MainExArenaEnabled && memSize == OS_CONSOLE_SIZE_32MB) {
    OSIntrMode enabled = OS_DisableInterrupts();

    OS_SetProtectionRegionEx(3, HW_CTRDG_ROM, OS_PR_SIZE_128MB);

    (void)OS_RestoreInterrupts(enabled);
  } else {

    OS_SetProtectionRegionEx(3, HW_CTRDG_ROM, OS_PR_SIZE_64MB);
  }
#endif
}
#include <twl/ltdmain_end.h>
#endif

void OS_InitArenaEx(void) {
  u32 consoleType = OS_GetConsoleType();
  u32 memSize = consoleType & OS_CONSOLE_SIZE_MASK;

  OS_InitArenaHiAndLo(OS_ARENA_MAINEX);

#ifdef SDK_NITRO

#ifdef SDK_FINALROM
  OS_SetProtectionRegionEx(1, HW_MAIN_MEM_MAIN, OS_PR_SIZE_4MB);
#else
  OSi_SetForMainPR(consoleType);
#endif
#else // ifdef SDK_NITRO

  if (!OS_IsRunOnTwl()) {

#ifdef SDK_FINALROM
    OS_SetProtectionRegionEx(1, HW_MAIN_MEM_MAIN, OS_PR_SIZE_4MB);
#else
    OSi_SetForMainPR(consoleType);
#endif
    OS_SetProtectionRegionEx(
        2, HW_MAIN_MEM_IM_SHARED_END - HW_MAIN_MEM_IM_SHARED_SIZE,
        OS_PR_SIZE_4KB);
  } else {

    OSi_InitArenaEx_ltdmain();
  }
#endif // ifdef SDK_NITRO
}
#endif // ifdef SDK_ARM9

void *OS_GetArenaInfo(void) { return (void *)&OSi_GetArenaInfo(); }

void *OS_GetArenaHi(OSArenaId id) {
  SDK_ASSERT(OSi_Initialized);
  SDK_ARENAID_ASSERT(id);
  SDK_TASSERTMSG((u32)OSi_GetArenaInfo().lo[id] != 0xffffffff,
                 OS_ERR_GETARENALO_INIT);
  SDK_TASSERTMSG((u32)OSi_GetArenaInfo().lo[id] <=
                     (u32)OSi_GetArenaInfo().hi[id],
                 OS_ERR_GETARENALO_INVALID);

  return OSi_GetArenaInfo().hi[id];
}

void *OS_GetArenaLo(OSArenaId id) {
  SDK_ASSERT(OSi_Initialized);
  SDK_ARENAID_ASSERT(id);
  SDK_TASSERTMSG((u32)OSi_GetArenaInfo().lo[id] != 0xffffffff,
                 OS_ERR_GETARENALO_INIT);
  SDK_TASSERTMSG((u32)OSi_GetArenaInfo().lo[id] <=
                     (u32)OSi_GetArenaInfo().hi[id],
                 OS_ERR_GETARENALO_INVALID);

  return OSi_GetArenaInfo().lo[id];
}

void *OS_GetInitArenaHi(OSArenaId id) {
  SDK_ASSERT(OSi_Initialized);
  SDK_ARENAID_ASSERT(id);

  switch (id) {
#if defined(SDK_ARM9) || defined(SDK_PORT)
  case OS_ARENA_MAIN:
#ifdef SDK_NITRO
    return (void *)OSi_MAIN_ARENA_HI_DEFAULT;
#else
    return (OS_IsRunOnTwl()) ? (void *)OSi_MAIN_ARENA_HI_DEFAULT_forTWL
                             : (void *)OSi_MAIN_ARENA_HI_DEFAULT_forNITRO;
#endif

  case OS_ARENA_MAINEX:
    if (!OSi_MainExArenaEnabled) {
      return 0;
    } else {
      u32 size = OS_GetConsoleType() & OS_CONSOLE_SIZE_MASK;
#ifdef SDK_NITRO
      return (void *)((size == OS_CONSOLE_SIZE_4MB)
                          ? 0
                          : OSi_MAINEX_ARENA_HI_DEFAULT);
#else
      return (OS_IsRunOnTwl())
                 ? (void *)((size == OS_CONSOLE_SIZE_16MB)
                                ? 0
                                : OSi_MAINEX_ARENA_HI_DEFAULT_forTWL)
                 : (void *)((size == OS_CONSOLE_SIZE_4MB)
                                ? 0
                                : OSi_MAINEX_ARENA_HI_DEFAULT_forNITRO);
#endif
    }

  case OS_ARENA_ITCM:
    return (void *)HW_ITCM_ARENA_HI_DEFAULT;

  case OS_ARENA_DTCM: {
    u32 irqStackLo;
    u32 sysStackLo;

    irqStackLo = (u32)HW_DTCM_IRQ_STACK_END - OSi_IRQ_STACKSIZE;

    if (OSi_SYS_STACKSIZE == 0) // maximum SysStack in DTCM
    {
      sysStackLo = HW_DTCM;
      if (sysStackLo < OSi_DTCM_ARENA_LO_DEFAULT) {
        sysStackLo = OSi_DTCM_ARENA_LO_DEFAULT;
      }
    } else if (OSi_SYS_STACKSIZE < 0) {
      sysStackLo = OSi_DTCM_ARENA_LO_DEFAULT - OSi_SYS_STACKSIZE;
    } else {
      sysStackLo = irqStackLo - OSi_SYS_STACKSIZE;
    }
    SDK_ASSERT((u32)OSi_DTCM_ARENA_LO_DEFAULT <= sysStackLo &&
               sysStackLo < irqStackLo);
    return (void *)sysStackLo;
  }

  case OS_ARENA_SHARED:
    return (void *)HW_SHARED_ARENA_HI_DEFAULT;

  case OS_ARENA_WRAM_MAIN:
    return (void *)OSi_WRAM_MAIN_ARENA_HI_DEFAULT;
#else
  case OS_ARENA_MAIN_SUBPRIV:
#ifdef SDK_NITRO
    return (void *)OSi_MAIN_SUBPRIV_ARENA_HI_DEFAULT;
#else
    return (OS_IsRunOnTwl())
               ? (void *)OSi_MAIN_SUBPRIV_ARENA_HI_DEFAULT_forTWL
               : (void *)OSi_MAIN_SUBPRIV_ARENA_HI_DEFAULT_forNITRO;
#endif

  case OS_ARENA_WRAM_SUB:
    return (void *)OSi_WRAM_SUB_ARENA_HI_DEFAULT;

  case OS_ARENA_WRAM_SUBPRIV: {
    u32 irqStackLo;
    u32 sysStackLo;

    irqStackLo = (u32)HW_PRV_WRAM_IRQ_STACK_END - OSi_IRQ_STACKSIZE;
    sysStackLo = (u32)HW_PRV_WRAM;

    if ((u32)SDK_WRAM_ARENA_LO > (u32)HW_PRV_WRAM) {
      sysStackLo = (u32)SDK_WRAM_ARENA_LO;
    }

    if (OSi_SYS_STACKSIZE == 0) // maximum SysStack in SURPRIV-WRAM
    {

    } else if (OSi_SYS_STACKSIZE < 0) {
      sysStackLo -= OSi_SYS_STACKSIZE;
    } else {
      sysStackLo = irqStackLo - OSi_SYS_STACKSIZE;
    }
    SDK_ASSERT((u32)HW_PRV_WRAM <= sysStackLo && sysStackLo < irqStackLo);
    return (void *)sysStackLo;
  }
#endif

  default:
    SDK_WARNING(0, "Bad arena id");
  }

  return NULL;
}

void *OS_GetInitArenaLo(OSArenaId id) {
  SDK_ASSERT(OSi_Initialized);
  SDK_ARENAID_ASSERT(id);

  switch (id) {
#if defined(SDK_ARM9) || defined(SDK_PORT)
  case OS_ARENA_MAIN:

#ifdef SDK_NITRO
    return (void *)OSi_MAIN_ARENA_LO_DEFAULT;
#else
    return (OS_IsRunOnTwl()) ? (void *)OSi_MAIN_ARENA_LO_DEFAULT_forTWL
                             : (void *)OSi_MAIN_ARENA_LO_DEFAULT_forNITRO;
#endif

  case OS_ARENA_MAINEX:
    if (!OSi_MainExArenaEnabled) {
      return (void *)0;
    } else {
      u32 size = OS_GetConsoleType() & OS_CONSOLE_SIZE_MASK;
#ifdef SDK_NITRO
      return (void *)((size == OS_CONSOLE_SIZE_4MB)
                          ? 0
                          : OSi_MAINEX_ARENA_LO_DEFAULT);
#else
      return (OS_IsRunOnTwl())
                 ? (void *)((size == OS_CONSOLE_SIZE_16MB)
                                ? 0
                                : OSi_MAINEX_ARENA_LO_DEFAULT_forTWL)
                 : (void *)((size == OS_CONSOLE_SIZE_4MB)
                                ? 0
                                : OSi_MAINEX_ARENA_LO_DEFAULT_forNITRO);
#endif
    }
  case OS_ARENA_ITCM:
    return (void *)OSi_ITCM_ARENA_LO_DEFAULT;

  case OS_ARENA_DTCM:
    return (void *)OSi_DTCM_ARENA_LO_DEFAULT;

  case OS_ARENA_SHARED:
    return (void *)HW_SHARED_ARENA_LO_DEFAULT;

  case OS_ARENA_WRAM_MAIN:
    return (void *)OSi_WRAM_MAIN_ARENA_LO_DEFAULT;
#else
  case OS_ARENA_MAIN_SUBPRIV:
#ifdef SDK_NITRO
    return (void *)OSi_MAIN_SUBPRIV_ARENA_LO_DEFAULT;
#else
    return (OS_IsRunOnTwl())
               ? (void *)OSi_MAIN_SUBPRIV_ARENA_LO_DEFAULT_forTWL
               : (void *)OSi_MAIN_SUBPRIV_ARENA_LO_DEFAULT_forNITRO;

#endif
  case OS_ARENA_WRAM_SUB: {
    u32 wramSubLo = OSi_WRAM_SUB_ARENA_LO_DEFAULT;
    if ((u32)HW_WRAM_END < (u32)wramSubLo) {
      wramSubLo = (u32)HW_WRAM_END;
    }
    return (void *)wramSubLo;
  }
  case OS_ARENA_WRAM_SUBPRIV: {
    u32 privWramLo = OSi_WRAM_SUBPRIV_ARENA_LO_DEFAULT;
    if ((u32)SDK_WRAM_ARENA_LO > (u32)privWramLo) {
      privWramLo = (u32)SDK_WRAM_ARENA_LO;
    }
    return (void *)privWramLo;
  }
#endif
  default:
    SDK_WARNING(0, "Bad arena id");
  }

  return NULL;
}

void OS_SetArenaHi(OSArenaId id, void *newHi) {
  SDK_ASSERT(OSi_Initialized);
  SDK_ARENAID_ASSERT(id);

  OSi_GetArenaInfo().hi[id] = newHi;
}

void OS_SetArenaLo(OSArenaId id, void *newLo) {
  SDK_ASSERT(OSi_Initialized);
  SDK_ARENAID_ASSERT(id);

  OSi_GetArenaInfo().lo[id] = newLo;
}

void *OS_AllocFromArenaLo(OSArenaId id, u32 size, u32 align) {
  void *ptr;
  u8 *arenaLo;

  ptr = OS_GetArenaLo(id);
  if (!ptr) {
    return NULL;
  }

  arenaLo = ptr = (void *)OSi_ROUND(ptr, align);
  arenaLo += size;
  arenaLo = (u8 *)OSi_ROUND(arenaLo, align);

  if (arenaLo > OS_GetArenaHi(id)) {
    return NULL;
  }

  OS_SetArenaLo(id, arenaLo);

  return ptr;
}

void *OS_AllocFromArenaHi(OSArenaId id, u32 size, u32 align) {
  void *ptr;
  u8 *arenaHi;

  arenaHi = OS_GetArenaHi(id);
  if (!arenaHi) {
    return NULL;
  }

  arenaHi = (u8 *)OSi_TRUNC(arenaHi, align);
  arenaHi -= size;
  arenaHi = ptr = (void *)OSi_TRUNC(arenaHi, align);

  if (arenaHi < OS_GetArenaLo(id)) {
    return NULL;
  }

  OS_SetArenaHi(id, arenaHi);

  return ptr;
}

#if defined(SDK_ARM9) || defined(SDK_PORT)

void OS_EnableMainExArena(void) {
  SDK_ASSERT(!OSi_Initialized);

  OSi_MainExArenaEnabled = TRUE;
}

void OS_DisableMainExArena(void) {
  SDK_ASSERT(!OSi_Initialized);

  OSi_MainExArenaEnabled = FALSE;
}
#endif

#ifndef SDK_FINALROM

void OS_DumpArenaInfo(OSArenaId id, BOOL isInfoLine) {
  static const char *arenaName[] = {
      "MainRAM-Main", "MainRAM-Sub ", "MainRAM-EX  ",
      "ITCM        ", "DTCM        ", "SharedMemory",
      "WRAM-Main   ", "WRAM-Sub    ", "WRAM-Subpriv"};

  SDK_ASSERT(0 <= id && id <= sizeof(arenaName) / sizeof(char *));

  if (isInfoLine) {
    OS_TPrintf(" NAME         ID   LO addr   HI addr     size\n");
  }

  OS_TPrintf("%12s   %1d  %8x  %8x  %8x\n", arenaName[(int)id], id,
             OS_GetArenaLo(id), OS_GetArenaHi(id),
             (u32)OS_GetArenaHi(id) - (u32)OS_GetArenaLo(id));
}

void OS_DumpAllArenaInfo(void) {
  static const OSArenaId id[] = {
      OS_ARENA_MAIN,      OS_ARENA_MAIN_SUBPRIV, OS_ARENA_MAINEX,
      OS_ARENA_ITCM,      OS_ARENA_DTCM,         OS_ARENA_SHARED,
      OS_ARENA_WRAM_MAIN, OS_ARENA_WRAM_SUB,     OS_ARENA_WRAM_SUBPRIV};

  int n;
  BOOL isInfoLine = TRUE;

  for (n = 0; n < sizeof(id) / sizeof(OSArenaId); n++) {
    OS_DumpArenaInfo(id[n], isInfoLine);
    isInfoLine = FALSE;
  }

  OS_TPrintf("\n");
}
#endif // ifndef SDK_FINALROM

BOOL OS_GetArenaResource(OSArenaResource *resource) {
  int n;
  OSIntrMode enabled = OS_DisableInterrupts();

  for (n = 0; n < OS_ARENA_MAX; n++) {
    resource->info.lo[n] = OSi_GetArenaInfo().lo[n];
    resource->info.hi[n] = OSi_GetArenaInfo().hi[n];
  }

  (void)OS_RestoreInterrupts(enabled);
  return TRUE;
}
