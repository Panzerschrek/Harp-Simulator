#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <vector>

#include <SDL.h>

#include "spectrum.h"

namespace
{

// Constants
const SDL_AudioDeviceID g_first_valid_device_id= 2u;

// Audio
SDL_AudioDeviceID device_id_= 0u;
unsigned int frequency_; // samples per second
std::vector<int> mix_buffer_;
const unsigned int c_sample_rate= 44100;

// Window
SDL_Window* window_= nullptr;
SDL_Surface* surface_= nullptr;

// View
const unsigned int c_window_width= 400;
const unsigned int c_window_height= 144;

const unsigned int c_harp_apertures_count= 10;
const unsigned int c_harp_aprture_modes_count= 2;

const unsigned int c_edge_border_size= 8;
const unsigned int c_inner_border_size= 4;

const unsigned char c_background_color[]= { 32, 32, 32 };
const unsigned char c_inactive_aperture_color[]= { 200, 190, 170 };
const unsigned char c_active_aperture_color[]= { 150, 148, 130 };
const unsigned char c_note_name_color[]= { 84, 80, 76 };

// Notes
bool note_state_table[c_harp_apertures_count][c_harp_aprture_modes_count]=
{
	{ false, false },
	{ false, false },
	{ false, false },
	{ false, false },
	{ false, false },
	{ false, false },
	{ false, false },
	{ false, false },
	{ false, false },
	{ false, false }
};

typedef std::vector<int16_t> NoteData;
NoteData notes_data_[c_harp_apertures_count][c_harp_aprture_modes_count];
uint64_t sample_pos_= 0u;

} // namespace

static int NearestPowerOfTwoFloor( int x )
{
	int r= 1 << 30;
	while( r > x ) r>>= 1;
	return r;
}

void GenNote( unsigned int note_number, NoteData& out_note_data )
{
	const float amplitude_multipler= 1.0f /
		std::accumulate(
			notes_specturm_table[note_number],
			notes_specturm_table[note_number] + notes_spectrum_size_table[note_number],
			0.0f );
		//ArraySum( notes_specturm_table[note_number], notes_spectrum_size_table[note_number] );

	const float two_pi= 3.1415926535f * 2.0f;
	const float c_sample_length= 0.33f;

	float freq= float( notes_freq_base_table[ note_number ] );

	float sample_length= std::ceil( freq * c_sample_length ) / freq;

	unsigned int sample_count= int( std::round( float(c_sample_rate) * sample_length ) );

	out_note_data.resize( sample_count );

	float mult= two_pi * freq / float(c_sample_rate);
	for( unsigned int i= 0; i< sample_count; i++ )
	{
		float val= 0.0f;
		for( unsigned int j= 0; j< notes_spectrum_size_table[note_number]; j++ )
			val+= notes_specturm_table[note_number][j] * std::sin( float((j+1)*i) * mult );
		out_note_data[i]= int16_t( 32767.0f * amplitude_multipler * val );
	}
}

void AudioCallback( void* /*userdata*/, Uint8* stream, int len_bytes )
{
	int16_t* const out_data= reinterpret_cast<int16_t*>(stream);
	const unsigned int sample_count= len_bytes / sizeof(int16_t);

	for( int i= 0; i < sample_count; ++i )
		mix_buffer_[i]= 0;

	for( int y= 0; y < c_harp_aprture_modes_count; ++y )
	for( int x= 0; x < c_harp_apertures_count; ++x )
	{
		if( !note_state_table[x][y] )
			continue;

		const NoteData& data= notes_data_[x][y];
		unsigned int samples_added= 0u;
		while( samples_added < sample_count )
		{
			const unsigned int pos= static_cast<unsigned int>( ( sample_pos_ + samples_added ) % static_cast<uint64_t>(data.size()) );
			const unsigned int samples_to_write= std::min( static_cast<unsigned int>( data.size() - pos ), sample_count - samples_added );
			for( unsigned int i= 0u; i < samples_to_write; ++i )
				mix_buffer_[ samples_added + i ]+= data[ pos + i ];
			samples_added+= samples_to_write;
		}
	}

	sample_pos_+= sample_count;

	for( int i= 0; i < sample_count; ++i )
		out_data[i]= std::max( -32767, std::min( mix_buffer_[i], 32767 ) );
}

