#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "sound_out.h"

const float c_min_volume= 1.0f / 16.0f;

static const float ArraySum( const float* arr, unsigned int size )
{
	float s= 0.0f;
	for( unsigned int i= 0; i< size; i++ )
		s+= arr[i];
	return s;
}

#include "spectrum.h"

SoundOut::SoundOut()
{
	al_device_= alcOpenDevice(NULL);
    al_context_= alcCreateContext(al_device_ , NULL);
    alcMakeContextCurrent(al_context_);

	float pos_vel[]= { 0.0f, 0.0f, 0.0f };
	float listener_ori[6];
	listener_ori[1]=  0.0;
	listener_ori[2]= -1.0;
	listener_ori[3]=  0.0;
	listener_ori[4]=  1.0;
	listener_ori[5]=  0.0;

	alListenerfv(AL_POSITION,    pos_vel);
    alListenerfv(AL_VELOCITY,    pos_vel);
    alListenerfv(AL_ORIENTATION, listener_ori);

	float src_pos[]= { 0.0f, 0.0f, -1.0f };

    for( unsigned int i= 0; i< 20; i++ )
    {
    	GenNote( i, &note_buffers_[i] );

    	alGenSources( 1, &note_sources_[i] );

    	alSourcei( note_sources_[i], AL_BUFFER, note_buffers_[i] );

		alSourcef( note_sources_[i], AL_PITCH, 1.0f );
		alSourcef( note_sources_[i], AL_GAIN, /*c_min_volume*/1.0f );

		alSourcefv( note_sources_[i] ,AL_POSITION, src_pos );
		alSourcefv( note_sources_[i], AL_VELOCITY, pos_vel );

		alSourcei( note_sources_[i], AL_LOOPING, true );

		note_state_[i]= IDLE;
		note_volume_[i]= /*c_min_volume*/1.0f;
    }
}

SoundOut::~SoundOut()
{
	alcDestroyContext( al_context_ );
	alcCloseDevice( al_device_ );
}

void SoundOut::StartNote( unsigned int aperture, unsigned int mode )
{
	/*unsigned int i= aperture * 2 + mode;
	if( note_state_[i] == IDLE )
		alSourcePlay( note_sources_[i] );
	note_state_[i]= INCREASE;*/

	alSourcePlay( note_sources_[aperture * 2 + mode] );
}

void SoundOut::StopNote( unsigned int aperture, unsigned int mode )
{
	//note_state_[aperture * 2 + mode]= DECREASE;
	alSourcePause( note_sources_[aperture * 2 + mode] );
}

void SoundOut::Update( float dt )
{
	/*const float up_speed= 32.0f;
	const float down_speed= 1.0f / ( 64.0f * 65536.0f );

	for( unsigned int i= 0; i< 20; i++ )
		switch(note_state_[i])
		{
			case IDLE: break;
			case FULL: break;
			case INCREASE:
			{
				note_volume_[i]+= up_speed * dt;
				if( note_volume_[i] >= 1.0f )
				{
					note_state_[i]= FULL;
					note_volume_[i]= 1.0f;
				}
				alSourcef( note_sources_[i], AL_GAIN, note_volume_[i] );
			}
			break;
			case DECREASE:
			{
				note_volume_[i]*= powf( down_speed, dt );
				if( note_volume_[i] <= c_min_volume )
				{
					note_state_[i]= IDLE;
					note_volume_[i]= c_min_volume;
					alSourcePause( note_sources_[i] );
				}
				else
					alSourcef( note_sources_[i], AL_GAIN, note_volume_[i] );
			}
			break;
		};*/
}

void SoundOut::SetVolume( float volume )
{
	alListenerf( AL_GAIN, volume );
}

void SoundOut::GenNote( unsigned int note_number, ALuint* out_buffer )
{
	const float amplitude_multipler= 1.0f /
		ArraySum( notes_specturm_table[note_number], notes_spectrum_size_table[note_number] );

	const float two_pi= 3.1415926535f * 2.0f;
	const unsigned int c_sample_rate= 40000;
	const float c_sample_length= 0.33f;

	const float note_freq_iterval= 1.2f;
	const float note_freq_randomization_interval= 0.08f;

	float freq= float( notes_freq_base_table[ note_number ] );

	//freq+= ( float(rand())* 2.0f / float(RAND_MAX) - 1.0f ) * note_freq_iterval * note_freq_randomization_interval;

	float sample_length= ceilf( freq * c_sample_length ) / freq;

	unsigned int sample_count= int( roundf( float(c_sample_rate) * sample_length ) );

	short* data= new short[ sample_count ];

	float mult= two_pi * freq / float(c_sample_rate);
	for( unsigned int i= 0; i< sample_count; i++ )
	{
		float val= 0.0f;
		for( unsigned int j= 0; j< notes_spectrum_size_table[note_number]; j++ )
			val+= notes_specturm_table[note_number][j] * sinf( float((j+1)*i) * mult );
		data[i]= short( 32767.0f * amplitude_multipler * val );
	}

	/*{
		int avg= data[0] + data[1] + data[sample_count-2] + data[sample_count-1];
		data[0]= (avg + data[0])>>1;
		data[1]= (avg + data[1] * 3 )>>2;
		data[sample_count-1]= (avg + data[sample_count-1])>>1;
		data[sample_count-2]= (avg + data[sample_count-2] * 3 )>>2;
	}*/

	alGenBuffers( 1, out_buffer );
	alBufferData(
		*out_buffer,
		AL_FORMAT_MONO16,
		data,
		sizeof(short) * sample_count,
		c_sample_rate);

	delete[] data;
}
