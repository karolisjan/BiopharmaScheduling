#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
    #pragma GCC diagnostic ignored "-Wreorder"
	#pragma GCC diagnostic ignored "-Wsign-compare"
	#pragma GCC diagnostic ignored "-Wunused-variable"
	#pragma GCC diagnostic ignored "-Wformat="
#endif 

#ifndef __SINGLE_OBJECTIVE_CHROMOSOME_H__
#define __SINGLE_OBJECTIVE_CHROMOSOME_H__

#include "base_chromosome.h"


namespace types
{
	template<class Gene>
	class SingleObjectiveChromosome : public BaseChromosome<Gene>
	{
	public:
		using BaseChromosome<Gene>::BaseChromosome;

		double objective;
        double constraints;
	};
}

#endif 