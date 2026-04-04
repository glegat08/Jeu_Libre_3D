#pragma once
#include <chrono>
#include <stdexcept>

namespace KGR
{
    namespace Tools
    {
        /**
         * @brief Alias for std::ratio to simplify ratio usage.
         *
         * @tparam lhs Numerator of the ratio.
         * @tparam rhs Denominator of the ratio (defaults to lhs).
         */
        template<long long lhs, long long rhs = lhs>
        using ratio = std::ratio<lhs, rhs>;

        /**
         * @brief High‑precision timer with pause/resume and time conversion utilities.
         *
         * @tparam type Floating‑point type used for time conversions (default: float).
         *
         * This class provides:
         * - high‑resolution time measurement
         * - pause/resume functionality
         * - elapsed time queries
         * - conversion to seconds, milliseconds, microseconds, nanoseconds
         * - ratio‑based conversions (e.g., minutes, hours)
         */
        template<typename type = float>
        class Chrono
        {
        public:
            using clock = std::chrono::high_resolution_clock;
            using time_point = std::chrono::time_point<clock>;
            using duration = clock::duration;

            /**
             * @brief Represents a time interval between two time points.
             *
             * Supports arithmetic operations, comparisons, and unit conversions.
             */
            struct Time
            {
                Time();
                Time(const time_point& start, const time_point& end);
                Time(const Time& other);
                Time(Time&& other) noexcept;
                ~Time() = default;

                /**
                 * @brief Creates a Time object from a numeric value and a ratio.
                 *
                 * @tparam ratio Ratio representing the unit (default: seconds).
                 * @param fixTime The numeric time value.
                 * @return A Time object representing the given duration.
                 */
                template<typename ratio = ratio<1>>
                static Time CreateFromValue(const type& fixTime);

                Time& operator=(const Time& other);
                Time& operator=(Time&& other) noexcept;

                bool operator==(const Time& other) const;
                bool operator!=(const Time& other) const;
                bool operator>=(const Time& other) const;
                bool operator<=(const Time& other) const;
                bool operator>(const Time& other) const;
                bool operator<(const Time& other) const;

                Time operator+(const Time& other);
                Time operator-(const Time& other);
                Time& operator+=(const Time& other);
                Time& operator-=(const Time& other);

                Time operator*(const type& factor);
                Time operator/(const type& divider);
                Time& operator*=(const type& factor);
                Time& operator/=(const type& divider);

                /**
                 * @brief Returns the raw duration (end - start).
                 */
                duration Duration() const;

                /**
                 * @brief Converts the duration to seconds.
                 */
                type AsSeconds() const;

                /**
                 * @brief Converts the duration to milliseconds.
                 */
                type AsMilliSeconds() const;

                /**
                 * @brief Converts the duration to nanoseconds.
                 */
                type AsNanoSeconds() const;

                /**
                 * @brief Converts the duration to microseconds.
                 */
                type AsMicroSeconds() const;

                /**
                 * @brief Converts the duration using a custom ratio.
                 *
                 * @tparam ratio Ratio representing the desired unit.
                 */
                template<typename ratio>
                type AsRatio() const;

            private:
                /**
                 * @brief Constructs a Time object directly from a duration.
                 *
                 * Used internally by CreateFromValue().
                 */
                Time(const duration& nano);

                time_point m_start; ///< Start timestamp.
                time_point m_end;   ///< End timestamp.
            };

            /**
             * @brief Constructs a running Chrono starting at the current time.
             */
            Chrono();

            Chrono(const Chrono& other);
            Chrono(Chrono&& other) noexcept;
            ~Chrono() = default;

            Chrono<type>& operator=(const Chrono&);
            Chrono<type>& operator=(Chrono&&) noexcept;

            /**
             * @brief Returns the elapsed time since start or last reset.
             */
            Time GetElapsedTime() const;

            /**
             * @brief Calculates and returns the time in seconds since the last call to GetDeltaTime.
             * Essential for frame-independent game loops.
             */
            type GetDeltaTime();

            /**
             * @brief Pauses the timer and returns the elapsed time.
             */
            Time Pause();

            /**
             * @brief Resumes the timer after a pause.
             */
            Time Resume();

            /**
             * @brief Returns true if the timer is currently paused.
             */
            bool IsPause() const;

            /**
             * @brief Resets the timer and returns the elapsed time before reset.
             */
            Time Reset();

