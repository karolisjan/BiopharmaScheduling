#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
    #pragma GCC diagnostic ignored "-Wreorder"
	#pragma GCC diagnostic ignored "-Wsign-compare"
	#pragma GCC diagnostic ignored "-Wunused-variable"
#endif 

#ifndef __BASE_GA_H__
#define __BASE_GA_H__

#include <cassert>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <numeric>

#include "utils.h"

namespace algorithms
{
	/*
	Abstract base GA class. Not to be called directly.
	Individual<Gene> class object is expected to have the following methods:

	void cross(Individual& other)
	void mutate() methods
	*/
	template<class Individual, class FitnessFunctor>
	class BaseGA
	{
	protected:
		typedef std::vector<Individual> Population;
		FitnessFunctor fitness_functor;
		Population parents, offspring;
		std::vector<int> indices;

		virtual Individual Tournament(Individual& p, Individual& q) = 0;

		virtual void Select()
		{
			offspring.clear();

			utils::shuffle(indices);
			for (size_t p = 0; p != parents.size(); p += 2)
				offspring.push_back(
					Tournament(parents[indices[p]], parents[indices[p + 1]]));

			utils::shuffle(indices);
			for (size_t p = 0; p != parents.size(); p += 2)
				offspring.push_back(
					Tournament(parents[indices[p]], parents[indices[p + 1]]));
		}

		virtual void Reproduce()
		{
			for (size_t p = 0; p != offspring.size(); p += 2) {
				offspring[p].cross(offspring[p + 1]);
				offspring[p].mutate();
				offspring[p + 1].mutate();
			}
		}

	public:
		explicit BaseGA() {}
		explicit BaseGA(
			FitnessFunctor fitness_functor,
			int seed
		) :
			fitness_functor(fitness_functor)
		{
			if (seed != -1)
				srand(seed);
		}

		// Updates the algorithm by one generation. 
		// Call this for n generations.
		virtual void Update() = 0;
	};
}

#endif // !__BASE_GA_H__
