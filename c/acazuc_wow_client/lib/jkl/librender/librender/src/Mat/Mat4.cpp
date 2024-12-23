#ifndef LIBRENDER_MAT4_CPP
# define LIBRENDER_MAT4_CPP

#include "Mat4.h"
#include "Mat3.h"
#include <cmath>

namespace librender
{

	template <typename T>
	TMat4<T>::TMat4(TVec4<T> vec1, TVec4<T> vec2, TVec4<T> vec3, TVec4<T> vec4)
	: x(vec1)
	, y(vec2)
	, z(vec3)
	, w(vec4)
	{
	}

	template <typename T>
	TMat4<T>::TMat4(TMat3<T> mat)
	: x(mat[0], T(0))
	, y(mat[1], T(0))
	, z(mat[2], T(0))
	, w(T(0), T(0), T(0), T(1))
	{
	}

	template <typename T>
	TMat4<T>::TMat4(TVec4<T> vec)
	: x(vec.x, T(0), T(0), T(0))
	, y(T(0), vec.y, T(0), T(0))
	, z(T(0), T(0), vec.z, T(0))
	, w(T(0), T(0), T(0), vec.w)
	{
	}

	template <typename T>
	TMat4<T>::TMat4(T value)
	: x(value, T(0), T(0), T(0))
	, y(T(0), value, T(0), T(0))
	, z(T(0), T(0), value, T(0))
	, w(T(0), T(0), T(0), value)
	{
	}

	template <typename T>
	TMat4<T> TMat4<T>::rotate(TMat4<T> mat, T angle, TVec3<T> axis)
	{
		T c(cos(angle));
		T s(sin(angle));
		TVec3<T> t(axis * (T(1) - c));
		TVec3<T> vx(t * axis.x);
		TVec3<T> u(axis * s);
		T yy(axis.y * t.y);
		T yz(axis.y * t.z);
		T zz(axis.z * t.z);
		axis = normalize(axis);
		TMat3<T> rotate;
		rotate[0][0] = vx.x + c;
		rotate[0][1] = vx.y + u.z;
		rotate[0][2] = vx.z - u.y;
		rotate[1][0] = vx.y - u.x;
		rotate[1][1] = yy + c;
		rotate[1][2] = yz + u.z;
		rotate[2][0] = vx.z + u.y;
		rotate[2][1] = yz - u.x;
		rotate[2][2] = zz + c;
		return TMat4<T>(
				mat[0] * rotate[0][0] + mat[1] * rotate[0][1] + mat[2] * rotate[0][2],
				mat[0] * rotate[1][0] + mat[1] * rotate[1][1] + mat[2] * rotate[1][2],
				mat[0] * rotate[2][0] + mat[1] * rotate[2][1] + mat[2] * rotate[2][2],
				mat[3]);
	}

	template <typename T>
	TMat4<T> TMat4<T>::rotateX(TMat4<T> mat, T angle)
	{
		T c(cos(angle));
		T s(sin(angle));
		return TMat4<T>(
				mat[0],
				mat[1] *  c + mat[2] * s,
				mat[1] * -s + mat[2] * c,
				mat[3]);
	}

	template <typename T>
	TMat4<T> TMat4<T>::rotateY(TMat4<T> mat, T angle)
	{
		T c(cos(angle));
		T s(sin(angle));
		return TMat4<T>(
				mat[0] * c + mat[2] * -s,
				mat[1],
				mat[0] * s + mat[2] *  c,
				mat[3]);
	}

	template <typename T>
	TMat4<T> TMat4<T>::rotateZ(TMat4<T> mat, T angle)
	{
		T c(cos(angle));
		T s(sin(angle));
		return TMat4<T>(
				mat[0] *  c + mat[1] * s,
				mat[0] * -s + mat[1] * c,
				mat[2],
				mat[3]);
	}

	template <typename T>
	TMat4<T> TMat4<T>::translate(TMat4<T> mat, TVec3<T> vec)
	{
		TMat4<T> result(mat);
		result[3] = mat[0] * vec[0] + mat[1] * vec[1] + mat[2] * vec[2] + mat[3];
		return result;
	}

