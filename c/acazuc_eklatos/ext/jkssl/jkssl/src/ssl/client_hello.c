#include "ssl/ssl.h"

#include <arpa/inet.h>

#include <jkssl/rand.h>
#include <jkssl/bio.h>

#include <stdio.h>
#include <time.h>

static int write_cipher_suites(struct bio *bio)
{
	uint16_t cipher_suites_length;
	uint16_t cipher_suites[2];
	int ret = 0;

	cipher_suites_length = 2 * 2;
	if (!ssl_write_u16(bio, cipher_suites_length))
		goto end;
	cipher_suites[0] = htons(TLS_DHE_RSA_WITH_AES_128_CBC_SHA256);
	cipher_suites[1] = htons(TLS_DHE_RSA_WITH_AES_256_CBC_SHA256);
	if (bio_write(bio, cipher_suites, sizeof(cipher_suites)) != sizeof(cipher_suites))
		goto end;
	ret = 1;

end:
	return ret;
}

static int write_compression_methods(struct bio *bio)
{
	uint8_t compression_length;
	int ret = 0;

	compression_length = 1;
	if (!ssl_write_u8(bio, compression_length))
		goto end;
	if (!ssl_write_u8(bio, 0))
		goto end;
	ret = 1;

end:
	return ret;
}

static int write_extension_signature_algorithms(struct bio *bio)
{
	uint16_t signature_algorithms[3];
	int ret = 0;

	if (!ssl_write_u16(bio, EXTENSION_SIGNATURE_ALGORITHMS))
		goto end;
	if (!ssl_write_u16(bio, 2 + 2 * 3))
		goto end;
	if (!ssl_write_u16(bio, 2 * 3))
		goto end;
	signature_algorithms[0] = htons(SIGNATURE_RSA_PKCS1_SHA256);
	signature_algorithms[1] = htons(SIGNATURE_RSA_PKCS1_SHA384);
	signature_algorithms[2] = htons(SIGNATURE_RSA_PKCS1_SHA512);
	if (!ssl_write_data(bio, signature_algorithms, sizeof(signature_algorithms)))
		goto end;
	ret = 1;

end:
	return ret;
}

static int write_extensions(struct bio *bio)
{
	uint16_t extensions_length;
	struct buf_mem *mem;
	struct bio *extensions;
	int ret = 0;

	extensions = bio_new(bio_s_mem());
	if (!extensions)
		goto end;
	if (!write_extension_signature_algorithms(extensions))
		goto end;
	bio_get_mem_ptr(extensions, &mem);
	extensions_length = mem->size;
	if (!ssl_write_u16(bio, extensions_length))
		goto end;
	if (!ssl_write_data(bio, mem->data, mem->size))
		goto end;
	ret = 1;

end:
	bio_free(extensions);
	return ret;
}

int ssl_write_client_hello(struct ssl *ssl, struct bio *bio)
{
	uint32_t length;
	uint8_t client_id_length;
	uint32_t gmt;
	int ret = 0;

	length = 2  /* protocol version */
	       + 4  /* gmt */
	       + 28 /* random */
	       + 1  /* session id length */
	       + 32 /* session id */
	       + 2  /* cipher suites length */
	       + 4  /* cipher suites */
	       + 1  /* compression length */
	       + 1  /* compression */
	       + 2  /* ext length */
	       + 2  /* sig alg id */
	       + 2  /* sig alg length */
	       + 2  /* sig alg length */
	       + 6  /* sig alg */
	       ;
	if (!ssl_write_u8(bio, CLIENT_HELLO))
		goto end;
	if (!ssl_write_u24(bio, length))
		goto end;
	if (!ssl_write_u16(bio, TLS_VERSION_1_2))
		goto end;
	gmt = htonl(time(NULL));
	if (!ssl_write_data(bio, &gmt, sizeof(gmt)))
		goto end;
	if (rand_bytes(ssl->client_random, sizeof(ssl->client_random)) != 1)
		goto end;
	if (!ssl_write_data(bio, ssl->client_random, sizeof(ssl->client_random)))
		goto end;
	client_id_length = sizeof(ssl->client_id);
	if (!ssl_write_data(bio, &client_id_length, sizeof(client_id_length)))
		goto end;
	if (rand_bytes(ssl->client_id, sizeof(ssl->client_id)) != 1)
		goto end;
	if (!ssl_write_data(bio, ssl->client_id, sizeof(ssl->client_id)))
		goto end;
	if (!write_cipher_suites(bio))
		goto end;
	if (!write_compression_methods(bio))
		goto end;
	if (!write_extensions(bio))
		goto end;
	ret = 1;

end:
	return ret;
}
