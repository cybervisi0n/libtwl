#include <nitro/code32.h>
#include "../include/gxasm.h"

#if defined(SDK_PORT)
void GX_SendFifo48B(const void *pSrc, void *pDest) { return; }

void GX_SendFifo64B(const void *pSrc, void *pDest) { return; }

void GX_SendFifo128B(const void *pSrc, void *pDest) { return; }
#else
asm void GX_SendFifo48B(register const void *pSrc, register void *pDest) {
  ldmia r0 !,
      {r2, r3, r12} // r0-r3, r12 need not be saved.
  stmia r1,
      {r2, r3, r12} ldmia r0 !, {r2, r3, r12} stmia r1,
      {r2, r3, r12} ldmia r0 !, {r2, r3, r12} stmia r1,
      {r2, r3, r12} ldmia r0 !, {r2, r3, r12} stmia r1,
      {r2, r3, r12}

  bx lr
}

asm void GX_SendFifo64B(register const void *pSrc, register void *pDest) {
  stmfd sp !,
      {r4 - r8} // r0-r3, r12 need not be saved.

  ldmia r0 !,
      {r2 - r8, r12} stmia r1, {r2 - r8, r12} ldmia r0 !,
      {r2 - r8, r12} stmia r1,
      {r2 - r8, r12}

  ldmfd sp !,
      {r4 - r8} bx lr
}

asm void GX_SendFifo128B(register const void *pSrc, register void *pDest) {
  stmfd sp !,
      {r4 - r8} // r0-r3, r12 need not be saved.

  ldmia r0 !,
      {r2 - r8, r12} stmia r1, {r2 - r8, r12} ldmia r0 !,
      {r2 - r8, r12} stmia r1, {r2 - r8, r12} ldmia r0 !,
      {r2 - r8, r12} stmia r1, {r2 - r8, r12} ldmia r0 !,
      {r2 - r8, r12} stmia r1,
      {r2 - r8, r12}

  ldmfd sp !,
      {r4 - r8} bx lr
}
#endif
