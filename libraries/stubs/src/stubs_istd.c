#include <nitro.h>

typedef enum { OVERLAYPROC_ARM9, OVERLAYPROC_ARM7 } OVERLAYPROC;
typedef enum { ISTDOVERLAYPROC_ARM9, ISTDOVERLAYPROC_ARM7 } ISTDOVERLAYPROC;

void ISTDPrint(const char *);
void ISTDPrintf(const char *, ...);
void ISTDPrintEx(int, const char *);
void ISTDPrintfEx(int, const char *, ...);
void ISTDAssert(const char *, int, const char *, int);
void _ISTDbgLib_Initialize(void);
void _ISTDbgLib_AllocateEmualtor(void);
void _ISTDbgLib_FreeEmulator(void);
int _ISTDbgLib_RegistOverlayInfo(OVERLAYPROC, u32, u32, u32, u32);
int _ISTDbgLib_UnregistOverlayInfo(OVERLAYPROC, u32, u32);
int _ISTDbgLib_IsDebuggerPresent(void);
void _ISTDbgLib_OnLoadChildBinary(void);
BOOL _ISTDbgLib_RegistOverlayInfoByAddr(ISTDOVERLAYPROC, u32, u32, u32, u32);
BOOL _ISTDbgLib_UnregistOverlayInfoByAddr(ISTDOVERLAYPROC, u32, u32);
#ifdef SDK_ARM9
void _ISTDbgLib_OnBeforeResetHard(void);
#endif // SDK_ARM9
void ISTDPrintSetBlockingMode(BOOL);
BOOL ISTDPrintGetBlockingMode(void);

SDK_WEAK_SYMBOL void ISTDPrint(const char *) {}

SDK_WEAK_SYMBOL void ISTDPrintf(const char *, ...) {}

SDK_WEAK_SYMBOL void ISTDPrintEx(int, const char *) {}

SDK_WEAK_SYMBOL void ISTDPrintfEx(int, const char *, ...) {}

SDK_WEAK_SYMBOL void ISTDAssert(const char *, int, const char *, int) {}

SDK_WEAK_SYMBOL void _ISTDbgLib_Initialize(void) {}
SDK_WEAK_SYMBOL void _ISTDbgLib_AllocateEmualtor(void) {}

SDK_WEAK_SYMBOL void _ISTDbgLib_FreeEmulator(void) {}

SDK_WEAK_SYMBOL int _ISTDbgLib_RegistOverlayInfo(OVERLAYPROC, u32, u32, u32,
                                                 u32) {
  return 1;
}

SDK_WEAK_SYMBOL int _ISTDbgLib_UnregistOverlayInfo(OVERLAYPROC, u32, u32) {
  return 1;
}

SDK_WEAK_SYMBOL BOOL _ISTDbgLib_RegistOverlayInfoByAddr(ISTDOVERLAYPROC, u32,
                                                        u32, u32, u32) {
  return TRUE;
}

SDK_WEAK_SYMBOL BOOL _ISTDbgLib_UnregistOverlayInfoByAddr(ISTDOVERLAYPROC, u32,
                                                          u32) {
  return TRUE;
}

SDK_WEAK_SYMBOL int _ISTDbgLib_IsDebuggerPresent(void) { return 1; }

SDK_WEAK_SYMBOL void _ISTDbgLib_OnLoadChildBinary(void) {}

#ifdef SDK_ARM9
SDK_WEAK_SYMBOL void _ISTDbgLib_OnBeforeResetHard(void) {}
#endif // SDK_ARM9

SDK_WEAK_SYMBOL void ISTDPrintSetBlockingMode(BOOL) {}

SDK_WEAK_SYMBOL BOOL ISTDPrintGetBlockingMode(void) { return FALSE; }