        private:
            /**
             * @brief Returns the time elapsed at the moment of pause.
             */
            Time GetPauseTime() const;

            time_point m_start;    ///< Start time of the Chrono.
            time_point m_lastTick; ///< Time of the last GetDeltaTime call.
            bool m_stop;           ///< Indicates whether the timer is paused.
            Time m_stopTime;       ///< Time recorded at pause.
        };

        template <typename type>
        Chrono<type>::Time::Time() : m_start(clock::now()), m_end(clock::now())
        {
        }

        template <typename type>
        template <typename ratio>
        typename Chrono<type>::Time Chrono<type>::Time::CreateFromValue(const type& fixTime)
        {
            auto durationNano = std::chrono::duration_cast<duration>(std::chrono::duration<type, ratio>(fixTime));
            return Time{ durationNano };
        }

        template <typename type>
        Chrono<type>::Time::Time(const time_point& start, const time_point& end) :m_start(start), m_end(end)
        {
        }

        template <typename type>
        Chrono<type>::Time::Time(const Time& other) : m_start(other.m_start), m_end(other.m_end)
        {
        }

        template <typename type>
        Chrono<type>::Time::Time(Time&& other) noexcept : m_start(std::move(other.m_start)), m_end(std::move(other.m_end))
        {
        }

        template <typename type>
        typename Chrono<type>::Time& Chrono<type>::Time::operator=(const Time& other)
        {
            m_start = other.m_start;
            m_end = other.m_end;

            return *this;
        }

        template <typename type>
        typename Chrono<type>::Time& Chrono<type>::Time::operator=(Time&& other) noexcept
        {
            m_start = std::move(other.m_start);
            other.m_start = time_point{};
            m_end = std::move(other.m_end);
            other.m_end = time_point{};
            return *this;
        }

        template <typename type>
        typename Chrono<type>::duration Chrono<type>::Time::Duration() const
        {
            return  m_end - m_start;
        }

        template <typename type>
        bool Chrono<type>::Time::operator==(const Time& other) const
        {
            return Duration() == other.Duration();
        }

        template <typename type>
        bool Chrono<type>::Time::operator!=(const Time& other) const
        {
            return Duration() != other.Duration();
        }

        template <typename type>
        bool Chrono<type>::Time::operator>=(const Time& other) const
        {
            return Duration() >= other.Duration();
        }

        template <typename type>
        bool Chrono<type>::Time::operator<=(const Time& other) const
        {
            return Duration() <= other.Duration();
        }

        template <typename type>
        bool Chrono<type>::Time::operator>(const Time& other) const
        {
            return Duration() > other.Duration();
        }

        template <typename type>
        bool Chrono<type>::Time::operator<(const Time& other) const
        {
            return Duration() < other.Duration();
        }

        template <typename type>
        typename Chrono<type>::Time Chrono<type>::Time::operator+(const Time& other)
        {
            return Time(Duration() + other.Duration());
        }

        template <typename type>
        typename Chrono<type>::Time Chrono<type>::Time::operator-(const Time& other)
        {
            return Time(Duration() - other.Duration());
        }

        template <typename type>
        typename Chrono<type>::Time& Chrono<type>::Time::operator+=(const Time& other)
        {
            duration totalduration = Duration() + other.Duration();
            m_end = m_start + totalduration;
            return *this;
        }

        template <typename type>
        typename Chrono<type>::Time& Chrono<type>::Time::operator-=(const Time& other)
        {
            duration totalduration = Duration() - other.Duration();
            m_end = m_start + totalduration;
            return *this;
        }

        template <typename type>
        typename Chrono<type>::Time Chrono<type>::Time::operator*(const type& factor)
        {
            return Time(std::chrono::duration_cast<duration>(std::chrono::duration<type, std::nano>(Duration() * factor)));
        }

        template <typename type>
        typename Chrono<type>::Time Chrono<type>::Time::operator/(const type& divider)
        {
            if (divider == 0)
                throw std::out_of_range("can't divide by 0");
            return Time(std::chrono::duration_cast<duration>(std::chrono::duration<type, std::nano>(Duration() / divider)));
        }

        template <typename type>
        typename Chrono<type>::Time& Chrono<type>::Time::operator*=(const type& factor)
        {
            duration totalduration = std::chrono::duration_cast<duration>(std::chrono::duration<type, std::nano>(std::chrono::duration<type, std::nano>(Duration() * factor)));
            m_end = m_start + totalduration;
            return *this;
        }

