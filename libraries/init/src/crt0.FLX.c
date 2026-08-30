#include <nitro/types.h>
#include <nitro/hw/common/armArch.h>
#include <nitro/mi/uncompress.h>
#include <nitro/os/common/systemWork.h>
#include <twl/init/crt0.h>
#include <twl/memorymap.h>
#include <twl/misc.h>
#include <twl/version.h>
#include "boot_sync.h"

void _start(void);
void _start_AutoloadDoneCallback(void *argv[]);

#define SDK_NITROCODE_LE 0x2106c0de
#define SDK_NITROCODE_BE 0xdec00621

#define SDK_TWLCODE_LE 0x6314c0de
#define SDK_TWLCODE_BE 0xdec01463

#define SDK_BUILDCODE_LE 0x3381c0de
#define SDK_BUILDCODE_BE 0xdec08133

#if defined(SDK_DEBUG)
#define SDK_BUILDCODE 2
#elif defined(SDK_RELEASE)
#define SDK_BUILDCODE 1
#elif defined(SDK_FINALROM)
#define SDK_BUILDCODE 0
#else
#define SDK_BUILDCODE 255 //  Error
#endif

#if defined(SDK_ARM9)
#define SDK_TARGET 9
#elif defined(SDK_ARM7)
#define SDK_TARGET 7
#else
#define SDK_TARGET 255 //  Error
#endif

extern void OS_IrqHandler(void);
extern void _fp_init(void);
extern void __call_static_initializers(void);
extern void OS_ShowAttentionOfLimitedRom(void);

static void INITi_CpuClear32(register u32 data, register void *destp,
                             register u32 size);
static void INITi_InitCoprocessor(void);
static void INITi_InitRegion(void);
static BOOL INITi_IsRunOnTwl(void);
static void INITi_DoAutoload(void);
#ifndef SDK_NOINIT
static void INITi_ShelterStaticInitializer(u32 *ptr);
static void INITi_CallStaticInitializers(void);
#endif
static void *INITi_Copy32(void *dst, void *src, u32 size);
static void *INITi_Fill32(void *dst, u32 value, u32 size);

extern void SDK_AUTOLOAD_LIST(void);
extern void SDK_AUTOLOAD_LIST_END(void);
extern void SDK_AUTOLOAD_START(void);
extern void SDK_STATIC_BSS_START(void);
extern void SDK_STATIC_BSS_END(void);

void *const _start_ModuleParams[] = {
    (void *)SDK_AUTOLOAD_LIST,
    (void *)SDK_AUTOLOAD_LIST_END,
    (void *)SDK_AUTOLOAD_START,
    (void *)SDK_STATIC_BSS_START,
    (void *)SDK_STATIC_BSS_END,
    (void *)0, // CompressedStaticEnd. This fixed number will be updated by the
               // compstatic tool
    (void *)SDK_CURRENT_VERSION_NUMBER,
    (void *)SDK_NITROCODE_BE,
    (void *)SDK_NITROCODE_LE,
};

#pragma force_active on
void *const _start_BuildParams[] = {
    (void *)0,                                   // Reserved
    (void *)0,                                   // Reserved
    (void *)0,                                   // Reserved
    (void *)0,                                   // Reserved
    (void *)(SDK_BUILDCODE | (SDK_TARGET << 8)), // Build target and build type
    (void *)1, // Version (1byte) Reserved (3byte)
    (void *)SDK_BUILDCODE_BE,
    (void *)SDK_BUILDCODE_LE,
};

extern void SDK_LTDAUTOLOAD_LIST(void);
extern void SDK_LTDAUTOLOAD_LIST_END(void);
extern void SDK_LTDAUTOLOAD_START(void);

void *const _start_LtdModuleParams[] = {
    (void *)SDK_LTDAUTOLOAD_LIST,
    (void *)SDK_LTDAUTOLOAD_LIST_END,
    (void *)SDK_LTDAUTOLOAD_START,
    (void *)0, // CompressedLtdautoloadEnd. This fixed number will be updated by
               // the compstatic tool
    (void *)SDK_TWLCODE_BE,
    (void *)SDK_TWLCODE_LE,
};

