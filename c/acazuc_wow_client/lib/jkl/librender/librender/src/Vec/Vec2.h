#ifndef LIBRENDER_VEC2_H
# define LIBRENDER_VEC2_H

namespace librender
{

	template <typename T> class TVec2
	{

	public:
		union {T x, u;};
		union {T y, v;};
		TVec2<T>(T x, T y);
		TVec2<T>(T xy);
		TVec2<T>();
		TVec2<T> xx() const;
		TVec2<T> xy() const;
		TVec2<T> yx() const;
		TVec2<T> yy() const;
		T &operator [] (int idx);
		const T &operator [] (int idx) const;
		TVec2<T> operator - () const;
		TVec2<T> operator += (T val);
		TVec2<T> operator -= (T val);
		TVec2<T> operator *= (T val);
		TVec2<T> operator /= (T val);
		TVec2<T> operator += (TVec2<T> vec);
		TVec2<T> operator -= (TVec2<T> vec);
		TVec2<T> operator *= (TVec2<T> vec);
		TVec2<T> operator /= (TVec2<T> vec);
		bool operator == (TVec2<T> vec) const;
		bool operator != (TVec2<T> vec) const;

	};

	template <typename T>
	TVec2<T> operator + (TVec2<T> vec1, TVec2<T> vec2);
	template <typename T>
	TVec2<T> operator + (TVec2<T> vec, T val);
	template <typename T>
	TVec2<T> operator + (T val, TVec2<T> vec);
	template <typename T>
	TVec2<T> operator - (TVec2<T> vec1, TVec2<T> vec2);
	template <typename T>
	TVec2<T> operator - (TVec2<T> vec, T val);
	template <typename T>
	TVec2<T> operator - (T val, TVec2<T> vec);
	template <typename T>
	TVec2<T> operator * (TVec2<T> vec1, TVec2<T> vec2);
	template <typename T>
	TVec2<T> operator * (TVec2<T> vec, T val);
	template <typename T>
	TVec2<T> operator * (T val, TVec2<T> vec);
	template <typename T>
	TVec2<T> operator / (TVec2<T> vec1, TVec2<T> vec2);
	template <typename T>
	TVec2<T> operator / (TVec2<T> vec, T val);
	template <typename T>
	TVec2<T> operator / (T val, TVec2<T> vec);

	template <typename T>
	TVec2<T> min(TVec2<T> vec1, TVec2<T> vec2);
	template <typename T>
	TVec2<T> min(TVec2<T> vec, T val);
	template <typename T>
	TVec2<T> min(T val, TVec2<T> vec);

	template <typename T>
	TVec2<T> max(TVec2<T> vec1, TVec2<T> vec2);
	template <typename T>
	TVec2<T> max(TVec2<T> vec, T val);
	template <typename T>
	TVec2<T> max(T val, TVec2<T> vec);

	template <typename T>
	TVec2<T> clamp(TVec2<T> vec, T vmin, T vmax);
	template <typename T>
	TVec2<T> clamp(TVec2<T> vec, TVec2<T> vmin, TVec2<T> vmax);

	template <typename T>
	TVec2<T> mix(TVec2<T> vec1, TVec2<T> vec2, T per);
	template <typename T>
	TVec2<T> mod(TVec2<T> vec1, TVec2<T> vec2);
	template <typename T>
	TVec2<T> mod(TVec2<T> vec, T val);
	template <typename T>
	TVec2<T> floor(TVec2<T> vec);
	template <typename T>
	TVec2<T> round(TVec2<T> vec);
	template <typename T>
	TVec2<T> ceil(TVec2<T> vec);
	template <typename T>
	TVec2<T> fract(TVec2<T> vec);

	template<typename T>
	TVec2<T> normalize(TVec2<T> vec);
	template<typename T>
	TVec2<T> reflect(TVec2<T> vec1, TVec2<T> vec2);
	template<typename T>
	T dot(TVec2<T> vec1, TVec2<T> vec2);
	template<typename T>
	T length(TVec2<T> vec);

	typedef TVec2<float> Vec2;

}

# include "Vec2.cpp"

#endif
