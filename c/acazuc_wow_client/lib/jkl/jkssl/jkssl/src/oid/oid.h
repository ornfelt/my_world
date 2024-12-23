#ifndef JKSSL_OID_H
#define JKSSL_OID_H

#include <stddef.h>
#include <stdint.h>

struct asn1_oid;

static const uint32_t oid_dsa[] = {1, 2, 840, 10040, 4, 1};
static const uint32_t oid_rsaEncryption[] = {1, 2, 840, 113549, 1, 1, 1};
static const uint32_t oid_sha256WithRSAEncryption[] = {1, 2, 840, 113549, 1, 1, 11};
static const uint32_t oid_commonName[] = {2, 5, 4, 3};
static const uint32_t oid_countryName[] = {2, 5, 4, 6};
static const uint32_t oid_localityName[] = {2, 5, 4, 7};
static const uint32_t oid_stateOrProvinceName[] = {2, 5, 4, 8};
static const uint32_t oid_organizationName[] = {2, 5, 4, 10};
static const uint32_t oid_organizationalUnitName[] = {2, 5, 4, 11};
static const uint32_t oid_subjectKeyIdentifier[] = {2, 5, 29, 14};
static const uint32_t oid_keyUsage[] = {2, 5, 29, 15};
static const uint32_t oid_subjectAltName[] = {2, 5, 29, 17};
static const uint32_t oid_basicConstraints[] = {2, 5, 29, 19};
static const uint32_t oid_cRLDistributionPoints[] = {2, 5, 29, 31};
static const uint32_t oid_certificatePolicies[] = {2, 5, 29, 32};
static const uint32_t oid_authorityKeyIdentifier[] = {2, 5, 29, 35};
static const uint32_t oid_extKeyUsage[] = {2, 5, 29, 37};
static const uint32_t oid_emailAddress[] = {1, 2, 840, 113549, 1, 9, 1};
static const uint32_t oid_certtype[] = {2, 16, 840, 1, 113730, 1, 1};
static const uint32_t oid_sha1[] = {1, 3, 14, 3, 2, 26};
static const uint32_t oid_sha256[] = {2, 16, 840, 1, 101, 3, 4, 2, 1};
static const uint32_t oid_sha384[] = {2, 16, 840, 1, 101, 3, 4, 2, 2};
static const uint32_t oid_sha512[] = {2, 16, 840, 1, 101, 3, 4, 2, 3};
static const uint32_t oid_sha224[] = {2, 16, 840, 1, 101, 3, 4, 2, 4};
static const uint32_t oid_sha512_224[] = {2, 16, 840, 1, 101, 3, 4, 2, 5};
static const uint32_t oid_sha512_256[] = {2, 16, 840, 1, 101, 3, 4, 2, 6};
static const uint32_t oid_ripemd160[] = {1, 3, 36, 3, 2, 1};
static const uint32_t oid_ripemd128[] = {1, 3, 36, 3, 2, 2};
static const uint32_t oid_ripemd256[] = {1, 3, 36, 3, 2, 3};
static const uint32_t oid_md2[] = {1, 2, 840, 113549, 2, 2};
static const uint32_t oid_md4[] = {1, 2, 840, 113549, 2, 3};
static const uint32_t oid_md5[] = {1, 2, 840, 113549, 2, 4};

char *oid2txt(const struct asn1_oid *oid, char *buf, size_t size);

#endif
