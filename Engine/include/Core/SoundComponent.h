#pragma once
#include <memory>
#include <stdexcept>

#include "RessourcesManager.h"
#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"
// wav 
struct WavComponent
{
	WavComponent();

	WavComponent(SoLoud::Wav& wav);

	void SetWav(SoLoud::Wav& wav);

	void SetLoop(bool isLooping);

	bool IsOver() const;

	void SetVolume(float volume);

	float GetVolume() const;

	void Play();

	void Pause();

	void Resume();

	void Stop();

	bool IsPause() const;

	bool IsPlaying() const;
	static void Init(const std::filesystem::path&);
private:
	void ErrorValidWav() const;

	void ErrorMusicNotPlay() const;


	int m_handle = -1;
	SoLoud::Wav* m_wav = nullptr;

    static SoLoud::Soloud m_music;
};

inline SoLoud::Soloud WavComponent::m_music = SoLoud::Soloud{};

std::unique_ptr<SoLoud::Wav> LoadWav(const std::string& path);

using WavManager = KGR::ResourceManager<SoLoud::Wav, KGR::TypeWrapper<>, LoadWav>;

struct WavStreamComponent
{
	WavStreamComponent();

	WavStreamComponent(SoLoud::WavStream& wav);

	void SetWav(SoLoud::WavStream& wav);

	void SetLoop(bool isLooping);

	bool IsOver() const;

	void SetVolume(float volume);

	float GetVolume() const;

	void Play();

	void Pause();

	void Resume();

	void Stop();

	bool IsPause() const;

	bool IsPlaying() const;
	static void Init(const std::filesystem::path& globFilePath);
private:
	void ErrorValidWav() const;

	void ErrorMusicNotPlay() const;

	
	int m_handle = -1;
	SoLoud::WavStream* m_wav = nullptr;

	static SoLoud::Soloud m_music;
};

inline SoLoud::Soloud WavStreamComponent::m_music = SoLoud::Soloud{};

std::unique_ptr<SoLoud::WavStream> LoadWavStream(const std::string& path);

using WavStreamManager = KGR::ResourceManager<SoLoud::WavStream, KGR::TypeWrapper<>, LoadWavStream>;
//wav stream

