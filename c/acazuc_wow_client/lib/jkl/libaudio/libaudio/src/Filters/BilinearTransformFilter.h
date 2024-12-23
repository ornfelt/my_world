#ifndef BILINEAR_TRANSFORM_FILTER_H
# define BILINEAR_TRANSFORM_FILTER_H

# include "./Filter.h"
# include <vector>
# include <array>

namespace libaudio
{

	class BilinearTransformFilter : public Filter
	{

	protected:
		std::vector<std::array<float, 3>> backSamples;
		std::vector<std::array<float, 2>> prevSamples;
		float a0;
		float a1;
		float a2;
		float b0;
		float b1;
		float b2;
		BilinearTransformFilter(uint32_t channels, uint32_t sampling);

	public:
		BilinearTransformFilter(uint32_t channels, uint32_t sampling, float a0, float a1, float a2, float b0, float b1, float b2);
		void filter(float *data, uint32_t samples);

	};

}

#endif
