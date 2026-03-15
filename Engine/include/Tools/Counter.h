#pragma once

#include <stdexcept>

namespace KGR
{
    namespace Tools
    {
        /**
         * @brief A generic bounded counter supporting cyclic and clamped operations.
         *
         * @tparam type Numeric type used for counting (defaults to int).
         *
         * This class provides a flexible counter that operates between a minimum
         * and maximum bound. It supports:
         * - clamped increment/decrement
         * - cyclic iteration (min → max → min)
         * - directional iteration (max → min → max)
         * - custom sub‑ranges
         */
        template <typename type = int>
        class Counter
        {
        public:

            /**
             * @brief Constructs a counter with explicit minimum and maximum bounds.
             *
             * @param min Minimum allowed value.
             * @param max Maximum allowed value.
             */
            Counter(const type& min, const type& max)
                : m_maxCounter(max), m_minCounter(min), m_currentCounter(min)
            {
            }

            /// @brief Returns the current counter value.
            type GetCurrentCounter() const { return m_currentCounter; }

            /// @brief Returns the minimum allowed value.
            type GetMinCounter() const { return m_minCounter; }

            /// @brief Returns the maximum allowed value.
            type GetMaxCounter() const { return m_maxCounter; }

            /**
             * @brief Sets the minimum bound and clamps the current value if needed.
             */
            void SetMinCounter(const type& val)
            {
                m_minCounter = val;
                VerifyMin();
            }

            /**
             * @brief Sets the maximum bound and clamps the current value if needed.
             */
            void SetMaxCounter(const type& val)
            {
                m_maxCounter = val;
                VerifyMax();
            }

            /**
             * @brief Sets the current counter value and clamps it to valid bounds.
             */
            void SetCurrentCounter(const type& val)
            {
                m_currentCounter = val;
                VerifyMax();
                VerifyMin();
            }

            /**
             * @brief Increments the counter by 1 and clamps to the maximum bound.
             */
            void Next()
            {
                ++m_currentCounter;
                VerifyMax();
            }

            /**
             * @brief Decrements the counter by 1 and clamps to the minimum bound.
             */
            void Previous()
            {
                --m_currentCounter;
                VerifyMin();
            }

            /// @brief Resets the counter to the minimum bound.
            void ResetBegin() { m_currentCounter = m_minCounter; }

            /// @brief Resets the counter to the maximum bound.
            void ResetEnd() { m_currentCounter = m_maxCounter; }

            /// @brief Returns true if the counter is at or beyond the maximum bound.
            bool IsEnd() const { return m_currentCounter >= m_maxCounter; }

            /// @brief Returns true if the counter is at or below the minimum bound.
            bool IsBegin() const { return m_currentCounter <= m_minCounter; }

            /**
             * @brief Returns the current value and automatically increments.
             *
             * Cycles back to the minimum when reaching the maximum.
             */
            type GetAutoCounterMinToMax()
            {
                VerifyMax();
                VerifyMin();
                type tmp = m_currentCounter;
                IsEnd() ? ResetBegin() : Next();
                return tmp;
            }

            /**
             * @brief Returns the current value and automatically decrements.
             *
             * Cycles back to the maximum when reaching the minimum.
             */
            type GetAutoCounterMaxToMin()
            {
                VerifyMax();
                VerifyMin();
                type tmp = m_currentCounter;
                IsBegin() ? ResetEnd() : Previous();
                return tmp;
            }

            /**
             * @brief Cyclic counter within a custom sub‑range [min, max].
             *
             * @throws std::out_of_range if the sub‑range exceeds global bounds.
             */
            type GetCounterMinToMax(const type& min, const type& max)
            {
                if (min < m_minCounter || max > m_maxCounter)
                    throw std::out_of_range("Counter: Range out of bounds.");

                VerifyMin(min);
                VerifyMax(max);

                type tmp = m_currentCounter;
                m_currentCounter = (m_currentCounter >= max) ? min : m_currentCounter + 1;
                return tmp;
            }

            /**
             * @brief Cyclic counter within a custom sub‑range [min, max], descending.
             *
             * @throws std::out_of_range if the sub‑range exceeds global bounds.
             */
            type GetCounterMaxToMin(const type& min, const type& max)
            {
                if (min < m_minCounter || max > m_maxCounter)
                    throw std::out_of_range("Counter: Range out of bounds.");

                VerifyMin(min);
                VerifyMax(max);

                type tmp = m_currentCounter;
                m_currentCounter = (m_currentCounter <= min) ? max : m_currentCounter - 1;
                return tmp;
            }

            /**
             * @brief Automatically increments and returns true when reaching the end.
             *
             * Resets to the minimum when the maximum is reached.
             */
            bool AutoIsTimeMinToMax()
            {
                VerifyMin();
                VerifyMax();
                if (IsEnd())
                {
                    ResetBegin();
                    return true;
                }
                Next();
                return false;
            }

            /**
             * @brief Automatically decrements and returns true when reaching the beginning.
             *
             * Resets to the maximum when the minimum is reached.
             */
            bool AutoIsTimeMaxToMin()
            {
                VerifyMin();
                VerifyMax();
                if (IsBegin())
                {
                    ResetEnd();
                    return true;
                }
                Previous();
                return false;
            }

            /**
             * @brief Checks if the counter reached a custom max, then cycles to min.
             *
             * @return true if the limit was reached.
             */
            bool IsTimeMinToMax(const type& min, const type& max)
            {
                if (min < m_minCounter || max > m_maxCounter)
                    throw std::out_of_range("Counter: Range out of bounds.");

                VerifyMin(min);
                VerifyMax(max);

                if (m_currentCounter >= max)
                {
                    m_currentCounter = min;
                    return true;
                }
                Next();
                return false;
            }

            /**
             * @brief Checks if the counter reached a custom min, then cycles to max.
             *
             * @return true if the limit was reached.
             */
            bool IsTimeMaxToMin(const type& min, const type& max)
            {
                if (min < m_minCounter || max > m_maxCounter)
                    throw std::out_of_range("Counter: Range out of bounds.");

                VerifyMin(min);
                VerifyMax(max);

                if (m_currentCounter <= min)
                {
                    m_currentCounter = max;
                    return true;
                }
                Previous();
                return false;
            }

            /**
             * @brief Adds a value to the counter and clamps to the maximum.
             */
            void Increment(const type& add)
            {
                m_currentCounter += add;
                VerifyMax();
            }

            /**
             * @brief Subtracts a value from the counter and clamps to the minimum.
             */
            void Decrement(const type& remove)
            {
                m_currentCounter -= remove;
                VerifyMin();
            }

        private:

            /// @brief Clamps the current value to the maximum bound.
            void VerifyMax()
            {
                if (m_currentCounter > m_maxCounter)
                    m_currentCounter = m_maxCounter;
            }

            /// @brief Clamps the current value to the minimum bound.
            void VerifyMin()
            {
                if (m_currentCounter < m_minCounter)
                    m_currentCounter = m_minCounter;
            }

            /// @brief Clamps the current value to a custom maximum.
            void VerifyMax(const type& max)
            {
                if (m_currentCounter > max)
                    m_currentCounter = max;
            }

            /// @brief Clamps the current value to a custom minimum.
            void VerifyMin(const type& min)
            {
                if (m_currentCounter < min)
                    m_currentCounter = min;
            }

            type m_maxCounter;      ///< Maximum allowed value.
            type m_minCounter;      ///< Minimum allowed value.
            type m_currentCounter;  ///< Current counter value.
        };
    }
}