static const u32 microcode_ShakeHand[10] = {
    0xe1d120b0, /*      ldrh    r2, [r1]    ; Read synchronous variable 7 from
                   shared memory */
    0xe1d030b0, /*      ldrh    r3, [r0]    ; Read synchronous variable 9 from
                   shared memory */
    0xe2833001, /*  @1: add     r3, r3, #1  ; Synchronous variable 9++ */
    0xe1c030b0, /*      strh    r3, [r0]    ; Write synchronous variable 9 to
                   shared memory */
    0xe1d1c0b0, /*      ldrh    r12, [r1]   ; Read the current state of
                   synchronous variable 7 from shared memory */
    0xe152000c, /*      cmp     r2, r12     ; Determine change of synchronous
                   variable 7 */
    0x0afffffa, /*      beq     @1          ; If not changed, loop */
    0xe2833001, /*      add     r3, r3, #1  ; Synchronous variable 9++ */
    0xe1c030b0, /*      strh    r3, [r0]    ; Write synchronous variable 9 to
                   shared memory */
    0xe12fff1e  /*      bx      lr          ; Complete handshake */
};

static const u32 microcode_WaitAgreement[7] = {
    0xe1d020b0, /*  @1: ldrh    r2, [r0]    ; Read the phase management variable
                   from shared memory */
    0xe1510002, /*      cmp     r1, r2      ; Compare with idling phase number
                 */
    0x012fff1e, /*      bxeq    lr          ; If matched, complete idling */
    0xe3a03010, /*      mov     r3, #16     ; Initialize empty loop count */
    0xe2533001, /*  @2: subs    r3, r3, #1  ; Empty loop count  -- */
    0x1afffffd, /*      bne     @2          ; 16 loops */
    0xeafffff8  /*      b       @1          ; Return to top */
};

static const u32 microcode_SwitchCpuClock[13] = {
    0xe3500000, /*      cmp     r0, #0          ; Evaluate first argument */
    0xe59f3024, /*      ldr     r3, [pc, #36]   ; Read the REG_CLK_ADDR constant
                 */
    0xe1d300b0, /*      ldrh    r0, [r3]        ; Read the REG_CLK_ADDR content
                 */
    0x03c02001, /*      biceq   r2, r0, #1      ; Lower the
                   REG_SCFG_CLK_CPUSPD_MASK flag when changing to constant speed
                 */
    0x13802001, /*      orrne   r2, r0, #1      ; Raise the
                   REG_SCFG_CLK_CPUSPD_MASK flag when changing to double speed
                 */
    0xe1500002, /*      cmp     r0, r2          ; Evaluate the need to change
                   the content of REG_CLK_ADDR */
    0xe2000001, /*      and     r0, r0, #1      ; Edit the function return value
                 */
    0x012fff1e, /*      bxeq    lr              ; Quit function if change is not
                   necessary */
    0xe1c320b0, /*      strh    r2, [r3]        ; Write the changed contents to
                   REG_CLK_ADDR */
    0xe2511004, /*  @1: subs    r1, r1, #4      ; 1 cycle ; Empty loop count --
                 */
    0xaafffffd, /*      bge     @1              ; 3 cycles or 1 cycle ; (Idle
                   cycle count/4) numbers of loops; Complete clock change */
    0xe12fff1e, /*      bx      lr              ; Check instruction from branch
                   destination at second cycle */
    0x04004004  /*      REG_CLK_ADDR            ; REG_CLK_ADDR constant
                   definition */
};

#include <twl/code32.h>

