#ifndef LOW_PASS_FILTER_H
# define LOW_PASS_FILTER_H

# include "./BilinearTransformFilter.h"

namespace libaudio
{

	class LowPassFilter : public BilinearTransformFilter
	{

	public:
		LowPassFilter(uint32_t channels, uint32_t sampling, float cutoff, float Q);

	};

}

#endif
