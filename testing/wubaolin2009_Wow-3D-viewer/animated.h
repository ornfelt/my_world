#ifndef _ANIMATED_H_
#define _ANIMATED_H_
#include "vector3d.h"
#include "mpq.h"
#include "raw_model.h"
#include "raw_animated.h"

extern int globalTime;

template <class T>
class Identity {
public:
	static const T& conv(const T& t)
	{
		return t;
	}
};

// Convert opacity values stored as shorts to floating point
// I wonder why Blizzard decided to save 2 bytes by doing this
class ShortToFloat {
public:
	static const float conv(const short t)
	{
		return t/32767.0f;
	}
};

//插值函数
// interpolation functions
template<class T>
inline T interpolate(const float r, const T &v1, const T &v2)
{
	return static_cast<T>(v1*(1.0f - r) + v2*r);
}

template<class T>
inline T interpolateHermite(const float r, const T &v1, const T &v2, const T &in, const T &out)
{
	// basis functions
	float h1 = 2.0f*r*r*r - 3.0f*r*r + 1.0f;
	float h2 = -2.0f*r*r*r + 3.0f*r*r;
	float h3 = r*r*r - 2.0f*r*r + r;
	float h4 = r*r*r - r*r;

	// interpolation
	return static_cast<T>(v1*h1 + v2*h2 + in*h3 + out*h4);
}


template<class T>
inline T interpolateBezier(const float r, const T &v1, const T &v2, const T &in, const T &out)
{
	float InverseFactor = (1.0f - r);
	float FactorTimesTwo = r*r;
	float InverseFactorTimesTwo = InverseFactor*InverseFactor;
	// basis functions
	float h1 = InverseFactorTimesTwo * InverseFactor;
	float h2 = 3.0f * r * InverseFactorTimesTwo;
	float h3 = 3.0f * FactorTimesTwo * InverseFactor;
	float h4 = FactorTimesTwo * r;

	// interpolation
	return static_cast<T>(v1*h1 + v2*h2 + in*h3 + out*h4);
}

// "linear" interpolation for quaternions should be slerp by default
template<>
inline Quaternion interpolate<Quaternion>(const float r, const Quaternion &v1, const Quaternion &v2)
{
	return Quaternion::slerp(r, v1, v2);
}


typedef std::pair<size_t, size_t> AnimRange;

/*
Generic animated value class:

T is the data type to animate
D is the data type stored in the file (by default this is the same as T)
Conv is a conversion object that defines T conv(D) to convert from D to T
(by default this is an identity function)
(there might be a nicer way to do this? meh meh)
*/
#define	MAX_ANIMATED	500
template <class T, class D=T, class Conv=Identity<T> >
class Animated {
public:

	ssize_t type, seq;
	uint32 *globals;
	std::vector<size_t> times[MAX_ANIMATED];
	std::vector<T> data[MAX_ANIMATED];
	// for nonlinear interpolations:
	std::vector<T> in[MAX_ANIMATED], out[MAX_ANIMATED];
	size_t sizes; // for fix function
	bool uses(ssize_t anim)
	{
		if ((seq) && (seq>-1))
			anim = 0;
		return ((data[anim].size()) > 0);
	}

	T getValue(ssize_t anim, size_t time)
	{
		// obtain a time value and a data range
		if (seq>-1) {
			// TODO
			if ((!globals)||(!globals[seq]))
				return T();
			if (globals[seq]==0) 
				time = 0;
			else {
				//int globalTime = 0; //这个导致很多动画运行不了 改动 以后这种情况一定要记下来 我这里先试试不让time有改动
				//结果和我预料的一致 所以先用extern global吧 在main中对其进行改动好了 这个具体为什么 暂时还写不出 只能意会。
				time = globalTime % globals[seq];
			}
			anim = 0;
		}
		if (data[anim].size()>1 && times[anim].size()>1) {
			size_t t1, t2;
			size_t pos=0;
			float r;
			size_t max_time = times[anim][times[anim].size()-1];
			//if (max_time > 0)
			//	time %= max_time; // I think this might not be necessary?
			if (time > max_time) {
				pos=times[anim].size()-1;
				r = 1.0f;

				if (type == INTERPOLATION_NONE) 
					return data[anim][pos];
				else if (type == INTERPOLATION_LINEAR) 
					return interpolate<T>(r,data[anim][pos],data[anim][pos]);
				else if (type==INTERPOLATION_HERMITE){
					// INTERPOLATION_HERMITE is only used in cameras afaik?
					return interpolateHermite<T>(r,data[anim][pos],data[anim][pos],in[anim][pos],out[anim][pos]);
				}
				else if (type==INTERPOLATION_BEZIER){
					//Is this used ingame or only by custom models?
					return interpolateBezier<T>(r,data[anim][pos],data[anim][pos],in[anim][pos],out[anim][pos]);
				}
				else //this shouldn't appear!
					return data[anim][pos];
			} else {
				for (size_t i=0; i<times[anim].size()-1; i++) {
					if (time >= times[anim][i] && time < times[anim][i+1]) {
						pos = i;
						break;
					}
				}
				t1 = times[anim][pos];
				t2 = times[anim][pos+1];
				r = (time-t1)/(float)(t2-t1);

				if (type == INTERPOLATION_NONE) 
					return data[anim][pos];
				else if (type == INTERPOLATION_LINEAR) 
					return interpolate<T>(r,data[anim][pos],data[anim][pos+1]);
				else if (type==INTERPOLATION_HERMITE){
					// INTERPOLATION_HERMITE is only used in cameras afaik?
					return interpolateHermite<T>(r,data[anim][pos],data[anim][pos+1],in[anim][pos],out[anim][pos]);
				}
				else if (type==INTERPOLATION_BEZIER){
					//Is this used ingame or only by custom models?
					return interpolateBezier<T>(r,data[anim][pos],data[anim][pos+1],in[anim][pos],out[anim][pos]);
				}
				else //this shouldn't appear!
					return data[anim][pos];
			}
		} else {
			// default value
			if (data[anim].size() == 0)
				return T();
			else
				return data[anim][0];
		}
	}

