#ifndef JKSSL_SRP_H
#define JKSSL_SRP_H

struct bignum;

struct bignum *srp_calc_server_key(const struct bignum *A,
                                   const struct bignum *v,
                                   const struct bignum *u,
                                   const struct bignum *b,
                                   const struct bignum *n);
struct bignum *srp_calc_B(const struct bignum *b, const struct bignum *n,
                          const struct bignum *g, const struct bignum *v);
struct bignum *srp_calc_u(const struct bignum *A, const struct bignum *B,
                          const struct bignum *n);
struct bignum *srp_calc_client_key(const struct bignum *n,
                                   const struct bignum *B,
                                   const struct bignum *g,
                                   const struct bignum *x,
                                   const struct bignum *a,
                                   const struct bignum *u);
struct bignum *srp_calc_x(const struct bignum *s, const char *user,
                          const char *pass);
struct bignum *srp_calc_A(const struct bignum *a, const struct bignum *n,
                          const struct bignum *g);

#endif
