#include <nitro/code32.h>
#include <nitro/types.h>
#include <nitro/os/common/interrupt.h>
#include <nitro/os/common/thread.h>
#include <nitro/os/common/system.h>

#ifdef SDK_NITRO
#ifdef SDK_ARM9
#include <nitro/hw/ARM9/mmap_global.h>
#include <nitro/hw/ARM9/ioreg_OS.h>
#elif defined(SDK_PORT)
#include <nitro/hw/X86/mmap_global.h>
#include <nitro/hw/X86/ioreg_OS.h>
#else // SDK_ARM9
#include <nitro/hw/ARM7/mmap_global.h>
#include <nitro/hw/ARM7/ioreg_OS.h>
#endif // SDK_ARM9
#else
#ifdef SDK_ARM9
#include <twl/hw/ARM9/mmap_global.h>
#include <twl/hw/ARM9/ioreg_OS.h>
#elif defined(SDK_PORT)
#include <twl/hw/X86/mmap_global.h>
#include <twl/hw/X86/ioreg_OS.h>
#else // SDK_ARM9
#include <twl/hw/ARM7/mmap_global.h>
#include <twl/hw/ARM7/ioreg_OS.h>
#endif // SDK_ARM9
#endif

#ifdef SDK_ARM9
#include <nitro/dtcm_begin.h>
#endif

OSThreadQueue OSi_IrqThreadQueue = {NULL, NULL};

#ifdef SDK_ARM9
#include <nitro/dtcm_end.h>
#include <nitro/itcm_begin.h>
#endif

