#ifndef QUATERNION_CPP
# define QUATERNION_CPP

# include "Quaternion.h"

namespace librender
{

	template <typename T>
	TQuaternion<T>::operator TMat3<T> ()
	{
		TMat3<T> result;
		T xx(this->x * this->x);
		T yy(this->y * this->y);
		T zz(this->z * this->z);
		T xz(this->x * this->z);
		T xy(this->x * this->y);
		T yz(this->y * this->z);
		T wx(this->w * this->x);
		T wy(this->w * this->y);
		T wz(this->w * this->z);
		result[0][0] = T(1) - T(2) * (yy +  zz);
		result[0][1] = T(2) * (xy + wz);
		result[0][2] = T(2) * (xz - wy);
		result[1][0] = T(2) * (xy - wz);
		result[1][1] = T(1) - T(2) * (xx +  zz);
		result[1][2] = T(2) * (yz + wx);
		result[2][0] = T(2) * (xz + wy);
		result[2][1] = T(2) * (yz - wx);
		result[2][2] = T(1) - T(2) * (xx +  yy);
		return result;

	};

	template <typename T>
	TQuaternion<T>::operator TMat4<T> ()
	{
		return TMat4<T>(static_cast<TMat3<T>>(*this));
	};

}

#endif
