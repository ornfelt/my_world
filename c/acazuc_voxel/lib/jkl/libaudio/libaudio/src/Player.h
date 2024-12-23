#ifndef LIBAUDIO_PLAYER_H
# define LIBAUDIO_PLAYER_H

# include "./Filters/Filter.h"
# include <cstdint>
# include <vector>

namespace libaudio
{


	class Player
	{

	friend class Stream;

	protected:
		std::vector<Filter*> filters;
		uint32_t sampling;
		float pitch;
		float gain;
		float pan;
		bool active;
		bool loop;
		bool callback(float *out, unsigned long frameCount, uint32_t sampling);
		virtual void getData(float *out, uint32_t outLen, uint32_t inLen) {(void)out;(void)outLen;(void)inLen;};
		Player();

	public:
		virtual ~Player();
		void start();
		void stop();
		void addFilter(Filter *filter);
		void removeFilter(Filter *filter);
		inline uint32_t getSampling() {return this->sampling;};
		void setPitch(float pitch);
		inline float getPitch() {return this->pitch;};
		void setGain(float gain);
		inline float getGain() {return this->gain;};
		void setPan(float pan);
		inline float getPan() {return this->pan;};
		void setLoop(bool loop);
		inline bool getLoop();
		inline bool isPlaying() {return this->active;};

	};

}

#endif
