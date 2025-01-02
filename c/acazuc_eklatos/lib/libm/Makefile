BIN = libm

SRC = src/common.c \
      src/e_acos.c \
      src/e_acosf.c \
      src/e_acosh.c \
      src/e_acoshf.c \
      src/e_asin.c \
      src/e_asinf.c \
      src/e_atan2.c \
      src/e_atan2f.c \
      src/e_atanh.c \
      src/e_atanhf.c \
      src/e_cosh.c \
      src/e_coshf.c \
      src/e_expf.c \
      src/e_hypot.c \
      src/e_hypotf.c \
      src/e_j0.c \
      src/e_j0f.c \
      src/e_j1.c \
      src/e_j1f.c \
      src/e_jn.c \
      src/e_jnf.c \
      src/e_lgamma.c \
      src/e_lgamma_r.c \
      src/e_lgammaf.c \
      src/e_lgammaf_r.c \
      src/e_log2.c \
      src/e_log2f.c \
      src/e_pow.c \
      src/e_powf.c \
      src/e_rem_pio2.c \
      src/e_rem_pio2f.c \
      src/e_sinh.c \
      src/e_sinhf.c \
      src/k_cos.c \
      src/k_cosf.c \
      src/k_exp.c \
      src/k_expf.c \
      src/k_rem_pio2.c \
      src/k_sin.c \
      src/k_sinf.c \
      src/k_tan.c \
      src/k_tanf.c \
      src/s_asinh.c \
      src/s_asinhf.c \
      src/s_atan.c \
      src/s_atanf.c \
      src/s_cacos.c \
      src/s_cacosf.c \
      src/s_cacosh.c \
      src/s_cacoshf.c \
      src/s_carg.c \
      src/s_cargf.c \
      src/s_casin.c \
      src/s_casinf.c \
      src/s_casinh.c \
      src/s_casinhf.c \
      src/s_catan.c \
      src/s_catanf.c \
      src/s_catanh.c \
      src/s_catanhf.c \
      src/s_cbrt.c \
      src/s_cbrtf.c \
      src/s_ccosh.c \
      src/s_ccoshf.c \
      src/s_cexp.c \
      src/s_cexpf.c \
      src/s_cimag.c \
      src/s_cimagf.c \
      src/s_clog.c \
      src/s_clogf.c \
      src/s_conj.c \
      src/s_conjf.c \
      src/s_cosf.c \
      src/s_cpow.c \
      src/s_cpowf.c \
      src/s_cproj.c \
      src/s_cprojf.c \
      src/s_creal.c \
      src/s_crealf.c \
      src/s_csinh.c \
      src/s_csinhf.c \
      src/s_csqrt.c \
      src/s_csqrtf.c \
      src/s_ctanh.c \
      src/s_ctanhf.c \
      src/s_erf.c \
      src/s_erff.c \
      src/s_exp2.c \
      src/s_exp2f.c \
      src/s_expm1.c \
      src/s_expm1f.c \
      src/s_fabs.c \
      src/s_fabsf.c \
      src/s_fdim.c \
      src/s_fma.c \
      src/s_fmaf.c \
      src/s_fmax.c \
      src/s_fmaxf.c \
      src/s_fmin.c \
      src/s_fminf.c \
      src/s_fpclassify.c \
      src/s_frexp.c \
      src/s_frexpf.c \
      src/s_ilogb.c \
      src/s_ilogbf.c \
      src/s_isfinite.c \
      src/s_isinf.c \
      src/s_isnan.c \
      src/s_isnormal.c \
      src/s_llround.c \
      src/s_llroundf.c \
      src/s_log1p.c \
      src/s_log1pf.c \
      src/s_lround.c \
      src/s_lroundf.c \
      src/s_modf.c \
      src/s_modff.c \
      src/s_nan.c \
      src/s_nearbyint.c \
      src/s_nextafter.c \
      src/s_nextafterf.c \
      src/s_round.c \
      src/s_roundf.c \
      src/s_scalbln.c \
      src/s_signbit.c \
      src/s_signgam.c \
      src/s_sincos.c \
      src/s_sincosf.c \
      src/s_sinf.c \
      src/s_tanf.c \
      src/s_tanh.c \
      src/s_tanhf.c \
      src/s_tgammaf.c \
      src/w_cabs.c \
      src/w_cabsf.c \
      bsdsrc/b_exp.c \
      bsdsrc/b_log.c \
      bsdsrc/b_tgamma.c \

ifeq ($(ARCH), i386)

LD80 = 1
LONG = 1

