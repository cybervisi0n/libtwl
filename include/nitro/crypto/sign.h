#ifndef NITRO_CRYPTO_SIGN_H_
#define NITRO_CRYPTO_SIGN_H_

#ifdef __cplusplus
extern "C" {
#endif

int CRYPTO_VerifySignatureWithHash(const void *hash_ptr, const void *sign_ptr,
                                   const void *mod_ptr);

int CRYPTO_VerifySignature(const void *data_ptr, int data_len,
                           const void *sign_ptr, const void *mod_ptr);

void *CRYPTO_SIGN_GetModulus(const void *pub_ptr);

#ifdef __cplusplus
}
#endif

#endif //_NITRO_CRYPTO_SIGN_H_
