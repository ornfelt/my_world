#ifndef LIBAUDIO_AUDIO_DEVICE_H
# define LIBAUDIO_AUDIO_DEVICE_H

# include <string>

namespace libaudio
{

	class AudioDevice
	{

	private:
		std::string name;
		double defaultSampleRate;
		int maxOutputChannels;
		int maxInputChannels;

	public:
		AudioDevice(std::string name, double defaultSampleRate, int maxOutputChannels, int maxInputChannels);

	};

}

#endif
