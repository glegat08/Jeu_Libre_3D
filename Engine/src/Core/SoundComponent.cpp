#include "Core/SoundComponent.h"

WavComponent::WavComponent()
{
	
}

WavComponent::WavComponent(SoLoud::Wav& wav): m_wav(&wav)
{
	
}

void WavComponent::SetWav(SoLoud::Wav& wav)
{
	m_wav = &wav;
}

void WavComponent::SetLoop(bool isLooping)
{
	ErrorValidWav();
	m_wav->setLooping(isLooping);
}

bool WavComponent::IsOver() const
{
	ErrorValidWav();
	return !IsPlaying();
}

void WavComponent::SetVolume(float volume)
{
	ErrorValidWav();
	m_music.setVolume(m_handle, volume);
}

float WavComponent::GetVolume() const
{
	return m_music.getVolume(m_handle);
}

void WavComponent::Play()
{
	ErrorValidWav();
	if (IsPlaying())
		throw std::out_of_range("already playing");

	m_handle = static_cast<int>(m_music.play(*m_wav));
}

void WavComponent::Pause()
{
	ErrorValidWav();
	ErrorMusicNotPlay();
	if (!IsPause())
		m_music.setPause(m_handle, true);
}

void WavComponent::Resume()
{
	ErrorValidWav();
	ErrorMusicNotPlay();
	if (IsPause())
		m_music.setPause(m_handle, false);
}

void WavComponent::Stop()
{
	ErrorValidWav();
	ErrorMusicNotPlay();
	m_music.stop(m_handle);
	m_handle = -1;
}

bool WavComponent::IsPause() const
{
	return m_music.getPause(m_handle);
}

bool WavComponent::IsPlaying() const
{
	return m_handle != -1 && m_music.isValidVoiceHandle(m_handle);
}

void WavComponent::ErrorValidWav() const
{
	if (!m_wav)
		throw std::runtime_error("no wav set");
}

void WavComponent::ErrorMusicNotPlay() const
{
	if (!IsPlaying())
		throw std::out_of_range("music not play");
}

void WavComponent::Init(const std::filesystem::path& globFilePath)
{
	m_music.init();
	WavManager::SetGlobalFIlePath(globFilePath);
}

std::unique_ptr<SoLoud::Wav> LoadWav(const std::string& path)
{
	std::unique_ptr<SoLoud::Wav> sound = std::make_unique<SoLoud::Wav>();
	if (sound->load(path.c_str()) != SoLoud::SO_NO_ERROR)
	{
		throw std::runtime_error("file not found");
	}
	return std::move(sound);
}

WavStreamComponent::WavStreamComponent()
{
	
}

WavStreamComponent::WavStreamComponent(SoLoud::WavStream& wav): m_wav(&wav)
{
	
}

void WavStreamComponent::SetWav(SoLoud::WavStream& wav)
{
	m_wav = &wav;
}

void WavStreamComponent::SetLoop(bool isLooping)
{
	ErrorValidWav();
	m_wav->setLooping(isLooping);
}

bool WavStreamComponent::IsOver() const
{
	ErrorValidWav();
	return !IsPlaying();
}

void WavStreamComponent::SetVolume(float volume)
{
	ErrorValidWav();
	m_music.setVolume(m_handle, volume);
}

float WavStreamComponent::GetVolume() const
{
	return m_music.getVolume(m_handle);
}

void WavStreamComponent::Play()
{
	ErrorValidWav();
	if (IsPlaying())
		throw std::out_of_range("already playing");

	m_handle = static_cast<int>(m_music.play(*m_wav));
}

void WavStreamComponent::Pause()
{
	ErrorValidWav();
	ErrorMusicNotPlay();
	if (!IsPause())
		m_music.setPause(m_handle, true);
}

void WavStreamComponent::Resume()
{
	ErrorValidWav();
	ErrorMusicNotPlay();
	if (IsPause())
		m_music.setPause(m_handle, false);
}

void WavStreamComponent::Stop()
{
	ErrorValidWav();
	ErrorMusicNotPlay();
	m_music.stop(m_handle);
	m_handle = -1;
}

bool WavStreamComponent::IsPause() const
{
	return m_music.getPause(m_handle);
}

bool WavStreamComponent::IsPlaying() const
{
	return m_handle != -1 && m_music.isValidVoiceHandle(m_handle);
}

void WavStreamComponent::ErrorValidWav() const
{
	if (!m_wav)
		throw std::runtime_error("no wav set");
}

void WavStreamComponent::ErrorMusicNotPlay() const
{
	if (!IsPlaying())
		throw std::out_of_range("music not play");
}

void WavStreamComponent::Init(const std::filesystem::path& globFilePath)
{
	m_music.init();
	WavStreamManager::SetGlobalFIlePath(globFilePath);
}

std::unique_ptr<SoLoud::WavStream> LoadWavStream(const std::string& path)
{
	std::unique_ptr<SoLoud::WavStream> sound = std::make_unique<SoLoud::WavStream>();
	if (sound->load(path.c_str()) != SoLoud::SO_NO_ERROR)
	{
		throw std::runtime_error("file not found");
	}
	return std::move(sound);
}
