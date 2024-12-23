#include "EchoFilter.h"

namespace libaudio
{

	EchoFilter::EchoFilter(uint32_t channels, uint32_t sampling, float delay, float decay)
	: Filter(channels, sampling)
	, buffer(channels, std::vector<float>(delay * sampling, 0))
	, bufferPos(0)
	, decay(decay)
	{
	}

	void EchoFilter::filter(float *data, uint32_t samples)
	{
		float *dst = data;
		for (uint32_t i = 0; i < samples; ++i)
		{
			uint32_t next = (this->bufferPos + 1) % this->buffer[0].size();
			for (uint32_t j = 0; j < this->channels; ++j)
			{
				*dst = std::min(1.f, std::max(-1.f, *dst + this->buffer[j][next] * decay));
				this->buffer[j][this->bufferPos] = *dst;
				dst++;
			}
			this->bufferPos = next;
		}
	}

}
