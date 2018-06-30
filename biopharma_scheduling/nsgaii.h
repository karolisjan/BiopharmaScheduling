#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
    #pragma GCC diagnostic ignored "-Wreorder"
	#pragma GCC diagnostic ignored "-Wsign-compare"
#endif 

#ifndef __NSGAII_H__
#define __NSGAII_H__

#include <numeric>
#include <utility>
#include <limits>
#include <algorithm>

#include "utils.h"
#include "base_ga.h"


namespace algorithms
{
	/*
		Multi-objective genetic algorithm based on NSGA-II 
		Deb, K., Pratap, A., Agarwal, S. and Meyarivan, T.A.M.T., 2002. A fast and elitist multiobjective genetic algorithm: NSGA-II. IEEE transactions on evolutionary computation, 6(2), pp.182-197.
		http://ieeexplore.ieee.org/document/996017/?reload=true
	*/
	template<class Individual, class FitnessFunction>
	class NSGAII : public BaseGA<Individual, FitnessFunction>
	{
		using BaseGA<Individual, FitnessFunction>::BaseGA;
		using BaseGA<Individual, FitnessFunction>::Select;
		using BaseGA<Individual, FitnessFunction>::Reproduce;
		using BaseGA<Individual, FitnessFunction>::fitness_function;
		using BaseGA<Individual, FitnessFunction>::indices;
		using BaseGA<Individual, FitnessFunction>::parents;
		using BaseGA<Individual, FitnessFunction>::offspring;

		typedef typename BaseGA<Individual, FitnessFunction>::Population Population;

		Population top_front;

		/*
			Checks the dominance.

			1 if p dominates q
			-1 if q dominates p
			0 if both are non-dominated
		*/
		static inline int CheckDominance(const Individual &p, const Individual &q)
		{
			// If either p or q is infeasible
			if (p.constraints != utils::Approx(q.constraints)) { // Checks for floating point 'equality'
				return (p.constraints  < q.constraints) ? 1 : -1;
			}	

			bool p_dominates = false, q_dominates = false; 

			for (size_t m = 0; m != p.objectives.size(); ++m) {
				if (p.objectives[m] < q.objectives[m]) {
					p_dominates = true;
				}

				if (p.objectives[m] > q.objectives[m]) {
					q_dominates = true;
				}
			}

			if (p_dominates && !q_dominates) {
				return 1;
			}
			else if (!p_dominates && q_dominates) {
				return -1;
			}

			return 0;
		}

		/*
			Returns true if p wins the tournament against q, false otherwise.
		*/
		inline bool Tournament(const Individual &p, const Individual &q) override
		{	
			int domination_flag = CheckDominance(p, q);

			// If p is better than q in all objectives or has better constraint satisfaction
			if (domination_flag == 1) {
				return true;
			}
			else if (domination_flag == -1) {
				return false;
			}

			if (p.d > q.d) {
				return true;
			}
			else if (p.d < q.d) {
				return false;
			}

			return utils::random() < 0.5;
		}

		void NonDominatedSort(Population &R, std::vector<Population> &F)
		{
			for (auto &i : R) {
				i.S.resize(0);
				i.S.reserve(R.size() / 2);
				i.n = 0;
			}

			// First first_front
			F.resize(1);
			for (size_t p = 0; p != R.size(); ++p) {
				for (size_t q = p + 1; q != R.size(); ++q) {
					auto domination_flag = CheckDominance(R[p], R[q]);

					// If p dominates q
					if (domination_flag == 1) {
						R[p].S.push_back(q);
						++R[q].n;
					}
					// If q dominates p
					else if (domination_flag == -1) {
						R[q].S.push_back(p);
						++R[p].n;
					}
				}

				if (R[p].n == 0) {
					R[p].rank = 1;
					F[0].push_back(std::move(R[p]));
				}
			}

			size_t i = 0;
			Population Q;

			while (1) {
				Q.resize(0);

				for (auto &p : F[i]) {
					for (size_t q : p.S) {
						--R[q].n;

						if (R[q].n == 0) {
							R[q].rank = i + 2; // +2 because i starts at 0
							Q.push_back(R[q]);
						}
					}
				}

				if (Q.empty()) {
					break;
				}

				F.push_back(std::move(Q));
				++i;
			}
		}

