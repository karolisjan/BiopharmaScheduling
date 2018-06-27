#ifndef __NSGA_INDIVIDUAL_H__
#define __NSGA_INDIVIDUAL_H__

#include "gene.h"
#include "base_individual.h"


namespace types
{
	template<class Gene>
	class NSGAIndividual : public BaseIndividual<Gene>
	{
	public:
		using BaseIndividual<Gene>::BaseIndividual;

		std::vector<double> objectives; // All objectives are minimised
		double constraints; 

		double d; // Crowding distance
		int rank; // Domination rank
		int n; // Number of solutions which dominate this solution
		std::vector<int> S; // Set of solutions (indices) that are dominated by this solution
	};
}

#endif 
