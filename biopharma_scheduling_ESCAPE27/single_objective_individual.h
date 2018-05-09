#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
    #pragma GCC diagnostic ignored "-Wreorder"
	#pragma GCC diagnostic ignored "-Wsign-compare"
	#pragma GCC diagnostic ignored "-Wunused-variable"
#endif 

#ifndef __SINGLE_OBJECTIVE_INDIVIDUAL_H__
#define __SINGLE_OBJECTIVE_INDIVIDUAL_H__

#include"gene.h"
#include "dynamic_individual.h"

namespace types
{
	struct SingleObjectiveIndividual : public DynamicIndividual<Gene>
	{
		using DynamicIndividual::DynamicIndividual;

		double objective, constraint;
	};
}

#endif // !__SINGLE_OBJECTIVE_INDIVIDUAL_H__
