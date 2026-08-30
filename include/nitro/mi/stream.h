#ifndef NITRO_MI_STREAM_H_
#define NITRO_MI_STREAM_H_

#include <nitro/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef s32 (*MIi_InitReadStreamCallback)(const u8 *devicep, void *ramp,
                                          const void *paramp);
typedef s32 (*MIi_TerminateReadStreamCallback)(const u8 *devicep);
typedef u8 (*MIi_ReadByteStreamCallback)(const u8 *devicep);
typedef u16 (*MIi_ReadShortStreamCallback)(const u8 *devicep);
typedef u32 (*MIi_ReadWordStreamCallback)(const u8 *devicep);

typedef struct {
  MIi_InitReadStreamCallback initStream;           //---- initialization
  MIi_TerminateReadStreamCallback terminateStream; //---- termination
  MIi_ReadByteStreamCallback readByteStream;       //---- reading byte stream
  MIi_ReadShortStreamCallback readShortStream; //---- reading half word stream
  MIi_ReadWordStreamCallback readWordStream;   //---- reading word stream
} MIReadStreamCallbacks;

MIReadStreamCallbacks *MI_GetReadStreamFromMemoryCallbacks(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
