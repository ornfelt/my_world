#ifndef LIBRENDER_VEC3_CPP
# define LIBRENDER_VEC3_CPP

#include "Vec3.h"
#include <cmath>

namespace librender
{

	template <typename T>
	TVec3<T>::TVec3(TVec2<T> xy, T z)
	: x(xy.x)
	, y(xy.y)
	, z(z)
	{
	}

	template <typename T>
	TVec3<T>::TVec3(T x, TVec2<T> yz)
	: x(x)
	, y(yz.y)
	, z(yz.z)
	{
	}

	template <typename T>
	TVec3<T>::TVec3(T x, T y, T z)
	: x(x)
	, y(y)
	, z(z)
	{
	}

	template <typename T>
	TVec3<T>::TVec3(T xyz)
	: x(xyz)
	, y(xyz)
	, z(xyz)
	{
	}

	template <typename T>
	TVec3<T>::TVec3()
	{
	}

	template <typename T>
	TVec2<T> TVec3<T>::xx() const
	{
		return TVec2<T>(this->x, this->x);
	}

	template <typename T>
	TVec2<T> TVec3<T>::xy() const
	{
		return TVec2<T>(this->x, this->y);
	}

	template <typename T>
	TVec2<T> TVec3<T>::xz() const
	{
		return TVec2<T>(this->x, this->z);
	}

	template <typename T>
	TVec2<T> TVec3<T>::yx() const
	{
		return TVec2<T>(this->y, this->x);
	}

	template <typename T>
	TVec2<T> TVec3<T>::yy() const
	{
		return TVec2<T>(this->y, this->y);
	}

	template <typename T>
	TVec2<T> TVec3<T>::yz() const
	{
		return TVec2<T>(this->y, this->z);
	}

	template <typename T>
	TVec2<T> TVec3<T>::zx() const
	{
		return TVec2<T>(this->z, this->x);
	}

	template <typename T>
	TVec2<T> TVec3<T>::zy() const
	{
		return TVec2<T>(this->z, this->y);
	}

	template <typename T>
	TVec2<T> TVec3<T>::zz() const
	{
		return TVec2<T>(this->z, this->z);
	}

	template <typename T>
	T &TVec3<T>::operator [] (int idx)
	{
		return reinterpret_cast<T*>(this)[idx];
	}

	template <typename T>
	const T &TVec3<T>::operator [] (int idx) const
	{
		return reinterpret_cast<const T*>(this)[idx];
	}

	template <typename T>
	TVec3<T> TVec3<T>::operator - () const
	{
		return TVec3<T>(-this->x, -this->y, -this->z);
	}

	template <typename T>
	TVec3<T> TVec3<T>::operator += (T val)
	{
		return *this = *this + val;
	}

	template <typename T>
	TVec3<T> TVec3<T>::operator -= (T val)
	{
		return *this = *this - val;
	}

	template <typename T>
	TVec3<T> TVec3<T>::operator *= (T val)
	{
		return *this = *this * val;
	}

	template <typename T>
	TVec3<T> TVec3<T>::operator /= (T val)
	{
		return *this = *this / val;
	}

	template <typename T>
	TVec3<T> TVec3<T>::operator += (TVec3<T> vec)
	{
		return *this = *this + vec;
	}

	template <typename T>
	TVec3<T> TVec3<T>::operator -= (TVec3<T> vec)
	{
		return *this = *this - vec;
	}

	template <typename T>
	TVec3<T> TVec3<T>::operator *= (TVec3<T> vec)
	{
		return *this = *this * vec;
	}

	template <typename T>
	TVec3<T> TVec3<T>::operator /= (TVec3<T> vec)
	{
		return *this = *this / vec;
	}

	template <typename T>
	bool TVec3<T>::operator == (TVec3<T> vec) const
	{
		return this->x == vec.x && this->y == vec.y && this->z == vec.z;
	}

	template <typename T>
	bool TVec3<T>::operator != (TVec3<T> vec) const
	{
		return !(*this == vec);
	}

	template <typename T>
	TVec3<T> operator + (TVec3<T> vec1, TVec3<T> vec2)
	{
		return TVec3<T>(vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z);
	}

	template <typename T>
	TVec3<T> operator + (TVec3<T> vec, T val)
	{
		return TVec3<T>(vec.x + val, vec.y + val, vec.z + val);
	}

	template <typename T>
	TVec3<T> operator + (T val, TVec3<T> vec)
	{
		return TVec3<T>(val + vec.x, val + vec.y, val + vec.z);
	}

	template <typename T>
	TVec3<T> operator - (TVec3<T> vec1, TVec3<T> vec2)
	{
		return TVec3<T>(vec1.x - vec2.x, vec1.y - vec2.y, vec1.z - vec2.z);
	}

	template <typename T>
	TVec3<T> operator - (TVec3<T> vec, T val)
	{
		return TVec3<T>(vec.x - val, vec.y - val, vec.z - val);
	}

	template <typename T>
	TVec3<T> operator - (T val, TVec3<T> vec)
	{
		return TVec3<T>(val - vec.x, val - vec.y, val - vec.z);
	}

	template <typename T>
	TVec3<T> operator * (TVec3<T> vec1, TVec3<T> vec2)
	{
		return TVec3<T>(vec1.x * vec2.x, vec1.y * vec2.y, vec1.z * vec2.z);
	}

