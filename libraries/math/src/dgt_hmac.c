#include <nitro/math/dgt.h>
#include "hmac.h"

#ifdef SDK_TWLHYB
#ifdef SDK_ARM7
#include <twl/ltdwram_begin.h>
#endif
#endif

void MATHi_CalcHMAC(void *mac, const void *message, u32 message_length,
                    const void *key, u32 key_length, MATHiHMACFuncs *funcs) {
  int i;
  u8 new_key[MATH_HASH_BLOCK_SIZE];
  u8 *use_key;
  u32 use_key_length;
  u8 ipad_key[MATH_HASH_BLOCK_SIZE];
  u8 opad_key[MATH_HASH_BLOCK_SIZE];

  if ((mac == NULL) || (message == NULL) || (message_length == 0) ||
      (key == NULL) || (key_length == 0) || (funcs == NULL)) {
    return;
  }

  if (key_length > funcs->blength) {
    funcs->HashReset(funcs->context);
    funcs->HashSetSource(funcs->context, key, key_length);
    funcs->HashGetDigest(funcs->context, new_key);
    use_key = new_key;
    use_key_length = funcs->dlength;
  } else {
    use_key = (u8 *)key;
    use_key_length = key_length;
  }

  for (i = 0; i < use_key_length; i++) {
    ipad_key[i] = (u8)(use_key[i] ^ 0x36);
  }

  for (; i < funcs->blength; i++) {
    ipad_key[i] = 0x00 ^ 0x36;
  }

  funcs->HashReset(funcs->context);
  funcs->HashSetSource(funcs->context, ipad_key, funcs->blength);
  funcs->HashSetSource(funcs->context, message, message_length);
  funcs->HashGetDigest(funcs->context, funcs->hash_buf);

  for (i = 0; i < use_key_length; i++) {
    opad_key[i] = (u8)(use_key[i] ^ 0x5c);
  }

  for (; i < funcs->blength; i++) {
    opad_key[i] = 0x00 ^ 0x5c;
  }

  funcs->HashReset(funcs->context);
  funcs->HashSetSource(funcs->context, opad_key, funcs->blength);
  funcs->HashSetSource(funcs->context, funcs->hash_buf, funcs->dlength);
  funcs->HashGetDigest(funcs->context, mac);
}

#ifdef SDK_TWLHYB
#ifdef SDK_ARM7
#include <twl/ltdwram_end.h>
#endif
#endif
