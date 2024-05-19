#pragma once
#include <string>
#include "core.h"



typedef struct _Mix_Music Mix_Music;
struct Mix_Chunk;
typedef uint32_t SDL_AudioDeviceID;



namespace ZED
{
	class Sound
	{
		friend class SoundDevice;

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

		DLLEXPORT uint32_t GetSize() const;//Returns the size of the sound in bytes

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



	class SoundDevice
	{
	public:
		SoundDevice() = default;
		SoundDevice(SoundDevice&) = delete;
		SoundDevice(const SoundDevice&) = delete;
		SoundDevice(SoundDevice&& rhs) noexcept {
			this->m_Device = rhs.m_Device;
			rhs.m_Device   = 0;
		}
		void operator =(SoundDevice&& rhs) noexcept {
			this->m_Device = rhs.m_Device;
			rhs.m_Device   = 0;
		}
		DLLEXPORT SoundDevice(int Index);
		//DLLEXPORT SoundDevice(const char* DeviceName);
		//SoundDevice(const std::string& DeviceName) : SoundDevice(DeviceName.c_str()) {}
		DLLEXPORT ~SoundDevice();

		DLLEXPORT static const char* GetDeviceName(int Index);
		DLLEXPORT static int GetNumberOfDevices();

		bool IsValid() const { return m_Device > 0; }
		int  GetDeviceIndex() const { return m_DeviceIndex; }

		DLLEXPORT void Play(const Sound& Sound);
		DLLEXPORT void Pause(bool Enable = true);
		DLLEXPORT void Stop();

	private:
		SDL_AudioDeviceID m_Device = 0;
		int m_DeviceIndex = -1;
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