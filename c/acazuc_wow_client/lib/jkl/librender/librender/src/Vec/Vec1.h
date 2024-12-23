#ifndef LIBRENDER_VEC1_H
# define LIBRENDER_VEC1_H

namespace librender
{

	template <typename T> class TVec1
	{

	public:
		union {T x;};
		TVec1<T>(T x);
		TVec1<T>();
		T &operator [] (int idx);
		const T &operator [] (int idx) const;
		TVec1<T> operator - () const;
		TVec1<T> operator += (T val);
		TVec1<T> operator -= (T val);
		TVec1<T> operator *= (T val);
		TVec1<T> operator /= (T val);
		TVec1<T> operator += (TVec1<T> vec);
		TVec1<T> operator -= (TVec1<T> vec);
		TVec1<T> operator *= (TVec1<T> vec);
		TVec1<T> operator /= (TVec1<T> vec);
		bool operator == (TVec1<T> vec) const;
		bool operator != (TVec1<T> vec) const;

	};

	template <typename T>
	TVec1<T> operator + (TVec1<T> vec1, TVec1<T> vec2);
	template <typename T>
	TVec1<T> operator + (TVec1<T> vec, T val);
	template <typename T>
	TVec1<T> operator + (T val, TVec1<T> vec);
	template <typename T>
	TVec1<T> operator - (TVec1<T> vec1, TVec1<T> vec2);
	template <typename T>
	TVec1<T> operator - (TVec1<T> vec, T val);
	template <typename T>
	TVec1<T> operator - (T val, TVec1<T> vec);
	template <typename T>
	TVec1<T> operator * (TVec1<T> vec1, TVec1<T> vec2);
	template <typename T>
	TVec1<T> operator * (TVec1<T> vec, T val);
	template <typename T>
	TVec1<T> operator * (T val, TVec1<T> vec);
	template <typename T>
	TVec1<T> operator / (TVec1<T> vec1, TVec1<T> vec2);
	template <typename T>
	TVec1<T> operator / (TVec1<T> vec, T val);
	template <typename T>
	TVec1<T> operator / (T val, TVec1<T> vec);

	template <typename T>
	TVec1<T> min(TVec1<T> vec1, TVec1<T> vec2);
	template <typename T>
	TVec1<T> min(TVec1<T> vec, T val);
	template <typename T>
	TVec1<T> min(T val, TVec1<T> vec);
	template <typename T>
	TVec1<T> max(TVec1<T> vec1, TVec1<T> vec2);
	template <typename T>
	TVec1<T> max(TVec1<T> vec, T val);
	template <typename T>
	TVec1<T> max(T val, TVec1<T> vec);
	template <typename T>
	TVec1<T> clamp(TVec1<T> vec, T vmin, T vmax);
	template <typename T>
	TVec1<T> clamp(TVec1<T> vec, TVec1<T> vmin, TVec1<T> vmax);

	template <typename T>
	TVec1<T> mix(TVec1<T> vec1, TVec1<T> vec2, T per);

	template <typename T>
	TVec1<T> mod(TVec1<T> vec1, TVec1<T> vec2);
	template <typename T>
	TVec1<T> mod(TVec1<T> vec, T val);
	template <typename T>
	TVec1<T> floor(TVec1<T> vec);
	template <typename T>
	TVec1<T> round(TVec1<T> vec);
	template <typename T>
	TVec1<T> ceil(TVec1<T> vec);
	template <typename T>
	TVec1<T> fract(TVec1<T> vec);

	template<typename T>
	TVec1<T> normalize(TVec1<T> vec);
	template<typename T>
	TVec1<T> reflect(TVec1<T> vec1, TVec1<T> vec2);
	template<typename T>
	T dot(TVec1<T> vec1, TVec1<T> vec2);
	template<typename T>
	T length(TVec1<T> vec);

	typedef TVec1<float> Vec1;

}

# include "Vec1.cpp"

#endif
