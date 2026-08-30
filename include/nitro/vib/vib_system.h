#ifndef PULSE_VIB_H
#define PULSE_VIB_H

#include <nitro.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VIB_PULSE_NUM_MAX 6

#define VIB_ON_TIME_MAX 15

#define VIB_REST_TIME_MIN 15

typedef struct {
  u32 pulse_num; /*! How many pulses to generate in a single pulse set. This
                    must be at least 1 and no more than VIB_PULSE_NUM_MAX. */
  u32 rest_time; /*! Length of pause during pulse-set period. 1=0.1 millisecond.
                  */
  u32 on_time[VIB_PULSE_NUM_MAX]; /*! Length of the activation time. Set a value
                                     larger than 0. 1=0.1 millisecond. */
  u32 off_time[VIB_PULSE_NUM_MAX]; /*! Length of the stop time. Set a value
                                      larger than 0. 1=0.1 millisecond. */
  u32 repeat_num; /*! Number of times to repeat pulse set. When 0, repeats
                     endlessly. */
} VIBPulseState;

typedef void (*VIBCartridgePulloutCallback)(void);

extern BOOL VIB_Init(void);

extern void VIB_End(void);

extern void VIB_StartPulse(const VIBPulseState *state);

extern void VIB_StopPulse(void);

extern BOOL VIB_IsExecuting(void);

extern void VIB_SetCartridgePulloutCallback(VIBCartridgePulloutCallback func);

extern BOOL VIB_IsCartridgeEnabled(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PULSE_VIB_H */
