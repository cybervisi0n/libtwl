#include <nitro/math/dgt.h>
#include <nitro/mi/memory.h>
#include <nitro/mi/endian.h>
#include "hmac.h"

#define NOT_USE_SHA256FILL 0 // Do not use MATH_SHA256Fill

#define memcpy(_x_, _y_, _z_) MI_CpuCopy8(_y_, _x_, _z_)
#define memset MI_CpuFill8
#define NETConvert32HToBE(val) MI_HToBE32(val)

#if !defined(MATH_SHA256_ASM)
static void MATH_SHA256iProcessBlock(MATHSHA256Context *context);
#else
extern void MATH_SHA256iProcessBlock(MATHSHA256Context *context);
#endif

#define SHA256_ROTR(r, x) (NETRotateLeft32((32 - r), x))
#define SHA256_SHR(s, x) (x >> s)

#define SHA256_CH(x, y, z) (((x) & (y)) ^ ((~x) & (z)))
#define SHA256_MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#define SHA256_LSIGMA_0(x)                                                     \
  (SHA256_ROTR(2, x) ^ SHA256_ROTR(13, x) ^ SHA256_ROTR(22, x))

#define SHA256_LSIGMA_1(x)                                                     \
  (SHA256_ROTR(6, x) ^ SHA256_ROTR(11, x) ^ SHA256_ROTR(25, x))

#define SHA256_SSIGMA_0(x)                                                     \
  (SHA256_ROTR(7, x) ^ SHA256_ROTR(18, x) ^ SHA256_SHR(3, x))

#define SHA256_SSIGMA_1(x)                                                     \
  (SHA256_ROTR(17, x) ^ SHA256_ROTR(19, x) ^ SHA256_SHR(10, x))

