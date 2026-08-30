#include <nitro/mi/memory.h>
#include <nitro/math/dgt.h>
#include "hmac.h"

#ifdef SDK_WIN32
#include <string.h>
#define MI_CpuCopy8(_x_, _y_, _z_) memcpy(_y_, _x_, _z_)
#define MI_CpuFill8 memset
#endif

#define MATH_SHA1_SMALL_STACK

#if !defined(MATH_SHA1_ASM)
static void MATHi_SHA1ProcessBlock(MATHSHA1Context *context);
#else
extern void MATHi_SHA1ProcessBlock(MATHSHA1Context *context);
#endif
static void MATHi_SHA1ProcessBlockForOverlay(MATHSHA1Context *context);
static void MATHi_SHA1Fill(MATHSHA1Context *context, u8 input, u32 length);

static int MATHi_OverlayTableMode = 0;
static void (*MATHi_SHA1ProcessMessageBlockFunc)(MATHSHA1Context *) =
    MATHi_SHA1ProcessBlock;

#if defined(PLATFORM_ENDIAN_LITTLE)
#define NETConvert32HToBE NETSwapBytes32
#else
#define NETConvert32HToBE(val) (val)
#endif

inline static u32 NETSwapBytes32(u32 val) {
  return (
      u32)((((val) >> 24UL) & 0x000000FFUL) | (((val) >> 8UL) & 0x0000FF00UL) |
           (((val) << 8UL) & 0x00FF0000UL) | (((val) << 24UL) & 0xFF000000UL));
}

inline static u32 NETRotateLeft32(int shift, u32 value) {
  return (u32)((value << shift) | (value >> (u32)(32 - shift)));
}

#if !defined(MATH_SHA1_ASM)

static void MATHi_SHA1ProcessBlock(MATHSHA1Context *context) {
  u32 a = context->h[0];
  u32 b = context->h[1];
  u32 c = context->h[2];
  u32 d = context->h[3];
  u32 e = context->h[4];
#if defined(MATH_SHA1_SMALL_STACK)
  u32 w[16];
#define w_alias(t) w[(t) & 15]
#define w_update(t)                                                            \
  if (t >= 16) {                                                               \
    w_alias(t) =                                                               \
        NETRotateLeft32(1, w_alias(t - 16 + 0) ^ w_alias(t - 16 + 2) ^         \
                               w_alias(t - 16 + 8) ^ w_alias(t - 16 + 13));    \
  }                                                                            \
  (void)0

#else
  u32 w[80];
#define w_alias(t) w[t]
#define w_update(t) (void)0
#endif /* defined(MATH_SHA1_SMALL_STACK) */

  int t;
  u32 tmp;
  for (t = 0; t < 16; ++t) {
    w[t] = NETConvert32HToBE(((u32 *)context->block)[t]);
  }
#if !defined(MATH_SHA1_SMALL_STACK)
  for (; t < 80; ++t) {
    u32 *prev = &w[t - 16];
    w[t] = NETRotateLeft32(1, prev[0] ^ prev[2] ^ prev[8] ^ prev[13]);
  }
#endif /* !defined(MATH_SHA1_SMALL_STACK) */
  for (t = 0; t < 20; ++t) {
    tmp = 0x5A827999UL + ((b & c) | (~b & d));
    w_update(t);
    tmp += w_alias(t) + NETRotateLeft32(5, a) + e;
    e = d;
    d = c;
    c = NETRotateLeft32(30, b);
    b = a;
    a = tmp;
  }
  for (; t < 40; ++t) {
    tmp = 0x6ED9EBA1UL + (b ^ c ^ d);
    w_update(t);
    tmp += w_alias(t) + NETRotateLeft32(5, a) + e;
    e = d;
    d = c;
    c = NETRotateLeft32(30, b);
    b = a;
    a = tmp;
  }
  for (; t < 60; ++t) {
    tmp = 0x8F1BBCDCUL + ((b & c) | (b & d) | (c & d));
    w_update(t);
    tmp += w_alias(t) + NETRotateLeft32(5, a) + e;
    e = d;
    d = c;
    c = NETRotateLeft32(30, b);
    b = a;
    a = tmp;
  }
  for (; t < 80; ++t) {
    tmp = 0xCA62C1D6UL + (b ^ c ^ d);
    w_update(t);
    tmp += w_alias(t) + NETRotateLeft32(5, a) + e;
    e = d;
    d = c;
    c = NETRotateLeft32(30, b);
    b = a;
    a = tmp;
  }
  context->h[0] += a;
  context->h[1] += b;
  context->h[2] += c;
  context->h[3] += d;
  context->h[4] += e;
}
#endif /* !defined(MATH_SHA1_ASM) */

static void MATHi_SHA1ProcessBlockForOverlay(MATHSHA1Context *context) {
  u32 s0, s1;
  u32 *block = (u32 *)context->block;

  s0 = block[6];     // 6   = location of file_id
  s1 = block[6 + 8]; // 6+8 = location of next file_id
  block[6] = 0;
  block[6 + 8] = 0;

  MATHi_SHA1ProcessBlock(context);

  block[6] = s0;
  block[6 + 8] = s1;
}

