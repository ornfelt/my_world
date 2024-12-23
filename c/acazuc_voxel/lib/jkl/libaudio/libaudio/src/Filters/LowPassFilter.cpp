#include "LowPassFilter.h"
#define _USE_MATH_DEFINES
#include <cmath>

namespace libaudio
{

	LowPassFilter::LowPassFilter(uint32_t channels, uint32_t sampling, float cutoff, float Q)
	: BilinearTransformFilter(channels, sampling)
	{
		float w0 = 2 * M_PI * cutoff / sampling;
		float alpha = sin(w0) / (2 * Q);
		float cw0 = cos(w0);
		this->a0 = 1 + alpha;
		this->a1 = (-2 * cw0) / a0;
		this->a2 = (1 - alpha) / a0;
		this->b0 = ((1 - cw0) / 2) / a0;
		this->b1 = (1 - cw0) / a0;
		this->b2 = ((1 - cw0) / 2) / a0;
	}

}
