#include <nitro/types.h>
#include <nitro/mi/swap.h>

#include <nitro/code32.h>

#ifdef SDK_PORT
u32 MI_SwapWord(u32 setData, vu32 *destp) { return 0; }
#else
asm u32 MI_SwapWord(register u32 setData, register vu32 *destp){swp r0,
                                                                r0, [r1] bx lr}
#endif

#ifdef SDK_PORT
u8 MI_SwapByte(u32 setData, vu8 *destp) { return 0; }
#else
asm u8 MI_SwapByte(register u32 setData, register vu8 *destp) {
  swpb r0, r0, [r1] bx lr
}
#endif

#include <nitro/codereset.h>
