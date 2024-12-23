#include "Stream.h"
#include "CachedPlayer.h"
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <cmath>

using libaudio::Stream;
using libaudio::CachedPlayer;

int main()
{
	srand(time(NULL));
	PaError error = Pa_Initialize();
	if (error)
	{
		std::cerr << "Failed to initialize portaudio" << std::endl;
		return EXIT_FAILURE;
	}
	Stream *stream = new Stream(2, 44100);
	float *datas = new float[44100 * 2];
	for (uint32_t i = 0; i < 44100 * 2; ++i)
		datas[i] = rand() / (float)RAND_MAX;
	CachedPlayer *player = new CachedPlayer(datas, 44100 * 2, 44100, 2);
	stream->addPlayer(player);
	player->start();
	stream->start();
	for (uint32_t i = 0; i < 100; ++i)
	{
		stream->setPan(std::abs(std::cos(i / 100. * M_PI * 2)));
		usleep(10000);
	}
	return EXIT_SUCCESS;
}
