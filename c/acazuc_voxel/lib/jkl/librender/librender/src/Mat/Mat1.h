#ifndef LIBRENDER_MAT1_H
# define LIBRENDER_MAT1_H

# include "../Vec/Vec1.h"

namespace librender
{

	template <typename T> class TMat1
	{

	public:
		TVec1<T> x;
		TMat1(TVec1<T> vec);
		TMat1(T val);
		TMat1() {};
		TVec1<T> &operator [] (int i);
		const TVec1<T> &operator [] (int i) const;

	};

	template <typename T>
	TMat1<T> operator * (TMat1<T> mat1, TMat1<T> mat2);
	template <typename T>
	TVec1<T> operator * (TMat1<T> mat, TVec1<T> vec);
	template <typename T>
	TVec1<T> operator * (TVec1<T> vec, TMat1<T> mat);

	template <typename T>
	TMat1<T> transpose(TMat1<T> mat);
	template <typename T>
	TMat1<T> inverse(TMat1<T> mat);

	typedef TMat1<float> Mat1;

}

# include "Mat1.cpp"

#endif
