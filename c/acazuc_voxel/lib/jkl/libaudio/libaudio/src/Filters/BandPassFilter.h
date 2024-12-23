#ifndef BAND_PASS_FILTER_H
# define BAND_PASS_FILTER_H

# include "./BilinearTransformFilter.h"

namespace libaudio
{

	class BandPassFilter : public BilinearTransformFilter
	{

	public:
		BandPassFilter(uint32_t channels, uint32_t sampling, float cutoff, float Q);

	};

}

#endif
