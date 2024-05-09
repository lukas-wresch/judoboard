#include "../external/SDL2-2.0.14/include/SDL_mixer.h"
#include "../include/sound.h"
#include "../include/log.h"


using namespace ZED;



#ifdef _DEBUG
#pragma comment(lib, "SDL2_mixer.lib")
#else
#pragma comment(lib, "SDL2_mixer.lib")
#endif



Sound::Sound(const char* Filename)
{
	m_SndChunk = Mix_LoadWAV(Filename);

	if (!m_SndChunk)
		Log::Error("Could not load: " + std::string(Filename));
	else
		Log::Info("Loaded: " + std::string(Filename));
}



uint32_t Sound::GetSize() const
{
	if (!m_SndChunk)
		return 0;
	return m_SndChunk->alen;
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



SoundDevice::SoundDevice(int Index) : m_DeviceIndex(Index)
{
	SDL_AudioSpec desired, obtained;
	SDL_memset(&desired, 0, sizeof(desired));
	desired.freq   = 44100;
	desired.format = AUDIO_S16;//16-bit
	desired.channels = 2;//Stereo
	desired.samples  = 4096;

	m_Device = SDL_OpenAudioDevice(GetDeviceName(Index), 0, &desired, &obtained, 0);
	Pause(false);//The device is paused by default
}



SoundDevice::~SoundDevice()
{
	if (IsValid())
		SDL_CloseAudioDevice(m_Device);
}



const char* SoundDevice::GetDeviceName(int Index)
{
	return SDL_GetAudioDeviceName(Index, 0);
}



int SoundDevice::GetNumberOfDevices()
{
	return SDL_GetNumAudioDevices(0);
}



void SoundDevice::Play(const Sound& Sound)
{
	if (IsValid() && Sound)
	{
		SDL_ClearQueuedAudio(m_Device);//Can not mix multiple audio streams, so clear all queued audio data
		SDL_QueueAudio(m_Device, Sound.m_SndChunk->abuf, Sound.m_SndChunk->alen);
		Pause(false);
	}
}



void SoundDevice::Pause(bool Enable)
{
	if (IsValid())
		SDL_PauseAudioDevice(m_Device, Enable ? 1 : 0);
}



void SoundDevice::Stop()
{
	if (IsValid())
		SDL_ClearQueuedAudio(m_Device);
}



bool SoundEngine::Init()
{
	int audio_rate = 44100;
	Uint16 audio_format = AUDIO_S16;//16-bit
	int audio_channels  = 2;//Stereo
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