#ifdef SDK_PORT
void OS_IrqHandler(void) {}
void OS_IrqHandler_ThreadSwitch(void) {}
#else
asm void OS_IrqHandler(void) {
  stmfd sp !,
      {lr} // Save LR

  mov r12,
      #HW_REG_BASE add r12, r12,
      #REG_IE_OFFSET // r12: REG_IE address

      ldr r1,
      [ r12, #REG_IME_ADDR - REG_IE_ADDR ] // r1: IME

      cmp r1,
      #0 ldmeqfd sp !,
      {pc}

  ldmia r12,
      {r1 - r2} // r1: IE, r2: IF
  ands r1,
      r1,
      r2 // r1: IE & IF

          ldmeqfd sp !,
      {pc}

#if defined(SDK_ARM9) && !defined(SDK_CWBUG_PROC_OPT)

  mov r3,
      #1 << 31 @1 : clz r0,
      r1 // Count zero of high bit
          bics r1,
      r1, r3,
      LSR r0 bne @1

      mov r1,
      r3, LSR r0 str r1,
      [ r12, #REG_IF_ADDR - REG_IE_ADDR ]

      rsbs r0,
      r0,
      #31

#else  // defined(SDK_ARM9) && !defined(SDK_CWBUG_PROC_OPT)

  mov r3,
      #1 mov r0, #0 @1 : ands r2,
                         r1,
                         r3,
                         LSL r0 // Count zero of high bit
                             addeq r0,
                         r0,
                         #1 beq @1

                         str r2,
                         [ r12, #REG_IF_ADDR - REG_IE_ADDR ]
#endif // defined(SDK_ARM9) && !defined(SDK_CWBUG_PROC_OPT)

#ifdef SDK_DEBUG
      cmp r0,
      #OS_IRQ_TABLE_MAX @2
      : bge @2 // Error Trap
#endif // SDK_DEBUG
        ldr r1,
        = OS_IRQTable ldr r0,
        [ r1, r0, LSL #2 ]

        ldr lr,
        = OS_IrqHandler_ThreadSwitch
            bx r0 // Set return address for thread rescheduling
}

asm void OS_IrqHandler_ThreadSwitch(void) {

  ldr r12, = OSi_IrqThreadQueue mov r3,
      #0 // Avoid stall
      ldr r12,
      [ r12, #OSThreadQueue.head ] // r12 = OSi_IrqThreadQueue.head
      mov r2,
      #OS_THREAD_STATE_READY // Avoid stall
      cmp r12,
      #0

      beq @thread_switch // if r12 == 0 exit

      @1 : str r2,
      [ r12, #OSThread.state ] str r3,
      [ r12, #OSThread.queue ] str r3,
      [ r12, #OSThread.link.prev ] ldr r0,
      [ r12, #OSThread.link.next ] str r3,
      [ r12, #OSThread.link.next ] mov r12,
      r0

               cmp r12,
      #0 bne @1

           ldr r12,
      = OSi_IrqThreadQueue str r3,
      [ r12, #OSThreadQueue.head ] // Clear OSi_IrqThreadQueue.head
           str r3,
      [ r12, #OSThreadQueue.tail ] // Clear OSi_IrqThreadQueue.tail

           ldr r12,
      = OSi_ThreadInfo // Need to do scheduling
               mov r1,
      #1 strh r1,
      [ r12, #OS_THREADINFO_OFFSET_ISNEEDRESCHEDULING ]

           @thread_switch :

      ldr r12,
      = OSi_ThreadInfo ldrh r1,
      [ r12, #OS_THREADINFO_OFFSET_ISNEEDRESCHEDULING ] cmp r1,
      #0 ldreq pc,
      [sp],
      #4 // Return if OSi_IsNeedResceduling == 0

      mov r1,
      #0 strh r1,
      [ r12, #OS_THREADINFO_OFFSET_ISNEEDRESCHEDULING ]

      mov r3,
      #HW_PSR_IRQ_MODE | HW_PSR_FIQ_DISABLE | HW_PSR_IRQ_DISABLE |
          HW_PSR_ARM_STATE msr cpsr_c,
      r3

          add r2,
      r12,
      #OS_THREADINFO_OFFSET_LIST // r2 = &OSi_ThreadInfo.list
      ldr r1,
      [r2] // r1 = *r2 = TopOfList
      @11 : cmp r1,
      #0 ldrneh r0,
      [ r1, #OS_THREAD_OFFSET_STATE ] // r0 = t->state
            cmpne r0,
      #OS_THREAD_STATE_READY ldrne r1,
      [ r1, #OS_THREAD_OFFSET_NEXT ] bne @11

            cmp r1,
      #0 bne @12

            _dont_switched_ : mov r3,
      #HW_PSR_IRQ_MODE | HW_PSR_IRQ_DISABLE | HW_PSR_ARM_STATE msr cpsr_c,
      r3

                                  ldr pc,
      [sp],
      #4 // Return to irq master handler

                              @12 :

      ldr r0,
      [ r12, #OS_THREADINFO_OFFSET_CURRENT ] cmp r1,
      r0 beq _dont_switched_ // Return if no thread switching

          ldr r3,
      [ r12, #OS_THREADINFO_OFFSET_SWITCHCALLBACK ] cmp r3,
      #0 beq @13 // Skip calling callback when callback == 0
      stmfd sp !,
      {r0, r1, r12} mov lr,
      pc bx r3 ldmfd sp !,
  {
    r0, r1, r12
  }

  @13 :

      str r1,
      [ r12, #OS_THREADINFO_OFFSET_CURRENT ]

      mrs r2,
      SPSR str r2,
      [ r0, #OS_THREAD_OFFSET_CONTEXT ] ! // *r0=context:CPSR

#if defined(SDK_ARM9)

      stmfd sp !,
      {r0, r1} add r0,
      r0,
      #OS_THREAD_OFFSET_CONTEXT add r0,
      r0,
      #OS_CONTEXT_CP_CONTEXT ldr r1,
      = CP_SaveContext blx r1 ldmfd sp !,
      {r0, r1}
#endif

      ldmib sp !,
      {r2, r3} // Get R0,R1    // *sp=stack:R1
      stmib r0 !,
      {r2, r3} // Put R0,R1    // *r0=context:R1

      ldmib sp !,
      {r2, r3, r12, r14} // Get R2,R3,R12,LR // *sp=stack:LR
      stmib r0,
      {r2 - r14} ^ // Put R2-R14^  // *r0=context:R14
          add r0,
      r0,
      #0x34 // 0x34 is size of r2-r14(13 resisters)
      stmib r0 !,
      {r14} // Put R14_irq  // *r0=context:R15+4
#ifdef SDK_CONTEXT_HAS_SP_SVC
      mov r3,
      #HW_PSR_SVC_MODE | HW_PSR_FIQ_DISABLE | HW_PSR_IRQ_DISABLE |
          HW_PSR_ARM_STATE msr cpsr_c,
      r3 stmib r0 !,
      {sp}
#endif

#if defined(SDK_ARM9)

      stmfd sp !,
      {r1} add r0,
      r1,
      #OS_THREAD_OFFSET_CONTEXT add r0,
      r0,
      #OS_CONTEXT_CP_CONTEXT ldr r1,
      = CPi_RestoreContext blx r1

          ldmfd sp !,
      {r1}

#endif // If defined(SDK_ARM9)

#ifdef SDK_CONTEXT_HAS_SP_SVC
      ldr sp,
      [ r1, #OS_THREAD_OFFSET_CONTEXT + OS_CONTEXT_SP_SVC ] mov r3,
      #HW_PSR_IRQ_MODE | HW_PSR_FIQ_DISABLE | HW_PSR_IRQ_DISABLE |
          HW_PSR_ARM_STATE msr cpsr_c,
      r3
#endif

          ldr r2,
      [ r1, #OS_THREAD_OFFSET_CONTEXT ] ! // *r1=context:CPSR
      msr SPSR,
      r2 // Put SPSR

          ldr r14,
      [r1, #OS_CONTEXT_PC_PLUS4 - OS_CONTEXT_CPSR] // Get R15
      ldmib r1,
  { r0 - r14 } ^ // Get R0-R14^  // *r1=over written
          nop stmda sp !,
      {r0 - r3, r12, r14} // Put R0-R3,R12,LR / *sp=stack:LR

      ldmfd sp !,
  {
    pc
  } // Return to irq master handler
}

#ifdef SDK_ARM9
#include <nitro/itcm_end.h>
#endif
#endif

void OS_WaitIrq(BOOL clear, OSIrqMask irqFlags) {
  OSIntrMode enabled = OS_DisableInterrupts();

  if (clear) {
    (void)OS_ClearIrqCheckFlag(irqFlags);
  }

  while (!(OS_GetIrqCheckFlag() & irqFlags)) {
    OS_SleepThread(&OSi_IrqThreadQueue);
  }

  (void)OS_RestoreInterrupts(enabled);
}

#if defined(SDK_TWL) && defined(SDK_ARM7)
void OS_WaitIrqEx(BOOL clear, OSIrqMask irqFlags) {
  OSIntrMode enabled = OS_DisableInterrupts();

  if (clear) {
    (void)OS_ClearIrqCheckFlagEx(irqFlags);
  }

  while (!(OS_GetIrqCheckFlagEx() & irqFlags)) {
    OS_SleepThread(&OSi_IrqThreadQueue);
  }

  (void)OS_RestoreInterrupts(enabled);
}
#endif // defined(SDK_TWL) && defined(SDK_ARM7)

void OS_WaitAnyIrq(void) { OS_SleepThread(&OSi_IrqThreadQueue); }
#if defined(SDK_TWL) && defined(SDK_ARM7)
inline void OS_WaitAnyIrqEx(void) { OS_WaitAnyIrq(); }
#endif // defined(SDK_TWL) && defined(SDK_ARM7)
