#ifndef LIBAUDIO_STREAM_H
# define LIBAUDIO_STREAM_H

# include "./Filters/Filter.h"
# include "./Player.h"
# include <portaudio.h>
# include <vector>

namespace libaudio
{

	class Device;

	class Stream
	{

	private:
		std::vector<Player*> players;
		std::vector<Filter*> filters;
		PaStreamParameters outputParameters;
		PaStream *stream;
		uint32_t sampling;
		uint8_t channels;
		float pitch;
		float gain;
		float pan;
		static int callback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo *paTimeInfo, PaStreamCallbackFlags statusFlags, void *userData);

	public:
		Stream(uint8_t channels, uint32_t sampling);
		~Stream();
		void setDevice(const Device &device);
		void start();
		void stop();
		bool isActive();
		void addPlayer(Player *player);
		void removePlayer(Player *player);
		void addFilter(Filter *filter);
		void removeFilter(Filter *filter);
		inline uint32_t getSampling() {return this->sampling;};
		void setPitch(float pitch);
		inline float getPitch() {return this->pitch;};
		void setGain(float gain);
		inline float getGain() {return this->gain;};
		void setPan(float pan);
		inline float getPan() {return this->pan;};

	};

}

#endif
