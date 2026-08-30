#ifndef EXT_KEYCONTROL_H_
#define EXT_KEYCONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  u16 key;   // Key status
  u16 count; // The time period to maintain key status. 0 will be considered a
             // termination.
} EXTKeys;

void EXT_AutoKeys(const EXTKeys *sequence, u16 *cont, u16 *trig);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* EXT_KEYCONTROL_H_ */