SRC+= i387/e_exp.S \
      i387/e_fmod.S \
      i387/e_log.S \
      i387/e_log10.S \
      i387/e_log10f.S \
      i387/e_logf.S \
      i387/e_remainder.S \
      i387/e_remainderf.S \
      i387/e_remainderl.S \
      i387/e_sqrt.S \
      i387/e_sqrtf.S \
      i387/e_sqrtl.S \
      i387/fenv.c \
      i387/s_ceil.S \
      i387/s_ceilf.S \
      i387/s_ceill.S \
      i387/s_copysign.S \
      i387/s_copysignf.S \
      i387/s_copysignl.S \
      i387/s_cos.S \
      i387/s_floor.S \
      i387/s_floorf.S \
      i387/s_floorl.S \
      i387/s_llrint.S \
      i387/s_llrintf.S \
      i387/s_llrintl.S \
      i387/s_logb.S \
      i387/s_logbf.S \
      i387/s_logbl.S \
      i387/s_lrint.S \
      i387/s_lrintf.S \
      i387/s_lrintl.S \
      i387/s_remquo.S \
      i387/s_remquof.S \
      i387/s_remquol.S \
      i387/s_rint.S \
      i387/s_rintf.S \
      i387/s_rintl.S \
      i387/s_scalbn.S \
      i387/s_scalbnf.S \
      i387/s_scalbnl.S \
      i387/s_sin.S \
      i387/s_tan.S \
      i387/s_trunc.S \
      i387/s_truncf.S \
      i387/s_truncl.S \

SRC+= src/e_fmodf.c \
      src/e_fmodl.c \

endif

ifeq ($(ARCH), amd64)

LD80 = 1
LONG = 1

SRC+= amd64/e_fmod.S \
      amd64/e_fmodf.S \
      amd64/e_fmodl.S \
      amd64/e_remainder.S \
      amd64/e_remainderf.S \
      amd64/e_remainderl.S \
      amd64/e_sqrt.S \
      amd64/e_sqrtf.S \
      amd64/e_sqrtl.S \
      amd64/fenv.c \
      amd64/s_llrint.S \
      amd64/s_llrintf.S \
      amd64/s_llrintl.S \
      amd64/s_logbl.S \
      amd64/s_lrint.S \
      amd64/s_lrintf.S \
      amd64/s_lrintl.S \
      amd64/s_remquo.S \
      amd64/s_remquof.S \
      amd64/s_remquol.S \
      amd64/s_rintl.S \
      amd64/s_scalbn.S \
      amd64/s_scalbnf.S \
      amd64/s_scalbnl.S \

SRC+= src/e_exp.c \
      src/e_log.c \
      src/e_log10.c \
      src/e_log10f.c \
      src/e_logf.c \
      src/s_ceil.c \
      src/s_ceilf.c \
      src/s_ceill.c \
      src/s_copysign.c \
      src/s_copysignf.c \
      src/s_copysignl.c \
      src/s_cos.c \
      src/s_floor.c \
      src/s_floorf.c \
      src/s_floorl.c \
      src/s_logb.c \
      src/s_logbf.c \
      src/s_rint.c \
      src/s_rintf.c \
      src/s_sin.c \
      src/s_tan.c \
      src/s_trunc.c \
      src/s_truncf.c \
      src/s_truncl.c \

CFLAGS += -I src/amd64 \

endif

ifeq ($(ARCH), aarch64)

LD128 = 1
LONG = 1

SRC+= aarch64/fenv.c \

SRC+= src/e_fmod.c \
      src/e_fmodf.c \
      src/e_fmodl.c \
      src/e_remainder.c \
      src/e_remainderf.c \
      src/e_remainderl.c \
      src/e_sqrt.c \
      src/e_sqrtf.c \
      src/e_sqrtl.c \
      src/s_llrint.c \
      src/s_llrintf.c \
      src/s_llrintl.c \
      src/s_logbl.c \
      src/s_lrint.c \
      src/s_lrintf.c \
      src/s_lrintl.c \
      src/s_remquo.c \
      src/s_remquof.c \
      src/s_remquol.c \
      src/s_rintl.c \
      src/s_scalbn.c \
      src/s_scalbnf.c \
      src/s_scalbnl.c \
      src/e_exp.c \
      src/e_log.c \
      src/e_log10.c \
      src/e_log10f.c \
      src/e_logf.c \
      src/s_ceil.c \
      src/s_ceilf.c \
      src/s_ceill.c \
      src/s_copysign.c \
      src/s_copysignf.c \
      src/s_copysignl.c \
      src/s_cos.c \
      src/s_floor.c \
      src/s_floorf.c \
      src/s_floorl.c \
      src/s_logb.c \
      src/s_logbf.c \
      src/s_rint.c \
      src/s_rintf.c \
      src/s_sin.c \
      src/s_tan.c \
      src/s_trunc.c \
      src/s_truncf.c \
      src/s_truncl.c \

