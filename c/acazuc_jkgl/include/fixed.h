#ifndef FIXED_H
#define FIXED_H

enum vert_attr
{
	VERT_ATTR_X,
	VERT_ATTR_Y,
	VERT_ATTR_Z,
	VERT_ATTR_W,
	VERT_ATTR_R,
	VERT_ATTR_G,
	VERT_ATTR_B,
	VERT_ATTR_A,
	VERT_ATTR_S,
	VERT_ATTR_T,
	VERT_ATTR_P,
	VERT_ATTR_Q,
	VERT_ATTR_NX,
	VERT_ATTR_NY,
	VERT_ATTR_NZ,
};

enum vert_varying
{
	VERT_VARYING_R,
	VERT_VARYING_G,
	VERT_VARYING_B,
	VERT_VARYING_A,
	VERT_VARYING_S,
	VERT_VARYING_T,
	VERT_VARYING_P,
	VERT_VARYING_Q,
	VERT_VARYING_FOG,
};

#endif