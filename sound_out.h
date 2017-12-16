#pragma once

#include <al.h>
#include <alc.h>

class SoundOut
{
public:
	SoundOut();
	~SoundOut();

	void StartNote( unsigned int aperture, unsigned int mode );
	void StopNote( unsigned int aperture, unsigned int mode );
	void Update(float dt);
	void SetVolume( float volume );

private:
	void GenNote( unsigned int note_number, ALuint* out_buffer );

private:
	ALuint note_buffers_[20];
	ALuint note_sources_[20];
	enum
	{
		IDLE,
		INCREASE,
		FULL,
		DECREASE

	}note_state_[20];
	float note_volume_[20];

	ALCcontext* al_context_;
	ALCdevice* al_device_;
};
