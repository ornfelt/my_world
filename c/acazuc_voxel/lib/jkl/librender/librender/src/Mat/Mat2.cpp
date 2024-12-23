#ifndef LIBRENDER_MAT2_CPP
# define LIBRENDER_MAT2_CPP

#include "Mat2.h"

namespace librender
{

	template <typename T>
	TMat2<T>::TMat2(TVec2<T> vec1, TVec2<T> vec2)
	: x(vec1)
	, y(vec2)
	{
	}

	template <typename T>
	TMat2<T>::TMat2(TVec2<T> vec)
	: x(vec.x, T(0))
	, y(T(0), vec.y)
	{
	}

	template <typename T>
	TMat2<T>::TMat2(T value)
	: x(value, T(0))
	, y(T(0), value)
	{
	}

	template <typename T>
	TVec2<T> &TMat2<T>::operator [] (int i)
	{
		return (&this->x)[i];
	}

	template <typename T>
	const TVec2<T> &TMat2<T>::operator [] (int i) const
	{
		return (&this->x)[i];
	}

	template <typename T>
	TMat2<T> operator * (TMat2<T> mat1, TMat2<T> mat2)
	{
		TMat2<T> result;
		for (int i = 0; i < 2; ++i)
			result[i] = mat1[0] * mat2[i][0] + mat1[1] * mat2[i][1];
		return result;
	}

	template <typename T>
	TVec2<T> operator * (TMat2<T> mat, TVec2<T> vec)
	{
		TVec2<T> result;
		for (int i = 0; i < 2; ++i)
			result[i] = vec.x * mat[0][i] + vec.y * mat[1][i];
		return result;
	}

	template <typename T>
	TVec2<T> operator * (TVec2<T> vec, TMat2<T> mat)
	{
		TVec2<T> result;
		for (int i = 0; i < 2; ++i)
			result[i] = vec.x * mat[i].x + vec.y * mat[i].y;
		return result;
	}

	template <typename T>
	TMat2<T> transpose(TMat2<T> mat)
	{
		return TMat2<T>(
				TVec2<T>(mat[0][0], mat[1][0]),
				TVec2<T>(mat[0][1], mat[1][1]));
	}

	template <typename T>
	TMat2<T> inverse(TMat2<T> mat)
	{
		T invDeter(T(1) / (mat[0][0] * mat[1][1] - mat[1][0] * mat[0][1]));
		return TMat2<T>(
				TVec2<T>( mat[1][1] * invDeter, -mat[0][1] * invDeter),
				TVec2<T>(-mat[1][0] * invDeter,  mat[0][0] * invDeter));
	}

}

#endif
