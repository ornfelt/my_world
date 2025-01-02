#ifndef JKSSL_OID_H
#define JKSSL_OID_H

struct asn1_oid;

char *oid2txt(const struct asn1_oid *oid, char *buf, size_t size);

#endif
