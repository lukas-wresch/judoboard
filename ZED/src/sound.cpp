#include "../external/SDL2-2.0.14/include/SDL_mixer.h"
#include "../include/sound.h"
#include "../include/log.h"


using namespace ZED;



#ifdef _DEBUG
#pragma comment(lib, "../external/SDL2_mixer-2.0.4/VisualC/Win32/Debug/SDL2_mixer.lib")
#else
#pragma comment(lib, "../external/SDL2_mixer-2.0.4/VisualC/Win32/Release/SDL2_mixer.lib")
#endif



Sound::Sound(const char* Filename)
{
	m_SndChunk = Mix_LoadWAV(Filename);

	if (!m_SndChunk)
		Log::Error("Could not load: " + std::string(Filename));
	else
		Log::Info("Loaded: " + std::string(Filename));
}



void Sound::Play(int Loops) const
{
	if (m_SndChunk)
		m_Channel = Mix_PlayChannel(-1, m_SndChunk, Loops);
}



void Sound::Stop() const
{
	if (m_Channel == -1)
		return;

	Mix_HaltChannel(m_Channel);
	m_Channel = -1;
}



void Sound::Unload()
{
	if (m_SndChunk)
		Mix_FreeChunk(m_SndChunk);
	m_SndChunk = nullptr;
}



bool SoundEngine::Init()
{
	int audio_rate = 44100;
	Uint16 audio_format = AUDIO_S16;//16-bit
	int audio_channels = 2;//Stereo
	const int audio_buffers = 4096;

	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) == -1)
		return false;

	Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);	

	//Mix_AllocateChannels();
	return true;
}



void SoundEngine::Release()
{
	Mix_CloseAudio();
}



void SoundEngine::SetSoundVolume(uint32_t Volume)
{
	Mix_Volume(-1, Volume);
}



void SoundEngine::SetMusicVolume(uint32_t Volume)
{
	Mix_VolumeMusic(Volume);
}