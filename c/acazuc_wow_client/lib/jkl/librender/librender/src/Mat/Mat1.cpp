#ifndef LIBRENDER_MAT1_CPP
# define LIBRENDER_MAT1_CPP

#include "Mat1.h"

namespace librender
{

	template <typename T>
	TMat1<T>::TMat1(TVec1<T> vec1)
	: x(vec1)
	{
	}

	template <typename T>
	TMat1<T>::TMat1(T value)
	: x(value)
	{
	}

	template <typename T>
	TVec1<T> &TMat1<T>::operator [] (int i)
	{
		return (&this->x)[i];
	}

	template <typename T>
	const TVec1<T> &TMat1<T>::operator [] (int i) const
	{
		return (&this->x)[i];
	}

	template <typename T>
	TMat1<T> operator * (TMat1<T> mat1, TMat1<T> mat2)
	{
		return TMat1<T>(mat1[0][0] * mat2[0][0]);
	}

	template <typename T>
	TVec1<T> operator * (TMat1<T> mat, TVec1<T> vec)
	{
		return TVec1<T>(mat[0][0] * vec[0][0]);
	}

	template <typename T>
	TVec1<T> operator * (TVec1<T> vec, TMat1<T> mat)
	{
		return TVec1<T>(mat[0][0] * vec[0][0]);
	}

	template <typename T>
	TMat1<T> transpose(TMat1<T> mat)
	{
		return mat;
	}

	template <typename T>
	TMat1<T> inverse(TMat1<T> mat)
	{
		return TMat1<T>(T(1) / mat[0][0]);
	}

}

#endif
