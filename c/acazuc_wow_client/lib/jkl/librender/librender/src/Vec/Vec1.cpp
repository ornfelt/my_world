#ifndef LIBRENDER_VEC1_CPP
# define LIBRENDER_VEC1_CPP

#include "Vec1.h"
#include <cmath>

namespace librender
{

	template <typename T>
	TVec1<T>::TVec1(T x)
	: x(x)
	{
	}

	template <typename T>
	TVec1<T>::TVec1()
	{
	}

	template <typename T>
	T &TVec1<T>::operator [] (int idx)
	{
		return reinterpret_cast<T*>(this)[idx];
	}

	template <typename T>
	const T &TVec1<T>::operator [] (int idx) const
	{
		return reinterpret_cast<const T*>(this)[idx];
	}

	template <typename T>
	TVec1<T> TVec1<T>::operator - () const
	{
		return TVec1<T>(-this->x);
	}

	template <typename T>
	TVec1<T> TVec1<T>::operator += (T val)
	{
		return *this = *this + val;
	}

	template <typename T>
	TVec1<T> TVec1<T>::operator -= (T val)
	{
		return *this = *this - val;
	}

	template <typename T>
	TVec1<T> TVec1<T>::operator *= (T val)
	{
		return *this = *this * val;
	}

	template <typename T>
	TVec1<T> TVec1<T>::operator /= (T val)
	{
		return *this = *this / val;
	}

	template <typename T>
	TVec1<T> TVec1<T>::operator += (TVec1<T> vec)
	{
		return *this = *this + vec;
	}

	template <typename T>
	TVec1<T> TVec1<T>::operator -= (TVec1<T> vec)
	{
		return *this = *this - vec;
	}

	template <typename T>
	TVec1<T> TVec1<T>::operator *= (TVec1<T> vec)
	{
		return *this = *this * vec;
	}

	template <typename T>
	TVec1<T> TVec1<T>::operator /= (TVec1<T> vec)
	{
		return *this = *this / vec;
	}

	template <typename T>
	bool TVec1<T>::operator == (TVec1<T> vec) const
	{
		return this->x == vec.x;
	}

	template <typename T>
	bool TVec1<T>::operator != (TVec1<T> vec) const
	{
		return !(*this == vec);
	}

	template <typename T>
	TVec1<T> operator + (TVec1<T> vec1, TVec1<T> vec2)
	{
		return TVec1<T>(vec1.x + vec2.x);
	}

	template <typename T>
	TVec1<T> operator + (TVec1<T> vec, T val)
	{
		return TVec1<T>(vec.x + val);
	}

	template <typename T>
	TVec1<T> operator + (T val, TVec1<T> vec)
	{
		return TVec1<T>(val + vec.x);
	}

	template <typename T>
	TVec1<T> operator - (TVec1<T> vec1, TVec1<T> vec2)
	{
		return TVec1<T>(vec1.x - vec2.x);
	}

	template <typename T>
	TVec1<T> operator - (TVec1<T> vec, T val)
	{
		return TVec1<T>(vec.x - val);
	}

	template <typename T>
	TVec1<T> operator - (T val, TVec1<T> vec)
	{
		return TVec1<T>(val - vec.x);
	}

	template <typename T>
	TVec1<T> operator * (TVec1<T> vec1, TVec1<T> vec2)
	{
		return TVec1<T>(vec1.x * vec2.x);
	}

	template <typename T>
	TVec1<T> operator * (TVec1<T> vec, T val)
	{
		return TVec1<T>(vec.x * val);
	}

	template <typename T>
	TVec1<T> operator * (T val, TVec1<T> vec)
	{
		return TVec1<T>(val * vec.x);
	}

	template <typename T>
	TVec1<T> operator / (TVec1<T> vec1, TVec1<T> vec2)
	{
		return TVec1<T>(vec1.x / vec2.x);
	}

	template <typename T>
	TVec1<T> operator / (TVec1<T> vec, T val)
	{
		return TVec1<T>(vec.x / val);
	}

	template <typename T>
	TVec1<T> operator / (T val, TVec1<T> vec)
	{
		return TVec1<T>(val / vec.x);
	}

	template <typename T>
	TVec1<T> min(TVec1<T> vec1, TVec1<T> vec2)
	{
		return TVec1<T>(vec1.x < vec2.x ? vec1.x : vec2.x);
	}

	template <typename T>
	TVec1<T> min(TVec1<T> vec, T val)
	{
		return TVec1<T>(val < vec.x ? val : vec.x);
	}

	template <typename T>
	TVec1<T> min(T val, TVec1<T> vec)
	{
		return TVec1<T>(val < vec.x ? val : vec.x);
	}

	template <typename T>
	TVec1<T> max(TVec1<T> vec1, TVec1<T> vec2)
	{
		return TVec1<T>(vec1.x > vec2.x ? vec1.x : vec2.x);
	}

	template <typename T>
	TVec1<T> max(TVec1<T> vec, T val)
	{
		return TVec1<T>(val > vec.x ? val : vec.x);
	}

	template <typename T>
	TVec1<T> max(T val, TVec1<T> vec)
	{
		return TVec1<T>(val > vec.x ? val : vec.x);
	}

	template <typename T>
	TVec1<T> clamp(TVec1<T> vec, T vmin, T vmax)
	{
		return max(min(vec, vmax), vmin);
	}

	template <typename T>
	TVec1<T> clamp(TVec1<T> vec, TVec1<T> vmin, TVec1<T> vmax)
	{
		return max(min(vec, vmax), vmin);
	}

	template <typename T>
	TVec1<T> mix(TVec1<T> vec1, TVec1<T> vec2, T a)
	{
		return vec1 * (T(1) - a) + vec2 * a;
	}

	template <typename T>
	TVec1<T> mod(TVec1<T> vec1, TVec1<T> vec2)
	{
		return TVec1<T>(std::fmod(vec1.x, vec2.x));
	}

	template <typename T>
	TVec1<T> mod(TVec1<T> vec, T val)
	{
		return TVec1<T>(std::fmod(vec.x, val));
	}

	template <typename T>
	TVec1<T> floor(TVec1<T> vec)
	{
		return TVec1<T>(std::floor(vec.x));
	}

	template <typename T>
	TVec1<T> round(TVec1<T> vec)
	{
		return TVec1<T>(std::round(vec.x));
	}

	template <typename T>
	TVec1<T> ceil(TVec1<T> vec)
	{
		return TVec1<T>(std::ceil(vec.x));
	}

	template <typename T>
	TVec1<T> fract(TVec1<T> vec)
	{
		return vec - floor(vec);
	}

	template<typename T>
	TVec1<T> normalize(TVec1<T> vec)
	{
		return vec / length(vec);
	}

	template<typename T>
	TVec1<T> reflect(TVec1<T> vec1, TVec1<T> vec2)
	{
		return vec1 - vec2 * T(2) * dot(vec1, vec2);
	}

	template<typename T>
	T dot(TVec1<T> vec1, TVec1<T> vec2)
	{
		return vec1.x * vec2.x;
	}

	template<typename T>
	T length(TVec1<T> vec)
	{
		return vec.x;
	}

}

#endif
