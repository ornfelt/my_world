//
// CMath.hpp - 
//

#pragma once

namespace NTempest
{
    class CMath 
    {
    public:
        static unsigned long __fastcall left1_(unsigned long);
        static double __fastcall logoid_(double x, double _a, double _b, double _c, double _d, double _ln2);
        static double __fastcall logoid2_(double x, double _a, double _b, double _c, double _d);
        static double __fastcall logoid10_(double x, double _a, double _b, double _c, double _d, double _ln10);
        static float __fastcall log2_(float);
        static double __fastcall log2_(double y);
        static float __fastcall exp2_(float);
        static double __fastcall exp2_(double x);
        static double __fastcall log_(double);
        static float __fastcall log10_(float);
        static double __fastcall log10_(double);
        static float __fastcall exp_(float);
        static double __fastcall exp_(double);
        static short __fastcall ftol_round_n32768_32767_(float);
        static short __fastcall ftol_n32767_32767_(float);
        static unsigned char __fastcall ftol_round_0_256_(float);
        static unsigned char __fastcall ftol_0_256_(float x);
        static unsigned char __fastcall ftol_0_1_(float);
        static long long __fastcall iabs_(long long);
        static long __fastcall iabs_(long);
        static short __fastcall iabs_(short);
        static char __fastcall iabs_(char);
        static long long __fastcall inabs_(long long);
        static long __fastcall inabs_(long);
        static short __fastcall inabs_(short);
        static char __fastcall inabs_(char);
        static float __fastcall fabs_(float);
        static double __fastcall fabs_(double);
        static float __fastcall fnabs_(float);
        static double __fastcall fnabs_(double);
        static float __fastcall fmod_(float, float);
        static double __fastcall fmod_(double, double);
        static unsigned char __fastcall fequalz_(float, float, float);
        static unsigned char __fastcall fequalz_(double, double, double);
        static unsigned char __fastcall fequal_(float, float);
        static unsigned char __fastcall fequal_(double, double);
        static unsigned char __fastcall fequal4_(float, float);
        static unsigned char __fastcall fequal4_(double, double);
        static unsigned char __fastcall fequal8_(float, float);
        static unsigned char __fastcall fequal8_(double, double);
        static unsigned char __fastcall fnotequalz_(float, float, float);
        static unsigned char __fastcall fnotequalz_(double, double, double);
        static unsigned char __fastcall fnotequal_(float, float);
        static unsigned char __fastcall fnotequal_(double, double);
        static unsigned char __fastcall fnotequal4_(float, float);
        static unsigned char __fastcall fnotequal4_(double, double);
        static unsigned char __fastcall fnotequal8_(float, float);
        static unsigned char __fastcall fnotequal8_(double, double);
        static float __fastcall fcleanupz_(float, float, float);
        static double __fastcall fcleanupz_(double, double, double);
        static float __fastcall fcleanup_(float, float);
        static double __fastcall fcleanup_(double, double);
        static float __fastcall fcleanup4_(float, float);
        static double __fastcall fcleanup4_(double, double);
        static float __fastcall fcleanup8_(float, float);
        static double __fastcall fcleanup8_(double, double);
        static unsigned long __fastcall fuint_n(float r);
        static unsigned long __fastcall fuint_(float r);
        static unsigned long __fastcall fuint_pi(float);
        static long __fastcall fint_(float);
        static long __fastcall fint_n(float);
        static long __fastcall fint_pi(float);
        static long __fastcall fint_mi(float);
        static long __fastcall fint_si(float);
        static float __fastcall int32asreal_(long);
        static long __fastcall realasint32_(float);
        static double __fastcall int64aslreal_(long long);
        static long long __fastcall lrealasint64_(double);
        static unsigned long __fastcall rotl16_(unsigned long);
        static unsigned long __fastcall rotl15_(unsigned long);
        static unsigned long __fastcall rotl14_(unsigned long);
        static unsigned long __fastcall rotl13_(unsigned long);
        static unsigned long __fastcall rotl12_(unsigned long);
        static unsigned long __fastcall rotl11_(unsigned long);
        static unsigned long __fastcall rotl10_(unsigned long);
        static unsigned long __fastcall rotl9_(unsigned long);
        static unsigned long __fastcall rotl8_(unsigned long);
        static unsigned long __fastcall rotl7_(unsigned long);
        static unsigned long __fastcall rotl6_(unsigned long);
        static unsigned long __fastcall rotl5_(unsigned long);
        static unsigned long __fastcall rotl4_(unsigned long);
        static unsigned long __fastcall rotl3_(unsigned long);
        static unsigned long __fastcall rotl2_(unsigned long);
        static unsigned long __fastcall rotl1_(unsigned long);
        static unsigned long __fastcall rotl_(unsigned long, unsigned long);
        static unsigned long __fastcall rotr_(unsigned long, unsigned long);
        static unsigned long __fastcall rotr1_(unsigned long);
        static unsigned long __fastcall rotr2_(unsigned long);
        static unsigned long __fastcall rotr3_(unsigned long);
        static unsigned long __fastcall rotr4_(unsigned long);
        static unsigned long __fastcall rotr5_(unsigned long);
        static unsigned long __fastcall rotr6_(unsigned long);
        static unsigned long __fastcall rotr7_(unsigned long);
        static unsigned long __fastcall rotr8_(unsigned long);
        static unsigned long __fastcall rotr9_(unsigned long);
        static unsigned long __fastcall rotr10_(unsigned long);
        static unsigned long __fastcall rotr11_(unsigned long);
        static unsigned long __fastcall rotr12_(unsigned long);
        static unsigned long __fastcall rotr13_(unsigned long);
        static unsigned long __fastcall rotr14_(unsigned long);
        static unsigned long __fastcall rotr15_(unsigned long);
        static unsigned long __fastcall rotr16_(unsigned long);
        static float __fastcall cos_(float);
        static double __fastcall cos_(double);
        static float __fastcall sin_(float);
        static double __fastcall sin_(double);
        static void __fastcall sincos_(double, double&, double&);
        static void __fastcall sincos_(float, float&, float&);
        static float __fastcall tan_(float);
        static double __fastcall tan_(double);
        static float __fastcall acos_(float);
        static double __fastcall acos_(double);
        static float __fastcall asin_(float);
        static double __fastcall asin_(double);
        static float __fastcall atan_(float);
        static double __fastcall atan_(double);
        static float __fastcall atan2_(float, float);
        static double __fastcall atan2_(double, double);
        static float __fastcall sinoid_(float xr, float _1_pi);
        static float __fastcall cosoid_(float xr, float _1_pi);
        static float __fastcall atanoid_(float x, float _pi_2);
        static float __fastcall pow_(float, float);
        static double __fastcall pow_(double, double);
        static float __fastcall hypot_(float, float, float, float);
        static double __fastcall hypot_(double, double, double, double);
        static float __fastcall hypot_(float, float, float);
        static double __fastcall hypot_(double, double, double);
        static float __fastcall hypot_(float, float);
        static double __fastcall hypot_(double, double);
        static float __fastcall hypotinv_(float, float, float, float);
        static double __fastcall hypotinv_(double, double, double, double);
        static float __fastcall hypotinv_(float, float, float);
        static double __fastcall hypotinv_(double, double, double);
        static float __fastcall hypotinv_(float, float);
        static double __fastcall hypotinv_(double, double);
        static unsigned char __fastcall solvequad_(float a, float b, float c, float& r1, float& r2);
        static unsigned char __fastcall solvequad_(double a, double b, double c, double& r1, double& r2);
        static void __fastcall normalize_(float& x, float& y, float& z);
        static void __fastcall normalize_(double& x, double& y, double& z);
        static void __fastcall normalize_(float& x, float& y);
        static void __fastcall normalize_(double& x, double& y);
        static unsigned char __fastcall xsectunitsphere_(double x, double y, double z, double dx, double dy, double dz, double _r2);
        static unsigned char __fastcall xsectunitcube_(double, double, double, double, double, double);
        static double __fastcall frsqrte_(double* x, unsigned long magic);
        static float __fastcall frsqrte_(float* x, unsigned long magic);
        static double __fastcall frsqrte_(double x, unsigned long magic);
        static float __fastcall frsqrte_(float x, unsigned long magic);
        static double __fastcall fres_(double* x, unsigned long magic);
        static float __fastcall fres_(float* x, unsigned long magic);
        static double __fastcall fres_(double x, unsigned long magic);
        static float __fastcall fres_(float x, unsigned long magic);
        static long __fastcall mulhw_(long x, long y);
        static unsigned long __fastcall mulhwu_(unsigned long x, unsigned long y);
        static long __fastcall div3_(long n);
        static unsigned long __fastcall div3_(unsigned long n);
        static long __fastcall div5_(long x);
        static unsigned long __fastcall div5_(unsigned long x);
        static long __fastcall div9_(long x);
        static unsigned long __fastcall div9_(unsigned long x);
        static long __fastcall min_(long a, long b, long c, long d, long e, long f, long g, long h, long i);
        static long __fastcall min_(long a, long b, long c, long d, long e);
        static long __fastcall min_(long a, long b, long c);
        static long __fastcall med_(long a, long b, long c, long d, long e, long f, long g, long h, long i);
        static long __fastcall med_(long a, long b, long c, long d, long e);
        static long __fastcall med_(long a, long b, long c);
        static long __fastcall max_(long a, long b, long c, long d, long e, long f, long g, long h, long i);
        static long __fastcall max_(long a, long b, long c, long d, long e);
        static long __fastcall max_(long a, long b, long c);
        static long __fastcall span_(long a, long b, long c, long d, long e, long f, long g, long h, long i);
        static long __fastcall span_(long a, long b, long c, long d, long e);
        static long __fastcall span_(long a, long b, long c);
        static long __fastcall mean_(long a, long b, long c, long d, long e, long f, long g, long h, long i);
        static long __fastcall mean_(long a, long b, long c, long d, long e);
        static long __fastcall mean_(long a, long b, long c);
        static double __fastcall sqrt_(double, double);
        static float __fastcall sqrt_(float, float);
        static double __fastcall sqrt_(double);
        static float __fastcall sqrt_(float x);
        static unsigned long __fastcall sqrt_(unsigned long a);
        static double __fastcall sqrtinv_(double);
        static float __fastcall sqrtinv_(float);
        static double __fastcall sqrtx_(double);
        static float __fastcall sqrtx_(float);
        static double __fastcall sqrtxinv_(double);
        static float __fastcall sqrtxinv_(float);
        static int __fastcall isnan_(double);
        static int __fastcall isinf_(double);
        static void __fastcall invertarray_(double* a, unsigned long n);
        static void __fastcall sqrtarray_(double* a, unsigned long n);
        static void __fastcall sqrtinvarray_(double* a, unsigned long n);
        static double __fastcall cbrt_(double);
        static float __fastcall cbrt_(float);
        static unsigned long __fastcall cntlzw_(unsigned long);
        static void __fastcall split_(float xr, float& xf, long& xi);
        static void __fastcall split_(double xlr, double& xf, long& xi);
        static void __fastcall splitr_(float xr, float& xf, float& xi);
        static void __fastcall splitr_(double xlr, double& xf, double& xi);
        static double __fastcall copysign_(double, double);
        static float __fastcall copysign_(float, float);
        static long __fastcall iclamp_(long, long, long);
        static long __fastcall iclamp_(long, unsigned long);
        static void __fastcall iclamp_x(unsigned long&, long, long);
        static void __fastcall iclamp_x(long&, long, long);
        static void __fastcall iclamp_x(unsigned long&, unsigned long);
        static void __fastcall iclamp_x(long&, unsigned long);
        static float __fastcall clamp_(float, float, float);
        static double __fastcall clamp_(double, double, double);
        static void __fastcall clamp_x(float&, float, float);
        static void __fastcall clamp_x(double&, double, double);
        static float __fastcall step_(float x, float a);
        static float __fastcall pulse_(float x, float a, float b);
        static float __fastcall bstep_(float x, float a, float b);
        static float __fastcall smoothstep_(float x, float a, float b);
        static double __fastcall gammai_(float x, float g);
        static double __fastcall gamma_(float x, float g);
        static double __fastcall bias_(float x, float g);
        static double __fastcall gain_(float x, float g);
        static float __fastcall sinc_(float x);
        static double __fastcall sinc_(double x);
        static float __fastcall sinc_(float x, float a);
        static double __fastcall sinc_(double x, double a);
        static float __fastcall spline_(float x, float* k, unsigned long n);
        static double __fastcall spline_(double x, double* k, unsigned long n);
        static void __fastcall Initialize();
        static void __fastcall Terminate();
    };
}