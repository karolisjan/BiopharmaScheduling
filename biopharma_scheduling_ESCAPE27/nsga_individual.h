#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
    #pragma GCC diagnostic ignored "-Wreorder"
	#pragma GCC diagnostic ignored "-Wsign-compare"
#endif 

#ifndef __NSGA_INDIVIDUAL_H__
#define __NSGA_INDIVIDUAL_H__

#include "gene.h"
#include "base_individual.h"


namespace types
{
	struct NSGAIndividual : public BaseIndividual<Gene>
	{
		using BaseIndividual::BaseIndividual;

		std::vector<double> objectives; // All objectives are minimised
		double constraints; 

		double d; // Crowding distance
		int rank; // Domination rank
		int n; // Number of solutions which dominate this solution
		std::vector<int> S; // Set of solutions (indices) that are dominated by this solution
	};
}

#endif 