	void init(AnimationBlock &b, MPQFile &f, uint32 *gs)
	{
		globals = gs;
		type = b.type;
		seq = b.seq;
		if (seq!=-1) {
			if (!gs)
				return;
			//assert(gs);
		}
		// times
		if (b.nTimes != b.nKeys)
			return;
		//assert(b.nTimes == b.nKeys);
		sizes = b.nTimes;
		if( b.nTimes == 0 )
			return;

		for(size_t j=0; j < b.nTimes; j++) {
			AnimationBlockHeader* pHeadTimes = (AnimationBlockHeader*)(f.getBuffer() + b.ofsTimes + j*sizeof(AnimationBlockHeader));

			unsigned int *ptimes = (unsigned int*)(f.getBuffer() + pHeadTimes->ofsEntrys);
			for (size_t i=0; i < pHeadTimes->nEntrys; i++)
				times[j].push_back(ptimes[i]);
		}

		// keyframes
		for(size_t j=0; j < b.nKeys; j++) {
			AnimationBlockHeader* pHeadKeys = (AnimationBlockHeader*)(f.getBuffer() + b.ofsKeys + j*sizeof(AnimationBlockHeader));

			D *keys = (D*)(f.getBuffer() + pHeadKeys->ofsEntrys);
			switch (type) {
				case INTERPOLATION_NONE:
				case INTERPOLATION_LINEAR:
					for (size_t i = 0; i < pHeadKeys->nEntrys; i++) 
						data[j].push_back(Conv::conv(keys[i]));
					break;
				case INTERPOLATION_HERMITE:
					for (size_t i = 0; i < pHeadKeys->nEntrys; i++) {
						data[j].push_back(Conv::conv(keys[i*3]));
						in[j].push_back(Conv::conv(keys[i*3+1]));
						out[j].push_back(Conv::conv(keys[i*3+2]));
					}
					break;
					//let's use same values like hermite?!?
				case INTERPOLATION_BEZIER:
					for (size_t i = 0; i < pHeadKeys->nEntrys; i++) {
						data[j].push_back(Conv::conv(keys[i*3]));
						in[j].push_back(Conv::conv(keys[i*3+1]));
						out[j].push_back(Conv::conv(keys[i*3+2]));
					}
					break;
			}
		}
	}

