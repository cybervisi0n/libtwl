#ifndef NITRO_MATH_DGT_H_
#define NITRO_MATH_DGT_H_

#ifndef SDK_WIN32
#include <nitro/misc.h>
#endif
#include <nitro/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MATH_MD5_DIGEST_SIZE (128 / 8)  // 128-bit
#define MATH_SHA1_DIGEST_SIZE (160 / 8) // 160-bit
#define MATH_SHA256_DIGEST_SIZE 32      // 256-bit

#define MATH_HASH_DIGEST_SIZE_MAX MATH_SHA1_DIGEST_SIZE

#define MATH_HASH_BLOCK_SIZE (512 / 8)
#define MATH_MD5_BLOCK_SIZE MATH_HASH_BLOCK_SIZE  // 512-bit
#define MATH_SHA1_BLOCK_SIZE MATH_HASH_BLOCK_SIZE // 512-bit

typedef struct MATHMD5Context {
  union {
    struct {
      unsigned long a, b, c, d;
    };
    unsigned long state[4];
  };
  unsigned long long length;
  union {
    unsigned long buffer32[16];
    unsigned char buffer8[64];
  };
} MATHMD5Context;

typedef struct MATHSHA1Context {
  u32 h[5];                       /* H0,H1,H2,H3,H4 */
  u8 block[MATH_SHA1_BLOCK_SIZE]; /* current message block */
  u32 pool;                       /* message length in 'block' */
  u32 blocks_low;                 /* total blocks (in bytes) */
  u32 blocks_high;
} MATHSHA1Context;

void MATH_MD5Init(MATHMD5Context *context);

void MATH_MD5Update(MATHMD5Context *context, const void *input, u32 length);

void MATH_MD5GetHash(MATHMD5Context *context, void *digest);

static inline void MATH_MD5GetDigest(MATHMD5Context *context, void *digest) {
  MATH_MD5GetHash(context, digest);
}

void MATH_SHA1Init(MATHSHA1Context *context);

void MATH_SHA1Update(MATHSHA1Context *context, const void *input, u32 length);

void MATH_SHA1GetHash(MATHSHA1Context *context, void *digest);

static inline void MATH_SHA1GetDigest(MATHSHA1Context *context, void *digest) {
  MATH_SHA1GetHash(context, digest);
}

#define MATHSHA256_CBLOCK 64
#define MATHSHA256_LBLOCK 16
#define MATHSHA256_BLOCK 16
#define MATHSHA256_LAST_BLOCK 56
#define MATHSHA256_LENGTH_BLOCK 8
#define MATHSHA256_DIGEST_LENGTH 32

typedef struct MATHSHA256Context MATHSHA256Context;
typedef void(MATHSHA256_BLOCK_FUNC)(MATHSHA256Context *c, u32 *W, int num);

struct MATHSHA256Context {
  u32 h[8];
  u32 Nl, Nh;
  u8 data[MATHSHA256_CBLOCK];
  int num;
};

void MATH_SHA256Init(MATHSHA256Context *c);
void MATH_SHA256Update(MATHSHA256Context *c, const void *data, u32 len);
void MATH_SHA256GetHash(MATHSHA256Context *c, void *digest);
void MATH_CalcSHA256(void *digest, const void *data, u32 dataLength);

void MATH_CalcMD5(void *digest, const void *data, u32 dataLength);

void MATH_CalcSHA1(void *digest, const void *data, u32 dataLength);

void MATH_CalcHMACMD5(void *digest, const void *data, u32 dataLength,
                      const void *key, u32 keyLength);

void MATH_CalcHMACSHA1(void *digest, const void *data, u32 dataLength,
                       const void *key, u32 keyLength);

void MATH_CalcHMACSHA256(void *digest, const void *data, u32 dataLength,
                         const void *key, u32 keyLength);

int MATHi_SetOverlayTableMode(int flag);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
