#include "CachedPlayer.h"

namespace libaudio
{

	CachedPlayer::CachedPlayer(float *data, uint32_t length, uint32_t sampling, uint32_t channels)
	: position(0)
	, channels(channels)
	, length(length)
	, data(data)
	{
		this->sampling = sampling;
	}

	void CachedPlayer::getData(float *out, uint32_t outLen, uint32_t inLen)
	{
		if (!this->length)
			return;
		float ratio = inLen / static_cast<float>(outLen);
		for (uint32_t i = 0; i < outLen; i += 2)
		{
			uint32_t pos = this->position + i * ratio;
			if (pos >= this->length)
			{
				if (!this->loop)
				{
					out[i] = 0;
					out[i + 1] = 0;
					continue;
				}
				pos %= this->length;
			}
			out[i] = this->data[pos];
			out[i + 1] = this->data[pos + 1];
		}
		this->position += inLen;
		if (this->loop)
			this->position %= this->length;
	}

}