		void CalculateCrowdingDistance(Population &I)
		{
			for (auto &i : I) {
				i.d = 0;
			}

			I[0].d = I.back().d = std::numeric_limits<int>::infinity();

			if (I.size() > 2) {
				for (size_t m = 0; m != I[0].objectives.size(); ++m) {
					std::sort(I.begin(), I.end(), [&m](const auto &i1, const auto &i2) { return i1.objectives[m] < i2.objectives[m]; });

					double min = I[0].objectives[m], max = I.back().objectives[m], abs_max_min = std::fabs(max - min);

					if (abs_max_min != utils::Approx(0.0)) {
						for (size_t k = 1; k != I.size() - 1; ++k) {
							I[k].d += (std::fabs(I[k + 1].objectives[m] - I[k - 1].objectives[m]) / abs_max_min);
						}
					}
				}
			}
		}

		void Rank()
		{
			size_t popsize = parents.size();

			parents.insert(
				parents.end(), 
				std::make_move_iterator(offspring.begin()),
				std::make_move_iterator(offspring.end())
			);

			std::vector<Population> F;
			NonDominatedSort(parents, F);

			parents.resize(0);
			size_t i = 0;

			for (; i != F.size(); ++i) {
				CalculateCrowdingDistance(F[i]);

				if (parents.size() + F[i].size() > popsize) {
					break;
				}
				else {
					parents.insert(parents.end(), F[i].begin(), F[i].end());
				}
			}

			if (parents.size() < popsize) {
				std::sort(F[i].begin(), F[i].end(), [](const auto& i1, const auto &i2){ return i1.d > i2.d; });
				parents.insert(parents.end(), F[i].begin(), F[i].begin() + (popsize - parents.size()));
			}

			top_front = std::move(F[0]);
		}

	public:
		template<class... IndividualParams>
		void Init(
			int popsize,
			IndividualParams... params
		)
		{
			indices.resize(popsize);
			std::iota(indices.begin(), indices.end(), 0);
			parents.reserve(popsize);
			offspring.reserve(popsize);
			parents.resize(0);

			while (popsize-- > 0) {
				parents.push_back(std::move(Individual(params...)));
			}

			#pragma omp parallel for
			for (int i = 0; i < parents.size(); ++i) {
				fitness_function(parents[i]);
			}
		}

		void Update()
		{
			Rank();
			Select();
			Reproduce();

			#pragma omp parallel for 
			for (int i = 0; i < offspring.size(); ++i) {
				fitness_function(offspring[i]);
			}
		}

		// TODO: Review performance
		Population TopFront()
		{
			std::sort(
				top_front.begin(),
				top_front.end(),
				[](const Individual &i1, const Individual &i2) { 
					return i1.objectives[0] > i2.objectives[0]; 
				}
			);
			
			auto duplicates_begin = unique(
				top_front.begin(), 
				top_front.end(), 
				[](const Individual &i1, const Individual &i2) {
					for (size_t m = 0; m < i1.objectives.size(); ++m) {
						if (i1.objectives[m] != utils::Approx(i2.objectives[m])) {
							return false;
						}
					}
					return true;
				}
			);
			
			if (duplicates_begin != top_front.end()) {
				top_front.erase(duplicates_begin, top_front.end());
			}

			return std::move(top_front);
		}

		Population TopFront(Population R)
		{
			std::vector<Population> F;
			NonDominatedSort(R, F);
			top_front = std::move(F[0]);
			return std::move(TopFront());
		}
	};
}

#endif 
