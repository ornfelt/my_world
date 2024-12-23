#ifndef JKSSL_EC_H
#define JKSSL_EC_H

struct bignum_ctx;
struct ec_group;
struct ec_point;
struct bignum;

struct ec_point *ec_point_new(const struct ec_group *group);
void ec_point_free(struct ec_point *point);
int ec_point_copy(struct ec_point *dst, const struct ec_point *src);
struct ec_point *ec_point_dup(const struct ec_point *point);

int ec_point_add(const struct ec_group *group, struct ec_point *r,
                 const struct ec_point *a, const struct ec_point *b,
                 struct bignum_ctx *ctx);
int ec_point_dbl(const struct ec_group *group, struct ec_point *r,
                 const struct ec_point *a, struct bignum_ctx *ctx);
int ec_point_invert(const struct ec_group *group, struct ec_point *r,
                    const struct ec_point *a, struct bignum_ctx *ctx);
int ec_point_is_at_infinity(const struct ec_group *group,
                            const struct ec_point *p);
int ec_point_set_to_infinity(const struct ec_group *group, struct ec_point *p);
int ec_point_is_on_curve(const struct ec_group *group, const struct ec_point *p,
                         struct bignum_ctx *ctx);
int ec_point_cmp(const struct ec_group *group, const struct ec_point *a,
                 const struct ec_point *b, struct bignum_ctx *ctx);
int ec_point_mul(const struct ec_group *group, struct ec_point *r,
                 const struct bignum *n, const struct ec_point *p,
                 const struct bignum *m, struct bignum_ctx *ctx);

#endif
