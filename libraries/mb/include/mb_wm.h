#ifndef _MB_WM_H_
#define _MB_WM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>
#include <nitro/mb.h>
#include <nitro/wm.h>

typedef void (*MBWMCallbackFunc)(u16 type, void *arg);

typedef struct {
  MBWMCallbackFunc mpCallback;
  u32 *sendBuf;
  u32 *recvBuf;
  BOOL start_mp_busy;
  u16 sendBufSize;  // Size of send buffer
  u16 recvBufSize;  // Size of receive buffer
  u16 pSendLen;     // Child receive size in one MP
  u16 pRecvLen;     // Parent receive size in one MP
  u16 blockSizeMax; //
  u16 mpStarted;    // Flag indicating that MP has started
  u16 endReq;       // End request
  u16 child_bitmap; // Child connection state
  u16 mpBusy;       // MP sending (busy) flag
  u8 _padding[2];
} MBWMWork;

void MBi_WMSetBuffer(void *buf);
void MBi_WMSetCallback(MBWMCallbackFunc callback);
void MBi_WMStartConnect(WMBssDesc *bssDesc);
void MBi_ChildStartMP(u16 *sendBuf, u16 *recvBuf);
WMErrCode MBi_MPSendToParent(u32 body_len, u16 pollbmp, u32 *sendbuf);
void MBi_WMDisconnect(void);
void MBi_WMReset(void);
void MBi_WMClearCallback(void);

#ifdef __cplusplus
}
#endif

#endif /*  _MB_WM_H_    */
