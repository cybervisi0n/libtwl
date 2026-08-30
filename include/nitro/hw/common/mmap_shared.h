#ifndef NITRO_HW_COMMON_MMAP_SHARED_H_
#define NITRO_HW_COMMON_MMAP_SHARED_H_

#ifdef __cplusplus
extern "C" {
#endif

#define HW_RED_RESERVED (HW_MAIN_MEM_EX_END - 0x800) // maybe change later
#define HW_RED_RESERVED_END (HW_RED_RESERVED + HW_RED_RESERVED_SIZE)
#define HW_RED_RESERVED_SIZE 0x200

#define HW_CARD_ROM_HEADER_SIZE 0x160

#define HW_DOWNLOAD_PARAMETER_SIZE 0x20

#define HW_SHARED_ARENA_LO_DEFAULT HW_MAIN_MEM_SHARED
#ifdef HW_RED_RESERVED
#define HW_SHARED_ARENA_HI_DEFAULT                                             \
  (HW_RED_RESERVED - HW_CARD_ROM_HEADER_SIZE - HW_DOWNLOAD_PARAMETER_SIZE)
#else
#define HW_SHARED_ARENA_HI_DEFAULT                                             \
  (HW_MAIN_MEM_SYSTEM - HW_CARD_ROM_HEADER_SIZE - HW_DOWNLOAD_PARAMETER_SIZE)
#endif

#define HW_CARD_ROM_HEADER (HW_MAIN_MEM + 0x00fffa80)

#define HW_DOWNLOAD_PARAMETER (HW_MAIN_MEM + 0x00fffbe0)

#define HW_MAIN_MEM_SYSTEM_SIZE 0x400
#define HW_MAIN_MEM_SYSTEM (HW_MAIN_MEM_EX_END - 0x400)

#define HW_BOOT_CHECK_INFO_BUF                                                 \
  (HW_MAIN_MEM_SYSTEM + 0x000) // Boot check info (END-0x400)
#define HW_BOOT_CHECK_INFO_BUF_END                                             \
  (HW_MAIN_MEM_SYSTEM + 0x020) //                 (END-0x3e0)

#define HW_RESET_PARAMETER_BUF                                                 \
  (HW_MAIN_MEM_SYSTEM + 0x020) // reset parameter (END-0x3e0)

#define HW_ROM_BASE_OFFSET_BUF                                                 \
  (HW_MAIN_MEM_SYSTEM + 0x02c) // ROM offset of own program (END-0x3d4)
#define HW_ROM_BASE_OFFSET_BUF_END                                             \
  (HW_MAIN_MEM_SYSTEM + 0x030) //                           (END-0x3d0)

#define HW_CTRDG_MODULE_INFO_BUF                                               \
  (HW_MAIN_MEM_SYSTEM + 0x030) // Cartridge module info (END-0x3d0)
#define HW_CTRDG_MODULE_INFO_BUF_END                                           \
  (HW_MAIN_MEM_SYSTEM + 0x03c) //                       (END-0x3c4)

#define HW_VBLANK_COUNT_BUF                                                    \
  (HW_MAIN_MEM_SYSTEM + 0x03c) // V-Blank counter (END-0x3c4)

#define HW_WM_BOOT_BUF                                                         \
  (HW_MAIN_MEM_SYSTEM + 0x040) // WM buffer for Multi-Boot (END-0x3c0)
#define HW_WM_BOOT_BUF_END                                                     \
  (HW_MAIN_MEM_SYSTEM + 0x080) //                          (END-0x380)

#define HW_NVRAM_USER_INFO                                                     \
  (HW_MAIN_MEM_SYSTEM + 0x080) // NVRAM user info (END-0x380)
#define HW_NVRAM_USER_INFO_END                                                 \
  (HW_MAIN_MEM_SYSTEM + 0x180) //                 (END-0x280)

#define HW_BIOS_EXCP_STACK_MAIN                                                \
  (HW_MAIN_MEM_SYSTEM +                                                        \
   0x180) // MAINP Debugger monitor exception handler (END-0x280)
#define HW_BIOS_EXCP_STACK_MAIN_END                                            \
  (HW_MAIN_MEM_SYSTEM + 0x19c) //                                 (END-0x264)
#define HW_EXCP_VECTOR_MAIN                                                    \
  (HW_MAIN_MEM_SYSTEM + 0x19c) // HW_EXCP_VECTOR_BUF for MAINP      (END-0x264)

#define HW_ARENA_INFO_BUF                                                      \
  (HW_MAIN_MEM_SYSTEM +                                                        \
   0x1a0) // Arena data structure (27F_FDA0 to 27F_FDE7) (END-0x260)
#define HW_REAL_TIME_CLOCK_BUF (HW_MAIN_MEM_SYSTEM + 0x1e8) // RTC

#define HW_SYS_CONF_BUF                                                        \
  (HW_MAIN_MEM_SYSTEM + 0x1f0) // System config data (END-0x210)
#define HW_SYS_CONF_BUF_END                                                    \
  (HW_MAIN_MEM_SYSTEM + 0x1f6) //                    (END-0x20a)

#define HW_PRINT_OUTPUT_ARM9                                                   \
  (HW_MAIN_MEM_SYSTEM + 0x1f6) // debug print window for ARM9
#define HW_PRINT_OUTPUT_ARM7                                                   \
  (HW_MAIN_MEM_SYSTEM + 0x1f7) // debug print window for ARM7
#define HW_PRINT_OUTPUT_ARM9ERR                                                \
  (HW_MAIN_MEM_SYSTEM + 0x1f8) // debug print window for ARM9 error
#define HW_PRINT_OUTPUT_ARM7ERR                                                \
  (HW_MAIN_MEM_SYSTEM + 0x1f9) // debug print window for ARM7 error

#define HW_ROM_HEADER_BUF                                                      \
  (HW_MAIN_MEM_SYSTEM +                                                        \
   0x200) // ROM-internal registration area data buffer (END-0x200)
#define HW_ROM_HEADER_BUF_END                                                  \
  (HW_MAIN_MEM_SYSTEM + 0x360) //                                    (END-0x0a0)
#define HW_ISD_RESERVED                                                        \
  (HW_MAIN_MEM_SYSTEM + 0x360) // IS DEBUGGER Reserved (END-0xa0)
#define HW_ISD_RESERVED_END                                                    \
  (HW_MAIN_MEM_SYSTEM + 0x380) //                      (END-0x80)

#define HW_PXI_SIGNAL_PARAM_ARM9                                               \
  (HW_MAIN_MEM_SYSTEM + 0x380) // PXI Signal Param for ARM9
#define HW_PXI_SIGNAL_PARAM_ARM7                                               \
  (HW_MAIN_MEM_SYSTEM + 0x384) // PXI Signal Param for ARM7
#define HW_PXI_HANDLE_CHECKER_ARM9                                             \
  (HW_MAIN_MEM_SYSTEM + 0x388) // PXI Handle Checker for ARM9
#define HW_PXI_HANDLE_CHECKER_ARM7                                             \
  (HW_MAIN_MEM_SYSTEM + 0x38c) // PXI Handle Checker for ARM7

#define HW_MIC_LAST_ADDRESS                                                    \
  (HW_MAIN_MEM_SYSTEM + 0x390) // MIC newest sampling data storage address
#define HW_MIC_SAMPLING_DATA                                                   \
  (HW_MAIN_MEM_SYSTEM + 0x394) // MIC individual sampling results

#define HW_WM_CALLBACK_CONTROL                                                 \
  (HW_MAIN_MEM_SYSTEM + 0x396) // Parameter for WM callback synchronization
#define HW_WM_RSSI_POOL                                                        \
  (HW_MAIN_MEM_SYSTEM +                                                        \
   0x398) // Random number source depends on WM received signal intensity

#define HW_SET_CTRDG_MODULE_INFO_ONCE                                          \
  (HW_MAIN_MEM_SYSTEM + 0x39a) // set ctrdg module info flag
#define HW_IS_CTRDG_EXIST (HW_MAIN_MEM_SYSTEM + 0x39b) // ctrdg exist flag

#define HW_COMPONENT_PARAM                                                     \
  (HW_MAIN_MEM_SYSTEM + 0x39c) // Parameter for component synchronization

#define HW_THREADINFO_MAIN                                                     \
  (HW_MAIN_MEM_SYSTEM + 0x3a0) // ThreadInfo for main processor
#define HW_THREADINFO_SUB                                                      \
  (HW_MAIN_MEM_SYSTEM + 0x3a4) // ThreadInfo for sub processor
#define HW_BUTTON_XY_BUF                                                       \
  (HW_MAIN_MEM_SYSTEM + 0x3a8) // buffer for X and Y Buttons
#define HW_TOUCHPANEL_BUF (HW_MAIN_MEM_SYSTEM + 0x3aa) // buffer for touch panel
#define HW_AUTOLOAD_SYNC_BUF                                                   \
  (HW_MAIN_MEM_SYSTEM + 0x3ae) // buffer for autoload sync

#define HW_LOCK_ID_FLAG_MAIN                                                   \
  (HW_MAIN_MEM_SYSTEM + 0x3b0) // lockID flag for main processor
#define HW_LOCK_ID_FLAG_SUB                                                    \
  (HW_MAIN_MEM_SYSTEM + 0x3b8) // lockID flag for sub processor

#define HW_VRAM_C_LOCK_BUF                                                     \
  (HW_MAIN_MEM_SYSTEM + 0x3c0) // VRAM-C lock buffer (END-0x40)
#define HW_VRAM_D_LOCK_BUF                                                     \
  (HW_MAIN_MEM_SYSTEM + 0x3c8) // VRAM-D lock buffer (END-0x38)
#define HW_WRAM_BLOCK0_LOCK_BUF                                                \
  (HW_MAIN_MEM_SYSTEM +                                                        \
   0x3d0) // CPU internal work RAM - Block0 - lock buffer (END-0x30)
#define HW_WRAM_BLOCK1_LOCK_BUF                                                \
  (HW_MAIN_MEM_SYSTEM +                                                        \
   0x3d8) // CPU internal work RAM - Block1 - lock buffer (END-0x28)
#define HW_CARD_LOCK_BUF                                                       \
  (HW_MAIN_MEM_SYSTEM + 0x3e0) // Game Card - lock buffer (END-0x20)
#define HW_CTRDG_LOCK_BUF                                                      \
  (HW_MAIN_MEM_SYSTEM + 0x3e8) // Game Pak - lock buffer (END-0x18)
#define HW_INIT_LOCK_BUF                                                       \
  (HW_MAIN_MEM_SYSTEM + 0x3f0) // Initialization lock buffer (END-0x10)

#define HW_MMEMCHECKER_MAIN                                                    \
  (HW_MAIN_MEM_SYSTEM +                                                        \
   0x3f8) // MainMemory Size Checker for main processor (END-8)
#define HW_MMEMCHECKER_SUB                                                     \
  (HW_MAIN_MEM_SYSTEM +                                                        \
   0x3fa) // MainMemory Size Checker for sub processor (END-6)

#define HW_CMD_AREA                                                            \
  (HW_MAIN_MEM_SYSTEM +                                                        \
   0x3fe) // Main memory command issue area (Prohibited use area) (END-2)

#define HW_SHARED_LOCK_BUF (HW_VRAM_C_LOCK_BUF)
#define HW_SHARED_LOCK_BUF_END (HW_INIT_LOCK_BUF + 8)

#define HW_CHECK_DEBUGGER_SW                                                   \
  (HW_MAIN_MEM_SYSTEM +                                                        \
   0x010) // (u16) debugger check switch. If 0, check buf1; else buf2.
#define HW_CHECK_DEBUGGER_BUF1                                                 \
  (HW_RED_RESERVED + 0x014) // (u16) debugger checker. 1 if run on debugger.
#define HW_CHECK_DEBUGGER_BUF2                                                 \
  (HW_MAIN_MEM_SYSTEM + 0x014) // (u16) debugger checker. 1 if run on debugger.

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
