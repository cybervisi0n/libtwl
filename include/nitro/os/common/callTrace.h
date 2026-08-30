#ifndef NITRO_OS_CALLTRACE_H_
#define NITRO_OS_CALLTRACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/os/common/profile.h>

#define OS_CALLTRACE_CHECK_OVERSTACK

#define OS_CALLTRACE_RECORD_R0
#define OS_CALLTRACE_RECORD_R1
#define OS_CALLTRACE_RECORD_R2
#define OS_CALLTRACE_RECORD_R3

#define OS_CALLTRACE_LEVEL_AVAILABLE

typedef struct {
  u32 name;          // pointer to function name
  u32 returnAddress; // value of lr register

#ifdef OS_CALLTRACE_LEVEL_AVAILABLE
  u32 level; // call level
#endif

#ifdef OS_CALLTRACE_RECORD_R0
  u32 r0; // value of r0 register
#endif
#ifdef OS_CALLTRACE_RECORD_R1
  u32 r1; // value of r1 register
#endif
#ifdef OS_CALLTRACE_RECORD_R2
  u32 r2; // value of r2 register
#endif
#ifdef OS_CALLTRACE_RECORD_R3
  u32 r3; // value of r3 register
#endif
} OSCallTrace;

#define OSi_SIZEOF_CALLTRACE sizeof(OSCallTrace)

typedef struct {
  OSCallTrace *current; // current pointer
  OSCallTrace *limit;   // buffer limit as hi-address
  u16 enable;
  u16 circular;
#ifdef OS_CALLTRACE_LEVEL_AVAILABLE
  u32 level; // call level
#endif
  OSCallTrace array[1]; // valiable length
} OSCallTraceInfo;

#define OSi_TRACEINFO_SIZEOF_HEADERPART                                        \
  (sizeof(OSCallTraceInfo) - sizeof(OSCallTrace))

typedef enum { OS_CALLTRACE_STACK = 0, OS_CALLTRACE_LOG = 1 } OSCallTraceMode;

#ifndef OS_NO_CALLTRACE
void OS_InitCallTrace(void *buf, u32 size, OSCallTraceMode mode);
#else
static inline void OS_InitCallTrace(void *, u32,
                                    OSCallTraceMode) { /* do nothing */ }
#endif

#ifndef OS_NO_CALLTRACE
void OS_DumpCallTrace(void);
#else
static inline void OS_DumpCallTrace(void) { /* do nothing */ }
#endif

#ifndef OS_NO_CALLTRACE
int OS_CalcCallTraceLines(u32 size);
#else
static inline int OS_CalcCallTraceLines(u32) { return 0; }
#endif

#ifndef OS_NO_CALLTRACE
u32 OS_CalcCallTraceBufferSize(int lines);
#else
static inline u32 OS_CalcCallTraceBufferSize(int) { return 0; }
#endif

#ifndef OS_NO_CALLTRACE
void OS_ClearCallTraceBuffer(void);
#else
static inline void OS_ClearCallTraceBuffer(void) { /* do nothing */ }
#endif

#ifndef OS_NO_CALLTRACE
BOOL OS_EnableCallTrace();
#else
static inline BOOL OS_EnableCallTrace() { return FALSE; }
#endif

#ifndef OS_NO_CALLTRACE
BOOL OS_DisableCallTrace();
#else
static inline BOOL OS_DisableCallTrace() { return FALSE; }
#endif

#ifndef OS_NO_CALLTRACE
BOOL OS_RestoreCallTrace(BOOL enable);
#else
static inline BOOL OS_RestoreCallTrace(BOOL) { return FALSE; }
#endif

#ifndef OS_NO_CALLTRACE
void OSi_SetCallTraceEntry(const char *name, u32 lr);
#else
static inline void OSi_SetCallTraceEntry(const char *name, u32 lr);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
