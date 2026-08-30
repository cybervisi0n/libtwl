#include <nitro/os.h>
#include <nitro/code32.h>
#ifdef SDK_TWL
#include <twl/scfg/common/scfg.h>
#endif

#ifdef SDK_PORT
void OS_EnableITCM(void) {}

void OS_DisableITCM(void) {}

void OS_SetITCMParam(register u32 param) {}

u32 OS_GetITCMParam(void) { return 0; }

void OS_EnableDTCM(void) {}

void OS_DisableDTCM(void) {}

void OS_SetDTCMParam(register u32 param) {}

u32 OS_GetDTCMParam(void) { return 0; }

void OS_SetDTCMAddress(register u32 address) {}

u32 OS_GetDTCMAddress(void) { return 0; }
#else

#if defined(SDK_TCM_APPLY) && defined(SDK_ARM9)
#include <nitro/itcm_begin.h>
#endif

asm OSIntrMode OS_EnableInterrupts(void) {
  mrs r0, cpsr bic r1, r0, #HW_PSR_IRQ_DISABLE msr cpsr_c, r1 and r0, r0,
      #HW_PSR_IRQ_DISABLE

      bx lr
}

asm OSIntrMode OS_DisableInterrupts(void) {
  mrs r0, cpsr orr r1, r0, #HW_PSR_IRQ_DISABLE msr cpsr_c, r1 and r0, r0,
      #HW_PSR_IRQ_DISABLE

      bx lr
}

asm OSIntrMode OS_RestoreInterrupts(register OSIntrMode state) {
  mrs r1, cpsr bic r2, r1, #HW_PSR_IRQ_DISABLE orr r2, r2, r0 msr cpsr_c,
      r2 and r0, r1,
      #HW_PSR_IRQ_DISABLE

      bx lr
}
#if defined(SDK_TCM_APPLY) && defined(SDK_ARM9)
#include <nitro/itcm_end.h>
#endif

asm OSIntrMode OS_EnableInterrupts_IrqAndFiq(void) {
  mrs r0, cpsr bic r1, r0, #HW_PSR_IRQ_FIQ_DISABLE msr cpsr_c, r1 and r0, r0,
      #HW_PSR_IRQ_FIQ_DISABLE

      bx lr
}

asm OSIntrMode OS_DisableInterrupts_IrqAndFiq(void) {
  mrs r0, cpsr orr r1, r0, #HW_PSR_IRQ_FIQ_DISABLE msr cpsr_c, r1 and r0, r0,
      #HW_PSR_IRQ_FIQ_DISABLE

      bx lr
}

asm OSIntrMode OS_RestoreInterrupts_IrqAndFiq(register OSIntrMode state) {
  mrs r1, cpsr bic r2, r1, #HW_PSR_IRQ_FIQ_DISABLE orr r2, r2, r0 msr cpsr_c,
      r2 and r0, r1,
      #HW_PSR_IRQ_FIQ_DISABLE

      bx lr
}

asm OSIntrMode_Irq OS_GetCpsrIrq(void) {
  mrs r0, cpsr and r0, r0,
      #HW_PSR_IRQ_DISABLE

      bx lr
}

#if defined(SDK_TCM_APPLY) && defined(SDK_ARM9)
#include <nitro/itcm_begin.h>
#endif

asm OSProcMode OS_GetProcMode(void) {
  mrs r0, cpsr and r0, r0,
      #HW_PSR_CPU_MODE_MASK

      bx lr
}
#if defined(SDK_TCM_APPLY) && defined(SDK_ARM9)
#include <nitro/itcm_end.h>
#endif

#include <nitro/codereset.h>
#endif

#ifdef SDK_ARM9
#include <nitro/code32.h>
asm void OS_SpinWaitCpuCycles(u32 cycle) {
_1:
  subs r0, r0,
      #4     // 3 cycle
      bcs _1 // 1 cycle
          bx lr
}
#include <nitro/codereset.h>
#else
void OS_SpinWaitCpuCycles(u32 cycle) {
#ifdef SDK_BUILD_ARM
  SVC_WaitByLoop((s32)cycle / 4);
#endif
}
#endif

#if defined(SDK_ARM9) || defined(SDK_PORT)
void OS_SpinWaitSysCycles(u32 cycle) {
#ifdef SDK_TWL
  cycle <<= (SCFG_GetCpuSpeed() == SCFG_CPU_SPEED_2X) ? 2 : 1;
#else
  cycle <<= 1;
#endif

  if (cycle > 16) {
    OS_SpinWaitCpuCycles(cycle - 16);
  }
}
#else
void OS_SpinWaitSysCycles(u32 cycle) { SVC_WaitByLoop((s32)cycle / 4); }
#endif

void OS_WaitInterrupt(BOOL clear, OSIrqMask irqFlags) {
  OSIntrMode cpsrIrq = OS_DisableInterrupts();
  BOOL ime = OS_EnableIrq();

  if (clear) {
    (void)OS_ClearIrqCheckFlag(irqFlags);
  }

  while (!(OS_GetIrqCheckFlag() & irqFlags)) {
    OS_Halt();
    (void)OS_EnableInterrupts();
    (void)OS_DisableInterrupts();
  }

  (void)OS_ClearIrqCheckFlag(irqFlags);
  (void)OS_RestoreIrq(ime);
  (void)OS_RestoreInterrupts(cpsrIrq);
}

void OS_WaitVBlankIntr(void) {
  SVC_WaitByLoop(1);
#if defined(SDK_ENABLE_ARM7_PRINT) && defined(SDK_ARM9)

  OS_PrintServer();
#endif
  OS_WaitIrq(TRUE, OS_IE_V_BLANK);
}
