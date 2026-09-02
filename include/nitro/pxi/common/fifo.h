#ifndef NITRO_PXI_COMMON_FIFO_H_
#define NITRO_PXI_COMMON_FIFO_H_

#include <nitro/types.h>
#include <nitro/memorymap.h>
#include <nitro/pxi/common/regname.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  PXI_FIFO_TAG_EX = 0,     // Extension format
  PXI_FIFO_TAG_USER_0,     // for application programmer, use it in free
  PXI_FIFO_TAG_USER_1,     // for application programmer, use it in free
  PXI_FIFO_TAG_SYSTEM,     // SDK inner usage
  PXI_FIFO_TAG_NVRAM,      // NVRAM
  PXI_FIFO_TAG_RTC,        // RTC
  PXI_FIFO_TAG_TOUCHPANEL, // Touch Panel
  PXI_FIFO_TAG_SOUND,      // Sound
  PXI_FIFO_TAG_PM,         // Power Management
  PXI_FIFO_TAG_MIC,        // Microphone
  PXI_FIFO_TAG_WM,         // Wireless Manager
  PXI_FIFO_TAG_FS,         // File System
  PXI_FIFO_TAG_OS,         // OS
  PXI_FIFO_TAG_CTRDG,      // Cartridge
  PXI_FIFO_TAG_CARD,       // Card
  PXI_FIFO_TAG_WVR,        // Control driving wireless library
  PXI_FIFO_TAG_CTRDG_Ex,   // Cartridge Ex
  PXI_FIFO_TAG_CTRDG_PHI,  // Cartridge PHI
#ifdef SDK_TWL
  PXI_FIFO_TAG_MI,     // MI
  PXI_FIFO_TAG_AES,    // AES
  PXI_FIFO_TAG_FATFS,  // FATFS
  PXI_FIFO_TAG_CAMERA, // CAMERA
  PXI_FIFO_TAG_WMW,    // WM
  PXI_FIFO_TAG_SCFG,   // SCFG
  PXI_FIFO_TAG_SNDEX,  // SNDEX

  PXI_FIFO_TAG_SEA = 31, // SEA
#endif

  PXI_MAX_FIFO_TAG = 32 // MAX FIFO TAG
} PXIFifoTag;

#define PXI_FIFO_DEVICE_TEST PXI_FIFO_TAG_USR_0
#define PXI_FIFO_DEVICE_FLASH PXI_FIFO_TAG_NVRAM
#define PXI_FIFO_DEVICE_RTC PXI_FIFO_TAG_RTC
#define PXI_FIFO_DEVICE_TOUCHPANEL PXI_FIFO_TAG_TOUCHPANEL
#define PXI_MAX_DEVICES PXI_MAX_FIFO_TAG

typedef enum {
  PXI_FIFO_SUCCESS = 0,
  PXI_FIFO_FAIL_SEND_ERR = -1,
  PXI_FIFO_FAIL_SEND_FULL = -2,
  PXI_FIFO_FAIL_RECV_ERR = -3,
  PXI_FIFO_FAIL_RECV_EMPTY = -4,
  PXI_FIFO_NO_CALLBACK_ENTRY = -5
} PXIFifoStatus;

#define PXI_FIFOMESSAGE_BITSZ_TAG 5
#define PXI_FIFOMESSAGE_BITSZ_ERR 1
#define PXI_FIFOMESSAGE_BITSZ_DATA 26
typedef union {
  struct {
    u32 tag : PXI_FIFOMESSAGE_BITSZ_TAG;
    u32 err : PXI_FIFOMESSAGE_BITSZ_ERR;
    u32 data : PXI_FIFOMESSAGE_BITSZ_DATA;
  } e;
  u32 raw;

} PXIFifoMessage;

#ifdef SDK_PORT
typedef struct {
    struct {
        u32     tag;
        u32     err;
        u64     data;
    } e;
} WIN_PXIFifoMessage;
#endif

#ifdef SDK_PORT
typedef void (*PXIFifoCallback) (PXIFifoTag tag, u64 data, BOOL err);
#else
typedef void (*PXIFifoCallback)(PXIFifoTag tag, u32 data, BOOL err);
#endif
typedef void (*PXIFifoEmtpyCallback)(void);

static inline BOOL PXI_IsFifoError(PXIFifoStatus status) {
  return PXI_FIFO_SUCCESS == status;
}

void PXI_InitFifo(void);

void PXI_SetFifoRecvCallback(int fifotag, PXIFifoCallback callback);
#ifndef SDK_BUILD_ARM
void PXI_SetFifoRecvCallback7(int fifotag, PXIFifoCallback callback);
#endif

BOOL PXI_IsCallbackReady(int fifotag, PXIProc proc);

static inline BOOL PXI_IsArm7CallbackReady(int fifotag) {
  return PXI_IsCallbackReady(fifotag, PXI_PROC_ARM7);
}

static inline BOOL PXI_IsArm9CallbackReady(int fifotag) {
  return PXI_IsCallbackReady(fifotag, PXI_PROC_ARM9);
}

void PXI_SetFifoSendCallback(PXIFifoEmtpyCallback callback);

#ifdef SDK_PORT
int PXI_SendWordByFifo(int fifotag, u64 data, BOOL err);
int PXI_SendWordByFifo7(int fifotag, u64 data, BOOL err);
PXIFifoCallback PXI_WIN_GetCallback(u32 tag);
#else
int PXI_SendWordByFifo(int fifotag, u32 data, BOOL err);
#endif

void PXIi_HandlerRecvFifoNotEmpty(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_PXI_COMMON_FIFO_H_ */
