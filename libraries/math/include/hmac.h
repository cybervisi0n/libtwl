#ifndef NITRO_LIBRARIES_MATH_COMMON_HMAC_H_
#define NITRO_LIBRARIES_MATH_COMMON_HMAC_H_

#include <nitro/types.h>

typedef struct MATHiHMACFuncs {
  const u32 dlength;
  const u32 blength;
  void *context;
  u8 *hash_buf;
  void (*HashReset)(void *);
  void (*HashSetSource)(void *, const void *, u32);
  void (*HashGetDigest)(void *, void *);
} MATHiHMACFuncs;

void MATHi_CalcHMAC(void *mac, const void *message, u32 message_length,
                    const void *key, u32 key_length, MATHiHMACFuncs *funcs);

#endif // ifndef NITRO_LIBRARIES_MATH_COMMON_HMAC_H_