	template <typename T>
	TMat4<T> TMat4<T>::scale(TMat4<T> mat, TVec3<T> vec)
	{
		return TMat4<T>(mat[0] * vec[0], mat[1] * vec[1], mat[2] * vec[2], mat[3]);
	}

	template <typename T>
	TMat4<T> TMat4<T>::perspective(T fov, T aspect, T znear, T zfar)
	{
		T f(T(1) / tan(fov / T(2)));
		TMat4<T> mat(TVec4<T>(f / aspect, f, (zfar + znear) / (znear - zfar), T(0)));
		mat[2][3] = T(-1);
		mat[3][2] = (T(2) * zfar * znear) / (znear - zfar);
		return mat;
	}

	template <typename T>
	TMat4<T> TMat4<T>::lookAt(TVec3<T> eye, TVec3<T> center, TVec3<T> up)
	{
		up = normalize(up);
		TVec3<T> f(normalize(center - eye));
		TVec3<T> s(cross(f, up));
		TVec3<T> u(cross(normalize(s), f));
		f = -f;
		TMat4<T> mat(
				TVec4<T>(s[0], u[0], f[0], T(0)),
				TVec4<T>(s[1], u[1], f[1], T(0)),
				TVec4<T>(s[2], u[2], f[2], T(0)),
				TVec4<T>(T(0), T(0), T(0), T(1)));
		return translate(mat, TVec3<T>(-eye));
	}

	template <typename T>
	TMat4<T> TMat4<T>::ortho(T left, T right, T bottom, T top, T tnear, T tfar)
	{
		T rml(right - left);
		T tmb(top - bottom);
		T fmn(tfar - tnear);
		TMat4<T> mat(TVec4<T>(T(2) / rml, T(2) / tmb, T(-2) / fmn, T(1)));
		mat[3][0] = -(right + left) / rml;
		mat[3][1] = -(top + bottom) / tmb;
		mat[3][2] = -(tfar + tnear) / fmn;
		return mat;
	}

	template <typename T>
	TVec4<T> &TMat4<T>::operator [] (int i)
	{
		return (&this->x)[i];
	}

	template <typename T>
	const TVec4<T> &TMat4<T>::operator [] (int i) const
	{
		return (&this->x)[i];
	}

	template <typename T>
	TMat4<T> operator * (TMat4<T> mat1, TMat4<T> mat2)
	{
		TMat4<T> result;
		for (int i = 0; i < 4; ++i)
			result[i] = mat1[0] * mat2[i][0] + mat1[1] * mat2[i][1] + mat1[2] * mat2[i][2] + mat1[3] * mat2[i][3];
		return result;
	}

	template <typename T>
	TVec4<T> operator * (TMat4<T> mat, TVec4<T> vec)
	{
		TVec4<T> result;
		for (int i = 0; i < 4; ++i)
			result[i] = vec.x * mat[0][i] + vec.y * mat[1][i] + vec.z * mat[2][i] + vec.w * mat[3][i];
		return result;
	}

	template <typename T>
	TVec4<T> operator * (TVec4<T> vec, TMat4<T> mat)
	{
		TVec4<T> result;
		for (int i = 0; i < 4; ++i)
			result[i] = vec.x * mat[i].x + vec.y * mat[i].y + vec.z * mat[i].z + vec.w * mat[i].w;
		return result;
	}

	template <typename T>
	TMat4<T> transpose(TMat4<T> mat)
	{
		return TMat4<T>(
				Vec4(mat[0][0], mat[1][0], mat[2][0], mat[3][0]),
				Vec4(mat[0][1], mat[1][1], mat[2][1], mat[3][1]),
				Vec4(mat[0][2], mat[1][2], mat[2][2], mat[3][2]),
				Vec4(mat[0][3], mat[1][3], mat[2][3], mat[3][3]));
	}

