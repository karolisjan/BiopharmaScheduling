#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
 	// #pragma GCC diagnostic ignored "-Wreorder"
	// #pragma GCC diagnostic ignored "-Wunused-variable"
	#pragma GCC diagnostic ignored "-Wformat="
	#pragma GCC diagnostic ignored "-Wsign-compare"
#endif 

#ifndef  __GENE_H__
#define __GENE_H__

#include <utility>

#include "utils.h"


namespace types
{
	struct SingleSiteMultiSuiteGene
	{
		SingleSiteMultiSuiteGene() {}

		SingleSiteMultiSuiteGene(
			int num_products,
			int num_usp_suites,
			double p_product_mut,
			double p_usp_suite_mut,
			double p_plus_batch_mut,
			double p_minus_batch_mut
		) 
		// :
		// 	num_products(num_products),
		// 	num_usp_suites(num_usp_suites),
		// 	p_product_mut(p_product_mut),
		// 	p_usp_suite_mut(p_usp_suite_mut),
		// 	p_plus_batch_mut(p_plus_batch_mut),
		// 	p_minus_batch_mut(p_minus_batch_mut),
		// 	num_batches(1)
		{
			this->num_products = num_products,
			this->num_usp_suites = num_usp_suites,
			this->p_product_mut = p_product_mut,
			this->p_usp_suite_mut = p_usp_suite_mut,
			this->p_plus_batch_mut = p_plus_batch_mut,
			this->p_minus_batch_mut = p_minus_batch_mut,
			this->num_batches = 1;
			this->product_num = utils::random_int(1, num_products);
			this->usp_suite_num = utils::random_int(1, num_usp_suites);
		}

		SingleSiteMultiSuiteGene make_new()
		{
			return std::move(
				SingleSiteMultiSuiteGene(
					num_products, 
					num_usp_suites, 
					p_product_mut, 
					p_usp_suite_mut, 
					p_plus_batch_mut, 
					p_minus_batch_mut
				)
			);
		}

		inline void Mutate()
		{
			mutate_product_num();
			mutate_usp_suite_num();
			mutate_num_batches();
		}

		int product_num;
		int usp_suite_num;
		int num_batches;

	private:
		inline void mutate_product_num()
		{
			if (utils::random() >= p_product_mut) {
				return;
			}

			int random_product_num = 0;
			do { random_product_num = utils::random_int(1, num_products); }
			while (product_num == random_product_num);
			product_num = random_product_num;
		}

		inline void mutate_usp_suite_num()
		{
			if (utils::random() >= p_usp_suite_mut) {
				return;
			}

			int random_usp_suite_num = 0;
			do { random_usp_suite_num = utils::random_int(1, num_usp_suites); } 
			while (usp_suite_num == random_usp_suite_num);
			usp_suite_num = random_usp_suite_num;
		}

		inline void mutate_num_batches()
		{
			if (utils::random() < p_plus_batch_mut) {
				num_batches += 1;
			}

			if (num_batches > 0 && utils::random() < p_minus_batch_mut) {
				num_batches -= 1;
			}
		}

		int num_products;
		int num_usp_suites;
		double p_product_mut;
		double p_usp_suite_mut;
		double p_plus_batch_mut;
		double p_minus_batch_mut;
	};
	

	struct SingleSiteSimpleGene
	{
		SingleSiteSimpleGene() {}

		SingleSiteSimpleGene(
			int num_products,
			double p_product_mut,
			double p_plus_batch_mut,
			double p_minus_batch_mut
		) 
		// :
		// 	num_products(num_products),
		// 	num_batches(1),
		// 	p_product_mut(p_product_mut),
		// 	p_plus_batch_mut(p_plus_batch_mut),
		// 	p_minus_batch_mut(p_minus_batch_mut)
		{
			this->num_products = num_products;
			this->num_batches = 1;
			this->p_product_mut = p_product_mut;
			this->p_plus_batch_mut = p_plus_batch_mut;
			this->p_minus_batch_mut = p_minus_batch_mut;
			this->product_num = utils::random_int(1, num_products);
		}

		SingleSiteSimpleGene make_new()
		{
			return std::move(
				SingleSiteSimpleGene(
					num_products,
					p_product_mut, 
					p_plus_batch_mut, 
					p_minus_batch_mut
				)
			);
		}

		inline void Mutate()
		{
			mutate_product_num();
			mutate_num_batches();
		}

		int product_num;
		int num_batches;

	private:
		inline void mutate_product_num()
		{
			if (utils::random() >= p_product_mut) {
				return;
			}

			int random_product_num = 0;
			do { random_product_num = utils::random_int(1, num_products); }
			while (product_num == random_product_num);
			product_num = random_product_num;
		}

		inline void mutate_num_batches()
		{
			if (utils::random() < p_plus_batch_mut) {
				num_batches += 1;
			}

			if (num_batches > 1 && utils::random() < p_minus_batch_mut) {
				num_batches -= 1;
			}
		}

		int num_products;
		double p_product_mut;
		double p_plus_batch_mut;
		double p_minus_batch_mut;
	};
}

#endif 


