#include "loadDatas.h"
#include <vorbis/vorbisfile.h>
#include <vorbis/codec.h>
#include <cstring>
#include <vector>
#include <cstdio>

namespace libaudio
{

	int32_t getPCMSize(std::string filename)
	{
		OggVorbis_File vorbisFile;
		FILE *file = std::fopen(filename.c_str(), "rb");
		if (!file)
			return -1;
		if (ov_open_callbacks(file, &vorbisFile, NULL, 0, OV_CALLBACKS_NOCLOSE) < 0)
		{
			std::fclose(file);
			return -1;
		}
		ogg_int64_t len = ov_pcm_total(&vorbisFile, -1);
		std::fclose(file);
		if (len < 0)
			return -1;
		return len;
	}

	bool loadDatas(std::string filename, std::vector<float> &datas, int *sampling, int *channelsCount)
	{
		OggVorbis_File vorbisFile;
		vorbis_info *vorbisInfos;
		FILE *file = std::fopen(filename.c_str(), "rb");
		if (!file)
			return false;
		if (ov_open_callbacks(file, &vorbisFile, NULL, 0, OV_CALLBACKS_NOCLOSE) < 0)
		{
			std::fclose(file);
			return false;
		}
		if (!(vorbisInfos = ov_info(&vorbisFile, -1)))
		{
			ov_clear(&vorbisFile);
			std::fclose(file);
			return false;
		}
		*sampling = vorbisInfos->rate;
		*channelsCount = vorbisInfos->channels;
		datas.clear();
		float **tmp;
		long ret;
		while ((ret = ov_read_float(&vorbisFile, &tmp, 4096, NULL)) > 0)
		{
			uint32_t org = datas.size();
			datas.resize(datas.size() + ret * *channelsCount);
			for (long i = 0; i < ret; ++i)
			{
				for (int j = 0; j < *channelsCount; ++j)
					datas[org + i * *channelsCount + j] = tmp[j][i];
			}
		}
		ov_clear(&vorbisFile);
		std::fclose(file);
		if (ret < 0)
			return false;
		return true;
	}

}
