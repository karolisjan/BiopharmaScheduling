#ifndef __SINGLE_OBJECTIVE_INDIVIDUAL_H__
#define __SINGLE_OBJECTIVE_INDIVIDUAL_H__

#include <vector>

#include "base_individual.h"


namespace types
{
	template<class Gene>
	struct SingleObjectiveIndividual : public BaseIndividual<Gene>
	{
		using BaseIndividual<Gene>::BaseIndividual;

		double objective;
        double constraints;
	};
}

#endif 