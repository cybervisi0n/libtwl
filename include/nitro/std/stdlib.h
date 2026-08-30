#ifndef NITRO_STD_STDLIB_H_
#define NITRO_STD_STDLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>

#define STD_AToI STD_ConvertAsciiToInt
#define STD_AToL STD_ConvertAsciiToLong

extern int STD_ConvertAsciiToInt(const char *s);

extern long int STD_ConvertAsciiToLong(const char *s);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
