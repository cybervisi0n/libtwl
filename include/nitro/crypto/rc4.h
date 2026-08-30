#ifndef NITRO_CRYPTO_RC4_H_
#define NITRO_CRYPTO_RC4_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>

typedef struct CRYPTORC4Context {
  u8 i, j;
  u8 padd[2];
  u8 s[256];
} CRYPTORC4Context;

void CRYPTO_RC4Init(CRYPTORC4Context *context, const void *key, u32 key_len);

void CRYPTO_RC4Encrypt(CRYPTORC4Context *context, const void *in, u32 length,
                       void *out);

static inline void CRYPTO_RC4Decrypt(CRYPTORC4Context *context, const void *in,
                                     u32 length, void *out) {
  CRYPTO_RC4Encrypt(context, in, length, out);
}

static inline void CRYPTO_RC4(const void *key, u32 key_len, void *data,
                              u32 data_len) {
  CRYPTORC4Context context;
  CRYPTO_RC4Init(&context, key, key_len);
  CRYPTO_RC4Encrypt(&context, data, data_len, data);
}

typedef struct CRYPTORC4FastContext {
  u32 i, j;
  u32 s[256];
} CRYPTORC4FastContext;

void CRYPTO_RC4FastInit(CRYPTORC4FastContext *context, const void *key,
                        u32 key_len);

void CRYPTO_RC4FastEncrypt(CRYPTORC4FastContext *context, const void *in,
                           u32 length, void *out);

static inline void CRYPTO_RC4FastDecrypt(CRYPTORC4FastContext *context,
                                         const void *in, u32 length,
                                         void *out) {
  CRYPTO_RC4FastEncrypt(context, in, length, out);
}

static inline void CRYPTO_RC4Fast(const void *key, u32 key_len, void *data,
                                  u32 data_len) {
  CRYPTORC4FastContext context;
  CRYPTO_RC4FastInit(&context, key, key_len);
  CRYPTO_RC4FastEncrypt(&context, data, data_len, data);
}

#ifdef __cplusplus
}
#endif

#endif //_NITRO_CRYPTO_RC4_H_
