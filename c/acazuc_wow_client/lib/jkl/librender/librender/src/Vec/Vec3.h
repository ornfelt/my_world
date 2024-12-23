#ifndef LIBRENDER_VEC3_H
# define LIBRENDER_VEC3_H

# include "./Vec2.h"

namespace librender
{

	template <typename T> class TVec3
	{

	public:
		union {T x, r;};
		union {T y, g;};
		union {T z, b;};
		TVec3<T>(TVec2<T> xy, T z);
		TVec3<T>(T x, TVec2<T> yz);
		TVec3<T>(T x, T y, T z);
		TVec3<T>(T xyz);
		TVec3<T>();
		TVec2<T> xx() const;
		TVec2<T> xy() const;
		TVec2<T> xz() const;
		TVec2<T> yx() const;
		TVec2<T> yy() const;
		TVec2<T> yz() const;
		TVec2<T> zx() const;
		TVec2<T> zy() const;
		TVec2<T> zz() const;
		T &operator [] (int idx);
		const T &operator [] (int idx) const;
		TVec3<T> operator - () const;
		TVec3<T> operator += (T val);
		TVec3<T> operator -= (T val);
		TVec3<T> operator *= (T val);
		TVec3<T> operator /= (T val);
		TVec3<T> operator += (TVec3<T> vec);
		TVec3<T> operator -= (TVec3<T> vec);
		TVec3<T> operator *= (TVec3<T> vec);
		TVec3<T> operator /= (TVec3<T> vec);
		bool operator == (TVec3<T> vec) const;
		bool operator != (TVec3<T> vec) const;

	};

	template <typename T>
	TVec3<T> operator + (TVec3<T> vec1, TVec3<T> vec2);
	template <typename T>
	TVec3<T> operator + (TVec3<T> vec, T val);
	template <typename T>
	TVec3<T> operator + (T val, TVec3<T> vec);
	template <typename T>
	TVec3<T> operator - (TVec3<T> vec1, TVec3<T> vec2);
	template <typename T>
	TVec3<T> operator - (TVec3<T> vec, T val);
	template <typename T>
	TVec3<T> operator - (T val, TVec3<T> vec);
	template <typename T>
	TVec3<T> operator * (TVec3<T> vec1, TVec3<T> vec2);
	template <typename T>
	TVec3<T> operator * (TVec3<T> vec, T val);
	template <typename T>
	TVec3<T> operator * (T val, TVec3<T> vec);
	template <typename T>
	TVec3<T> operator / (TVec3<T> vec1, TVec3<T> vec2);
	template <typename T>
	TVec3<T> operator / (TVec3<T> vec, T val);
	template <typename T>
	TVec3<T> operator / (T val, TVec3<T> vec);

	template <typename T>
	TVec3<T> min(TVec3<T> vec1, TVec3<T> vec2);
	template <typename T>
	TVec3<T> min(TVec3<T> vec, T val);
	template <typename T>
	TVec3<T> min(T val, TVec3<T> vec);

	template <typename T>
	TVec3<T> max(TVec3<T> vec1, TVec3<T> vec2);
	template <typename T>
	TVec3<T> max(TVec3<T> vec, T val);
	template <typename T>
	TVec3<T> max(T val, TVec3<T> vec);

	template <typename T>
	TVec3<T> clamp(TVec3<T> vec, T vmin, T vmax);
	template <typename T>
	TVec3<T> clamp(TVec3<T> vec, TVec3<T> vmin, TVec3<T> vmax);

	template <typename T>
	TVec3<T> mix(TVec3<T> vec1, TVec3<T> vec2, T per);
	template <typename T>
	TVec3<T> mod(TVec3<T> vec1, TVec3<T> vec2);
	template <typename T>
	TVec3<T> mod(TVec3<T> vec, T val);
	template <typename T>
	TVec3<T> floor(TVec3<T> vec);
	template <typename T>
	TVec3<T> round(TVec3<T> vec);
	template <typename T>
	TVec3<T> ceil(TVec3<T> vec);
	template <typename T>
	TVec3<T> fract(TVec3<T> vec);

	template<typename T>
	TVec3<T> normalize(TVec3<T> vec);
	template<typename T>
	TVec3<T> reflect(TVec3<T> vec1, TVec3<T> vec2);
	template<typename T>
	T dot(TVec3<T> vec1, TVec3<T> vec2);
	template<typename T>
	T length(TVec3<T> vec);
	template<typename T>
	TVec3<T> cross(TVec3<T> vec1, TVec3<T> vec2);

	typedef TVec3<float> Vec3;

}

# include "Vec3.cpp"

#endif
