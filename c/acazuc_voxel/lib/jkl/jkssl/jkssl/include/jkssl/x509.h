#ifndef JKSSL_X509_H
#define JKSSL_X509_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define X509_VERSION_1 0
#define X509_VERSION_2 1
#define X509_VERSION_3 2

struct asn1_octet_string;
struct asn1_bit_string;
struct asn1_integer;
struct asn1_object;
struct asn1_oid;
struct x509_name_entry;
struct x509_extension;
struct x509_pubkey;
struct x509_algor;
struct x509_name;
struct x509;
struct bio;

struct x509 *x509_new(void);
void x509_free(struct x509 *x509);
int x509_up_ref(struct x509 *x509);
int x509_get_version(const struct x509 *x509);
struct asn1_integer *x509_get_serial_number(struct x509 *x509);
const struct asn1_integer *x509_get0_serial_number(const struct x509 *x509);
int x509_add_ext(struct x509 *x509, struct x509_extension *ext);
struct x509_name *x509_get_subject_name(struct x509 *x509);
struct x509_name *x509_get_issuer_name(struct x509 *x509);
const struct asn1_time *x509_get0_not_before(const struct x509 *x509);
const struct asn1_time *x509_get0_not_after(const struct x509 *x509);
int x509_print(struct bio *bio, struct x509 *x509);
int x509_print_fp(FILE *fp, struct x509 *x509);

struct x509_algor *x509_algor_new(void);
void x509_algor_free(struct x509_algor *algor);
int x509_algor_set0(struct x509_algor *algor, struct asn1_oid *oid,
                    struct asn1_object *params);
void x509_algor_get0(struct x509_algor *algor, const struct asn1_oid **oid,
                     const struct asn1_object **params);

struct x509_name *x509_name_new(void);
void x509_name_free(struct x509_name *name);
int x509_name_add0_entry(struct x509_name *name, struct x509_name_entry *entry);
int x509_name_print(struct bio *bio, struct x509_name *name);

struct x509_name_entry *x509_name_entry_new(void);
void x509_name_entry_free(struct x509_name_entry *entry);
struct asn1_oid *x509_name_entry_get_object(struct x509_name_entry *entry);
struct asn1_string *x509_name_entry_get_data(struct x509_name_entry *entry);

struct x509_extension *x509_extension_new(void);
void x509_extension_free(struct x509_extension *extension);
struct x509_extension *x509_extension_create(struct x509_extension **extension,
                                             struct asn1_oid *oid,
                                             bool critical,
                                             struct asn1_octet_string *object);

struct x509_pubkey *x509_pubkey_new(void);
void x509_pubkey_free(struct x509_pubkey *pubkey);

struct x509_algor *a2i_x509_algor(const struct asn1_object *object,
                                  struct x509_algor **algorp);
struct asn1_object *i2a_x509_algor(struct x509_algor *algor);
struct x509_name_entry *a2i_x509_name_entry(const struct asn1_object *object,
                                            struct x509_name_entry **entryp);
struct asn1_object *i2a_x509_name_entry(struct x509_name_entry *entry);
struct x509_name *a2i_x509_name(const struct asn1_object *object,
                                struct x509_name **namep);
struct asn1_object *i2a_x509_name(struct x509_name *name);
struct x509_extension *a2i_x509_extension(const struct asn1_object *object,
                                          struct x509_extension **extensionp);
struct asn1_object *i2a_x509_extension(struct x509_extension *extension);
struct x509_pubkey *a2i_x509_pubkey(const struct asn1_object *object,
                                    struct x509_pubkey **pubkeyp);
struct asn1_object *i2a_x509_pubkey(struct x509_pubkey *pubkey);
struct x509 *a2i_x509(const struct asn1_object *object,
                      struct x509 **x509);
struct asn1_object *i2a_x509(struct x509 *x509);

#endif
