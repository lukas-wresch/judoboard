#include <thread>
#include "../external/SDL2-2.0.14/include/SDL_mixer.h"
#include "../include/music.h"
#include "../include/log.h"


using namespace ZED;



Music::Music(const char* Filename)
{
	m_MusicChunk = Mix_LoadMUS(Filename);

	if (!m_MusicChunk)
		Log::Error("Could not load: " + std::string(Filename));
	else
		Log::Info("Loaded: " + std::string(Filename));
}



void Music::Play() const
{
	Mix_PlayMusic(m_MusicChunk, -1);
}



void Music::Stop() const
{
	Mix_HaltMusic();
}



void Music::FadeIn(uint32_t Milliseconds) const
{
	Mix_FadeInMusic(m_MusicChunk, -1, Milliseconds);
}



void Music::FadeOut(uint32_t Milliseconds)
{
	//Mix_FadeOutMusic(Milliseconds);
	std::thread(Mix_FadeOutMusic, Milliseconds).detach();
}



void Music::SkipTo(double Seconds)
{
	Mix_SetMusicPosition(Seconds);
}



void Music::Unload()
{
	if (m_MusicChunk)
		Mix_FreeMusic(m_MusicChunk);
	m_MusicChunk = nullptr;
}