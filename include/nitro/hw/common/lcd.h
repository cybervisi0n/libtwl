#ifndef NITRO_HW_COMMON_LCD_H_
#define NITRO_HW_COMMON_LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

#define HW_LCD_WIDTH 256 // The number of dots of the display screen width
#define HW_LCD_HBLANK 99 // The number of dots in the H-blank interval
#define HW_LCD_COLUMNS (HW_LCD_WIDTH + HW_LCD_HBLANK)

#define HW_LCD_HEIGHT 192 // The number of lines in the display screen height
#define HW_LCD_VBLANK 71  // The number of lines in the V-blank interval
#define HW_LCD_LINES (HW_LCD_HEIGHT + HW_LCD_VBLANK)

#define HW_LCD_CLOCK 33513982  // 33.513982 MHz
#define HW_LCD_CLOCK_PER_DOT 6 // 6 frequency divisions for LCD clock

#define HW_LCD_H_CYCLE_NS                                                      \
  ((u32)(1000000000ULL * HW_LCD_COLUMNS * HW_LCD_CLOCK_PER_DOT / HW_LCD_CLOCK))
#define HW_LCD_H_CYCLE_US                                                      \
  ((u32)(1000000ULL * HW_LCD_COLUMNS * HW_LCD_CLOCK_PER_DOT / HW_LCD_CLOCK))
#define HW_LCD_H_CYCLE_MS           ((u32)(       1000ULL * HW_LCD_COLUMNS * HW_LCD_CLOCK_PER_DOT / HW_LCD_CLOCK )

#define HW_LCD_V_CYCLE_NS                                                      \
  ((u32)(1000000000ULL * HW_LCD_LINES * HW_LCD_COLUMNS *                       \
         HW_LCD_CLOCK_PER_DOT / HW_LCD_CLOCK))
#define HW_LCD_V_CYCLE_US                                                      \
  ((u32)(1000000ULL * HW_LCD_LINES * HW_LCD_COLUMNS * HW_LCD_CLOCK_PER_DOT /   \
         HW_LCD_CLOCK))
#define HW_LCD_V_CYCLE_MS                                                      \
  ((u32)(1000ULL * HW_LCD_LINES * HW_LCD_COLUMNS * HW_LCD_CLOCK_PER_DOT /      \
         HW_LCD_CLOCK))

#define HW_LCD_LINES_CYCLE_NS(n)                                               \
  ((u32)(1000000000ULL * (n) * HW_LCD_COLUMNS * HW_LCD_CLOCK_PER_DOT /         \
         HW_LCD_CLOCK))
#define HW_LCD_LINES_CYCLE_US(n)                                               \
  ((u32)(1000000ULL * (n) * HW_LCD_COLUMNS * HW_LCD_CLOCK_PER_DOT /            \
         HW_LCD_CLOCK))
#define HW_LCD_LINES_CYCLE_MS(n)                                               \
  ((u32)(1000ULL * (n) * HW_LCD_COLUMNS * HW_LCD_CLOCK_PER_DOT / HW_LCD_CLOCK))

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_HW_COMMON_LCD_H_ */
