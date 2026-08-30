#ifndef NITRO_HW_MMAP_TCM_H_
#define NITRO_HW_MMAP_TCM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define HW_SYS_AND_IRQ_STACK_SIZE_MAX                                          \
  (HW_DTCM_SIZE - HW_SVC_STACK_SIZE - HW_DTCM_SYSRV_SIZE)
#define HW_SVC_STACK_SIZE 0x40

#define HW_DTCM_SYSRV_SIZE 0x40

#ifndef SDK_ASM
#include <nitro/types.h>

typedef volatile struct {

  u8 sys_and_irq_stack[HW_SYS_AND_IRQ_STACK_SIZE_MAX]; // 0000-3f80 system & irq
                                                       // stack
  u8 svc_stack[HW_SVC_STACK_SIZE];                     // 3f80-3fbf svc stack

  u8 reserved[HW_DTCM_SYSRV_SIZE - 8]; // 3fc0-3ff7 ????
  u32 intr_check;                      // 3ff8-3ffb intr_check for svc
  void *intr_vector;                   // 3ffc-3fff intr handler

} OS_DTCM;
#endif

#define HW_DTCM_SYS_STACK_DEFAULT (HW_DTCM)

static u8 s_HW_DTCM_IRQ_STACK[2048];
#define HW_DTCM_IRQ_STACK_END (s_HW_DTCM_IRQ_STACK + 2048)

static u8 s_HW_DTCM_SVC_STACK[HW_SVC_STACK_SIZE];
#define HW_DTCM_SVC_STACK s_HW_DTCM_SVC_STACK

#define HW_DTCM_SVC_STACK_END (s_HW_DTCM_SVC_STACK + HW_SVC_STACK_SIZE)

#define HW_DTCM_SYSRV_OFS_DEBUGGER 0x00
#define HW_DTCM_SYSRV_OFS_EXCP_VECTOR 0x1c
#define HW_DTCM_SYSRV_OFS_RESERVED 0x20
#define HW_DTCM_SYSRV_OFS_INTR_CHECK 0x38
#define HW_DTCM_SYSRV_OFS_INTR_VECTOR 0x3c

#define HW_DTCM_SYSRV (HW_DTCM + 0x00003fc0)

extern u32 s_HW_EXCP_VECTOR_BUF;
#define HW_EXCP_VECTOR_BUF ((u64) & s_HW_EXCP_VECTOR_BUF)

extern u32 s_HW_INTR_CHECK_BUF;
#define HW_INTR_CHECK_BUF ((u64) & s_HW_INTR_CHECK_BUF)
#define HW_INTR_VECTOR_BUF (HW_DTCM_SYSRV + HW_DTCM_SYSRV_OFS_INTR_VECTOR)

#ifdef SDK_CWBUG_VSNPRINTF

#define HW_DTCM_STRING_SIZE 256
#define HW_DTCM_STRING HW_DTCM
#endif

#define HW_DTCM_ARENA_LO_DEFAULT HW_DTCM
#define HW_DTCM_ARENA_HI_DEFAULT HW_DTCM

#define HW_ITCM_ARENA_LO_DEFAULT HW_ITCM
#define HW_ITCM_ARENA_HI_DEFAULT HW_ITCM_END

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
