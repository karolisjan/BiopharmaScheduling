#ifndef __NSGA_INDIVIDUAL_H__
#define __NSGA_INDIVIDUAL_H__

#include"gene.h"
#include "dynamic_individual.h"

namespace types
{
	struct NSGAIndividual : public DynamicIndividual<Gene>
	{
		using DynamicIndividual::DynamicIndividual;

		std::vector<double> objectives;
		double constraint;

		double d;
		int rank, n;
		std::vector<int> Sp;
	};
}

#endif // !__NSGA_INDIVIDUAL_H__
