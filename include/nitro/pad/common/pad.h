#ifndef NITRO_PAD_PAD_H_
#define NITRO_PAD_PAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>
#ifndef SDK_TWL
#include <nitro/hw/common/mmap_shared.h>
#else
#ifdef SDK_PORT
#include <twl/hw/X86/mmap_global.h>
#else
#ifdef SDK_ARM9
#include <twl/hw/ARM9/mmap_global.h>
#else // SDK_ARM7
#include <twl/hw/ARM7/mmap_global.h>
#endif
#endif
#include <twl/hw/common/mmap_shared.h>
#endif
#ifdef SDK_PORT
#include <nitro/hw/X86/ioreg_PAD.h>
#else
#ifdef SDK_ARM9
#include <nitro/hw/ARM9/ioreg_PAD.h>
#else // SDK_ARM7
#include <nitro/hw/ARM7/ioreg_PAD.h>
#endif
#endif

#define PAD_PLUS_KEY_MASK 0x00f0     // mask : cross keys
#define PAD_BUTTON_MASK 0x2f0f       // mask : buttons
#define PAD_DEBUG_BUTTON_MASK 0x2000 // mask : debug button
#define PAD_ALL_MASK 0x2fff          // mask : all buttons
#define PAD_RCNTPORT_MASK                                                      \
  0x2c00 // mask : factors ARM7 can read from RCNT register
#define PAD_KEYPORT_MASK                                                       \
  0x03ff // mask : factors ARM7/9 can read from KEY register

#define PAD_DETECT_FOLD_MASK 0x8000 // mask : folding

#define PAD_BUTTON_A 0x0001      // A
#define PAD_BUTTON_B 0x0002      // B
#define PAD_BUTTON_SELECT 0x0004 // SELECT
#define PAD_BUTTON_START 0x0008  // START
#define PAD_KEY_RIGHT 0x0010     // RIGHT of cross key
#define PAD_KEY_LEFT 0x0020      // LEFT  of cross key
#define PAD_KEY_UP 0x0040        // UP    of cross key
#define PAD_KEY_DOWN 0x0080      // DOWN  of cross key
#define PAD_BUTTON_R 0x0100      // R
#define PAD_BUTTON_L 0x0200      // L
#define PAD_BUTTON_X 0x0400      // X
#define PAD_BUTTON_Y 0x0800      // Y
#define PAD_BUTTON_DEBUG 0x2000  // Debug button

static inline u16 PADi_ReadRaw(void);

static inline u16 PAD_Read(void) {
  u16 paddata = PADi_ReadRaw();
  return (u16)(paddata & ~((paddata & PAD_KEY_UP) << 1) &
               ~((paddata & PAD_KEY_LEFT) >> 1));
}

static inline u16 PAD_ReadNoFilter(void) { return PADi_ReadRaw(); }

static inline BOOL PAD_DetectFold(void) {
  return (BOOL)((*(vu16 *)HW_BUTTON_XY_BUF & PAD_DETECT_FOLD_MASK) >> 15);
}

typedef enum {

  PAD_LOGIC_OR =
      (0 << REG_PAD_KEYCNT_LOGIC_SHIFT), //   pushing ONE of keys you specified
  PAD_LOGIC_AND = (1 << REG_PAD_KEYCNT_LOGIC_SHIFT), //   pushing ALL of keys at
                                                     //   a time you specified

  PAD_OR_INTR = PAD_LOGIC_OR,
  PAD_AND_INTR = PAD_LOGIC_AND
} PADLogic;

static inline u16 PADi_ReadRaw(void) {
  return (u16)(((reg_PAD_KEYINPUT | *(vu16 *)HW_BUTTON_XY_BUF) ^
                (PAD_PLUS_KEY_MASK | PAD_BUTTON_MASK)) &
               (PAD_PLUS_KEY_MASK | PAD_BUTTON_MASK));
}

static inline void PADi_SetIrq(PADLogic logic, BOOL enable, u16 padMask) {
  SDK_ASSERT(!(padMask & (~PAD_KEYPORT_MASK))); // check X, Y, Debug button
  reg_PAD_KEYCNT = (u16)(((u16)logic | (enable ? REG_PAD_KEYCNT_INTR_MASK : 0) |
                          (PAD_KEYPORT_MASK & padMask)));
}

static inline void PADi_ClearIrq(void) { reg_PAD_KEYCNT = 0; }

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
