#ifndef FILTER_H
# define FILTER_H

# include <cstdint>

namespace libaudio
{

	class Filter
	{

	protected:
		uint32_t channels;
		uint32_t sampling;

	public:
		Filter(uint32_t channels, uint32_t sampling);
		virtual void filter(float *data, uint32_t samples) = 0;

	};

}

#endif
