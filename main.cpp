#include <cstdlib>
#include <iostream>
#include <vector>

#include <SDL.h>

namespace
{

// Constants
const SDL_AudioDeviceID g_first_valid_device_id= 2u;

// Audio
SDL_AudioDeviceID device_id_= 0u;
unsigned int frequency_; // samples per second
std::vector<int> mix_buffer_;

// Window
SDL_Window* window_= nullptr;
SDL_Surface* surface_= nullptr;

} // namespace

static int NearestPowerOfTwoFloor( int x )
{
	int r= 1 << 30;
	while( r > x ) r>>= 1;
	return r;
}

void AudioCallback( void* userdata, Uint8* stream, int len_bytes )
{
}

void InitAudio()
{
	SDL_AudioSpec requested_format;
	SDL_AudioSpec obtained_format;

	requested_format.channels= 1u;
	requested_format.freq= 22050;
	requested_format.format= AUDIO_S16;
	requested_format.callback= AudioCallback;
	requested_format.userdata= nullptr;

	// ~ 1 callback call per two frames (60fps)
	requested_format.samples= NearestPowerOfTwoFloor( requested_format.freq / 30 );

	int device_count= SDL_GetNumAudioDevices(0);
	// Can't get explicit devices list. Trying to use first device.
	if( device_count == -1 )
		device_count= 1;

	for( int i= 0; i < device_count; i++ )
	{
		const char* const device_name= SDL_GetAudioDeviceName( i, 0 );

		const SDL_AudioDeviceID device_id=
			SDL_OpenAudioDevice( device_name, 0, &requested_format, &obtained_format, 0 );

		if( device_id >= g_first_valid_device_id &&
			obtained_format.channels == requested_format.channels &&
			obtained_format.format   == requested_format.format )
		{
			device_id_= device_id;
			std::cout << "Open audio device: " <<  device_name << std::endl;
			break;
		}
	}

	if( device_id_ < g_first_valid_device_id )
	{
		std::cout << "Can not open any audio device" << std::endl;
		std::exit(-1);
	}

	frequency_= obtained_format.freq;

	mix_buffer_.resize( obtained_format.samples  );

	// Run
	SDL_PauseAudioDevice( device_id_ , 0 );
}

void DeInitAudio()
{
	if( device_id_ >= g_first_valid_device_id )
		SDL_CloseAudioDevice( device_id_ );

	SDL_QuitSubSystem( SDL_INIT_AUDIO );
}

void InitWindow()
{
	int width= 400, height= 200;

	window_= SDL_CreateWindow(
		"Harp Simulator",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		width, height,
		0 );

	if( window_ == nullptr )
	{
		std::cout << "Can not create window" << std::endl;
		std::exit(-1);
	}

	surface_= SDL_GetWindowSurface( window_ );

	if( surface_->format == nullptr || surface_->format->BytesPerPixel != 4 )
	{
		std::cout << "Unexpected window pixel depth. Expected 32 bit" << std::endl;
		std::exit(-1);
	}
}

void DeInitWindow()
{
	SDL_DestroyWindow( window_ );
}

void MainLoop()
{
	while(true)
	{
		SDL_Event event;
		while( SDL_PollEvent(&event) )
		{
			switch(event.type)
			{
			case SDL_WINDOWEVENT:
				if( event.window.event == SDL_WINDOWEVENT_CLOSE )
					return;
				break;

			case SDL_QUIT:
				return;
			};
		};

		if( SDL_MUSTLOCK( surface_ ) )
			SDL_LockSurface( surface_ );

		if( SDL_MUSTLOCK( surface_ ) )
			SDL_UnlockSurface( surface_ );
	}
}

extern "C" int main( int argc, char *argv[] )
{
	SDL_Init( SDL_INIT_AUDIO | SDL_INIT_VIDEO );

	InitWindow();
	InitAudio();


	MainLoop();

	DeInitAudio();
	DeInitWindow();

	SDL_Quit();
	return 0;
}
