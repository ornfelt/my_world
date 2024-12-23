#ifndef LIBAUDIO_LOAD_DATAS_H
# define LIBAUDIO_LOAD_DATAS_H

# include <cstdint>
# include <string>
# include <vector>

namespace libaudio
{

	int32_t getPCMSize(std::string file);
	bool loadDatas(std::string file, std::vector<float> &datas, int *sampling, int *channelsCount);

}

#endif
