#ifndef JKSSL_PKCS8_H
#define JKSSL_PKCS8_H

#include <jkssl/pem.h>

struct asn1_object;
struct evp_cipher;
struct evp_pkey;

struct evp_pkey *a2i_pkcs8_private_key_info(const struct asn1_object *object,
                                            struct evp_pkey **pkeyp);
struct asn1_object *i2a_pkcs8_private_key_info(const struct evp_pkey *pkey);

struct evp_pkey *a2i_pkcs8_private_key(const struct asn1_object *object,
                                       struct evp_pkey **pkeyp,
                                       pem_password_cb *cb,
                                       void *userdata);
struct asn1_object *i2a_pkcs8_private_key(const struct evp_pkey *pkey,
                                          const struct evp_cipher *cipher,
                                          pem_password_cb *cb,
                                          void *userdata);

#endif