	template <typename T>
	TVec3<T> operator * (TVec3<T> vec, T val)
	{
		return TVec3<T>(vec.x * val, vec.y * val, vec.z * val);
	}

	template <typename T>
	TVec3<T> operator * (T val, TVec3<T> vec)
	{
		return TVec3<T>(val * vec.x, val * vec.y, val * vec.z);
	}

	template <typename T>
	TVec3<T> operator / (TVec3<T> vec1, TVec3<T> vec2)
	{
		return TVec3<T>(vec1.x / vec2.x, vec1.y / vec2.y, vec1.z / vec2.z);
	}

	template <typename T>
	TVec3<T> operator / (TVec3<T> vec, T val)
	{
		return TVec3<T>(vec.x / val, vec.y / val, vec.z / val);
	}

	template <typename T>
	TVec3<T> operator / (T val, TVec3<T> vec)
	{
		return TVec3<T>(val / vec.x, val / vec.y, val / vec.z);
	}

	template <typename T>
	TVec3<T> min(TVec3<T> vec1, TVec3<T> vec2)
	{
		return TVec3<T>(vec1.x < vec2.x ? vec1.x : vec2.x, vec1.y < vec2.y ? vec1.y : vec2.y, vec1.z < vec2.z ? vec1.z : vec2.z);
	}

	template <typename T>
	TVec3<T> min(TVec3<T> vec, T val)
	{
		return TVec3<T>(val < vec.x ? val : vec.x, val < vec.y ? val : vec.y, val < vec.z ? val : vec.z);
	}

	template <typename T>
	TVec3<T> min(T val, TVec3<T> vec)
	{
		return TVec3<T>(val < vec.x ? val : vec.x, val < vec.y ? val : vec.y, val < vec.z ? val : vec.z);
	}

	template <typename T>
	TVec3<T> max(TVec3<T> vec1, TVec3<T> vec2)
	{
		return TVec3<T>(vec1.x > vec2.x ? vec1.x : vec2.x, vec1.y > vec2.y ? vec1.y : vec2.y, vec1.z > vec2.z ? vec1.z : vec2.z);
	}

	template <typename T>
	TVec3<T> max(TVec3<T> vec, T val)
	{
		return TVec3<T>(val > vec.x ? val : vec.x, val > vec.y ? val : vec.y, val > vec.z ? val : vec.z);
	}

	template <typename T>
	TVec3<T> max(T val, TVec3<T> vec)
	{
		return TVec3<T>(val > vec.x ? val : vec.x, val > vec.y ? val : vec.y, val > vec.z ? val : vec.z);
	}

	template <typename T>
	TVec3<T> clamp(TVec3<T> vec, T vmin, T vmax)
	{
		return max(min(vec, vmax), vmin);
	}

	template <typename T>
	TVec3<T> clamp(TVec3<T> vec, TVec3<T> vmin, TVec3<T> vmax)
	{
		return max(min(vec, vmax), vmin);
	}

	template <typename T>
	TVec3<T> mix(TVec3<T> vec1, TVec3<T> vec2, T a)
	{
		return vec1 * (T(1) - a) + vec2 * a;
	}

	template <typename T>
	TVec3<T> mod(TVec3<T> vec1, TVec3<T> vec2)
	{
		return TVec3<T>(std::fmod(vec1.x, vec2.x), std::fmod(vec1.y, vec2.y), std::fmod(vec1.z, vec2.z));
	}

	template <typename T>
	TVec3<T> mod(TVec3<T> vec, T val)
	{
		return TVec3<T>(std::fmod(vec.x, val), std::fmod(vec.y, val), std::fmod(vec.z, val));
	}

	template <typename T>
	TVec3<T> floor(TVec3<T> vec)
	{
		return TVec3<T>(std::floor(vec.x), std::floor(vec.y), std::floor(vec.z));
	}

	template <typename T>
	TVec3<T> round(TVec3<T> vec)
	{
		return TVec3<T>(std::round(vec.x), std::round(vec.y), std::round(vec.z));
	}

	template <typename T>
	TVec3<T> ceil(TVec3<T> vec)
	{
		return TVec3<T>(std::ceil(vec.x), std::ceil(vec.y), std::ceil(vec.z));
	}

	template <typename T>
	TVec3<T> fract(TVec3<T> vec)
	{
		return vec - floor(vec);
	}

	template<typename T>
	TVec3<T> normalize(TVec3<T> vec)
	{
		return vec / length(vec);
	}

	template<typename T>
	TVec3<T> reflect(TVec3<T> vec1, TVec3<T> vec2)
	{
		return vec1 - vec2 * T(2) * dot(vec1, vec2);
	}

	template<typename T>
	T dot(TVec3<T> vec1, TVec3<T> vec2)
	{
		return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
	}

	template<typename T>
	T length(TVec3<T> vec)
	{
		return std::sqrt(dot(vec, vec));
	}

	template <typename T>
	TVec3<T> cross(TVec3<T> vec1, TVec3<T> vec2)
	{
		return TVec3<T>(vec1.y * vec2.z - vec1.z * vec2.y, vec1.z * vec2.x - vec1.x * vec2.z, vec1.x * vec2.y - vec1.y * vec2.x);
	}

}

#endif
