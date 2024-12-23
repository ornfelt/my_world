#include "DirectPlayer.h"
#include "Exception.h"
#include <cmath>

namespace libaudio
{

	DirectPlayer::DirectPlayer(std::string file)
	{
		this->file = std::fopen(file.c_str(), "rb");
		if (!this->file)
			throw Exception("fopen() error");
		int ov_error = ov_open_callbacks(this->file, &this->vorbisFile, NULL, 0, OV_CALLBACKS_NOCLOSE);
		if (ov_error)
		{
			std::fclose(this->file);
			throw Exception("ov_open_callbacks() error: " + std::to_string(ov_error));
		}
		if (!(this->vorbisInfos = ov_info(&this->vorbisFile, -1)))
		{
			ov_clear(&this->vorbisFile);
			std::fclose(this->file);
			throw Exception("ov_info() error");
		}
		this->sampling = this->vorbisInfos->rate;
	}

	DirectPlayer::~DirectPlayer()
	{
		std::fclose(this->file);
		ov_clear(&this->vorbisFile);
	}

	void DirectPlayer::getData(float *out, uint32_t outLen, uint32_t inLen)
	{
		uint32_t readed = 0;
		float ratio = inLen / static_cast<float>(outLen);
		while (readed < outLen)
		{
			float **tmp;
			long ret = ov_read_float(&vorbisFile, &tmp, (outLen - readed) / this->vorbisInfos->channels * ratio, NULL);
			if (ret < 0)
			{
				stop();
				for (uint32_t i = readed; i < outLen; ++i)
					out[i] = 0;
				return;
			}
			if (ret == 0)
			{
				if (ov_raw_seek(&this->vorbisFile, 0))
				{
					stop();
					for (uint32_t i = readed; i < outLen; ++i)
						out[i] = 0;
					return;
				}
				continue;
			}
			long outreaded = std::ceil(ret / ratio);
			if (this->vorbisInfos->channels == 1)
			{
				for (long i = 0; i < outreaded; ++i)
				{
					out[readed + i * 2] = tmp[0][(uint32_t)(i * ratio)];
					out[readed + i * 2 + 1] = tmp[0][(uint32_t)(i * ratio)];
				}
			}
			else if (this->vorbisInfos->channels == 2)
			{
				for (long i = 0; i < outreaded; ++i)
				{
					out[readed + i * 2] = tmp[0][(uint32_t)(i * ratio)];
					out[readed + i * 2 + 1] = tmp[1][(uint32_t)(i * ratio)];
				}
			}
			else
			{
				for (uint32_t i = readed; i < outLen; ++i)
					out[i] = 0;
				return;
			}
			readed += outreaded * 2;
		}
	}

}