endif

ifeq ($(ARCH), arm)

SRC+= arm/fenv.c \

SRC+= src/e_fmod.c \
      src/e_fmodf.c \
      src/e_remainder.c \
      src/e_remainderf.c \
      src/e_sqrt.c \
      src/e_sqrtf.c \
      src/s_llrint.c \
      src/s_llrintf.c \
      src/s_lrint.c \
      src/s_lrintf.c \
      src/s_remquo.c \
      src/s_remquof.c \
      src/s_scalbn.c \
      src/s_scalbnf.c \
      src/e_exp.c \
      src/e_log.c \
      src/e_log10.c \
      src/e_log10f.c \
      src/e_logf.c \
      src/s_ceil.c \
      src/s_ceilf.c \
      src/s_copysign.c \
      src/s_copysignf.c \
      src/s_cos.c \
      src/s_floor.c \
      src/s_floorf.c \
      src/s_logb.c \
      src/s_logbf.c \
      src/s_rint.c \
      src/s_rintf.c \
      src/s_sin.c \
      src/s_tan.c \
      src/s_trunc.c \
      src/s_truncf.c \

endif

ifeq ($(ARCH), riscv32)

LD128 = 1
LONG = 1

SRC+= riscv64/fenv.c \

SRC+= src/e_fmod.c \
      src/e_fmodf.c \
      src/e_fmodl.c \
      src/e_remainder.c \
      src/e_remainderf.c \
      src/e_remainderl.c \
      src/e_sqrt.c \
      src/e_sqrtf.c \
      src/e_sqrtl.c \
      src/s_llrint.c \
      src/s_llrintf.c \
      src/s_llrintl.c \
      src/s_logbl.c \
      src/s_lrint.c \
      src/s_lrintf.c \
      src/s_lrintl.c \
      src/s_remquo.c \
      src/s_remquof.c \
      src/s_remquol.c \
      src/s_rintl.c \
      src/s_scalbn.c \
      src/s_scalbnf.c \
      src/s_scalbnl.c \
      src/e_exp.c \
      src/e_log.c \
      src/e_log10.c \
      src/e_log10f.c \
      src/e_logf.c \
      src/s_ceil.c \
      src/s_ceilf.c \
      src/s_ceill.c \
      src/s_copysign.c \
      src/s_copysignf.c \
      src/s_copysignl.c \
      src/s_cos.c \
      src/s_floor.c \
      src/s_floorf.c \
      src/s_floorl.c \
      src/s_logb.c \
      src/s_logbf.c \
      src/s_rint.c \
      src/s_rintf.c \
      src/s_sin.c \
      src/s_tan.c \
      src/s_trunc.c \
      src/s_truncf.c \
      src/s_truncl.c \

endif

ifeq ($(ARCH), riscv64)

LD128 = 1
LONG = 1

SRC+= riscv64/fenv.c \

SRC+= src/e_fmod.c \
      src/e_fmodf.c \
      src/e_fmodl.c \
      src/e_remainder.c \
      src/e_remainderf.c \
      src/e_remainderl.c \
      src/e_sqrt.c \
      src/e_sqrtf.c \
      src/e_sqrtl.c \
      src/s_llrint.c \
      src/s_llrintf.c \
      src/s_llrintl.c \
      src/s_logbl.c \
      src/s_lrint.c \
      src/s_lrintf.c \
      src/s_lrintl.c \
      src/s_remquo.c \
      src/s_remquof.c \
      src/s_remquol.c \
      src/s_rintl.c \
      src/s_scalbn.c \
      src/s_scalbnf.c \
      src/s_scalbnl.c \
      src/e_exp.c \
      src/e_log.c \
      src/e_log10.c \
      src/e_log10f.c \
      src/e_logf.c \
      src/s_ceil.c \
      src/s_ceilf.c \
      src/s_ceill.c \
      src/s_copysign.c \
      src/s_copysignf.c \
      src/s_copysignl.c \
      src/s_cos.c \
      src/s_floor.c \
      src/s_floorf.c \
      src/s_floorl.c \
      src/s_logb.c \
      src/s_logbf.c \
      src/s_rint.c \
      src/s_rintf.c \
      src/s_sin.c \
      src/s_tan.c \
      src/s_trunc.c \
      src/s_truncf.c \
      src/s_truncl.c \

endif

ifeq ($(ARCH), mipsel)

SRC+= mips/fenv.c \

