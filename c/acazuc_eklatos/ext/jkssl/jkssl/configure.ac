AC_INIT([jkssl],[1.0])

AC_CONFIG_SRCDIR([src])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall foreign])

AC_PROG_CC
AM_PROG_AR
LT_INIT

case "$host" in
	*mingw*)
		windows_target=yes
		;;
	*)
		windows_target=no
		;;
esac

AM_CONDITIONAL([LWS2_32], [test "x$windows_target" = "xyes"])

AC_CHECK_HEADERS([wmmintrin.h])
AS_IF([test "x$ac_cv_header_wmmintrin_h" = "xyes"], [
	CFLAGS_save=$CFLAGS
	CFLAGS="$CFLAGS -maes"
	XXX_PROGRAM="#include <wmmintrin.h>
	int main(int argc, char **argv)
	{
		__m128i a = _mm_loadu_si128((__m128i*)(void*)main);
		__m128i b = _mm_loadu_si128((__m128i*)(void*)main);
		__m128i tmp = _mm_aesenclast_si128(a, b);
		return _mm_cvtsi128_si32(tmp);
	}"
	AC_MSG_CHECKING([if $CC supports -maes])
	AC_COMPILE_IFELSE(
		[AC_LANG_SOURCE([$XXX_PROGRAM])],
		[AC_MSG_RESULT([yes]); enable_aesni=yes],
		[AC_MSG_RESULT([no]); enable_aesni=no]
	)
	CFLAGS=$CFLAGS_save
])
AM_CONDITIONAL([ENABLE_AESNI], [test "x$enable_aesni" = "xyes"])

AC_CHECK_HEADERS([arm_neon.h])
AS_IF([test "x$ac_cv_header_arm_neon_h" = "xyes"], [
	CFLAGS_save=$CFLAGS
	CFLAGS="$CFLAGS -march=armv8-a+aes"
	XXX_PROGRAM="#include <arm_neon.h>
	int main(int argc, char **argv)
	{
		uint8x16_t a = (uint8x16_t){};
		uint8x16_t b = (uint8x16_t){};
		uint8x16_t tmp = vaeseq_u8(vaesmcq_u8(a), b);
		return vdupb_laneq_u8(tmp, 0);
	}"
	AC_MSG_CHECKING([if $CC supports -march=armv8-a+aes])
	AC_COMPILE_IFELSE(
		[AC_LANG_SOURCE([$XXX_PROGRAM])],
		[AC_MSG_RESULT([yes]); enable_aes_neon=yes],
		[AC_MSG_RESULT([no]); enable_aes_neon=no]
	)
	CFLAGS=$CFLAGS_save
])
AM_CONDITIONAL([ENABLE_AES_NEON], [test "x$enable_aes_neon" = "xyes"])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([libjkssl.pc])

AC_OUTPUT
