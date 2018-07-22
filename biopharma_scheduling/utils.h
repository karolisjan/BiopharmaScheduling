#ifndef __UTILS_H__
#define __UTILS_H__

#include <cmath>
#include <queue>
#include <random>
#include <limits>
#include <vector>
#include <numeric>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>
#include <functional>


namespace utils
{
    static const double EPSILON = 1E-5;

	// See  Catch.hpp https://github.com/catchorg/Catch2
	class Approx 
	{
	private:
		// Performs equivalent check of std::fabs(lhs - rhs) <= margin
		// But without the subtraction to allow for INFINITY in comparison
		static inline bool marginComparison(double lhs, double rhs, double margin = std::numeric_limits<float>::epsilon() * 100) {
			return (lhs + margin >= rhs) && (rhs + margin >= lhs);
		}

		bool equalityComparisonImpl(const double other) const {
			// First try with fixed margin, then compute margin based on epsilon, scale and Approx's value
			return marginComparison(m_value, other, m_margin) || marginComparison(m_value, other, m_epsilon * (m_scale + std::fabs(m_value)));
		}

	public:
		Approx(double value, double m_epsilon = std::numeric_limits<float>::epsilon() * 100, double m_scale = 0.0, double m_margin = 0.0)
			: 
			m_epsilon(m_epsilon),
			m_margin(m_margin),
			m_scale(m_scale),
			m_value(value) {}

		static Approx custom() {
			return Approx(0);
		}

		template <typename T, typename = typename std::enable_if<std::is_constructible<double, T>::value>::type>
		Approx operator()(T const& value) {
			Approx approx(static_cast<double>(value));
			approx.epsilon(m_epsilon);
			approx.margin(m_margin);
			approx.scale(m_scale);
			return approx;
		}

		template <typename T, typename = typename std::enable_if<std::is_constructible<double, T>::value>::type>
		explicit Approx(T const& value) : Approx(static_cast<double>(value))
		{}

		template <typename T, typename = typename std::enable_if<std::is_constructible<double, T>::value>::type>
		friend bool operator == (const T& lhs, Approx const& rhs) {
			auto lhs_v = static_cast<double>(lhs);
			return rhs.equalityComparisonImpl(lhs_v);
		}

		template <typename T, typename = typename std::enable_if<std::is_constructible<double, T>::value>::type>
		friend bool operator == (Approx const& lhs, const T& rhs) {
			return operator==(rhs, lhs);
		}

		template <typename T, typename = typename std::enable_if<std::is_constructible<double, T>::value>::type>
		friend bool operator != (T const& lhs, Approx const& rhs) {
			return !operator==(lhs, rhs);
		}

		template <typename T, typename = typename std::enable_if<std::is_constructible<double, T>::value>::type>
		friend bool operator != (Approx const& lhs, T const& rhs) {
			return !operator==(rhs, lhs);
		}

		template <typename T, typename = typename std::enable_if<std::is_constructible<double, T>::value>::type>
		friend bool operator <= (T const& lhs, Approx const& rhs) {
			return static_cast<double>(lhs) < rhs.m_value || lhs == rhs;
		}

		template <typename T, typename = typename std::enable_if<std::is_constructible<double, T>::value>::type>
		friend bool operator <= (Approx const& lhs, T const& rhs) {
			return lhs.m_value < static_cast<double>(rhs) || lhs == rhs;
		}

		template <typename T, typename = typename std::enable_if<std::is_constructible<double, T>::value>::type>
		friend bool operator >= (T const& lhs, Approx const& rhs) {
			return static_cast<double>(lhs) > rhs.m_value || lhs == rhs;
		}

		template <typename T, typename = typename std::enable_if<std::is_constructible<double, T>::value>::type>
		friend bool operator >= (Approx const& lhs, T const& rhs) {
			return lhs.m_value > static_cast<double>(rhs) || lhs == rhs;
		}

		template <typename T, typename = typename std::enable_if<std::is_constructible<double, T>::value>::type>
		Approx& epsilon(T const& newEpsilon) {
			double epsilonAsDouble = static_cast<double>(newEpsilon);
			if (epsilonAsDouble < 0 || epsilonAsDouble > 1.0) {
				throw std::domain_error("Invalid Approx::epsilon: Approx::epsilon has to be between 0 and 1");
			}
			m_epsilon = epsilonAsDouble;
			return *this;
		}

