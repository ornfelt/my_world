#include "asn1/asn1.h"
#include "oid/oid.h"

#include <inttypes.h>
#include <string.h>
#include <stdio.h>

char *oid2txt(const struct asn1_oid *oid, char *buf, size_t size)
{
#define OID_TEST(n) \
	if (!asn1_oid_cmp_uint32(oid, oid_##n, sizeof(oid_##n) / sizeof(*oid_##n))) \
	{ \
		snprintf(buf, size, "%s", #n); \
		return buf; \
	} \

	OID_TEST(dsa);
	OID_TEST(rsaEncryption);
	OID_TEST(sha256WithRSAEncryption);
	OID_TEST(commonName);
	OID_TEST(countryName);
	OID_TEST(localityName);
	OID_TEST(stateOrProvinceName);
	OID_TEST(organizationName);
	OID_TEST(organizationalUnitName);
	OID_TEST(subjectKeyIdentifier);
	OID_TEST(keyUsage);
	OID_TEST(subjectAltName);
	OID_TEST(basicConstraints);
	OID_TEST(cRLDistributionPoints);
	OID_TEST(certificatePolicies);
	OID_TEST(authorityKeyIdentifier);
	OID_TEST(extKeyUsage);
	OID_TEST(emailAddress);
	OID_TEST(certtype);
	OID_TEST(rsa);
	OID_TEST(ecdh);
	OID_TEST(ecdsa);
	OID_TEST(ecPublicKey);
	OID_TEST(x9_62_prime_field);
	OID_TEST(pkcs5_pbkdf2);
	OID_TEST(pkcs5_pbes2);

	OID_TEST(sha1);
	OID_TEST(sha256);
	OID_TEST(sha384);
	OID_TEST(sha512);
	OID_TEST(sha224);
	OID_TEST(sha512_224);
	OID_TEST(sha512_256);
	OID_TEST(ripemd160);
	OID_TEST(ripemd128);
	OID_TEST(ripemd256);
	OID_TEST(md2);
	OID_TEST(md4);
	OID_TEST(md5);

	OID_TEST(sect163k1);
	OID_TEST(sect163r1);
	OID_TEST(sect239k1);
	OID_TEST(sect113r1);
	OID_TEST(sect113r2);
	OID_TEST(secp112r1);
	OID_TEST(secp112r2);
	OID_TEST(secp160r1);
	OID_TEST(secp160k1);
	OID_TEST(secp256k1);
	OID_TEST(sect163r2);
	OID_TEST(sect283k1);
	OID_TEST(sect283r1);
	OID_TEST(sect131r1);
	OID_TEST(sect131r2);
	OID_TEST(sect193r1);
	OID_TEST(sect193r2);
	OID_TEST(sect233k1);
	OID_TEST(sect233r1);
	OID_TEST(secp128r1);
	OID_TEST(secp128r2);
	OID_TEST(secp160r2);
	OID_TEST(secp192k1);
	OID_TEST(secp224k1);
	OID_TEST(secp224r1);
	OID_TEST(secp384r1);
	OID_TEST(secp521r1);
	OID_TEST(sect409k1);
	OID_TEST(sect409r1);
	OID_TEST(sect571k1);
	OID_TEST(sect571r1);
	OID_TEST(prime192v1);
	OID_TEST(prime192v2);
	OID_TEST(prime192v3);
	OID_TEST(prime239v1);
	OID_TEST(prime239v2);
	OID_TEST(prime239v3);
	OID_TEST(prime256v1);

	OID_TEST(brainpoolP160r1);
	OID_TEST(brainpoolP160t1);
	OID_TEST(brainpoolP192r1);
	OID_TEST(brainpoolP192t1);
	OID_TEST(brainpoolP224r1);
	OID_TEST(brainpoolP224t1);
	OID_TEST(brainpoolP256r1);
	OID_TEST(brainpoolP256t1);
	OID_TEST(brainpoolP320r1);
	OID_TEST(brainpoolP320t1);
	OID_TEST(brainpoolP384r1);
	OID_TEST(brainpoolP384t1);
	OID_TEST(brainpoolP512r1);
	OID_TEST(brainpoolP512t1);

	OID_TEST(frp256v1);
	OID_TEST(sm2);
	OID_TEST(gost3410_12_256);
	OID_TEST(gost3410_12_512);

	OID_TEST(aes128_ecb);
	OID_TEST(aes128_cbc);
	OID_TEST(aes128_ofb);
	OID_TEST(aes128_cfb);
	OID_TEST(aes128_wrap);
	OID_TEST(aes128_gcm);
	OID_TEST(aes128_ccm);
	OID_TEST(aes128_wrap_pad);
	OID_TEST(aes128_gmac);
	OID_TEST(aes192_ecb);
	OID_TEST(aes192_cbc);
	OID_TEST(aes192_ofb);
	OID_TEST(aes192_cfb);
	OID_TEST(aes192_wrap);
	OID_TEST(aes192_gcm);
	OID_TEST(aes192_ccm);
	OID_TEST(aes192_wrap_pad);
	OID_TEST(aes192_gmac);
	OID_TEST(aes256_ecb);
	OID_TEST(aes256_cbc);
	OID_TEST(aes256_ofb);
	OID_TEST(aes256_cfb);
	OID_TEST(aes256_wrap);
	OID_TEST(aes256_gcm);
	OID_TEST(aes256_ccm);
	OID_TEST(aes256_wrap_pad);
	OID_TEST(aes256_gmac);

	OID_TEST(aria128_ecb);
	OID_TEST(aria128_cbc);
	OID_TEST(aria128_cfb);
	OID_TEST(aria128_ofb);
	OID_TEST(aria128_ctr);
	OID_TEST(aria192_ecb);
	OID_TEST(aria192_cbc);
	OID_TEST(aria192_cfb);
	OID_TEST(aria192_ofb);
	OID_TEST(aria192_ctr);
	OID_TEST(aria256_ecb);
	OID_TEST(aria256_cbc);
	OID_TEST(aria256_cfb);
	OID_TEST(aria256_ofb);
	OID_TEST(aria256_ctr);
	OID_TEST(aria128_gcm);
	OID_TEST(aria192_gcm);
	OID_TEST(aria256_gcm);
	OID_TEST(aria128_ccm);
	OID_TEST(aria192_ccm);
	OID_TEST(aria256_ccm);

	OID_TEST(camellia128_cbc);
	OID_TEST(camellia192_cbc);
	OID_TEST(camellia256_cbc);
	OID_TEST(camellia128_wrap);
	OID_TEST(camellia192_wrap);
	OID_TEST(camellia256_wrap);
	OID_TEST(camellia128_ecb);
	OID_TEST(camellia128_ofb);
	OID_TEST(camellia128_cfb);
	OID_TEST(camellia128_gcm);
	OID_TEST(camellia128_ccm);
	OID_TEST(camellia128_ctr);
	OID_TEST(camellia128_cmac);
	OID_TEST(camellia192_ecb);
	OID_TEST(camellia192_ofb);
	OID_TEST(camellia192_cfb);
	OID_TEST(camellia192_gcm);
	OID_TEST(camellia192_ccm);
	OID_TEST(camellia192_ctr);
	OID_TEST(camellia192_cmac);
	OID_TEST(camellia256_ecb);
	OID_TEST(camellia256_ofb);
	OID_TEST(camellia256_cfb);
	OID_TEST(camellia256_gcm);
	OID_TEST(camellia256_ccm);
	OID_TEST(camellia256_ctr);
	OID_TEST(camellia256_cmac);

	OID_TEST(serpent128_ecb);
	OID_TEST(serpent128_cbc);
	OID_TEST(serpent128_ofb);
	OID_TEST(serpent128_cfb);
	OID_TEST(serpent192_ecb);
	OID_TEST(serpent192_cbc);
	OID_TEST(serpent192_ofb);
	OID_TEST(serpent192_cfb);
	OID_TEST(serpent256_ecb);
	OID_TEST(serpent256_cbc);
	OID_TEST(serpent256_ofb);
	OID_TEST(serpent256_cfb);

	OID_TEST(des_ecb);
	OID_TEST(des_cbc);
	OID_TEST(des_ofb);
	OID_TEST(des_cfb);

	OID_TEST(bf_ecb);
	OID_TEST(bf_cbc);
	OID_TEST(bf_ofb);
	OID_TEST(bf_cfb);

	OID_TEST(rc2_cbc);
	OID_TEST(rc2_ecb);
	OID_TEST(des_ede3_cbc);
	OID_TEST(cast5_cbc);
	OID_TEST(rc5_cbc);

#undef OID_TEST

	/* no strlcat :( */
	char *org = buf;
	buf[0] = '\0';
	for (size_t i = 0; i < oid->values_count; ++i)
	{
		if (i && size > 1)
		{
			buf[0] = '.';
			buf[1] = '\0';
			buf++;
			size--;
		}
		snprintf(buf, size, "%" PRIu32, oid->values[i]);
		size_t len = strlen(buf);
		buf += len;
		size -= len;
	}
	return org;
}
