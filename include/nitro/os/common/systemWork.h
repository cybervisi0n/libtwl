#ifndef NITRO_OS_COMMON_SYSTEMWORK_H_
#define NITRO_OS_COMMON_SYSTEMWORK_H_

#if !(defined(SDK_WIN32) || defined(SDK_FROM_TOOL))

#ifndef SDK_ASM
#include <nitro/types.h>

#ifndef SDK_TWL
#include <nitro/hw/common/mmap_shared.h>
#else // SDK_TWL
#include <twl/hw/common/mmap_shared.h>
#endif

#include <nitro/os/common/thread.h>
#include <nitro/os/common/spinLock.h>
#include <nitro/os/common/arena.h>

typedef union {
  u32 b32;
  u16 b16;
} OSDmaClearSrc;
typedef struct {
  u8 bootCheckInfo[0x20]; // 000-01f:   32-byte boot check info
  u32 resetParameter;     // 020-023:    4-byte reset parameter
#ifdef SDK_TWL
  u8 bootSync[0x8]; // 024-02c:    8-byte boot sync
#else
  u8 padding5[0x8]; // 024-02c:   (8-byte)
#endif
  u32 romBaseOffset;          // 02c-02f:    4-byte ROM offset of own program
  u8 cartridgeModuleInfo[12]; // 030-03b:   12-byte cartridge module info
  u32 vblankCount;            // 03c-03f:    4-byte V-Blank Count
  u8 wmBootBuf[0x40];         // 040-07f:   64-byte WM multiboot buffer
#ifdef SDK_TWL
  u8 nvramUserInfo[0xe8];  // 080-168: 232 bytes NVRAM user info
  u8 HW_secure_info[0x18]; // 18b-17f:  24 bytes NAND secure system information
                           // (flags, validLangBitmap, region, serialNo)
#else
  u8 nvramUserInfo[0x100]; // 080-17f: 256 bytes NVRAM user info
#endif
  u8 isd_reserved1[0x20]; // 180-19f:  32 bytes ISDebugger reservation
  u8 arenaInfo[0x48];     // 1a0-1e7:  72 byte Arena information
  u8 real_time_clock[8];  // 1e8-1ef:   8 bytes RTC
  u8 sys_conf[6];         // 1f0-1f5:   6 bytes System config
  u8 printWindowArm9;     // 1f6-1f6:    1 byte debug print window for ARM9
  u8 printWindowArm7;     // 1f7-1f7:    1 byte debug print window for ARM7
  u8 printWindowArm9Err; // 1f8-1f8:    1 byte debug print window for ARM9 error
  u8 printWindowArm7Err; // 1f9-1f9:    1 byte debug print window for ARM7 error
#ifdef SDK_TWL
  u8 nandFirmHotStartFlag; // 1fa-1fa:    1 byte HotStartFlag from NAND firmware
  u8 REDLauncherVersion;   // 1fb-1fb:    1 byte RED launcher version
  u32 preloadParameterAddr; // 1fc-1ff:   4 bytes preload Parameter Address
#else
  u8 padding1[6]; // 1fa-1ff:   (6-byte)
#endif
  u8 rom_header[0x160];    // 200-35f: 352 bytes ROM internal registration area
                           // info storage buffer
  u8 isd_reserved2[32];    // 360-37f:  32 bytes ISDebugger reservation
  u32 pxiSignalParam[2];   // 380-387:   8 bytes Param for PXI Signal
  u32 pxiHandleChecker[2]; // 388-38f:   8 bytes Flag  for PXI Command Handler
                           // Installed
  u32 mic_last_address;    // 390-393:   4 bytes MIC latest samppling result
                           // storage address
  u16 mic_sampling_data;   // 394-395:   2 bytes MIC individual sampling result
  u16 wm_callback_control; // 396-397:   2 bytes Parameter for WM callback
                           // synchronization
  u16 wm_rssi_pool; // 398-399:   2 bytes Random number source depending on WM
                    // received signal intensity
  u8 ctrdg_SetModuleInfoFlag; // 39a-39a:   1 byte set ctrdg module info flag
  u8 ctrdg_IsExisting;        // 39b-39b:   1 byte ctrdg exist flag
  u32 component_param;        // 39c-39f:   4 bytes Parameter for Component
                              // synchronization
  OSThreadInfo *threadinfo_mainp; // 3a0-3a3:   4 bytes Pointer to ARM9 thread
                                  // information; ensure its initial value is 0
  OSThreadInfo *threadinfo_subp;  // 3a4-3a7:   4 bytes Pointer to ARM9 thread
                                  // information; ensure its initial value is 0
  u16 button_XY; // 3a8-3a9:   2 bytes XY button information storage location
  u8 touch_panel[4]; // 3aa-3ad:   4 bytes Touch Screen information storage
                     // location
  u16 autoloadSync;  // 3ae-3af:   2 bytes autoload sync between processors
  u32 lockIDFlag_mainp[2]; // 3b0-3b7:   8 bytes lockID management flag (for
                           // ARM9)
  u32 lockIDFlag_subp[2];  // 3b8-3bf:   8 bytes lockID management flag (for
                           // ARM7)
  struct OSLockWord
      lock_VRAM_C; // 3c0-3c7:   8 bytes           VRAM-C - lock buffer
  struct OSLockWord lock_VRAM_D; // 3c8-3cf:   8 bytes VRAM-D - lock buffer
  struct OSLockWord
      lock_WRAM_BLOCK0; // 3d0-3d7:   8 bytes   Block0 - lock buffer
  struct OSLockWord lock_WRAM_BLOCK1; // 3d8-3df:   8 bytes CPU internal work
                                      // RAM - Block1 - lock buffer
  struct OSLockWord lock_CARD; // 3e0-3e7:   8 bytes Game Card - lock buffer
  struct OSLockWord lock_CARTRIDGE; // 3e8-3ef:   8 bytes Game Pak - lock buffer
  struct OSLockWord lock_INIT; // 3f0-3f7:   8 bytes Initialization lock buffer
  u16 mmem_checker_mainp; // 3f8-3f9:   2 bytes MainMemory Size Checker for Main
                          // processor
  u16 mmem_checker_subp;  // 3fa-3fb:   2 bytes MainMemory Size Checker for Sub
                          // processor
  u8 padding4[2];         // 3fc-3fd: (2 bytes)
  u16 command_area;       // 3fe-3ff:   2 bytes Command Area
} OSSystemWork;

