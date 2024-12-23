#ifndef LIBAUDIO_DIRECT_PLAYER_H
# define LIBAUDIO_DIRECT_PLAYER_H

# include "Player.h"
# include <vorbis/vorbisfile.h>
# include <vorbis/codec.h>
# include <cstdio>
# include <string>

namespace libaudio
{

	class DirectPlayer : public Player
	{

	private:
		OggVorbis_File vorbisFile;
		vorbis_info *vorbisInfos;
		FILE *file;
		void getData(float *out, uint32_t outLen, uint32_t inLen);

	public:
		DirectPlayer(std::string file);
		~DirectPlayer();

	};

}

#endif
