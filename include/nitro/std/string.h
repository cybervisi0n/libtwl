#ifndef NITRO_STD_STRING_H_
#define NITRO_STD_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/mi/memory.h>

#define STD_StrCpy STD_CopyString
#define STD_StrLCpy STD_CopyLString
#define STD_StrChr STD_SearchChar
#define STD_StrRChr STD_SearchCharReverse
#define STD_StrStr STD_SearchString
#define STD_StrLen STD_GetStringLength
#define STD_StrNLen STD_GetStringNLength
#define STD_StrCat STD_ConcatenateString
#define STD_StrLCat STD_ConcatenateLString
#define STD_StrCmp STD_CompareString
#define STD_StrNCmp STD_CompareNString
#define STD_StrLCmp STD_CompareLString

#define STD_MemCpy STD_CopyMemory
#define STD_MemMove STD_MoveMemory
#define STD_MemSet STD_FillMemory

extern char *STD_CopyString(char *destp, const char *srcp);

extern int STD_CopyLStringZeroFill(char *destp, const char *srcp, int n);

extern int STD_CopyLString(char *destp, const char *srcp, int siz);

extern char *STD_SearchChar(const char *srcp, int c);

extern char *STD_SearchCharReverse(const char *srcp, int c);

extern char *STD_SearchString(const char *srcp, const char *str);

extern int STD_GetStringLength(const char *str);

extern int STD_GetStringNLength(const char *str, int len);

extern char *STD_ConcatenateString(char *str1, const char *str2);

extern int STD_ConcatenateLString(char *str1, const char *str2, int size);

extern int STD_CompareString(const char *str1, const char *str2);

extern int STD_CompareNString(const char *str1, const char *str2, int len);

extern int STD_CompareLString(const char *str1, const char *str2, int len);

extern int STD_CompareNIString(const char *str1, const char *str2, int len);

extern int STD_TSScanf(const char *src, const char *fmt, ...);

extern int STD_TVSScanf(const char *src, const char *fmt, va_list vlist);

extern int STD_TSPrintf(char *dst, const char *fmt, ...);

extern int STD_TVSPrintf(char *dst, const char *fmt, va_list vlist);

extern int STD_TSNPrintf(char *dst, size_t len, const char *fmt, ...);

extern int STD_TVSNPrintf(char *dst, size_t len, const char *fmt,
                          va_list vlist);

static inline void *STD_CopyMemory(void *destp, const void *srcp, u32 size) {
  MI_CpuCopy(srcp, destp, size);
  return destp;
}

static inline void *STD_MoveMemory(void *destp, const void *srcp, u32 size) {
  MI_CpuMove(srcp, destp, size);
  return destp;
}

static inline void *STD_FillMemory(void *destp, u8 data, u32 size) {
  MI_CpuFill(destp, data, size);
  return destp;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
