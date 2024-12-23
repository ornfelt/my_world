#ifndef LIBAUDIO_DEVICE_H
# define LIBAUDIO_DEVICE_H

# include <portaudio.h>
# include <vector>
# include <string>

namespace libaudio
{

	class Device
	{

	private:
		const PaDeviceInfo *device;
		PaDeviceIndex index;

	public:
		Device(PaDeviceIndex index);
		~Device();
		std::string getName() const;
		inline const PaDeviceInfo *getInfo() const {return this->device;};
		inline PaDeviceIndex getIndex() const {return this->index;};
		static std::vector<Device> getDevices();
		static std::vector<Device> getInputDevices();
		static std::vector<Device> getOutputDevices();
		static Device *getDefaultInputDevice();
		static Device *getDefaultOutputDevice();

	};

}

#endif
