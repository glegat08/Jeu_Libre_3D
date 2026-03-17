#pragma once
#include <memory>
#include <stdexcept>

#include "Core/RessourcesManager.h"
#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"

//TODO add a pitch for the speed of the sound and music and a pan for the left right balance

namespace KGR
{
    namespace Audio
    {
        // ──────────────────────────────────────────────────────────────
        //   W A V   C O M P O N E N T
        // ──────────────────────────────────────────────────────────────
        

        /**
         * @brief Component that handles playback of an in‑memory WAV sound using SoLoud.
         *
         * This component wraps a SoLoud::Wav object and provides high‑level controls
         * such as play, pause, stop, looping, and volume adjustments.
         *
         * Unlike WavStreamComponent, this class loads the entire audio file into memory.
         */
        struct WavComponent
        {
            /**
             * @brief Default constructor. The component starts in an invalid state.
             */
            WavComponent();

            /**
             * @brief Constructs the component using an already loaded WAV resource.
             * @param wav Reference to a valid SoLoud::Wav instance.
             */
            WavComponent(SoLoud::Wav& wav);

            /**
             * @brief Assigns a WAV resource to this component.
             * @param wav Reference to a valid SoLoud::Wav instance.
             */
            void SetWav(SoLoud::Wav& wav);

            /**
             * @brief Enables or disables looping for the audio.
             * @param isLooping True to loop the sound, false otherwise.
             */
            void SetLoop(bool isLooping);

            /**
             * @brief Checks whether the sound has finished playing.
             * @return True if playback is finished, false otherwise.
             */
            bool IsOver() const;

            /**
             * @brief Sets the playback volume.
             * @param volume Volume value in the range [0.0f, 1.0f].
             */
            void SetVolume(float volume);

            /**
             * @brief Retrieves the current playback volume.
             * @return The volume value.
             */
            float GetVolume() const;

            /**
             * @brief Starts playback of the sound.
             */
            void Play();

            /**
             * @brief Pauses the sound if it is currently playing.
             */
            void Pause();

            /**
             * @brief Resumes playback if the sound is paused.
             */
            void Resume();

            /**
             * @brief Stops the sound immediately.
             */
            void Stop();

            /**
             * @brief Checks whether the sound is currently paused.
             * @return True if paused, false otherwise.
             */
            bool IsPause() const;

            /**
             * @brief Checks whether the sound is currently playing.
             * @return True if playing, false otherwise.
             */
            bool IsPlaying() const;

            /**
             * @brief Initializes the global SoLoud engine for WAV playback.
             * @param path Path to the audio configuration or assets directory.
             */
            static void Init(const std::filesystem::path&);

        private:
            /**
             * @brief Throws an exception if the WAV resource is invalid.
             */
            void ErrorValidWav() const;

            /**
             * @brief Throws an exception if playback operations are attempted
             *        while no sound is currently playing.
             */
            void ErrorMusicNotPlay() const;

            int m_handle = -1;              ///< Playback handle returned by SoLoud.
            SoLoud::Wav* m_wav = nullptr;   ///< Pointer to the associated WAV resource.

            static SoLoud::Soloud m_music;  ///< Shared SoLoud engine instance.
        };

        inline SoLoud::Soloud WavComponent::m_music = SoLoud::Soloud{};

        /**
         * @brief Loads a WAV file from disk into a SoLoud::Wav object.
         * @param path Path to the WAV file.
         * @return A unique_ptr containing the loaded WAV resource.
         */
        std::unique_ptr<SoLoud::Wav> LoadWav(const std::string& path);

        /// @brief Resource manager type for WAV assets.
        using WavManager = KGR::ResourceManager<SoLoud::Wav, KGR::TypeWrapper<>, LoadWav>;


        //
        // ──────────────────────────────────────────────────────────────
        //   W A V   S T R E A M   C O M P O N E N T
        // ──────────────────────────────────────────────────────────────
        //

        /**
         * @brief Component that handles streaming playback of large WAV files.
         *
         * This component wraps a SoLoud::WavStream, which streams audio from disk
         * instead of loading it entirely into memory. Ideal for long music tracks.
         */
        struct WavStreamComponent
        {
            /**
             * @brief Default constructor. The component starts in an invalid state.
             */
            WavStreamComponent();

            /**
             * @brief Constructs the component using an already loaded WAV stream.
             * @param wav Reference to a valid SoLoud::WavStream instance.
             */
            WavStreamComponent(SoLoud::WavStream& wav);

            /**
             * @brief Assigns a WAV stream resource to this component.
             * @param wav Reference to a valid SoLoud::WavStream instance.
             */
            void SetWav(SoLoud::WavStream& wav);

            /**
             * @brief Enables or disables looping for the streamed audio.
             * @param isLooping True to loop the sound, false otherwise.
             */
            void SetLoop(bool isLooping);

            /**
             * @brief Checks whether the streamed audio has finished playing.
             * @return True if playback is finished, false otherwise.
             */
            bool IsOver() const;

            /**
             * @brief Sets the playback volume.
             * @param volume Volume value in the range [0.0f, 1.0f].
             */
            void SetVolume(float volume);

            /**
             * @brief Retrieves the current playback volume.
             * @return The volume value.
             */
            float GetVolume() const;

            /**
             * @brief Starts playback of the streamed audio.
             */
            void Play();

            /**
             * @brief Pauses the audio stream.
             */
            void Pause();

            /**
             * @brief Resumes playback of the audio stream.
             */
            void Resume();

            /**
             * @brief Stops the audio stream immediately.
             */
            void Stop();

            /**
             * @brief Checks whether the audio stream is currently paused.
             * @return True if paused, false otherwise.
             */
            bool IsPause() const;

            /**
             * @brief Checks whether the audio stream is currently playing.
             * @return True if playing, false otherwise.
             */
            bool IsPlaying() const;

            /**
             * @brief Initializes the global SoLoud engine for WAV streaming.
             * @param globFilePath Path to the audio configuration or assets directory.
             */
            static void Init(const std::filesystem::path& globFilePath);

        private:
            /**
             * @brief Throws an exception if the WAV stream resource is invalid.
             */
            void ErrorValidWav() const;

            /**
             * @brief Throws an exception if playback operations are attempted
             *        while no stream is currently playing.
             */
            void ErrorMusicNotPlay() const;

            int m_handle = -1;                    ///< Playback handle returned by SoLoud.
            SoLoud::WavStream* m_wav = nullptr;  ///< Pointer to the associated WAV stream.

            static SoLoud::Soloud m_music;       ///< Shared SoLoud engine instance.
        };

        inline SoLoud::Soloud WavStreamComponent::m_music = SoLoud::Soloud{};

        /**
         * @brief Loads a WAV stream from disk into a SoLoud::WavStream object.
         * @param path Path to the WAV file.
         * @return A unique_ptr containing the loaded WAV stream resource.
         */
        std::unique_ptr<SoLoud::WavStream> LoadWavStream(const std::string& path);

        /// @brief Resource manager type for WAV stream assets.
        using WavStreamManager = KGR::ResourceManager<SoLoud::WavStream, KGR::TypeWrapper<>, LoadWavStream>;
    }
}