#if !defined(MATH_SHA256_ASM)
static u32 MATH_SHA256iConst[64] =
#else
u32 MATH_SHA256iConst[64] =
#endif
    {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
     0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
     0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
     0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
     0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
     0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
     0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
     0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
     0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
     0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
     0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

inline static u32 NETRotateLeft32(int shift, u32 value) {
  return (u32)((value << shift) | (value >> (u32)(32 - shift)));
}

void MATH_SHA256Init(MATHSHA256Context *context) {
  context->Nl = 0;
  context->Nh = 0;
  context->num = 0;
  context->h[0] = 0x6a09e667;
  context->h[1] = 0xbb67ae85;
  context->h[2] = 0x3c6ef372;
  context->h[3] = 0xa54ff53a;
  context->h[4] = 0x510e527f;
  context->h[5] = 0x9b05688c;
  context->h[6] = 0x1f83d9ab;
  context->h[7] = 0x5be0cd19;
}

void MATH_SHA256Update(MATHSHA256Context *context, const void *input,
                       u32 length) {
  while (length > 0) {

    u32 rest = MATHSHA256_CBLOCK - (u32)context->num;
    u8 *dat;
    if (rest > length) {
      rest = length;
    }
    dat = (u8 *)context->data;
    (void)memcpy(&dat[context->num], input, rest);
    input = (const u8 *)input + rest;
    length -= rest;
    context->num += rest;

    if (context->num >= MATHSHA256_CBLOCK) {
      MATH_SHA256iProcessBlock(context);
      context->num = 0;
      ++context->Nl;
      if (!context->Nl) {
        ++context->Nh;
      }
    }
  }
}

#if (NOT_USE_SHA256FILL == 0)

static void MATH_SHA256Fill(MATHSHA256Context *context, u8 input, u32 length) {
  while (length > 0) {

    u32 rest = MATHSHA256_CBLOCK - (u32)context->num;
    u8 *dat;
    if (rest > length) {
      rest = length;
    }
    dat = (u8 *)context->data;
    (void)memset(&dat[context->num], input, rest);
    length -= rest;
    context->num += rest;

    if (context->num >= MATHSHA256_CBLOCK) {
      MATH_SHA256iProcessBlock(context);
      context->num = 0;
      ++context->Nl;
      if (!context->Nl) {
        ++context->Nh;
      }
    }
  }
}
#endif /* (NOT_USE_SHA256FILL == 0) */

#if (NOT_USE_SHA256FILL == 1)
static u8 padding[64] = {0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                         0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                         0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                         0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif
void MATH_SHA256GetHash(MATHSHA256Context *context, void *digest) {

  u32 footer[2];
  static const u8 padlead[1] = {0x80};
  static const u8 padalign[sizeof(footer)] = {
      0x00,
  };

  footer[1] = NETConvert32HToBE((u32)(context->Nl << (6 + 3)) +
                                (context->num << (0 + 3)));
  footer[0] = NETConvert32HToBE((u32)(context->Nh << (6 + 3)) +
                                (context->Nl >> (u32)(32 - (6 + 3))));
#if (NOT_USE_SHA256FILL == 0)

  MATH_SHA256Update(context, padlead, sizeof(padlead));

  if (MATHSHA256_CBLOCK - context->num < sizeof(footer)) {
    MATH_SHA256Update(context, padalign, MATHSHA256_CBLOCK - (u32)context->num);
  }

  MATH_SHA256Fill(context, 0x00,
                  MATHSHA256_CBLOCK - context->num - sizeof(footer));

  MATH_SHA256Update(context, footer, sizeof(footer));
#else  /* (NOT_USE_SHA256FILL == 1) */
  {
    u32 last, padn;

    last = context->num;
    padn = (last < 56) ? (56 - last) : (120 - last);
    MATH_SHA256Update(context, padding, padn);
    MATH_SHA256Update(context, footer, sizeof(footer));
  }
#endif /* (NOT_USE_SHA256FILL == 1) */

  context->h[0] = NETConvert32HToBE((u32)context->h[0]);
  context->h[1] = NETConvert32HToBE((u32)context->h[1]);
  context->h[2] = NETConvert32HToBE((u32)context->h[2]);
  context->h[3] = NETConvert32HToBE((u32)context->h[3]);
  context->h[4] = NETConvert32HToBE((u32)context->h[4]);
  context->h[5] = NETConvert32HToBE((u32)context->h[5]);
  context->h[6] = NETConvert32HToBE((u32)context->h[6]);
  context->h[7] = NETConvert32HToBE((u32)context->h[7]);
  (void)memcpy(digest, &context->h, MATHSHA256_DIGEST_LENGTH);
}

#if 0









const NETHashInterface* NETGetSHA256Interface(void)
{
    static const NETHashInterface sha1template =
    {
        MATHSHA256_DIGEST_LENGTH,
        MATHSHA256_CBLOCK,
        sizeof(MATHSHA256Context),
        NULL,
        (void (*)(void*))MATH_SHA256Init,
        (void (*)(void*, const void*, u32))MATH_SHA256Update,
        (void (*)(void*, void *))MATH_SHA256GetDigest,
    };
    return &sha1template;
}
#endif

#if !defined(MATH_SHA256_ASM)

static void MATH_SHA256iProcessBlock(MATHSHA256Context *context) {
  u32 a = context->h[0];
  u32 b = context->h[1];
  u32 c = context->h[2];
  u32 d = context->h[3];
  u32 e = context->h[4];
  u32 f = context->h[5];
  u32 g = context->h[6];
  u32 h = context->h[7];
  u32 w[64];
  u32 tmp1, tmp2;
  u32 *prev;

  int t;
  for (t = 0; t < 16; ++t) {
    w[t] = NETConvert32HToBE(((u32 *)context->data)[t]);
  }

  for (; t < 64; ++t) {
    prev = &w[t - 16];
    w[t] = prev[0] + SHA256_SSIGMA_0(prev[1]) + prev[9] +
           SHA256_SSIGMA_1(prev[14]);
  }

  for (t = 0; t < 64; ++t) {
    tmp1 = h + SHA256_LSIGMA_1(e) + SHA256_CH(e, f, g) + MATH_SHA256iConst[t] +
           w[t];

    tmp2 = SHA256_LSIGMA_0(a) + SHA256_MAJ(a, b, c);

    h = g;
    g = f;
    f = e;
    e = d + tmp1;
    d = c;
    c = b;
    b = a;
    a = tmp1 + tmp2;
  }

  context->h[0] += a;
  context->h[1] += b;
  context->h[2] += c;
  context->h[3] += d;
  context->h[4] += e;
  context->h[5] += f;
  context->h[6] += g;
  context->h[7] += h;
}
#endif /* !defined(MATH_SHA256_ASM) */

void MATH_CalcSHA256(void *digest, const void *data, u32 dataLength) {
  MATHSHA256Context context;
  MATH_SHA256Init(&context);
  MATH_SHA256Update(&context, data, dataLength);
  MATH_SHA256GetHash(&context, digest);
}

void MATH_CalcHMACSHA256(void *digest, const void *bin_ptr, u32 bin_len,
                         const void *key_ptr, u32 key_len) {
  MATHSHA256Context context;
  unsigned char hash_buf[MATH_SHA256_DIGEST_SIZE]; /* Hash value gotten from the
                                                      hash function */

  MATHiHMACFuncs hash2funcs = {
      MATH_SHA256_DIGEST_SIZE,
      (512 / 8),
  };

  hash2funcs.context = &context;
  hash2funcs.hash_buf = hash_buf;
  hash2funcs.HashReset = (void (*)(void *))MATH_SHA256Init;
  hash2funcs.HashSetSource =
      (void (*)(void *, const void *, u32))MATH_SHA256Update;
  hash2funcs.HashGetDigest = (void (*)(void *, void *))MATH_SHA256GetHash;

  MATHi_CalcHMAC(digest, bin_ptr, bin_len, key_ptr, key_len, &hash2funcs);
}
