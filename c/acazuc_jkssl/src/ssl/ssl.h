#ifndef JKSSL_SSL_SSL_H
#define JKSSL_SSL_SSL_H

#include "refcount.h"

#include <jkssl/bio.h>
#include <jkssl/ssl.h>

#include <sys/types.h>

#define HELLO_REQUEST       0
#define CLIENT_HELLO        1
#define SERVER_HELLO        2
#define CERTIFICATE         11
#define SERVER_KEY_EXCHANGE 12
#define CERTIFICATE_REQUEST 13
#define SERVER_HELLO_DONE   14
#define CERTIFICATE_VERIFY  15
#define CLIENT_KEY_EXCHANGE 16
#define FINISHED            20

#define CHANGE_CIPHER_SPEC 20
#define ALERT              21
#define HANDSHAKE          22
#define APPLICATION_DATA   23

#define TLS_VERSION_3_0 0x0300
#define TLS_VERSION_1_0 0x0301
#define TLS_VERSION_1_1 0x0302
#define TLS_VERSION_1_2 0x0303

#define TLS_DHE_RSA_WITH_AES_128_CBC_SHA256 0x0067
#define TLS_DHE_RSA_WITH_AES_256_CBC_SHA256 0x006B

#define EXTENSION_SERVER_NAME                            0
#define EXTENSION_MAX_FRAGMENT_LENGTH                    1
#define EXTENSION_STATUS_REQUEST                         5
#define EXTENSION_SUPPORTED_GROUPS                       10
#define EXTENSION_SIGNATURE_ALGORITHMS                   13
#define EXTENSION_USE_SRTP                               14
#define EXTENSION_HEARTBEAT                              15
#define EXTENSION_APPLICATION_LAYER_PROTOCOL_NEGOCIATION 16
#define EXTENSION_SIGNED_CERTIFICATE_TIMESTAMP           18
#define EXTENSION_CLIENT_CERTIFICATE_TYPE                19
#define EXTENSION_SERVER_CERTIFICATE_TYPE                20
#define EXTENSION_PADDING                                21
#define EXTENSION_PRE_SHARED_KEY                         41
#define EXTENSION_EALY_DATA                              42
#define EXTENSION_SUPPORTED_VERSIONS                     43
#define EXTENSION_COOKIE                                 44
#define EXTENSION_PSK_KEY_EXCHANGE_MODES                 45
#define EXTENSION_CERTIFICATE_AUTHORITIES                47
#define EXTENSION_OID_FILTERS                            48
#define EXTENSION_POST_HANDSHAKE_AUTH                    49
#define EXTENSION_SIGNATURE_ALGORITHMS_CERT              50
#define EXTENSION_KEY_SHARE                              51

#define SIGNATURE_RSA_PKCS1_SHA256       0x0401
#define SIGNATURE_RSA_PKCS1_SHA384       0x0501
#define SIGNATURE_RSA_PKCS1_SHA512       0x0601
#define SIGNATURE_ECDSA_SECP256R1_SHA256 0x0403
#define SIGNATURE_ECDSA_SECP384R1_SHA384 0x0503
#define SIGNATURE_ECDSA_SECP521R1_SHA512 0x0603
#define SIGNATURE_RSA_PSS_RSAE_SHA256    0x0804
#define SIGNATURE_RSA_PSS_RSAE_SHA384    0x0805
#define SIGNATURE_RSA_PSS_RSAE_SHA512    0x0806
#define SIGNATURE_ED25519                0x0807
#define SIGNATURE_ED448                  0x0808
#define SIGNATURE_RSA_PSS_PSS_SHA256     0x0809
#define SIGNATURE_RSA_PSS_PSS_SHA384     0x080A
#define SIGNATURE_RSA_PSS_PSS_SHA512     0x080B
#define SIGNATURE_RSA_PKCS1_SHA1         0x0201
#define SIGNATURE_ECDSA_SHA1             0x0203

enum ssl_state
{
	SSL_ST_NONE,
	SSL_ST_CONNECT,
	SSL_ST_CLIENT_HELLO,
};

struct ssl
{
	refcount_t refcount;
	uint8_t client_random[28];
	uint8_t client_id[32];
	struct ssl_ctx *ctx;
	enum ssl_state state;
};

struct ssl_ctx
{
	refcount_t refcount;
	struct bio *bio;
};

static inline int ssl_write_data(struct bio *bio, const void *data, size_t size)
{
	return bio_write(bio, data, size) == (ssize_t)size;
}

static inline int ssl_write_u8(struct bio *bio, uint8_t value)
{
	return ssl_write_data(bio, &value, sizeof(value));
}

static inline int ssl_write_u16(struct bio *bio, uint16_t value)
{
	uint8_t data[2] = {value >> 8, value >> 0};
	return ssl_write_data(bio, data, sizeof(data));
}

static inline int ssl_write_u24(struct bio *bio, uint32_t value)
{
	uint8_t data[3] = {value >> 16, value >> 8, value >> 0};
	return ssl_write_data(bio, data, sizeof(data));
}

int ssl_write_client_hello(struct ssl *ssl, struct bio *bio);
int ssl_ctx_generate_layer_record(struct bio *bio, const void *data, size_t size);

#endif
