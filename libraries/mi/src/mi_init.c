#include <nitro.h>

#ifdef SDK_TWL
#include <twl/mi/common/dma.h>
#endif

void MI_Init(void) {
#ifdef SDK_ARM9

  MI_SetWramBank(MI_WRAM_ARM7_ALL);
#endif

  if (OS_IsRunOnTwl()) {
#ifdef SDK_TWL
    MI_InitNDma();
#endif
  }

  MI_StopDma(0);
}
