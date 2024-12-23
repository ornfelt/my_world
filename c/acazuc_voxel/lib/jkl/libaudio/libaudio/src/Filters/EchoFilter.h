#ifndef ECHO_FILTER_H
# define ECHO_FILTER_H

# include "./Filter.h"
# include <vector>

namespace libaudio
{

	class EchoFilter : public Filter
	{

	private:
		std::vector<std::vector<float>> buffer;
		uint32_t bufferPos;
		float decay;

	public:
		EchoFilter(uint32_t channels, uint32_t sampling, float delay, float decay);
		void filter(float *data, uint32_t samples);

	};

}

#endif
