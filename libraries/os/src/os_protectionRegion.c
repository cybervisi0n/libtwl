#include <nitro/os.h>
#include <nitro/code32.h>

#ifdef SDK_PORT
void OS_EnableICacheForProtectionRegion(u32 flags) {}
void OS_DisableICacheForProtectionRegion(u32 flags) {}
u32 OS_GetICacheEnableFlagsForProtectionRegion(void) { return 0; }
void OS_SetIPermissionsForProtectionRegion(u32 setMask, u32 flags) {}
u32 OS_GetIPermissionsForProtectionRegion(void) { return 0; }
void OS_EnableDCacheForProtectionRegion(u32 flags) {}
void OS_DisableDCacheForProtectionRegion(u32 flags) {}
u32 OS_GetDCacheEnableFlagsForProtectionRegion(void) { return 0; }
void OS_SetDPermissionsForProtectionRegion(u32 setMask, u32 flags) {}
u32 OS_GetDPermissionsForProtectionRegion(void) { return 0; }
void OS_EnableWriteBufferForProtectionRegion(u32 flags) {}
void OS_DisableWriteBufferForProtectionRegion(u32 flags) {}
u32 OS_GetWriteBufferEnableFlagsForProtectionRegion(u32 flags) { return 0; }
#else
asm void OS_EnableICacheForProtectionRegion(register u32 flags) {
  mrc p15, 0, r1, c2, c0, 1 orr r1, r1, r0 mcr p15, 0, r1, c2, c0, 1 bx lr
}

asm void OS_DisableICacheForProtectionRegion(register u32 flags) {
  mrc p15, 0, r1, c2, c0, 1 bic r1, r1, r0 mcr p15, 0, r1, c2, c0, 1 bx lr
}

asm u32 OS_GetICacheEnableFlagsForProtectionRegion(void) {
  mrc p15, 0, r0, c2, c0, 1 bx lr
}

asm void OS_SetIPermissionsForProtectionRegion(register u32 setMask,
                                               register u32 flags) {
  mrc p15, 0, r2, c5, c0, 3 bic r2, r2, r0 orr r2, r2, r1 mcr p15, 0, r2, c5,
      c0, 3 bx lr
}

asm u32 OS_GetIPermissionsForProtectionRegion(void) {
  mrc p15, 0, r0, c5, c0, 3 bx lr
}

asm void OS_EnableDCacheForProtectionRegion(register u32 flags) {
  mrc p15, 0, r1, c2, c0, 0 orr r1, r1, r0 mcr p15, 0, r1, c2, c0, 0 bx lr
}

asm void OS_DisableDCacheForProtectionRegion(register u32 flags) {
  mrc p15, 0, r1, c2, c0, 0 bic r1, r1, r0 mcr p15, 0, r1, c2, c0, 0 bx lr
}

asm u32 OS_GetDCacheEnableFlagsForProtectionRegion(void) {
  mrc p15, 0, r0, c2, c0, 0 bx lr
}

asm void OS_SetDPermissionsForProtectionRegion(register u32 setMask,
                                               register u32 flags) {
  mrc p15, 0, r2, c5, c0, 2 bic r2, r2, r0 orr r2, r2, r1 mcr p15, 0, r2, c5,
      c0, 2 bx lr
}

asm u32 OS_GetDPermissionsForProtectionRegion(void) {
  mrc p15, 0, r0, c5, c0, 2 bx lr
}

asm void OS_EnableWriteBufferForProtectionRegion(register u32 flags) {
  mrc p15, 0, r1, c3, c0, 0 orr r1, r1, r0 mcr p15, 0, r1, c3, c0, 0 bx lr
}

asm void OS_DisableWriteBufferForProtectionRegion(register u32 flags) {
  mrc p15, 0, r1, c3, c0, 0 bic r1, r1, r0 mcr p15, 0, r1, c3, c0, 0 bx lr
}

asm u32 OS_GetWriteBufferEnableFlagsForProtectionRegion(register u32 flags) {
  mrc p15, 0, r0, c3, c0, 0 bx lr
}
#endif

void OS_SetProtectionRegionParam(int regionNo, u32 param) {
  static void (*f[])(u32) = {OS_SetProtectionRegion0, OS_SetProtectionRegion1,
                             OS_SetProtectionRegion2, OS_SetProtectionRegion3,
                             OS_SetProtectionRegion4, OS_SetProtectionRegion5,
                             OS_SetProtectionRegion6, OS_SetProtectionRegion7};
  SDK_ASSERT(regionNo >= 0 && regionNo < OS_PROTECTION_REGION_NUM);

  f[regionNo](param);
}

#ifdef SDK_PORT
void OS_SetProtectionRegion0(u32 param) {}

void OS_SetProtectionRegion1(u32 param) {}

void OS_SetProtectionRegion2(u32 param) {}

void OS_SetProtectionRegion3(u32 param) {}

void OS_SetProtectionRegion4(u32 param) {}

void OS_SetProtectionRegion5(u32 param) {}

void OS_SetProtectionRegion6(u32 param) {}

void OS_SetProtectionRegion7(u32 param) {}
#else
asm void OS_SetProtectionRegion0(u32 param) { mcr p15, 0, r0, c6, c0, 0 bx lr }

asm void OS_SetProtectionRegion1(u32 param) { mcr p15, 0, r0, c6, c1, 0 bx lr }