		template <typename T, typename = typename std::enable_if<std::is_constructible<double, T>::value>::type>
		Approx& margin(T const& newMargin) {
			double marginAsDouble = static_cast<double>(newMargin);
			if (marginAsDouble < 0) {
				throw std::domain_error("Invalid Approx::margin: Approx::Margin has to be non-negative.");
			}
			m_margin = marginAsDouble;
			return *this;
		}

		template <typename T, typename = typename std::enable_if<std::is_constructible<double, T>::value>::type>
		Approx& scale(T const& newScale) {
			m_scale = static_cast<double>(newScale);
			return *this;
		}

	private:
		double m_epsilon;
		double m_margin;
		double m_scale;
		double m_value;
	};

    static inline int random_int(int min, int max)
    {
        return rand() % (max - min) + min;
    }

    inline double random()
    {
        return rand() / (RAND_MAX + 1.0);
    }
    
    template<typename T>
    static inline void shuffle(std::vector<T>& v)
    {
        size_t i, j;
        
        for (i = 0; i < v.size(); ++i) {
            j = random_int(i, v.size());
            std::swap(v[j], v[i]);
        }
    }

    inline int round(double val) 
    {
        return val + 0.5;
    }

    /*
        A hack for accessing the container of the priority queue by access with read and write rights.
    */
    template<class T, class S, class C>
    static S& access_queue_container(std::priority_queue<T, S, C> &q) 
    {
        struct hacked_queue : private std::priority_queue<T, S, C>
        {
            static S& access_queue_container(std::priority_queue<T, S, C> &q) 
            {
                return q.*&hacked_queue::c;
            }
        };

        return hacked_queue::access_queue_container(q);
    }

    /*
        Binary search for a [) range lookup.

        For example, for v = { 1, 10, 20, 30 } and:

        1. val = 10, 'search' will return 2.
        2. val = 9, 'search' will return 1.
    */
    template<class T1, class T2>
    static inline int search(const std::vector<T1> &v, const T2 &val)
    {
        if (val >= v.back()) {
            return -1;
        }

        return std::distance(v.cbegin(), std::upper_bound(v.cbegin(), v.cend(), val));
    }

	template<class RNG = std::mt19937_64, std::size_t N = RNG::state_size>
	class CustomRandom
	{
		public:
			inline void init()
			{
				typename RNG::result_type random_data[N];
				std::random_device r;
				std::generate(random_data, random_data + N, std::ref(r));
				std::seed_seq seed(random_data, random_data + N);
				RNG seeded_rng(seed);

				uniform_random_float = std::bind(std::uniform_real_distribution<float>(0.0f, 1.0f), seeded_rng);
				uniform_random_double = std::bind(std::uniform_real_distribution<double>(0.0, 1.0), seeded_rng);
			}

			inline void init(std::vector<int> seed_seq)
			{
				std::seed_seq seed(seed_seq.begin(), seed_seq.end());
				RNG seeded_rng(seed);

				uniform_random_float = std::bind(std::uniform_real_distribution<float>(0.0f, 1.0f), seeded_rng);
				uniform_random_double = std::bind(std::uniform_real_distribution<double>(0.0, 1.0), seeded_rng);
			}

			inline double uniform_rand_double(double max = 0, double min = 0)
			{ 
				return (max - min + 1) * uniform_random_double() + min;
			}

			inline float uniform_rand_float(float max = 0, float min = 0)
			{ 
				return (max - min + 1) * uniform_random_float() + min;
			}

		private:

			std::function<float()> uniform_random_float;
			std::function<double()> uniform_random_double;
	};

	template<class RNG>
	inline double triangular_distribution(double min, double mode, double max, RNG &rng)
	{
		if ((max - min) == Approx(0.0)) {
			return max;
		}

		double u = rng.uniform_rand_double(); 

		max += 1;

		if (((mode - min) / (max - min)) == Approx(u)){
			return mode;
		}

		if (u > ((mode - min) / (max - min))) {
			return max - sqrtf((1 - u) * (max - min) * (max - mode));
		}

		return min + sqrtf(u * (max - min) * (mode - min));
	}
}

#endif