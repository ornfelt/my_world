#include "Stream.h"
#include "loadDatas.h"
#include "DirectPlayer.h"
#include "Filters/LowPassFilter.h"
#include <cstdlib>
#include <unistd.h>
#include <iostream>

using libaudio::Stream;
using libaudio::DirectPlayer;
using libaudio::LowPassFilter;

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
	stream->addFilter(new LowPassFilter(2, 44100, 500, 1));
	DirectPlayer *player = new DirectPlayer("./test.ogg");
	player->setLoop(true);
	stream->addPlayer(player);
	player->start();
	stream->start();
	sleep(5);
	return EXIT_SUCCESS;
}
