#ifndef LIBAUDIO_CACHED_PLAYER_H
# define LIBAUDIO_CACHED_PLAYER_H

# include "Player.h"

namespace libaudio
{

	class CachedPlayer : public Player
	{

	private:
		uint32_t position;
		uint32_t channels;
		uint32_t length;
		float *data;
		void getData(float *out, uint32_t outLen, uint32_t inLen);

	public:
		CachedPlayer(float *data, uint32_t length, uint32_t sampling, uint32_t channels);

	};

}

#endif
