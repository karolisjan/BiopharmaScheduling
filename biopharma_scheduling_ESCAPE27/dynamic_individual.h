#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
    #pragma GCC diagnostic ignored "-Wreorder"
	#pragma GCC diagnostic ignored "-Wsign-compare"
	#pragma GCC diagnostic ignored "-Wunused-variable"
#endif 

#ifndef __DYNAMIC_INDIVIDUAL_H__
#define __DYNAMIC_INDIVIDUAL_H__

#include <utility>
#include <functional>
#include <cstdlib>
#include <vector>
#include <algorithm>

#include "utils.h"

namespace types
{
	/*
	Base dynamic individual class. Not to be called directly.
	*/
	template<class Gene>
	struct DynamicIndividual
	{
		explicit DynamicIndividual() {}

		template<class... GeneParams>
		explicit DynamicIndividual(
			double p_xo = 0.820769,
			double p_gene_swap = 0.766782,
			GeneParams... params
		) :
			p_xo(p_xo),
			p_gene_swap(p_gene_swap)

		{
			genes = { Gene(params...) };
		}

		void cross(DynamicIndividual& other) 
		{
			if (utils::random() >= p_xo)
				return;

			size_t i, shortest = (genes.size() < other.genes.size()) ? genes.size() : other.genes.size();

			if (shortest < 4)
				return;

			for (i = 0; i != shortest; ++i) {
				if (utils::random() >= 0.50)
					continue;
					
				std::swap(genes[i], other.genes[i]);
			}
		}

		inline void mutate() 
		{
			for (auto &gene : genes)
				gene.mutate();
			
			add_gene();
			swap_genes();
		}

		std::vector<Gene> genes;

	private:
		inline void add_gene()
		{
			genes.push_back(genes.back().make_new());
		}

		inline void swap_genes()
		{
			if (utils::random() >= p_gene_swap)
				return;

			size_t g1, g2;

			do {
				g1 = utils::random_int(0, genes.size());
				g2 = utils::random_int(0, genes.size());
			} while (g1 == g2);

			std::swap(genes[g1], genes[g2]);
		}

		double p_xo, p_gene_swap;
	};
}

#endif //!__DYNAMIC_INDIVIDUAL_H__