        template <typename type>
        typename Chrono<type>::Time& Chrono<type>::Time::operator/=(const type& divider)
        {
            if (divider == 0)
                throw std::out_of_range("can't divide by 0");
            duration totalduration = std::chrono::duration_cast<duration>(std::chrono::duration<type, std::nano>(Duration() / divider));
            m_end = m_start + totalduration;
            return *this;
        }

        template <typename type>
        type Chrono<type>::Time::AsSeconds() const
        {
            return AsRatio<std::ratio<1>>();
        }

        template <typename type>
        type Chrono<type>::Time::AsMilliSeconds() const
        {
            return AsRatio<std::milli>();
        }

        template <typename type>
        type Chrono<type>::Time::AsNanoSeconds() const
        {
            return AsRatio<std::nano>();
        }

        template <typename type>
        type Chrono<type>::Time::AsMicroSeconds() const
        {
            return AsRatio<std::micro>();
        }

        template <typename type>
        template <typename ratio>
        type Chrono<type>::Time::AsRatio() const
        {
            return std::chrono::duration<type, ratio>(Duration()).count();
        }

        template <typename type>
        Chrono<type>::Time::Time(const duration& nano) : m_start(time_point{}), m_end(time_point{ nano })
        {
        }

        template <typename type>
        Chrono<type>::Chrono() : m_start(clock::now()), m_lastTick(m_start), m_stop(false), m_stopTime(Time())
        {

        }

        template <typename type>
        Chrono<type>::Chrono(const Chrono& other) : m_start(other.m_start), m_lastTick(other.m_lastTick), m_stop(other.m_stop), m_stopTime(other.m_stopTime)
        {
        }

        template <typename type>
        Chrono<type>::Chrono(Chrono&& other) noexcept : m_start(std::move(other.m_start)), m_lastTick(std::move(other.m_lastTick)), m_stop(other.m_stop), m_stopTime(std::move(other.m_stopTime))
        {
        }

        template <typename type>
        Chrono<type>& Chrono<type>::operator=(const Chrono& other)
        {
            m_start = other.m_start;
            m_lastTick = other.m_lastTick;
            m_stop = other.m_stop;
            m_stopTime = other.m_stopTime;
            return *this;
        }

        template <typename type>
        Chrono<type>& Chrono<type>::operator=(Chrono&& other) noexcept
        {
            m_start = std::move(other.m_start);
            m_lastTick = std::move(other.m_lastTick);
            m_stop = other.m_stop;
            m_stopTime = std::move(other.m_stopTime);
            return *this;
        }

        template <typename type>
        typename Chrono<type>::Time Chrono<type>::GetElapsedTime() const
        {
            if (IsPause())
                return GetPauseTime();
            return Time(m_start, clock::now());
        }

        template <typename type>
        type Chrono<type>::GetDeltaTime()
        {
            auto now = clock::now();

            if (IsPause())
            {
                m_lastTick = now;
                return 0;
            }

            duration delta = now - m_lastTick;
            m_lastTick = now;
            return std::chrono::duration<type>(delta).count();
        }

        template <typename type>
        typename Chrono<type>::Time Chrono<type>::Pause()
        {
            if (IsPause())
                return GetPauseTime();
            m_stopTime = GetElapsedTime();
            m_stop = true;
            return m_stopTime;
        }

        template <typename type>
        typename Chrono<type>::Time Chrono<type>::Resume()
        {
            if (IsPause())
            {
                m_stop = false;

                m_start = clock::now() - m_stopTime.Duration();
                m_lastTick = clock::now();
                m_stopTime = Time();
                return GetElapsedTime();
            }
            return GetElapsedTime();
        }

        template <typename type>
        bool Chrono<type>::IsPause() const
        {
            return m_stop;
        }

        template <typename type>
        typename Chrono<type>::Time Chrono<type>::GetPauseTime() const
        {
            if (!IsPause())
                throw std::runtime_error("clock is running");
            return m_stopTime;
        }

        template <typename type>
        typename Chrono<type>::Time Chrono<type>::Reset()
        {
            auto result = GetElapsedTime();
            if (IsPause())
                m_stop = false;
            m_start = clock::now();
            m_lastTick = m_start;
            m_stopTime = Time();
            return result;
        }
    }
}