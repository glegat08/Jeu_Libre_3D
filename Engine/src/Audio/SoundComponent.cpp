#include "Audio/SoundComponent.h"

KGR::Audio::WavComponent::WavComponent()
{
	
}

KGR::Audio::WavComponent::WavComponent(SoLoud::Wav& wav): m_wav(&wav)
{
	
}

void KGR::Audio::WavComponent::SetWav(SoLoud::Wav& wav)
{
	m_wav = &wav;
}

void KGR::Audio::WavComponent::SetLoop(bool isLooping)
{
	ErrorValidWav();
	m_wav->setLooping(isLooping);
}

bool KGR::Audio::WavComponent::IsOver() const
{
	ErrorValidWav();
	return !IsPlaying();
}

void KGR::Audio::WavComponent::SetVolume(float volume)
{
	ErrorValidWav();
	m_music.setVolume(m_handle, volume);
}

float KGR::Audio::WavComponent::GetVolume() const
{
	return m_music.getVolume(m_handle);
}

void KGR::Audio::WavComponent::Play()
{
	ErrorValidWav();
	if (IsPlaying())
		throw std::out_of_range("already playing");

	m_handle = static_cast<int>(m_music.play(*m_wav));
}

void KGR::Audio::WavComponent::Pause()
{
	ErrorValidWav();
	ErrorMusicNotPlay();
	if (!IsPause())
		m_music.setPause(m_handle, true);
}

void KGR::Audio::WavComponent::Resume()
{
	ErrorValidWav();
	ErrorMusicNotPlay();
	if (IsPause())
		m_music.setPause(m_handle, false);
}

void KGR::Audio::WavComponent::Stop()
{
	ErrorValidWav();
	ErrorMusicNotPlay();
	m_music.stop(m_handle);
	m_handle = -1;
}

bool KGR::Audio::WavComponent::IsPause() const
{
	return m_music.getPause(m_handle);
}

bool KGR::Audio::WavComponent::IsPlaying() const
{
	return m_handle != -1 && m_music.isValidVoiceHandle(m_handle);
}

void KGR::Audio::WavComponent::ErrorValidWav() const
{
	if (!m_wav)
		throw std::runtime_error("no wav set");
}

void KGR::Audio::WavComponent::ErrorMusicNotPlay() const
{
	if (!IsPlaying())
		throw std::out_of_range("music not play");
}

void KGR::Audio::WavComponent::Init(const std::filesystem::path& globFilePath)
{
	m_music.init();
	WavManager::SetGlobalFIlePath(globFilePath);
}

std::unique_ptr<SoLoud::Wav> KGR::Audio::LoadWav(const std::string& path)
{
	std::unique_ptr<SoLoud::Wav> sound = std::make_unique<SoLoud::Wav>();
	if (sound->load(path.c_str()) != SoLoud::SO_NO_ERROR)
	{
		throw std::runtime_error("file not found");
	}
	return std::move(sound);
}

KGR::Audio::WavStreamComponent::WavStreamComponent()
{
	
}

KGR::Audio::WavStreamComponent::WavStreamComponent(SoLoud::WavStream& wav): m_wav(&wav)
{
	
}

void KGR::Audio::WavStreamComponent::SetWav(SoLoud::WavStream& wav)
{
	m_wav = &wav;
}

void KGR::Audio::WavStreamComponent::SetLoop(bool isLooping)
{
	ErrorValidWav();
	m_wav->setLooping(isLooping);
}

bool KGR::Audio::WavStreamComponent::IsOver() const
{
	ErrorValidWav();
	return !IsPlaying();
}

void KGR::Audio::WavStreamComponent::SetVolume(float volume)
{
	ErrorValidWav();
	m_music.setVolume(m_handle, volume);
}

float KGR::Audio::WavStreamComponent::GetVolume() const
{
	return m_music.getVolume(m_handle);
}

void KGR::Audio::WavStreamComponent::Play()
{
	ErrorValidWav();
	if (IsPlaying())
		throw std::out_of_range("already playing");

	m_handle = static_cast<int>(m_music.play(*m_wav));
}

void KGR::Audio::WavStreamComponent::Pause()
{
	ErrorValidWav();
	ErrorMusicNotPlay();
	if (!IsPause())
		m_music.setPause(m_handle, true);
}

void KGR::Audio::WavStreamComponent::Resume()
{
	ErrorValidWav();
	ErrorMusicNotPlay();
	if (IsPause())
		m_music.setPause(m_handle, false);
}

void KGR::Audio::WavStreamComponent::Stop()
{
	ErrorValidWav();
	ErrorMusicNotPlay();
	m_music.stop(m_handle);
	m_handle = -1;
}

bool KGR::Audio::WavStreamComponent::IsPause() const
{
	return m_music.getPause(m_handle);
}

bool KGR::Audio::WavStreamComponent::IsPlaying() const
{
	return m_handle != -1 && m_music.isValidVoiceHandle(m_handle);
}

void KGR::Audio::WavStreamComponent::ErrorValidWav() const
{
	if (!m_wav)
		throw std::runtime_error("no wav set");
}

void KGR::Audio::WavStreamComponent::ErrorMusicNotPlay() const
{
	if (!IsPlaying())
		throw std::out_of_range("music not play");
}

void KGR::Audio::WavStreamComponent::Init(const std::filesystem::path& globFilePath)
{
	m_music.init();
	WavStreamManager::SetGlobalFIlePath(globFilePath);
}

std::unique_ptr<SoLoud::WavStream> KGR::Audio::LoadWavStream(const std::string& path)
{
	std::unique_ptr<SoLoud::WavStream> sound = std::make_unique<SoLoud::WavStream>();
	if (sound->load(path.c_str()) != SoLoud::SO_NO_ERROR)
	{
		throw std::runtime_error("file not found");
	}
	return std::move(sound);
}