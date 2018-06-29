#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
    #pragma GCC diagnostic ignored "-Wreorder"
	#pragma GCC diagnostic ignored "-Wsign-compare"
	#pragma GCC diagnostic ignored "-Wunused-variable"
	#pragma GCC diagnostic ignored "-Wformat="
#endif 

#ifndef __SINGLE_OBJECTIVE_INDIVIDUAL_H__
#define __SINGLE_OBJECTIVE_INDIVIDUAL_H__

#include "base_individual.h"


namespace types
{
	template<class Gene>
	class SingleObjectiveIndividual : public BaseIndividual<Gene>
	{
	public:
		using BaseIndividual<Gene>::BaseIndividual;

		double objective;
        double constraints;
	};
}

#endif 