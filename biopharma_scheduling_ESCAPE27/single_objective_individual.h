#ifndef __SINGLE_OBJECTIVE_INDIVIDUAL_H__
#define __SINGLE_OBJECTIVE_INDIVIDUAL_H__

#include <vector>

#include "gene.h"
#include "base_individual.h"


namespace types
{
	template<class Gene>
	struct SingleObjectiveIndividual : public BaseIndividual<Gene>
	{
		using BaseIndividual::BaseIndividual;

		double objective;
        double constraints;
	};
}

#endif 