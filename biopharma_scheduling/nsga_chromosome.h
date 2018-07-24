#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
 	// #pragma GCC diagnostic ignored "-Wreorder"
	// #pragma GCC diagnostic ignored "-Wunused-variable"
	#pragma GCC diagnostic ignored "-Wformat="
	#pragma GCC diagnostic ignored "-Wsign-compare"
#endif 

#ifndef __NSGA_CHROMOSOME_H__
#define __NSGA_CHROMOSOME_H__

#include "base_chromosome.h"


namespace types
{
	template<class Gene>
	class NSGAChromosome : public BaseChromosome<Gene>
	{
	public:
		using BaseChromosome<Gene>::BaseChromosome;

		std::vector<double> objectives; // All objectives are minimised
		double constraints; 

		double d; // Crowding distance
		int rank; // Domination rank
		int n; // Number of solutions which dominate this solution
		std::vector<int> S; // Set of solutions (indices) that are dominated by this solution
	};
}

#endif 
