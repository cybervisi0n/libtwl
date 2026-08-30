#include <nitro/math.h>

#if defined(SDK_ARM9) &&                                                       \
    (defined(SDK_CW) || defined(SDK_RX) || defined(__MWERKS__))

#pragma thumb off
u32 MATH_CountLeadingZerosFunc(u32 x) {

  asm
  {
    clz     x, x}
  return x;
}
#pragma thumb reset

#else // !ARM9 || !(CW || __MWERKS__)

u32 MATH_CountLeadingZerosFunc(u32 x) {
  u32 y;
  u32 n = 32;

  y = x >> 16;
  if (y != 0) {
    n -= 16;
    x = y;
  }
  y = x >> 8;
  if (y != 0) {
    n -= 8;
    x = y;
  }
  y = x >> 4;
  if (y != 0) {
    n -= 4;
    x = y;
  }
  y = x >> 2;
  if (y != 0) {
    n -= 2;
    x = y;
  }
  y = x >> 1;
  if (y != 0) {
    n -= 2;
  } // x == 0b10 or 0b11 -> n -= 2
  else {
    n -= x;
  } // x == 0b00 or 0b01 -> n -= x

  return n;
}

#endif // ARM9 && (CW || __MWERKS__)

u8 MATH_CountPopulation(u32 x) {

  x -= ((x >> 1) & 0x55555555);

  x = (x & 0x33333333) + ((x >> 2) & 0x33333333);

  x += (x >> 4);

  x &= 0x0f0f0f0f;

  x += (x >> 8);

  x += (x >> 16);

  return (u8)x;
}

#if 0

#include <nitro/code32.h>
asm u32 MATH_CountPopulation_Asm(u32 value)
{
    mov     r1 ,    #0
    mov     r2 ,    #1

@loop:
    clz     r3 ,    r0
    rsbs    r3 ,    r3 ,    #31
    bcc     @end
    add     r1 ,    r1 ,    #1
    mvn     r3 ,    r2 ,    LSL r3
    and     r0 ,    r0 ,    r3
    b       @loop

@end:
    mov     r0 ,    r1
    bx      lr
}
#include <nitro/codereset.h>
#endif