asm void OS_SetProtectionRegion2(u32 param) { mcr p15, 0, r0, c6, c2, 0 bx lr }

asm void OS_SetProtectionRegion3(u32 param) { mcr p15, 0, r0, c6, c3, 0 bx lr }

asm void OS_SetProtectionRegion4(u32 param) { mcr p15, 0, r0, c6, c4, 0 bx lr }

asm void OS_SetProtectionRegion5(u32 param) { mcr p15, 0, r0, c6, c5, 0 bx lr }

asm void OS_SetProtectionRegion6(u32 param) { mcr p15, 0, r0, c6, c6, 0 bx lr }

asm void OS_SetProtectionRegion7(u32 param){mcr p15, 0, r0, c6, c7, 0 bx lr}
#endif

u32 OS_GetProtectionRegionParam(int regionNo) {
  static u32 (*f[])(void) = {OS_GetProtectionRegion0, OS_GetProtectionRegion1,
                             OS_GetProtectionRegion2, OS_GetProtectionRegion3,
                             OS_GetProtectionRegion4, OS_GetProtectionRegion5,
                             OS_GetProtectionRegion6, OS_GetProtectionRegion7};
  SDK_ASSERT(regionNo >= 0 && regionNo < OS_PROTECTION_REGION_NUM);

  return f[regionNo]();
}

#ifdef SDK_PORT
u32 OS_GetProtectionRegion0(void) { return 0; }

u32 OS_GetProtectionRegion1(void) { return 0; }

u32 OS_GetProtectionRegion2(void) { return 0; }

u32 OS_GetProtectionRegion3(void) { return 0; }

u32 OS_GetProtectionRegion4(void) { return 0; }

u32 OS_GetProtectionRegion5(void) { return 0; }

u32 OS_GetProtectionRegion6(void) { return 0; }

u32 OS_GetProtectionRegion7(void) { return 0; }
#else
asm u32 OS_GetProtectionRegion0(void) { mrc p15, 0, r0, c6, c0, 0 bx lr }

asm u32 OS_GetProtectionRegion1(void) { mrc p15, 0, r0, c6, c1, 0 bx lr }

asm u32 OS_GetProtectionRegion2(void) { mrc p15, 0, r0, c6, c2, 0 bx lr }

asm u32 OS_GetProtectionRegion3(void) { mrc p15, 0, r0, c6, c3, 0 bx lr }

asm u32 OS_GetProtectionRegion4(void) { mrc p15, 0, r0, c6, c4, 0 bx lr }

asm u32 OS_GetProtectionRegion5(void) { mrc p15, 0, r0, c6, c5, 0 bx lr }

asm u32 OS_GetProtectionRegion6(void) { mrc p15, 0, r0, c6, c6, 0 bx lr }

asm u32 OS_GetProtectionRegion7(void) { mrc p15, 0, r0, c6, c7, 0 bx lr }
#endif

void OS_SetProtectionRegionEx(int regionNo, u32 address, u32 sizeDef) {
  u32 mask = 0xfffff000 << OSi_PRDEF_TO_VAL(sizeDef);
  u32 param = OSi_CalcPRParam((address), sizeDef, mask) | HW_C6_PR_ENABLE;

  OS_SetProtectionRegionParam(regionNo, param);
}

#ifndef SDK_FINALROM
void OS_DumpProtectionRegion(void) {
  int n;
  OSIntrMode enable = OS_DisableInterrupts();
  u32 i_cache = OS_GetICacheEnableFlagsForProtectionRegion();
  u32 d_cache = OS_GetDCacheEnableFlagsForProtectionRegion();
  u32 i_permit = OS_GetIPermissionsForProtectionRegion();
  u32 d_permit = OS_GetDPermissionsForProtectionRegion();

  OS_TPrintf("----Protection Region\nPR ICache DCache IPermit DPermit  Address "
             "   Size\n");
  for (n = 0; n < OS_PROTECTION_REGION_NUM; n++) {
    char *dpermit;
    char *ipermit;

    switch ((i_permit >> (n << 2)) & HW_C5_PERMIT_MASK) {
    case HW_C5_PERMIT_NA:
      ipermit = "--";
      break;
    case HW_C5_PERMIT_RW:
      ipermit = "RW";
      break;
    case HW_C5_PERMIT_RO:
      ipermit = "R-";
      break;
    default:
      ipermit = "??";
      break;
    }
    switch ((d_permit >> (n << 2)) & HW_C5_PERMIT_MASK) {
    case HW_C5_PERMIT_NA:
      dpermit = "--";
      break;
    case HW_C5_PERMIT_RW:
      dpermit = "RW";
      break;
    case HW_C5_PERMIT_RO:
      dpermit = "R-";
      break;
    default:
      dpermit = "??";
      break;
    }

    OS_TPrintf(" %d   %c      %c      %s      %s    0x%08x 0x%x\n", n,
               (i_cache & (1 << n)) ? 'O' : '-',
               (d_cache & (1 << n)) ? 'O' : '-', ipermit, dpermit,
               OS_GetProtectionRegionAddress(n),
               0x1000 << OSi_PRDEF_TO_VAL(OS_GetProtectionRegionSize(n)),
               i_permit, d_permit);
  }

  (void)OS_RestoreInterrupts(enable);
}
#endif // ifndef SDK_FINALROM

#include <nitro/codereset.h>
