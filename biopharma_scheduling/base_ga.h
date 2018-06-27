#ifndef __BASE_GA_H__
#define __BASE_GA_H__

#include <omp.h>
#include <limits>

#include <vector>
#include <numeric>
#include <cstdlib>
#include <algorithm>

#include "utils.h"


namespace algorithms
{
	/*
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

		virtual bool Tournament(const Individual &p, const Individual &q) = 0;

		inline void Select()
		{
			int p;
			offspring.resize(0);
			utils::shuffle(indices);
			
			for (p = 0; p != parents.size(); p += 2) {
				if (Tournament(parents[indices[p]], parents[indices[p + 1]])) {
					offspring.push_back(parents[indices[p]]);
				}
				else {
					offspring.push_back(parents[indices[p + 1]]);
				}
			}

			utils::shuffle(indices);
			
			for (p = 0; p != parents.size(); p += 2) {
				if (Tournament(parents[indices[p]], parents[indices[p + 1]])) {
					offspring.push_back(parents[indices[p]]);
				}
				else {
					offspring.push_back(parents[indices[p + 1]]);
				}
			}
		}

		inline void Reproduce()
		{
			std::sort(offspring.begin(), offspring.end(), [](const auto& i1, const auto &i2){ return i1.genes.size() > i2.genes.size(); });

			int p = 0;

			for (p = 0; p != offspring.size(); p += 2) {
				offspring[p].cross(offspring[p + 1]);
			}

			for (p = 0; p != offspring.size(); p += 2) {
				offspring[p].mutate();
				offspring[p + 1].mutate();
			}
		}

	public:
		explicit BaseGA() {}
		explicit BaseGA(
			FitnessFunctor fitness_functor,
			int seed=-1,
			int num_procs=-1
		) :
			fitness_functor(fitness_functor)
		{
			if (seed != -1) {
				srand(seed);
			}

			int actual_num_threads = omp_get_num_procs();
			if (num_procs >= 1 && num_procs <= actual_num_threads) {
				omp_set_dynamic(0);
				omp_set_num_threads(num_procs);
			}
		}
	};
}

#endif 
