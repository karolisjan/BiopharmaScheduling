#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
    #pragma GCC diagnostic ignored "-Wreorder"
	#pragma GCC diagnostic ignored "-Wsign-compare"
#endif 

#ifndef __SINGLE_OBJECTIVE_INDIVIDUAL_H__
#define __SINGLE_OBJECTIVE_INDIVIDUAL_H__

#include <vector>

#include "gene.h"
#include "base_individual.h"


namespace types
{
	struct SingleObjectiveIndividual : public BaseIndividual<Gene>
	{
		using BaseIndividual::BaseIndividual;

		double objective;
        double constraints;
	};
}

#endif 