	template <typename T>
	TMat4<T> inverse(TMat4<T> mat)
	{
		T v2323(mat[2][2] * mat[3][3] - mat[3][2] * mat[2][3]);
		T v1323(mat[1][2] * mat[3][3] - mat[3][2] * mat[1][3]);
		T v1223(mat[1][2] * mat[2][3] - mat[2][2] * mat[1][3]);
		T v0323(mat[0][2] * mat[3][3] - mat[3][2] * mat[0][3]);
		T v0223(mat[0][2] * mat[2][3] - mat[2][2] * mat[0][3]);
		T v0123(mat[0][2] * mat[1][3] - mat[1][2] * mat[0][3]);
		T v2313(mat[2][1] * mat[3][3] - mat[3][1] * mat[2][3]);
		T v1313(mat[1][1] * mat[3][3] - mat[3][1] * mat[1][3]);
		T v1213(mat[1][1] * mat[2][3] - mat[2][1] * mat[1][3]);
		T v2312(mat[2][1] * mat[3][2] - mat[3][1] * mat[2][2]);
		T v1312(mat[1][1] * mat[3][2] - mat[3][1] * mat[1][2]);
		T v1212(mat[1][1] * mat[2][2] - mat[2][1] * mat[1][2]);
		T v0313(mat[0][1] * mat[3][3] - mat[3][1] * mat[0][3]);
		T v0213(mat[0][1] * mat[2][3] - mat[2][1] * mat[0][3]);
		T v0312(mat[0][1] * mat[3][2] - mat[3][1] * mat[0][2]);
		T v0212(mat[0][1] * mat[2][2] - mat[2][1] * mat[0][2]);
		T v0113(mat[0][1] * mat[1][3] - mat[1][1] * mat[0][3]);
		T v0112(mat[0][1] * mat[1][2] - mat[1][1] * mat[0][2]);
		T invDeter(T(1) / (
			+mat[0][0] * (mat[1][1] * v2323 - mat[2][1] * v1323 + mat[3][1] * v1223)
			-mat[1][0] * (mat[0][1] * v2323 - mat[2][1] * v0323 + mat[3][1] * v0223)
			+mat[2][0] * (mat[0][1] * v1323 - mat[1][1] * v0323 + mat[3][1] * v0123)
			-mat[3][0] * (mat[0][1] * v1223 - mat[1][1] * v0223 + mat[2][1] * v0123)));
		return TMat4<T>(
			TVec4<T>(
				+(mat[1][1] * v2323 - mat[2][1] * v1323 + mat[3][1] * v1223) * invDeter,
				-(mat[0][1] * v2323 - mat[2][1] * v0323 + mat[3][1] * v0223) * invDeter,
				+(mat[0][1] * v1323 - mat[1][1] * v0323 + mat[3][1] * v0123) * invDeter,
				-(mat[0][1] * v1223 - mat[1][1] * v0223 + mat[2][1] * v0123) * invDeter),
			TVec4<T>(
				-(mat[1][0] * v2323 - mat[2][0] * v1323 + mat[3][0] * v1223) * invDeter,
				+(mat[0][0] * v2323 - mat[2][0] * v0323 + mat[3][0] * v0223) * invDeter,
				-(mat[0][0] * v1323 - mat[1][0] * v0323 + mat[3][0] * v0123) * invDeter,
				+(mat[0][0] * v1223 - mat[1][0] * v0223 + mat[2][0] * v0123) * invDeter),
			TVec4<T>(
				+(mat[1][0] * v2313 - mat[2][0] * v1313 + mat[3][0] * v1213) * invDeter,
				-(mat[0][0] * v2313 - mat[2][0] * v0313 + mat[3][0] * v0213) * invDeter,
				+(mat[0][0] * v1313 - mat[1][0] * v0313 + mat[3][0] * v0113) * invDeter,
				-(mat[0][0] * v1213 - mat[1][0] * v0213 + mat[2][0] * v0113) * invDeter),
			TVec4<T>(
				-(mat[1][0] * v2312 - mat[2][0] * v1312 + mat[3][0] * v1212) * invDeter,
				+(mat[0][0] * v2312 - mat[2][0] * v0312 + mat[3][0] * v0212) * invDeter,
				-(mat[0][0] * v1312 - mat[1][0] * v0312 + mat[3][0] * v0112) * invDeter,
				+(mat[0][0] * v1212 - mat[1][0] * v0212 + mat[2][0] * v0112) * invDeter));
	}

}

#endif
