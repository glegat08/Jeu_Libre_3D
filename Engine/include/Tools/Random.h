#pragma once
#include <random>
#include <vector>
#include <type_traits>

/// @brief Namespace containing utility tools.
namespace KGR
{
    namespace Tools
    {
        /**
         * @brief Random number generator wrapper using std::mt19937.
         *
         * This class provides type-safe random number generation for both
         * floating-point and integral types. It uses a true random seed
         * from std::random_device to initialize the internal generator.
         *
         * @note Seed management could be improved for reproducibility.
         */
        class Random
        {
        public:

            /**
             * @brief Generates a random floating-point number in the range [min, max].
             *
             * @tparam type A floating-point type (float, double, long double).
             * @param min Lower bound of the range.
             * @param max Upper bound of the range.
             * @return A random floating-point value between min and max.
             */
            template<typename type>
                requires std::is_floating_point_v<type>
            type getRandomNumber(type min, type max)
            {
                std::uniform_real_distribution<type> dis(min, max);
                return static_cast<type>(dis(m_gen));
            }

            /**
             * @brief Generates a random integral number in the range [min, max].
             *
             * @tparam type An integral type (int, uint32_t, etc.).
             * @param min Lower bound of the range.
             * @param max Upper bound of the range.
             * @return A random integer between min and max.
             */
            template<typename type>
                requires std::is_integral_v<type>
            type getRandomNumber(type min, type max)
            {
                std::uniform_int_distribution<type> dis(min, max);
                return static_cast<type>(dis(m_gen));
            }

            /**
             * @brief Generates a vector of random numbers of any supported type.
             *
             * @tparam type Type of the generated numbers.
             * @param min Lower bound of the range.
             * @param max Upper bound of the range.
             * @param number Number of random values to generate.
             * @return A std::vector containing `number` random values.
             */
            template<typename type>
            std::vector<type> getRandomNumberRange(type min, type max, const size_t& number)
            {
                std::vector<type> result;
                result.reserve(number);

                for (std::make_signed_t<size_t> i = 0; i < number; ++i)
                {
                    result.push_back(getRandomNumber<type>(min, max));
                }
                return result;
            }

            /**
             * @brief Constructs the Random generator using a true random seed.
             */
            Random() : m_gen(m_rd())
            {
            }

        private:
            std::random_device m_rd; ///< Source of entropy for seeding.
            std::mt19937 m_gen;      ///< Mersenne Twister random number generator.
        };
    }
}