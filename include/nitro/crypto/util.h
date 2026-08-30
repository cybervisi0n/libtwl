#ifndef NITRO_CRYPTO_UTIL_H_
#define NITRO_CRYPTO_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>

#define CRYPTO_SetAllocator(alloc, free)                                       \
  CRYPTO_SetMemAllocator(alloc, free, NULL)

void CRYPTO_SetMemAllocator(void *(*alloc)(u32), void (*free)(void *),
                            void *(*realloc)(void *, u32, u32));

#define BER_INTEGER 2
#define BER_BIT_STRING 3
#define BER_OCTET_STRING 4
#define BER_NULL 5
#define BER_OBJECT 6
#define BER_SEQUENCE 16
#define BER_CONSTRUCTED 0x20

int CRYPTO_DerSkip(unsigned char **datap, unsigned int *dlenp,
                   unsigned char type, unsigned int *lenp);

#ifdef __cplusplus
}
#endif

#endif //_NITRO_CRYPTO_UTIL_H_
