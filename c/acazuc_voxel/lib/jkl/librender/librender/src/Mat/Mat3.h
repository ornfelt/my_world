#ifndef LIBRENDER_MAT3_H
# define LIBRENDER_MAT3_H

# include "../Vec/Vec3.h"

namespace librender
{

	template <typename T> class TMat3
	{

	public:
		TVec3<T> x;
		TVec3<T> y;
		TVec3<T> z;
		TMat3<T>(TVec3<T> vec1, TVec3<T> vec2, TVec3<T> vec3);
		TMat3<T>(TVec3<T> vec);
		TMat3<T>(T val);
		TMat3<T>() {};
		static TMat3<T> rotate(TMat3<T> mat, T angle, TVec3<T> axis);
		static TMat3<T> rotateX(TMat3<T> mat, T angle);
		static TMat3<T> rotateY(TMat3<T> mat, T angle);
		static TMat3<T> rotateZ(TMat3<T> mat, T angle);
		TVec3<T> &operator [] (int i);
		const TVec3<T> &operator [] (int i) const;

	};

	template <typename T>
	TMat3<T> operator * (TMat3<T> mat1, TMat3<T> mat2);
	template <typename T>
	TVec3<T> operator * (TMat3<T> mat, TVec3<T> vec);
	template <typename T>
	TVec3<T> operator * (TVec3<T> vec, TMat3<T> mat);

	template <typename T>
	TMat3<T> transpose(TMat3<T> mat);
	template <typename T>
	TMat3<T> inverse(TMat3<T> mat);

	typedef TMat3<float> Mat3;

}

# include "Mat3.cpp"

#endif
