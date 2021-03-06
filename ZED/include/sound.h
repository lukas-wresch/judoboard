#pragma once
#include <string>
#include "core.h"



typedef struct _Mix_Music Mix_Music;
struct Mix_Chunk;



namespace ZED
{
	class Sound
	{
	public:
		Sound() = default;
		Sound(Sound&) = delete;
		Sound(const Sound&) = delete;
		Sound(Sound&& rhs) noexcept : m_SndChunk(rhs.m_SndChunk), m_Channel(rhs.m_Channel)
		{
			rhs.m_SndChunk = nullptr;
		}

		void operator = (const Sound& rhs)
		{
			Unload();
			m_SndChunk = rhs.m_SndChunk;
			m_Channel  = rhs.m_Channel;
			m_Shared   = true;
		}

		void operator = (Sound&& rhs) noexcept
		{
			Unload();
			m_SndChunk = rhs.m_SndChunk;
			m_Channel  = rhs.m_Channel;
			m_Shared   = false;
			rhs.m_SndChunk = nullptr;
		}

		DLLEXPORT Sound(const char* Filename);
		Sound(const std::string& Filename) : Sound(Filename.c_str()) {}

		~Sound() { if (!m_Shared) Unload(); }

		bool IsValid() const { return m_SndChunk; }

		DLLEXPORT void Play(int Loops = 0) const;
		void Loop() const { Play(-1); }
		DLLEXPORT void Stop() const;

		DLLEXPORT void Unload();

		operator bool() const { return IsValid(); }

	private:
		Mix_Chunk* m_SndChunk = nullptr;
		mutable int m_Channel = -1;
		bool m_Shared = false;
	};



	class SoundEngine
	{
	public:
		DLLEXPORT static bool Init();
		DLLEXPORT static void Release();

		DLLEXPORT static void SetSoundVolume(uint32_t Volume);
		DLLEXPORT static void SetMusicVolume(uint32_t Volume);

	private:
	};
}