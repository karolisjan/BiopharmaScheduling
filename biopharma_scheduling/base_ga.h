#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
 	// #pragma GCC diagnostic ignored "-Wreorder"
	// #pragma GCC diagnostic ignored "-Wunused-variable"
	#pragma GCC diagnostic ignored "-Wformat="
	#pragma GCC diagnostic ignored "-Wsign-compare"
#endif 

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
		Chromosome<Gene> class object is expected to have the following methods:

		void Cross(Chromosome& other)
		void Mutate() methods
	*/
	template<class Chromosome, class FitnessFunction>
	class BaseGA
	{
	protected:
		typedef std::vector<Chromosome> Population;
		FitnessFunction fitness_function;
		Population parents, offspring;
		std::vector<int> indices;

		virtual bool Tournament(const Chromosome &p, const Chromosome &q) = 0;

		inline void Select()
		{
			int p;
			offspring.resize(0);
			utils::shuffle(indices);
			
			for (p = 0; p < parents.size(); p += 2) {
				if (Tournament(parents[indices[p]], parents[indices[p + 1]])) {
					offspring.push_back(parents[indices[p]]);
				}
				else {
					offspring.push_back(parents[indices[p + 1]]);
				}
			}

			utils::shuffle(indices);
			
			for (p = 0; p < parents.size(); p += 2) {
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

			for (p = 0; p < offspring.size(); p += 2) {
				offspring[p].Cross(offspring[p + 1]);
			}

			for (p = 0; p < offspring.size(); p += 2) {
				offspring[p].Mutate();
				offspring[p + 1].Mutate();
			}
		}

	public:
		explicit BaseGA() {}
		explicit BaseGA(
			FitnessFunction fitness_function,
			int seed,
			int num_procs
		) :
			fitness_function(fitness_function)
		{
			utils::set_seed(seed);

			int actual_num_threads = omp_get_num_procs();

			if (num_procs >= 1 && num_procs <= actual_num_threads) {
				omp_set_dynamic(0);
				omp_set_num_threads(num_procs);
			}
		}
	};
}

#endif 
