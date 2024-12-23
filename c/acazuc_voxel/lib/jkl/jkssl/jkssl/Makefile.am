lib_LTLIBRARIES = libjkssl.la

bin_PROGRAMS = jkssl

libjkssl_la_SOURCES = src/refcount.h \
                      src/adler32/adler32.c \
                      src/adler32/adler32.h \
                      src/adler32/evp.c \
                      src/aes/aes.c \
                      src/aes/aes.h \
                      src/aes/evp.c \
                      src/aria/aria.c \
                      src/aria/aria.h \
                      src/aria/evp.c \
                      src/asn1/asn1.c \
                      src/asn1/asn1.h \
                      src/base64/decode.c \
                      src/base64/encode.c \
                      src/bf/bf.c \
                      src/bf/bf.h \
                      src/bf/evp.c \
                      src/bignum/add.c \
                      src/bignum/bit.c \
                      src/bignum/cmp.c \
                      src/bignum/conv.c \
                      src/bignum/ctx.c \
                      src/bignum/div_mod.c \
                      src/bignum/exp.c \
                      src/bignum/gcd.c \
                      src/bignum/gencb.c \
                      src/bignum/mod.c \
                      src/bignum/mod_exp.c \
                      src/bignum/mod_inverse.c \
                      src/bignum/mul.c \
                      src/bignum/prime.c \
                      src/bignum/print.c \
                      src/bignum/rand.c \
                      src/bignum/shift.c \
                      src/bignum/sub.c \
                      src/bignum/utils.c \
                      src/bio/base64.c \
                      src/bio/bio.c \
                      src/bio/bio.h \
                      src/bio/cipher.c \
                      src/bio/fd.c \
                      src/bio/fp.c \
                      src/bio/md.c \
                      src/bio/mem.c \
                      src/bio/null.c \
                      src/camellia/camellia.c \
                      src/camellia/camellia.h \
                      src/camellia/evp.c \
                      src/cast/cast.c \
                      src/cast/cast.h \
                      src/cast/evp.c \
                      src/chacha20/chacha20.c \
                      src/chacha20/chacha20.h \
                      src/chacha20/evp.c \
                      src/crc32/crc32.c \
                      src/crc32/crc32.h \
                      src/crc32/evp.c \
                      src/der/decode.c \
                      src/der/der.h \
                      src/der/encode.c \
                      src/der/pkey.c \
                      src/des/des.c \
                      src/des/des.h \
                      src/des/evp.c \
                      src/dh/asn1.c \
                      src/dh/dh.c \
                      src/dsa/asn1.c \
                      src/dsa/dsa.c \
                      src/dsa/genkey.c \
                      src/dsa/param.c \
                      src/ec/point.c \
                      src/ec/ec.h \
                      src/evp/bytestokey.c \
                      src/evp/cipher.c \
                      src/evp/cipher.h \
                      src/evp/encode.h \
                      src/evp/md.c \
                      src/evp/md.h \
                      src/evp/pkey.c \
                      src/evp/pkey.h \
                      src/hmac/hmac.c \
                      src/md/evp.c \
                      src/md/md.h \
                      src/md/md2.c \
                      src/md/md4.c \
                      src/md/md5.c \
                      src/oid/oid.c \
                      src/oid/oid.h \
                      src/pbkdf2/pbkdf2.c \
                      src/pem/asn1.c \
                      src/pem/pem.h \
                      src/pem/pkey.c \
                      src/pem/read.c \
                      src/pem/utils.c \
                      src/pem/write.c \
                      src/pkcs1.5/pkcs1.5.h \
                      src/pkcs1.5/pad.c \
                      src/pkcs1.5/unpad.c \
                      src/rand/rand.c \
                      src/rc2/evp.c \
                      src/rc2/rc2.c \
                      src/rc2/rc2.h \
                      src/rc4/evp.c \
                      src/rc4/rc4.c \
                      src/rc4/rc4.h \
                      src/ripemd/evp.c \
                      src/ripemd/ripemd.h \
                      src/ripemd/ripemd128.c \
                      src/ripemd/ripemd160.c \
                      src/ripemd/ripemd256.c \
                      src/ripemd/ripemd320.c \
                      src/rsa/asn1.c \
                      src/rsa/genkey.c \
                      src/rsa/rsa.c \
                      src/seed/evp.c \
                      src/seed/seed.c \
                      src/seed/seed.h \
                      src/serpent/evp.c \
                      src/serpent/serpent.c \
                      src/serpent/serpent.h \
                      src/sha/evp.c \
                      src/sha/sha.h \
                      src/sha/sha0.c \
                      src/sha/sha1.c \
                      src/sha/sha224.c \
                      src/sha/sha256.c \
                      src/sha/sha384.c \
                      src/sha/sha512.c \
                      src/sm3/evp.c \
                      src/sm3/sm3.c \
                      src/sm3/sm3.h \
                      src/sm4/evp.c \
                      src/sm4/sm4.c \
                      src/sm4/sm4.h \
                      src/srp/srp.c \
                      src/ssl/ssl.c \
                      src/utils/ask_password.c \
                      src/utils/hex_bin.c \
                      src/utils/utils.h \
                      src/x509/asn1.c \
                      src/x509/x509.c \
                      src/x509/x509.h
libjkssl_la_CFLAGS = -isystem $(srcdir)/include -iquote $(srcdir)/src
libjkssl_la_LDFLAGS = -Wl,--version-script=$(srcdir)/libjkssl.map

jkssl_SOURCES = src/main.c \
                src/cmd/asn1parse.c \
                src/cmd/base64.c \
                src/cmd/bignum.c \
                src/cmd/cmd.h \
                src/cmd/dgst.c \
                src/cmd/dhparam.c \
                src/cmd/dsa.c \
                src/cmd/dsaparam.c \
                src/cmd/enc.c \
                src/cmd/gendsa.c \
                src/cmd/genrsa.c \
                src/cmd/help.c \
                src/cmd/rsa.c \
                src/cmd/rsautl.c \
                src/cmd/s_client.c \
                src/cmd/speed.c \
                src/cmd/x509.c

jkssl_CFLAGS = -I$(srcdir)/include -I$(srcdir)/src
jkssl_LDADD = libjkssl.la

if LWS2_32
jkssl_LDADD+= -lws2_32
endif

pkgincludedir = $(includedir)/jkssl
pkginclude_HEADERS = include/jkssl/asn1.h \
                     include/jkssl/bignum.h \
                     include/jkssl/bio.h \
                     include/jkssl/der.h \
                     include/jkssl/dh.h \
                     include/jkssl/dsa.h \
                     include/jkssl/ec.h \
                     include/jkssl/evp.h \
                     include/jkssl/evp_cipher.h \
                     include/jkssl/evp_encode.h \
                     include/jkssl/evp_md.h \
                     include/jkssl/evp_pkey.h \
                     include/jkssl/hmac.h \
                     include/jkssl/pem.h \
                     include/jkssl/rand.h \
                     include/jkssl/rsa.h \
                     include/jkssl/srp.h \
                     include/jkssl/ssl.h \
                     include/jkssl/x509.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libjkssl.pc

EXTRA_DIST = LICENSE libjkssl.map
AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
