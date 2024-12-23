#ifndef JKS_QUATERNION_H
#define JKS_QUATERNION_H

#define QUATERNION_TO_MAT3(t, m, q) \
	do \
	{ \
		t xx = (q).x * (q).x; \
		t yy = (q).y * (q).y; \
		t zz = (q).z * (q).z; \
		t xz = (q).x * (q).z; \
		t xy = (q).x * (q).y; \
		t yz = (q).y * (q).z; \
		t wx = (q).w * (q).x; \
		t wy = (q).w * (q).y; \
		t wz = (q).w * (q).z; \
		VEC3_SET((m).x, 1 - 2 * (yy + zz), 2 * (xy + wz), 2 * (xz - wy)); \
		VEC3_SET((m).y, 2 * (xy - wz), 1 - 2 * (xx + zz), 2 * (yz + wx)); \
		VEC3_SET((m).z, 2 * (xz + wy), 2 * (yz - wx), 1 - 2 * (xx + yy)); \
	} while (0)

#define QUATERNION_TO_MAT4(t, m, q) \
	do \
	{ \
		QUATERNION_TO_MAT3(t, m, q); \
		(m).x.w = 0; \
		(m).y.w = 0; \
		(m).z.w = 0; \
		VEC4_SET((m).w, 0, 0, 0, 1); \
	} while (0)

#endif