#define SET_PROTECTION_A(id, adr, siz)                                         \
  ldr r0, = (adr | HW_C6_PR_##siz | HW_C6_PR_ENABLE)
#define SET_PROTECTION_B(id, adr, siz) mcr p15, 0, r0, c6, id, 0

SDK_WEAK_SYMBOL asm void _start(void) {
  @000 :

      mov r12,
      #HW_REG_BASE str r12,
      [ r12, #REG_IME_OFFSET ] // Use that LSB of HW_REG_BASE is 0b0

      bl INITi_InitCoprocessor

          mov r0,
      #0 ldr r1,
      = HW_PXI_SIGNAL_PARAM_ARM9 ldr r2,
      = (HW_CTRDG_LOCK_BUF - HW_PXI_SIGNAL_PARAM_ARM9)
          bl INITi_CpuClear32 mov r0,
      #0 ldr r1,
      = HW_INIT_LOCK_BUF ldr r2,
      = (HW_MAIN_MEM_SYSTEM_END - HW_INIT_LOCK_BUF) bl INITi_CpuClear32

          mov r0,
      #0 ldr r1,
      = (HW_MAIN_MEM_PARAMETER_BUF_END - HW_PARAM_TWL_RESERVED_AREA_SIZE)
          ldr r2,
      = HW_PARAM_TWL_RESERVED_AREA_SIZE bl INITi_CpuClear32

          ldr r1,
      = HW_WM_BOOT_BUF ldrh r2,
      [ r1, #OSBootInfo.boot_type ] cmp r2,
      #OS_BOOTTYPE_ILLEGAL moveq r2,
      #OS_BOOTTYPE_ROM streqh r2,
      [ r1, #OSBootInfo.boot_type ]

      ldr r1,
      = microcode_ShakeHand ldr r2,
      = HW_ITCM add r3,
      r2,
      #40 @001 : ldr r0,
                 [r1],
                 #4 str r0,
                 [r2],
                 #4 cmp r2,
                 r3 blt @001

                 ldr r0,
                 = HW_BOOT_SYNC_PHASE mov r1,
                 #BOOT_SYNC_PHASE_1 strh r1,
                 [r0] ldr r0,
                 = HW_BOOT_SHAKEHAND_9 ldr r1,
                 = HW_BOOT_SHAKEHAND_7 ldr r2,
                 = HW_ITCM blx r2

#ifdef SDK_TWLLTD

                     bl INITi_IsRunOnTwl bne @020
#endif

                 ldr r1,
                 = microcode_WaitAgreement ldr r2,
                 = HW_ITCM add r3,
                 r2,
                 #28 @002 : ldr r0,
                            [r1],
                            #4 str r0,
                            [r2],
                            #4 cmp r2,
                            r3 blt @002

                            @003 :

      ldr r0,
      = HW_BOOT_SYNC_PHASE mov r1,
      #BOOT_SYNC_PHASE_4 ldr r2,
      = HW_ITCM blx r2

      @010 :

      bl INITi_IsRunOnTwl bne @020

      ldr r1,
      = microcode_SwitchCpuClock ldr r2,
      = HW_ITCM add r2,
      r2,
      #28 mov r3,
      #52 @011 : subs r3,
                 r3,
                 #4 ldr r0,
                 [ r1, r3 ] str r0,
                 [ r2, r3 ] bgt @011

                 mov r0,
                 #REG_SCFG_CLK_CPUSPD_MASK mov r1,
                 #8 blx r2

                 @020 :

      bl INITi_InitRegion

          mov r0,
      #HW_PSR_SVC_MODE // SuperVisor mode
      msr cpsr_c,
      r0 ldr r1,
      = SDK_AUTOLOAD_DTCM_START add r1,
      r1,
      #HW_DTCM_SIZE sub sp,
      r1,
      #HW_DTCM_SYSRV_SIZE sub r1,
      sp,
      #HW_SVC_STACK_SIZE mov r0,
      #HW_PSR_IRQ_MODE // IRQ mode
      msr cpsr_c,
      r0 sub sp,
      r1,
      #4 // 4 bytes for stack check code
      tst sp,
      #4 subeq sp,
      sp,
      #4 /* Adjust so that sp is 8-byte aligned at the time of the jump to the
            IRQ handler */
      ldr r0,
      = SDK_IRQ_STACKSIZE sub r1,
      r1,
      r0 mov r0,
      #HW_PSR_SYS_MODE // System mode
      msr cpsr_csfx,
      r0 sub sp,
      r1,
      #4 // 4 bytes for stack check code
      tst sp,
      #4 subne sp,
      sp,
      #4 /* Adjust so that sp is 8-byte aligned at the time of the jump to the
            Main function */

      mov r0,
      #0 ldr r1,
      = SDK_AUTOLOAD_DTCM_START mov r2,
      #HW_DTCM_SIZE bl INITi_CpuClear32

          mov r1,
      #0 // r1 = clear value for VRAM
      ldr r0,
      = HW_PLTT // r0 = start address of VRAM for pallet
          mov r2,
      #HW_PLTT_SIZE // r2 = size of VRAM for pallet
      bl INITi_Fill32 ldr r0,
      = HW_OAM // r0 = start address of VRAM for OAM
          mov r2,
      #HW_OAM_SIZE // r2 = size of VRAM for OAM
      bl INITi_Fill32

          bl INITi_DoAutoload

              mov r1,
      #0 // r1 = clear value for STATIC bss section
      ldr r3,
      = _start_ModuleParams ldr r0,
      [ r3, #12 ] // r0 = start address of STATIC bss section
      ldr r2,
      [ r3, #16 ] subs r2,
      r2,
      r0 // r2 = size of STATIC bss section
          blgt INITi_Fill32

              ldr r1,
      = SDK_AUTOLOAD_DTCM_START add r1,
      r1,
      #HW_DTCM_SIZE - HW_DTCM_SYSRV_SIZE add r1,
      r1,
      #HW_DTCM_SYSRV_OFS_INTR_VECTOR ldr r0,
      = OS_IrqHandler str r0,
      [r1]

#ifdef SDK_TWLLTD

      bl INITi_IsRunOnTwl blne OS_ShowAttentionOfLimitedRom

#endif

#ifndef SDK_NOINIT

          bl _fp_init bl TwlStartUp bl __call_static_initializers bl
              INITi_CallStaticInitializers
#endif

                  ldr r1,
      = REG_VCOUNT_ADDR @022 : ldrh r0,
      [r1] cmp r0,
      #0 bne @022

      @030 :

      ldr r1,
      = TwlMain ldr lr,
      = HW_RESET_VECTOR bx r1
}

static asm void INITi_CpuClear32(register u32 data, register void *destp,
                                 register u32 size) {
  add r12, r1, r2 @001 : cmp r1, r12 strlt r0, [r1], #4 blt @001 bx lr
}

static asm void INITi_InitCoprocessor(void) {

  mrc p15, 0, r0, c1, c0,
      0

      tst r0,
      #HW_C1_PROTECT_UNIT_ENABLE beq @010 tst r0,
      #HW_C1_DCACHE_ENABLE beq @003

      mov r1,
      #0 @001 : mov r2,
                #0 @002 : orr r3,
                          r1,
                          r2 mcr p15,
                          0,
                          r3,
                          c7,
                          c10,
                          2 add r2,
                          r2,
                          #HW_CACHE_LINE_SIZE cmp r2,
                          #HW_DCACHE_SIZE / 4 blt @002 adds r1,
                          r1,
                          #1 << HW_C7_CACHE_SET_NO_SHIFT bne @001

                              @003 : /* Wait until the write buffer is empty */
                                     mov r1,
                          #0 mcr p15,
                          0,
                          r1,
                          c7,
                          c10,
                          4

                          @010
      : /* Initialize the co-processor state */
        ldr r1,
        = HW_C1_ITCM_LOAD_MODE | HW_C1_DTCM_LOAD_MODE | HW_C1_ITCM_ENABLE |
          HW_C1_DTCM_ENABLE | HW_C1_LD_INTERWORK_DISABLE | HW_C1_ICACHE_ENABLE |
          HW_C1_DCACHE_ENABLE | HW_C1_PROTECT_UNIT_ENABLE bic r0,
        r0,
        r1 ldr r1,
        = HW_C1_SB1_BITSET | HW_C1_EXCEPT_VEC_UPPER orr r0,
        r0,
        r1 mcr p15,
        0,
        r0,
        c1,
        c0,
        0

        mov r1,
        #HW_C9_TCMR_32MB mcr p15,
        0,
        r1,
        c9,
        c1,
        1

        ldr r1,
        = SDK_AUTOLOAD_DTCM_START orr r1,
        r1,
        #HW_C9_TCMR_16KB mcr p15,
        0,
        r1,
        c9,
        c1,
        0

        mov r1,
        #HW_C1_ITCM_ENABLE | HW_C1_DTCM_ENABLE orr r0,
        r0,
        r1 mcr p15,
        0,
        r0,
        c1,
        c0,
        0

        bx lr
}

static asm void INITi_InitRegion(void) {
  mov r12,
      lr

#define REGION_BIT(a, b, c, d, e, f, g, h)                                     \
  (((a) << 0) | ((b) << 1) | ((c) << 2) | ((d) << 3) | ((e) << 4) |            \
   ((f) << 5) | ((g) << 6) | ((h) << 7))
#define REGION_ACC(a, b, c, d, e, f, g, h)                                     \
  (((a) << 0) | ((b) << 4) | ((c) << 8) | ((d) << 12) | ((e) << 16) |          \
   ((f) << 20) | ((g) << 24) | ((h) << 28))
#define NA 0
#define RW 1
#define RO 5

      SET_PROTECTION_A(c0, HW_IOREG, 64MB) SET_PROTECTION_B(c0, HW_IOREG, 64MB)

          ldr r0,
      = SDK_AUTOLOAD_DTCM_START orr r0, r0,
      #HW_C6_PR_16KB |
          HW_C6_PR_ENABLE SET_PROTECTION_B(c4, SDK_AUTOLOAD_DTCM_START, 16KB)

              SET_PROTECTION_A(c5, HW_ITCM_IMAGE, 16MB)
                  SET_PROTECTION_B(c5, HW_ITCM_IMAGE, 16MB)

                      SET_PROTECTION_A(c6, HW_BIOS, 32KB)
                          SET_PROTECTION_B(c6, HW_BIOS, 32KB)

                              bl INITi_IsRunOnTwl bne @002

          @001
      : /* When using TWL hardware */

        SET_PROTECTION_A(c1, HW_TWL_MAIN_MEM_MAIN,
                         32MB) SET_PROTECTION_B(c1, HW_TWL_MAIN_MEM_MAIN, 32MB)

            SET_PROTECTION_A(c2, HW_TWL_MAIN_MEM_SUB, 512KB)
                SET_PROTECTION_B(c2, HW_TWL_MAIN_MEM_SUB, 512KB)

                    SET_PROTECTION_A(c3, HW_CTRDG_ROM, 128MB) SET_PROTECTION_B(
                        c3, HW_CTRDG_ROM, 128MB)

                        SET_PROTECTION_A(c7, HW_TWL_MAIN_MEM_SHARED, 16KB)
                            SET_PROTECTION_B(c7, HW_TWL_MAIN_MEM_SHARED, 16KB)

                                mov r0,
      #REGION_BIT(0, 1, 0, 1, 0, 0, 1, 0) mcr p15, 0, r0, c2, c0,
      1

      mov r0,
      #REGION_BIT(0, 1, 0, 1, 0, 0, 1, 0) mcr p15, 0, r0, c2, c0,
      0

      mov r0,
      #REGION_BIT(0, 1, 0, 1, 0, 0, 0, 0) mcr p15, 0, r0, c3, c0,
      0

      ldr r0,
      = REGION_ACC(RW, RW, NA, RW, NA, RW, RO, NA) mcr p15, 0, r0, c5, c0,
      3

      ldr r0,
      = REGION_ACC(RW, RW, NA, RW, RW, RW, RO, RW) mcr p15, 0, r0, c5, c0,
      2

      b @003

      @002
      : /* When using NITRO hardware */

        SET_PROTECTION_A(c1, HW_MAIN_MEM_MAIN,
                         32MB) SET_PROTECTION_B(c1, HW_MAIN_MEM_MAIN, 32MB)

            SET_PROTECTION_A(
                c2, (HW_MAIN_MEM_IM_SHARED_END - HW_MAIN_MEM_IM_SHARED_SIZE),
                4KB) SET_PROTECTION_B(c2,
                                      (HW_MAIN_MEM_IM_SHARED_END -
                                       HW_MAIN_MEM_IM_SHARED_SIZE),
                                      4KB)

                SET_PROTECTION_A(c3, HW_CTRDG_ROM, 128MB)
                    SET_PROTECTION_B(c3, HW_CTRDG_ROM, 128MB)

                        SET_PROTECTION_A(c7, HW_MAIN_MEM_SHARED, 4KB)
                            SET_PROTECTION_B(c7, HW_MAIN_MEM_SHARED, 4KB)

                                mov r0,
        #REGION_BIT(0, 1, 0, 0, 0, 0, 1, 0) mcr p15,
        0,
        r0,
        c2,
        c0,
        1

        mov r0,
        #REGION_BIT(0, 1, 0, 0, 0, 0, 1, 0) mcr p15,
        0,
        r0,
        c2,
        c0,
        0

        mov r0,
        #REGION_BIT(0, 1, 0, 0, 0, 0, 0, 0) mcr p15,
        0,
        r0,
        c3,
        c0,
        0

        ldr r0,
        = REGION_ACC(RW, RW, NA, RW, NA, RW, RO, NA) mcr p15,
        0,
        r0,
        c5,
        c0,
        3

        ldr r0,
        = REGION_ACC(RW, RW, RW, RW, RW, RW, RO, RW) mcr p15,
        0,
        r0,
        c5,
        c0,
        2

        @003 : /* Set to allow use of the protection unit and cache */
               mrc p15,
               0,
               r0,
               c1,
               c0,
               0 ldr r1,
               = HW_C1_ICACHE_ENABLE | HW_C1_DCACHE_ENABLE |
                 HW_C1_CACHE_ROUND_ROBIN | HW_C1_PROTECT_UNIT_ENABLE orr r0,
               r0,
               r1 mcr p15,
               0,
               r0,
               c1,
               c0,
               0

               mov r1,
               #0 mcr p15,
               0,
               r1,
               c7,
               c6,
               0 mcr p15,
               0,
               r1,
               c7,
               c5,
               0

               bx r12
}

static asm void INITi_DoAutoload(void) {
  stmfd sp !, { lr }

  @01_uncompress :

      ldr r1,
      = _start_ModuleParams ldr r0,
      [ r1, #20 ] // r0 = bottom of compressed data
      bl MIi_UncompressBackward @01_autoload :

      ldr r0,
      = _start_ModuleParams ldr r12,
      [r0] // r12 = SDK_AUTOLOAD_LIST
      ldr r3,
      [ r0, #4 ] // r3 = SDK_AUTOLOAD_LIST_END
      ldr r1,
      [ r0, #8 ] // r1 = SDK_AUTOLOAD_START
      @01_segments_loop
      : cmp r12,
        r3 bge @02_start stmfd sp !,
        {r3}

        ldr r0,
        [r12],
        #4 // r0 = start address of destination range of fixed section
        ldr r2,
        [r12],
        #4 // r2 = size of fixed section
        stmfd sp !,
        {r0} bl INITi_Copy32 stmfd sp !,
        {r0, r1}

        ldr r0,
        [r12],
        #4 // r0 = start address of source range of static initializer table
#ifndef SDK_NOINIT
        stmfd sp !,
        {r12} bl INITi_ShelterStaticInitializer ldmfd sp !,
        {r12}
#endif

        ldmfd sp !,
        {r0} // r0 = start address of destination range of bss section
        mov r1,
        #0 // r1 = clear value for bss section
        ldr r2,
        [r12],
        #4 // r2 = size of bss section
        bl INITi_Fill32 ldmfd sp !,
        {r1, r2} // r1 = end address of source range of the autoload segment

        mov r3,
        #HW_ITCM_IMAGE cmp r2,
        r3 movge r3,
        #HW_ITCM_END cmpge r3,
        r2 bgt
        @01_next_segment // If I-TCM autoload block, skip cache store loop
        ldr r3,
        = SDK_AUTOLOAD_DTCM_START cmp r2,
        r3 addge r3,
        r3,
        #HW_DTCM_SIZE cmpge r3,
        r2 bgt
        @01_next_segment // If D-TCM autoload block, skip cache store loop

        bic r2,
        r2,
        #(HW_CACHE_LINE_SIZE - 1) @01_cache_store_loop
      : cmp r2,
        r0 bge @01_next_segment mcr p15,
        0,
        r2,
        c7,
        c14,
        1 // Store and invalidate D-Cache
        mcr p15,
        0,
        r2,
        c7,
        c5,
        1 // Invalidate I-Cache
        add r2,
        r2,
        #HW_CACHE_LINE_SIZE b @01_cache_store_loop @01_next_segment
      : ldmfd sp !,
        {r3} b @01_segments_loop

        @02_start :

      bl INITi_IsRunOnTwl bne @03_start

      ldr r1,
      = HW_TWL_ROM_HEADER_BUF +
        0x1cc // Extended static module ROM size for ARM9
        ldr r0,
      [r1] cmp r0,
      #0 beq @03_start

      @02_uncompress :

      ldr r1,
      = _start_LtdModuleParams ldr r0,
      [ r1, #12 ] // r0 = bottom of compressed data
      bl MIi_UncompressBackward @02_autoload :

      ldr r0,
      = _start_LtdModuleParams ldr r12,
      [r0] // r12 = SDK_LTDAUTOLOAD_LIST
      ldr r3,
      [ r0, #4 ] // r3 = SDK_LTDAUTOLOAD_LIST_END
      ldr r1,
      [ r0, #8 ] // r1 = SDK_LTDAUTOLOAD_START
      @02_segments_loop
      : cmp r12,
        r3 bge @03_start stmfd sp !,
        {r3}

        ldr r0,
        [r12],
        #4 // r0 = start address of destination range of fixed section
        ldr r2,
        [r12],
        #4 // r2 = size of fixed section
        stmfd sp !,
        {r0} bl INITi_Copy32 stmfd sp !,
        {r0, r1}

        ldr r0,
        [r12],
        #4 // r0 = start address of source range of static initializer table
#ifndef SDK_NOINIT
        stmfd sp !,
        {r12} bl INITi_ShelterStaticInitializer ldmfd sp !,
        {r12}
#endif

        ldmfd sp !,
        {r0} // r0 = start address of destination range of bss section
        mov r1,
        #0 // r1 = clear value for bss section
        ldr r2,
        [r12],
        #4 // r2 = size of bss section
        bl INITi_Fill32 ldmfd sp !,
        {r1, r2} // r1 = end address of source range of the ltdautoload segment

        mov r3,
        #HW_ITCM_IMAGE cmp r2,
        r3 movge r3,
        #HW_ITCM_END cmpge r3,
        r2 bgt
        @02_next_segment // If I-TCM autoload block, skip cache store loop
        ldr r3,
        = SDK_AUTOLOAD_DTCM_START cmp r2,
        r3 addge r3,
        r3,
        #HW_DTCM_SIZE cmpge r3,
        r2 bgt
        @02_next_segment // If D-TCM autoload block, skip cache store loop

        bic r2,
        r2,
        #(HW_CACHE_LINE_SIZE - 1) @02_cache_store_loop
      : cmp r2,
        r0 bge @02_next_segment mcr p15,
        0,
        r2,
        c7,
        c14,
        1 // Store and invalidate D-Cache
        mcr p15,
        0,
        r2,
        c7,
        c5,
        1 // Invalidate I-Cache
        add r2,
        r2,
        #HW_CACHE_LINE_SIZE b @02_cache_store_loop @02_next_segment ldmfd sp !,
        {r3} b @02_segments_loop

        @03_start :

      mov r0,
      #0 mcr p15,
      0,
      r0,
      c7,
      c10,
      4

      ldr r0,
      = _start_ModuleParams ldr r1,
      = _start_LtdModuleParams ldmfd sp !,
      {lr} b _start_AutoloadDoneCallback
}

#ifndef SDK_NOINIT
static asm void INITi_ShelterStaticInitializer(u32 *ptr) {

  cmp r0,
      #0 bxeq lr

          ldr r1,
      = SDK_AUTOLOAD_DTCM_START add r1, r1, #HW_DTCM_SIZE sub r1, r1,
      #HW_DTCM_SYSRV_SIZE sub r1, r1, #HW_SVC_STACK_SIZE ldr r2,
      = SDK_IRQ_STACKSIZE sub r1, r1, r2 add r1, r1,
      #4

      @001 : ldr r2,
             [r1] cmp r2,
             #0 addne r1,
             r1,
             #4 bne @001

             @002 : ldr r2,
                    [r0],
                    #4 str r2,
                    [r1],
                    #4 cmp r2,
                    #0 bne @002

                    bx lr
}

static asm void INITi_CallStaticInitializers(void) {
  stmdb sp !,
      {lr}

  ldr r1,
      = SDK_AUTOLOAD_DTCM_START add r1, r1, #HW_DTCM_SIZE sub r1, r1,
      #HW_DTCM_SYSRV_SIZE sub r1, r1, #HW_SVC_STACK_SIZE ldr r2,
      = SDK_IRQ_STACKSIZE sub r1, r1, r2 add r1, r1,
      #4

      @001 : ldr r0,
             [r1] cmp r0,
             #0 beq @002 stmdb sp !,
             {r1} blx r0 ldmia sp !,
             {r1}

             mov r0,
             #0 str r0,
             [r1],
             #4 b @001

             @002 : ldmia sp !,
                    {lr} bx lr
}
#endif

asm void MIi_UncompressBackward(register void *bottom) {
#define data r0
#define inp_top r1
#define outp r2
#define inp r3
#define outp_save r4
#define flag r5
#define count8 r6
#define index r7
#define dummy r8
#define len r12

  cmp bottom, #0 beq @exit stmfd sp !, {r4 - r8} ldmdb bottom,
      {r1 - r2} add outp, bottom, outp sub inp, bottom, inp_top,
      LSR #24 bic inp_top, inp_top, #0xff000000 sub inp_top, bottom,
      inp_top mov outp_save,
      outp @loop : cmp inp,
                   inp_top // Exit if inp==inp_top
                       ble @end_loop ldrb flag,
                   [ inp, # - 1 ] ! // r4 = compress_flag = *--inp
                   mov count8,
                   #8 @loop8 : subs count8,
                               count8,
                               #1 blt @loop tst flag,
                               #0x80 bne @blockcopy @bytecopy
      : ldrb data,
        [ inp, # - 1 ] !ldrb dummy,
        [ outp, # - 1 ] // Dummy read to use D-Cache efficiently
        strb data,
        [ outp, # - 1 ] ! // Copy 1 byte
        b @joinhere @blockcopy : ldrb len,
                                 [ inp, # - 1 ] !ldrb index,
                                 [ inp, # - 1 ] !orr index,
                                 index,
                                 len,
                                 LSL #8 bic index,
                                 index,
                                 #0xf000 add index,
                                 index,
                                 #0x0002 add len,
                                 len,
                                 #0x0020 @patterncopy
      : ldrb data,
        [ outp, index ] ldrb dummy,
        [ outp, # - 1 ] // Dummy read to use D-Cache efficiently
        strb data,
        [ outp, # - 1 ] !subs len,
        len,
        #0x0010 bge @patterncopy

        @joinhere : cmp inp,
                    inp_top mov flag,
                    flag,
                    LSL #1 bgt @loop8 @end_loop :

      mov r0,
      #0 bic inp,
      inp_top,
      #HW_CACHE_LINE_SIZE - 1 @cacheflush : mcr p15,
      0,
      r0,
      c7,
      c10,
      4 // Wait writebuffer empty
      mcr p15,
      0,
      inp,
      c7,
      c5,
      1 // ICache
      mcr p15,
      0,
      inp,
      c7,
      c14,
      1 // DCache
      add inp,
      inp,
      #HW_CACHE_LINE_SIZE cmp inp,
      outp_save blt @cacheflush

          ldmfd sp !,
  {
    r4 - r8
  }
  @exit bx lr
}

SDK_WEAK_SYMBOL asm void _start_AutoloadDoneCallback(void *argv[]){bx lr}

SDK_WEAK_SYMBOL void NitroStartUp(void) {}

void OSi_ReferSymbol(void *symbol) {
#pragma unused(symbol)
}

static asm BOOL INITi_IsRunOnTwl(void) {
  ldr r0, = REG_A9ROM_ADDR ldrb r0, [r0] and r0, r0,
      #REG_SCFG_A9ROM_SEC_MASK | REG_SCFG_A9ROM_RSEL_MASK cmp r0,
      #REG_SCFG_A9ROM_SEC_MASK moveq r0, #TRUE movne r0,
      #FALSE

      bx lr
}

static asm void *INITi_Copy32(void *dst, void *src, u32 size) {
  stmfd sp !, {r3 - r11} bics r3, r2, #0x0000001f beq @next add r3, r0,
      r3 @loop : ldmia r1 !,
                 {r4 - r11} stmia r0 !,
                 {r4 - r11} cmp r3,
                 r0 bgt @loop @next : tst r2,
                                      #0x00000010 ldmneia r1 !,
                                      {r4 - r7} stmneia r0 !,
                                      {r4 - r7} tst r2,
                                      #0x00000008 ldmneia r1 !,
                                      {r4 - r5} stmneia r0 !,
                                      {r4 - r5} tst r2,
                                      #0x00000004 ldmneia r1 !,
                                      {r4} stmneia r0 !,
                                      {r4} ldmfd sp !,
                                      {r3 - r11} bx lr
}

static asm void *INITi_Fill32(void *dst, u32 value, u32 size) {
  stmfd sp !, {r3 - r11} mov r4, r1 mov r5, r1 mov r6, r1 mov r7, r1 mov r8,
      r1 mov r9, r1 mov r10, r1 mov r11, r1 bics r3, r2,
      #0x0000001f beq @next add r3, r0,
      r3 @loop : stmia r0 !,
                 {r4 - r11} cmp r3,
                 r0 bgt @loop @next : tst r2,
                                      #0x00000010 stmneia r0 !,
                                      {r4 - r7} tst r2,
                                      #0x00000008 stmneia r0 !,
                                      {r4 - r5} tst r2,
                                      #0x00000004 stmneia r0 !,
                                      {r4} ldmfd sp !,
                                      {r3 - r11} bx lr
}

#include <twl/codereset.h>
