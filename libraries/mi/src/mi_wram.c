#include <nitro/mi/wram.h>
#include <nitro/misc.h>

#define MIi_WRAM_ASSERT(x)                                                     \
  SDK_ASSERT((x) == MI_WRAM_ARM9_ALL || (x) == MI_WRAM_1616_1 ||               \
             (x) == MI_WRAM_1616_2 || (x) == MI_WRAM_ARM7_ALL)

#if defined(SDK_ARM9) || defined(SDK_PORT)

#ifdef SDK_CW_WARNOFF_SAFESTRB
#include <nitro/code32.h>
#endif
void MI_SetWramBank(MIWram cnt) {
  MIi_WRAM_ASSERT(cnt);
  reg_GX_VRAMCNT_WRAM = (u8)cnt; // safe byte access
}

#ifdef SDK_CW_WARNOFF_SAFESTRB
#include <nitro/codereset.h>
#endif

#endif // SDK_ARM9

MIWram MI_GetWramBank(void) {

#if defined(SDK_ARM9) || defined(SDK_PORT)
  return (MIWram)(reg_GX_VRAMCNT_WRAM & MI_WRAM_ARM9_ALL); // safe byte access
#else
  return (MIWram)(reg_GX_WVRAMSTAT & MI_WRAM_ARM9_ALL);
#endif
}
