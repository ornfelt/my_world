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
	OID_TEST(subjectKeyIdentifier);
	OID_TEST(keyUsage);
	OID_TEST(subjectAltName);
	OID_TEST(basicConstraints);
	OID_TEST(cRLDistributionPoints);
	OID_TEST(certificatePolicies);
	OID_TEST(authorityKeyIdentifier);
	OID_TEST(extKeyUsage);
	OID_TEST(certtype);

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
