#ifndef HIGH_PASS_FILTER_H
# define HIGH_PASS_FILTER_H

# include "./BilinearTransformFilter.h"

namespace libaudio
{

	class HighPassFilter : public BilinearTransformFilter
	{

	public:
		HighPassFilter(uint32_t channels, uint32_t sampling, float cutoff, float Q);

	};

}

#endif