static void MATHi_SHA1Fill(MATHSHA1Context *context, u8 input, u32 length) {
  while (length > 0) {

    u32 rest = MATH_SHA1_BLOCK_SIZE - context->pool;
    if (rest > length) {
      rest = length;
    }
    MI_CpuFill8(&context->block[context->pool], input, rest);
    length -= rest;
    context->pool += rest;

    if (context->pool >= MATH_SHA1_BLOCK_SIZE) {
      MATHi_SHA1ProcessMessageBlockFunc(context);
      context->pool = 0;
      ++context->blocks_low;
      if (!context->blocks_low) {
        ++context->blocks_high;
      }
    }
  }
}

int MATHi_SetOverlayTableMode(int flag) {
  int prev = MATHi_OverlayTableMode;

  MATHi_OverlayTableMode = flag;

  if (MATHi_OverlayTableMode) {
    MATHi_SHA1ProcessMessageBlockFunc = MATHi_SHA1ProcessBlockForOverlay;
  } else {
    MATHi_SHA1ProcessMessageBlockFunc = MATHi_SHA1ProcessBlock;
  }

  return prev;
}

void MATH_SHA1Init(MATHSHA1Context *context) {
  context->blocks_low = 0;
  context->blocks_high = 0;
  context->pool = 0;
  context->h[0] = 0x67452301;
  context->h[1] = 0xEFCDAB89;
  context->h[2] = 0x98BADCFE;
  context->h[3] = 0x10325476;
  context->h[4] = 0xC3D2E1F0;
}

void MATH_SHA1Update(MATHSHA1Context *context, const void *input, u32 length) {
  while (length > 0) {

    u32 rest = MATH_SHA1_BLOCK_SIZE - context->pool;
    if (rest > length) {
      rest = length;
    }
    MI_CpuCopy8(input, &context->block[context->pool], rest);
    input = (const u8 *)input + rest;
    length -= rest;
    context->pool += rest;

    if (context->pool >= MATH_SHA1_BLOCK_SIZE) {
      MATHi_SHA1ProcessMessageBlockFunc(context);
      context->pool = 0;
      ++context->blocks_low;
      if (!context->blocks_low) {
        ++context->blocks_high;
      }
    }
  }
}

void MATH_SHA1GetHash(MATHSHA1Context *context, void *digest) {
  u32 footer[2];
  static const u8 padlead[1] = {0x80};
  static const u8 padalign[sizeof(footer)] = {
      0x00,
  };

  footer[1] = NETConvert32HToBE((u32)(context->blocks_low << (6 + 3)) +
                                (context->pool << (0 + 3)));
  footer[0] = NETConvert32HToBE((u32)(context->blocks_high << (6 + 3)) +
                                (context->blocks_low >> (u32)(32 - (6 + 3))));

  MATH_SHA1Update(context, padlead, sizeof(padlead));

  if (MATH_SHA1_BLOCK_SIZE - context->pool < sizeof(footer)) {
    MATH_SHA1Update(context, padalign, MATH_SHA1_BLOCK_SIZE - context->pool);
  }

  MATHi_SHA1Fill(context, 0x00,
                 MATH_SHA1_BLOCK_SIZE - context->pool - sizeof(footer));

  MATH_SHA1Update(context, footer, sizeof(footer));

  context->h[0] = NETConvert32HToBE((u32)context->h[0]);
  context->h[1] = NETConvert32HToBE((u32)context->h[1]);
  context->h[2] = NETConvert32HToBE((u32)context->h[2]);
  context->h[3] = NETConvert32HToBE((u32)context->h[3]);
  context->h[4] = NETConvert32HToBE((u32)context->h[4]);
  MI_CpuCopy8(context->h, digest, sizeof(context->h));
}

#if defined(MATH_SHA1_BSAFE_TEST)
extern unsigned char *SHA1(const unsigned char *d, unsigned long n,
                           unsigned char *md);
#endif

void MATH_CalcSHA1(void *digest, const void *data, u32 dataLength) {
#if !defined(MATH_SHA1_BSAFE_TEST)
  MATHSHA1Context context;
  MATH_SHA1Init(&context);
  MATH_SHA1Update(&context, data, dataLength);
  MATH_SHA1GetHash(&context, digest);
#else
  SHA1((unsigned char *)data, dataLength, (unsigned char *)digest);
#endif
}

void MATH_CalcHMACSHA1(void *digest, const void *bin_ptr, u32 bin_len,
                       const void *key_ptr, u32 key_len) {
  MATHSHA1Context context;
  unsigned char hash_buf[MATH_SHA1_DIGEST_SIZE]; /* Hash value gotten from the
                                                    hash function */

  MATHiHMACFuncs hash2funcs = {
      MATH_SHA1_DIGEST_SIZE,
      (512 / 8),
  };

  hash2funcs.context = &context;
  hash2funcs.hash_buf = hash_buf;
  hash2funcs.HashReset = (void (*)(void *))MATH_SHA1Init;
  hash2funcs.HashSetSource =
      (void (*)(void *, const void *, u32))MATH_SHA1Update;
  hash2funcs.HashGetDigest = (void (*)(void *, void *))MATH_SHA1GetHash;

  MATHi_CalcHMAC(digest, bin_ptr, bin_len, key_ptr, key_len, &hash2funcs);
}