#define OS_GetSystemWork() ((OSSystemWork *)HW_MAIN_MEM_SYSTEM)

#ifdef SDK_TWL
typedef struct {
  struct OSLockWord
      lock_WRAM_ex;  // 000-003:   4 bytes: Lock buffer for WRAM-A, B, and C
  u32 reset_flag;    // 004-007:   4 bytes: Reset flags (hardware reset)
  u8 padding[0x178]; // 008-17f:  (376 bytes)
} OSSystemWork2;

#define OS_GetSystemWork2() ((OSSystemWork2 *)HW_PSEG1_RESERVED_0)
#endif // SDK_TWL

#ifndef SDK_TWL
#define OS_IsCodecTwlMode() (FALSE)

#endif

typedef u16 OSBootType;
#define OS_BOOTTYPE_ILLEGAL 0     // Illegal status
#define OS_BOOTTYPE_ROM 1         // Boot from ROM
#define OS_BOOTTYPE_DOWNLOAD_MB 2 // Start a downloaded application
#define OS_BOOTTYPE_NAND 3        // Start an application in NAND memory
#define OS_BOOTTYPE_MEMORY 4      //

typedef struct OSBootInfo {
  OSBootType boot_type; // 2

  u16 length;     // 4
  u16 rssi;       // 6
  u16 bssid[3];   // 12
  u16 ssidLength; // 14
  u8 ssid[32];    // 46
  u16 capaInfo;   // 48
  struct {
    u16 basic;   // 50
    u16 support; // 52
  } rateSet;
  u16 beaconPeriod;   // 54
  u16 dtimPeriod;     // 56
  u16 channel;        // 58
  u16 cfpPeriod;      // 60
  u16 cfpMaxDuration; // 62
  u16 rsv1;           // 64
} OSBootInfo;

OSBootType OS_GetBootType(void);

const OSBootInfo *OS_GetBootInfo(void);

#endif // SDK_ASM

#endif // SDK_FROM_TOOL

#endif