SRC+= src/e_fmod.c \
      src/e_fmodf.c \
      src/e_remainder.c \
      src/e_remainderf.c \
      src/e_sqrt.c \
      src/e_sqrtf.c \
      src/s_llrint.c \
      src/s_llrintf.c \
      src/s_logbl.c \
      src/s_lrint.c \
      src/s_lrintf.c \
      src/s_remquo.c \
      src/s_remquof.c \
      src/s_scalbn.c \
      src/s_scalbnf.c \
      src/e_exp.c \
      src/e_log.c \
      src/e_log10.c \
      src/e_log10f.c \
      src/e_logf.c \
      src/s_ceil.c \
      src/s_ceilf.c \
      src/s_copysign.c \
      src/s_copysignf.c \
      src/s_cos.c \
      src/s_floor.c \
      src/s_floorf.c \
      src/s_logb.c \
      src/s_logbf.c \
      src/s_rint.c \
      src/s_rintf.c \
      src/s_sin.c \
      src/s_tan.c \
      src/s_trunc.c \
      src/s_truncf.c \

endif

ifeq ($(LD80), 1)

SRC+= ld80/e_acoshl.c \
      ld80/e_atanhl.c \
      ld80/e_coshl.c \
      ld80/e_expl.c \
      ld80/e_lgammal_r.c \
      ld80/e_log10l.c \
      ld80/e_log2l.c \
      ld80/e_logl.c \
      ld80/e_powl.c \
      ld80/e_sinhl.c \
      ld80/e_tgammal.c \
      ld80/invtrig.c \
      ld80/k_cosl.c \
      ld80/k_sinl.c \
      ld80/k_tanl.c \
      ld80/s_asinhl.c \
      ld80/s_erfl.c \
      ld80/s_exp2l.c \
      ld80/s_expm1l.c \
      ld80/s_log1pl.c \
      ld80/s_nanl.c \
      ld80/s_tanhl.c \

CFLAGS += -I src/ld80 \

endif

ifeq ($(LD128), 1)

SRC+= ld128/e_acoshl.c \
      ld128/e_atanhl.c \
      ld128/e_coshl.c \
      ld128/e_expl.c \
      ld128/e_lgammal_r.c \
      ld128/e_log10l.c \
      ld128/e_log2l.c \
      ld128/e_logl.c \
      ld128/e_powl.c \
      ld128/e_sinhl.c \
      ld128/e_tgammal.c \
      ld128/invtrig.c \
      ld128/k_cosl.c \
      ld128/k_sinl.c \
      ld128/k_tanl.c \
      ld128/s_asinhl.c \
      ld128/s_erfl.c \
      ld128/s_exp2l.c \
      ld128/s_expm1l.c \
      ld128/s_log1pl.c \
      ld128/s_nanl.c \
      ld128/s_tanhl.c \

CFLAGS += -I src/ld128 \

endif

ifeq ($(LONG), 1)

SRC+= src/e_acosl.c \
      src/e_asinl.c \
      src/e_atan2l.c \
      src/e_hypotl.c \
      src/e_lgammal.c \
      src/polevll.c \
      src/s_atanl.c \
      src/s_cacoshl.c \
      src/s_cacosl.c \
      src/s_cargl.c \
      src/s_casinhl.c \
      src/s_casinl.c \
      src/s_catanhl.c \
      src/s_catanl.c \
      src/s_cbrtl.c \
      src/s_ccoshl.c \
      src/s_ccosl.c \
      src/s_cexpl.c \
      src/s_cimagl.c \
      src/s_clogl.c \
      src/s_conjl.c \
      src/s_cosl.c \
      src/s_cpowl.c \
      src/s_cprojl.c \
      src/s_creall.c \
      src/s_csinhl.c \
      src/s_csinl.c \
      src/s_csqrtl.c \
      src/s_ctanhl.c \
      src/s_ctanl.c \
      src/s_fabsl.c \
      src/s_fmal.c \
      src/s_fmaxl.c \
      src/s_fminl.c \
      src/s_frexpl.c \
      src/s_ilogbl.c \
      src/s_llroundl.c \
      src/s_lroundl.c \
      src/s_modfl.c \
      src/s_nextafterl.c \
      src/s_nexttoward.c \
      src/s_nexttowardf.c \
      src/s_roundl.c \
      src/s_sincosl.c \
      src/s_sinl.c \
      src/s_tanl.c \
      src/w_cabsl.c \

endif

CFLAGS+= -I src \
         -I src/include \
         -I src/src \
         -I src/bsdsrc \
         -D__BSD_VISIBLE=1 \
         -fno-builtin

VERSION_MAJOR = 1
VERSION_MINOR = 0
VERSION_BUILD = 0

include $(MAKEDIR)/lib.mk
