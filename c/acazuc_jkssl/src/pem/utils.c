#include "utils/utils.h"

#include <jkssl/evp.h>
#include <jkssl/pem.h>

#include <string.h>

int pem_get_key(const struct evp_cipher *evp_cipher, uint8_t *key,
                const uint8_t *salt_iv, const char *password)
{
	return evp_bytestokey(evp_cipher, evp_md5(), salt_iv, (uint8_t*)password,
	                      strlen(password), 1, key, NULL);
}
