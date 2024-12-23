#include "BilinearTransformFilter.h"

namespace libaudio
{

	BilinearTransformFilter::BilinearTransformFilter(uint32_t channels, uint32_t sampling)
	: Filter(channels, sampling)
	, backSamples(channels, {0, 0, 0})
	, prevSamples(channels, {0, 0})
	{
	}

	BilinearTransformFilter::BilinearTransformFilter(uint32_t channels, uint32_t sampling, float a0, float a1, float a2, float b0, float b1, float b2)
	: Filter(channels, sampling)
	, backSamples(channels, {0, 0, 0})
	, prevSamples(channels, {0, 0})
	, a0(a0)
	, a1(a1)
	, a2(a2)
	, b0(b0)
	, b1(b1)
	, b2(b2)
	{
	}

	void BilinearTransformFilter::filter(float *data, uint32_t samples)
	{
		float *dst = data;
		for (uint32_t i = 0; i < samples; ++i)
		{
			for (uint32_t j = 0; j < this->channels; ++j)
			{
				this->backSamples[j][2] = this->backSamples[j][1];
				this->backSamples[j][1] = this->backSamples[j][0];
				this->backSamples[j][0] = *dst;
				float pcm =
					(b0 * this->backSamples[j][0]) +
					(b1 * this->backSamples[j][1]) +
					(b2 * this->backSamples[j][2]) -
					(a1 * this->prevSamples[j][0]) -
					(a2 * this->prevSamples[j][1]);
				pcm = std::min(1.f, std::max(-1.f, pcm));
				*dst = pcm;
				this->prevSamples[j][1] = this->prevSamples[j][0];
				this->prevSamples[j][0] = pcm;
				++dst;
			}
		}
	}

}