	void init(AnimationBlock &b, MPQFile &f, uint32 *gs, MPQFile *animfiles)
	{
		globals = gs;
		type = b.type;
		seq = b.seq;
		if (seq!=-1) {
			if (!gs)
				return;
			//assert(gs);
		}

		// times
		if (b.nTimes != b.nKeys)
			return;
		//assert(b.nTimes == b.nKeys);
		sizes = b.nTimes;
		if( b.nTimes == 0 )
			return;

		for(size_t j=0; j < b.nTimes; j++) {
			AnimationBlockHeader* pHeadTimes = (AnimationBlockHeader*)(f.getBuffer() + b.ofsTimes + j*sizeof(AnimationBlockHeader));
			uint32 *ptimes;
			if (animfiles[j].getSize() > pHeadTimes->ofsEntrys)
				ptimes = (uint32*)(animfiles[j].getBuffer() + pHeadTimes->ofsEntrys);
			else if (f.getSize() > pHeadTimes->ofsEntrys)
				ptimes = (uint32*)(f.getBuffer() + pHeadTimes->ofsEntrys);
			else
				continue;
			for (size_t i=0; i < pHeadTimes->nEntrys; i++)
				times[j].push_back(ptimes[i]);
		}

		// keyframes
		for(size_t j=0; j < b.nKeys; j++) {
			AnimationBlockHeader* pHeadKeys = (AnimationBlockHeader*)(f.getBuffer() + b.ofsKeys + j*sizeof(AnimationBlockHeader));
			assert((D*)(f.getBuffer() + pHeadKeys->ofsEntrys));
			D *keys;
			if (animfiles[j].getSize() > pHeadKeys->ofsEntrys)
				keys = (D*)(animfiles[j].getBuffer() + pHeadKeys->ofsEntrys);
			else if (f.getSize() > pHeadKeys->ofsEntrys)
				keys = (D*)(f.getBuffer() + pHeadKeys->ofsEntrys);
			else
				continue;
			switch (type) {
				case INTERPOLATION_NONE:
				case INTERPOLATION_LINEAR:
					for (size_t i = 0; i < pHeadKeys->nEntrys; i++) 
						data[j].push_back(Conv::conv(keys[i]));
					break;
				case INTERPOLATION_HERMITE:
					for (size_t i = 0; i < pHeadKeys->nEntrys; i++) {
						data[j].push_back(Conv::conv(keys[i*3]));
						in[j].push_back(Conv::conv(keys[i*3+1]));
						out[j].push_back(Conv::conv(keys[i*3+2]));
					}
					break;
				case INTERPOLATION_BEZIER:
					for (size_t i = 0; i < pHeadKeys->nEntrys; i++) {
						data[j].push_back(Conv::conv(keys[i*3]));
						in[j].push_back(Conv::conv(keys[i*3+1]));
						out[j].push_back(Conv::conv(keys[i*3+2]));
					}
					break;
			}
		}
	}
	void fix(T fixfunc(const T))
	{
		switch (type) {
			case INTERPOLATION_NONE:
			case INTERPOLATION_LINEAR:
				for (size_t i=0; i<sizes; i++) {
					for (size_t j=0; j<data[i].size(); j++) {
						data[i][j] = fixfunc(data[i][j]);
					}
				}
				break;
			case INTERPOLATION_HERMITE:
				for (size_t i=0; i<sizes; i++) {
					for (size_t j=0; j<data[i].size(); j++) {
						data[i][j] = fixfunc(data[i][j]);
						in[i][j] = fixfunc(in[i][j]);
						out[i][j] = fixfunc(out[i][j]);
					}
				}
				break;
			case INTERPOLATION_BEZIER:
				for (size_t i=0; i<sizes; i++) {
					for (size_t j=0; j<data[i].size(); j++) {
						data[i][j] = fixfunc(data[i][j]);
						in[i][j] = fixfunc(in[i][j]);
						out[i][j] = fixfunc(out[i][j]);
					}
				}
				break;
		}
	}
	friend std::ostream& operator<<(std::ostream& out, Animated& v)
	{
		if (v.sizes == 0)
			return out;
		out << "      <type>"<< v.type << "</type>" << endl;
		out << "      <seq>"<< v.seq << "</seq>" << endl;
		out << "      <anims>"<< endl;
		for(size_t j=0; j<v.sizes; j++) {
			if (j != 0) continue; // only output walk animation
			if (v.uses((unsigned int)j)) {
				out << "    <anim id=\"" << j << "\" size=\""<< v.data[j].size() <<"\">" << endl;
				for(size_t k=0; k<v.data[j].size(); k++) {
					out << "      <data time=\"" << v.times[j][k]  << "\">" << v.data[j][k] << "</data>" << endl;
				}
				out << "    </anim>" << endl;
			}
			if (v.seq > -1 && j > 0)
				break;
		}
		out << "      </anims>"<< endl;
		return out;
	}
};

typedef Animated<float,short,ShortToFloat> AnimatedShort;


//这个是一个关于颜色的 动画部分还没考虑
struct ModelColor {
	Animated<Vec3D> color;
	AnimatedShort opacity;

	void init(MPQFile &f, ModelColorDef &mcd, uint32 *global);
};

struct ModelTransparency {
	AnimatedShort trans;

	void init(MPQFile &f, ModelTransDef &mtd, uint32 *global);
};

//Texanim 1月26日加入
class TextureAnim {
public:
	Animated<Vec3D> trans, rot, scale;

	Vec3D tval, rval, sval;

	void calc(SSIZE_T anim, size_t time);
	void init(MPQFile &f, ModelTexAnimDef &mta, uint32 *global);
	void setup(SSIZE_T anim,LPDIRECT3DDEVICE9 p_d3ddevice);
};

#endif