void InitAudio()
{
	for( int y= 0; y < c_harp_aprture_modes_count; ++y )
	for( int x= 0; x < c_harp_apertures_count; ++x )
		GenNote( y + x * c_harp_aprture_modes_count, notes_data_[x][y] );

	SDL_AudioSpec requested_format;
	SDL_AudioSpec obtained_format;

	requested_format.channels= 1u;
	requested_format.freq= c_sample_rate;
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

	mix_buffer_.resize( obtained_format.samples );

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
	window_= SDL_CreateWindow(
		"Harp Simulator",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		c_window_width, c_window_height,
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

void Draw()
{
	const auto fill_rect=
	[]( const int x, const int y, const int width, const int height, const unsigned char* const color )
	{
		int pitch= surface_->pitch / 4;
		unsigned char* const dst= static_cast<unsigned char*>(surface_->pixels) + ( x + y * pitch ) * 4;
		for( int dy= 0; dy < height; ++dy )
		for( int dx= 0; dx < width ; ++dx )
		{
			for( int j= 0; j < 3; ++j )
				dst[ ( dx + dy * pitch ) * 4 + j ]= color[j];
			dst[ ( dx + dy * pitch ) * 4 + 3 ]= 128;
		}
	};

	fill_rect( 0, 0, surface_->w, surface_->h, c_background_color );

	const unsigned int c_edge_border_size= 8;
	const unsigned int c_inner_border_size= 2;

	unsigned int window_width=  surface_->w;
	unsigned int window_height= surface_->h;

	unsigned int plate_width=
		(window_width - c_edge_border_size*2 - 2 * c_harp_apertures_count * c_inner_border_size )
		/ c_harp_apertures_count;
	unsigned int plate_height=
		(window_height - c_edge_border_size*2 - (c_harp_aprture_modes_count-1) * c_inner_border_size )
		/ c_harp_aprture_modes_count;

	for( unsigned int x= 0; x< c_harp_apertures_count; x++ )
	for( unsigned int y= 0 ;y< c_harp_aprture_modes_count; y++ )
	{
		unsigned int r_x= c_edge_border_size + (plate_width + c_inner_border_size * 2) * x;
		unsigned int r_y= c_edge_border_size + (plate_height + c_inner_border_size * 2) * y;
		fill_rect( r_x, r_y, plate_width, plate_height, note_state_table[x][y] ? c_active_aperture_color : c_inactive_aperture_color );
	}
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

		{
			int key_count;
			const Uint8* const keyboard_state= SDL_GetKeyboardState( &key_count );

			for( int i= 0; i < key_count; i++ )
			{
				const int code= SDL_Scancode(i);
				if( code >= SDL_SCANCODE_1 && code <= SDL_SCANCODE_9 )
					note_state_table[ code - SDL_SCANCODE_1 ][0]= keyboard_state[i] != 0;
				else if( code == SDL_SCANCODE_0 )
					note_state_table[9][0]= keyboard_state[i] != 0;

				static const int c_code_to_aperture_number[c_harp_apertures_count]=
				{
					SDL_SCANCODE_Q,
					SDL_SCANCODE_W,
					SDL_SCANCODE_E,
					SDL_SCANCODE_R,
					SDL_SCANCODE_T,
					SDL_SCANCODE_Y,
					SDL_SCANCODE_U,
					SDL_SCANCODE_I,
					SDL_SCANCODE_O,
					SDL_SCANCODE_P,
				};
				for( int j= 0; j < c_harp_apertures_count; ++j )
					if( code == c_code_to_aperture_number[j] )
						note_state_table[j][1]= keyboard_state[i] != 0;
			}
		}

		if( SDL_MUSTLOCK( surface_ ) )
			SDL_LockSurface( surface_ );

		Draw();

		if( SDL_MUSTLOCK( surface_ ) )
			SDL_UnlockSurface( surface_ );

		SDL_UpdateWindowSurface( window_ );
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
