#if !defined(NITRO_FS_UTIL_H_)
#define NITRO_FS_UTIL_H_

#include <nitro/misc.h>
#include <nitro/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(SDK_ARM7) || defined(SDK_ARM7FS)
#define FS_IMPLEMENT
#endif

SDK_INLINE BOOL FSi_IsSlash(u32 c) { return (c == '/') || (c == '\\'); }

SDK_INLINE int FSi_IncrementSjisPosition(const char *str, int pos) {
  return pos + 1 + STD_IsSjisLeadByte(str[pos]);
}

int FSi_DecrementSjisPosition(const char *str, int pos);

int FSi_IncrementSjisPositionToSlash(const char *str, int pos);

int FSi_DecrementSjisPositionToSlash(const char *str, int pos);

int FSi_TrimSjisTrailingSlash(char *str);

SDK_INLINE int FSi_StrNICmp(const char *str1, const char *str2, u32 len) {
  int retval = 0;
  int i;
  for (i = 0; i < len; ++i) {
    u32 c = (u8)(str1[i] - 'A');
    u32 d = (u8)(str2[i] - 'A');
    if (c <= 'Z' - 'A') {
      c += 'a' - 'A';
    }
    if (d <= 'Z' - 'A') {
      d += 'a' - 'A';
    }
    retval = (int)(c - d);
    if (retval != 0) {
      break;
    }
  }
  return retval;
}

SDK_INLINE BOOL FSi_IsUnicodeSlash(u16 c) {
  return (c == L'/') || (c == L'\\');
}

int FSi_DecrementUnicodePosition(const u16 *str, int pos);

int FSi_DecrementUnicodePositionToSlash(const u16 *str, int pos);

SDK_INLINE void FSi_WaitConditionChange(u32 *flags, u32 on, u32 off,
                                        OSThreadQueue *queue) {
  OSIntrMode bak = OS_DisableInterrupts();
  while ((!on || ((*flags & on) == 0)) && (!off || ((*flags & off) != 0))) {
    OS_SleepThread(queue);
  }
  (void)OS_RestoreInterrupts(bak);
}

SDK_INLINE void FSi_WaitConditionOn(u32 *flags, u32 bits,
                                    OSThreadQueue *queue) {
  FSi_WaitConditionChange(flags, bits, 0, queue);
}

SDK_INLINE void FSi_WaitConditionOff(u32 *flags, u32 bits,
                                     OSThreadQueue *queue) {
  FSi_WaitConditionChange(flags, 0, bits, queue);
}

BOOL FSi_GetFileLengthIfProc(FSFile *file, u32 *length);

BOOL FSi_GetFilePositionIfProc(FSFile *file, u32 *length);

BOOL FSi_SeekFileIfProc(FSFile *file, s32 offset, FSSeekFileMode from);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_FS_UTIL_H_ */
