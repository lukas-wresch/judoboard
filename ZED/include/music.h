#pragma once
#include <string>
#include "core.h"
#include "sound.h"



typedef struct _Mix_Music Mix_Music;
struct Mix_Chunk;



namespace ZED
{
	class Music
	{
	public:
		Music() = default;
		Music(Music&) = delete;
		Music(const Music&) = delete;
		Music(Music&& rhs) noexcept : m_MusicChunk(rhs.m_MusicChunk), m_Channel(rhs.m_Channel)
		{
			rhs.m_MusicChunk = nullptr;
		}

		void operator = (const Music& rhs)
		{
			Unload();
			m_MusicChunk = rhs.m_MusicChunk;
			m_Channel = rhs.m_Channel;
			m_Shared = true;
		}

		void operator = (Music&& rhs) noexcept
		{
			Unload();
			m_MusicChunk = rhs.m_MusicChunk;
			m_Channel = rhs.m_Channel;
			m_Shared = false;
			rhs.m_MusicChunk = nullptr;
		}

		DLLEXPORT Music(const char* Filename);
		Music(const std::string& Filename) : Music(Filename.c_str()) {}

		~Music() { if (!m_Shared) Unload(); }

		bool IsValid() const { return m_MusicChunk; }

		DLLEXPORT void Play() const;
		DLLEXPORT void Stop() const;

		DLLEXPORT void FadeIn( uint32_t Milliseconds) const;
		DLLEXPORT static void FadeOut(uint32_t Milliseconds);

		DLLEXPORT static void SkipTo(double Seconds);

		DLLEXPORT void Unload();

		operator bool() const { return IsValid(); }

	private:
		Mix_Music* m_MusicChunk = nullptr;
		mutable int m_Channel = -1;
		bool m_Shared = false;
	};
}