#ifndef NITRO_RTC_COMMON_TYPE_H_
#define NITRO_RTC_COMMON_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>

#define RTC_INTERRUPT_MODE_NONE 0x0  // No interrupts
#define RTC_INTERRUPT_MODE_32kHz 0x8 // 32 kHz standing wave output
#define RTC_INTERRUPT_MODE_PULSE 0x1 // Selected-frequency steady interrupt
#define RTC_INTERRUPT_MODE_MINUTE_EDGE 0x2 // Once-a-minute edge interrupt
#define RTC_INTERRUPT_MODE_MINUTE_PULSE1                                       \
  0x3 // Once-a-minute steady interrupt 1 (50% duty)
#define RTC_INTERRUPT_MODE_ALARM 0x4         // Alarm interrupt
#define RTC_INTERRUPT_MODE_MINUTE_PULSE2 0x7 // Once-a-minute steady interrupt 2

#define RTC_INTERRUPT_MASK_32kHz                                               \
  0x8 // Bit mask meaning current 32 kHz standing wave output is being used
#define RTC_INTERRUPT_MASK_PULSE                                               \
  0xb // Bit mask meaning selected-frequency steady interrupt is being used
#define RTC_INTERRUPT_MASK_MINUTE_EDGE                                         \
  0xb // Bit mask meaning once-a-minute edge interrupt is being used

#define RTC_PULSE_DUTY_1HZ 0x01
#define RTC_PULSE_DUTY_2HZ 0x02
#define RTC_PULSE_DUTY_4HZ 0x04
#define RTC_PULSE_DUTY_8HZ 0x08
#define RTC_PULSE_DUTY_16HZ 0x10

typedef struct RTCRawDate {
  u32 year : 8;  // Year ( 00 - 99 )
  u32 month : 5; // Month ( 01 - 12 )
  u32 dummy0 : 3;
  u32 day : 6; // Day ( 01 - 31 ) Month / Upper limit will change with leap year
  u32 dummy1 : 2;
  u32 week : 3; // Day of week ( 00 - 06 )
  u32 dummy2 : 5;

} RTCRawDate;

typedef struct RTCRawTime {
  u32 hour : 6;      // Hour ( 00 - 23 or 00 - 11 )
  u32 afternoon : 1; // P.M. flag in the case of 12-hour notation
  u32 dummy0 : 1;
  u32 minute : 7; // Minutes ( 00 - 59 )
  u32 dummy1 : 1;
  u32 second : 7; // Second ( 00 - 59 )
  u32 dummy2 : 9;

} RTCRawTime;

typedef struct RTCRawStatus1 {
  u16 reset : 1;  // W:   Reset flag
  u16 format : 1; // Flag for R/W time notation (0: 12-hour notation, 1: 24-hour
                  // notation)
  u16 dummy0 : 2;
  u16 intr1 : 1; // R:   Interrupt 1 generated flag (low will be output on the
                 // interrupt signal line)
  u16 intr2 : 1; // R:   Interrupt 2 generated flag (low will be output on the
                 // interrupt signal line)
  u16 bld : 1;   // R:   Power source voltage drop detection flag
  u16 poc : 1;   // R:   Power source turned on detection flag
  u16 dummy1 : 8;

} RTCRawStatus1;

typedef struct RTCRawStatus2 {
  u16 intr_mode : 4; // R/W interrupt 1 selection ( RTC_INTERRUPT_MODE_* )
  u16 dummy0 : 2;
  u16 intr2_mode : 1; // R/W interrupt 2 enable flag
  u16 test : 1;       // R/W test mode flag
  u16 dummy1 : 8;

} RTCRawStatus2;

typedef struct RTCRawAlarm {
  u32 week : 3; // Day of week ( 00 - 06 )
  u32 dummy0 : 4;
  u32 we : 1; // Day of week setting enable flag
  u32 hour
      : 6; // Time ( 00 - 23 or 00 - 11 ). Hour notation depends on status 1.
  u32 afternoon : 1; // P.M. flag in the case of 12-hour notation
  u32 he : 1;        // Hour setting enable flag
  u32 minute : 7;    // Minutes ( 00 - 59 )
  u32 me : 1;        // Minute setting enable flag
  u32 dummy2 : 8;

} RTCRawAlarm;

typedef struct RTCRawPulse {
  u32 pulse : 5; // Frequency duty flag ( RTC_PULSE_DUTY_* )
  u32 dummy : 27;

} RTCRawPulse;

typedef struct RTCRawAdjust {
  u32 adjust
      : 8; // Clock adjustment register ( see other documents for settings )
  u32 dummy : 24;

} RTCRawAdjust;

typedef struct RTCRawFree {
  u32 free : 8; // Free register
  u32 dummy : 24;

} RTCRawFree;

typedef union RTCRawData {
  struct {
    RTCRawDate date; // Date
    RTCRawTime time; // Time
  } t;

  struct {
    RTCRawStatus1 status1; // Status 1 register
    RTCRawStatus2 status2; // Status 2 register
    union {
      RTCRawPulse pulse;   // Register setting for frequency periodic interrupt
      RTCRawAlarm alarm;   // Register setting for alarm ( 1 or 2 )
      RTCRawAdjust adjust; // Register setting for clock adjustment
      RTCRawFree free;     // Free register setting
    };
  } a;

  u32 words[2]; // For 4-byte access

  u16 halfs[4]; // For 2-byte access

  u8 bytes[8]; // For byte access

} RTCRawData;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_RTC_COMMON_TYPE_H_ */
