#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
    #pragma GCC diagnostic ignored "-Wreorder"
	#pragma GCC diagnostic ignored "-Wsign-compare"
	#pragma GCC diagnostic ignored "-Wunused-variable"
	#pragma GCC diagnostic ignored "-Wformat="
#endif 

#ifndef __SINGLE_OBJECTIVE_GA_H__
#define __SINGLE_OBJECTIVE_GA_H__

#include <utility>
#include <numeric>

#include "base_ga.h"


namespace algorithms
{
	template<class Chromosome, class FitnessFunction>
	class SingleObjectiveGA : public BaseGA<Chromosome, FitnessFunction>
	{
		using BaseGA<Chromosome, FitnessFunction>::BaseGA;
		using BaseGA<Chromosome, FitnessFunction>::Select;
		using BaseGA<Chromosome, FitnessFunction>::Reproduce;
		using BaseGA<Chromosome, FitnessFunction>::fitness_function;
		using BaseGA<Chromosome, FitnessFunction>::indices;
		using BaseGA<Chromosome, FitnessFunction>::parents;
		using BaseGA<Chromosome, FitnessFunction>::offspring;

		typedef typename BaseGA<Chromosome, FitnessFunction>::Population Population;

		// Updates the parents with the best individuals from the offspring population.
		void Replace()
		{
			auto on_objective_and_constraints = [](const Chromosome &p, const Chromosome &q)
			{
                // If either p or q is infeasible
                if (p.constraints != utils::Approx(q.constraints)) {
                    return p.constraints < q.constraints;
                }	

                return p.objective < q.objective;
			};

			std::sort(offspring.begin(), offspring.end(), on_objective_and_constraints);
			Population combo(parents.size() + offspring.size());

			std::merge(
                std::make_move_iterator(parents.begin()),
                std::make_move_iterator(parents.end()), 
                std::make_move_iterator(offspring.begin()),
                std::make_move_iterator(offspring.end()), 
                combo.begin(), 
                on_objective_and_constraints
            );

			parents = Population(
                std::make_move_iterator(combo.begin()), 
                std::make_move_iterator(combo.begin() + parents.size())
            );
		}

		inline bool Tournament(const Chromosome &p, const Chromosome &q) override
		{	
            // If either p or q is infeasible
            if (p.constraints != utils::Approx(q.constraints)) {
                return p.constraints < q.constraints;
            }	

			if (p.objective < q.objective) {
				return true;
			}
			else if (p.objective > q.objective) {
				return false;
			}

			return utils::random() < 0.5;
		}

	public:
		// Creates new parent population.
		template<class... ChromosomeParams>
		void Init(
			int popsize,
			ChromosomeParams... params
		)
		{
			indices.resize(popsize);
			std::iota(indices.begin(), indices.end(), 0);
			parents.reserve(popsize);
			offspring.reserve(popsize);
			parents.resize(0);

			while (popsize-- > 0) {
				parents.push_back(std::move(Chromosome(params...)));
			}

			#pragma omp parallel for
			for (int i = 0; i < parents.size(); ++i) {
				fitness_function(parents[i]);
			}

			// Sorts in an descending order of objective 
			// and ascending order of constraints values
			std::sort(parents.begin(), parents.end(),
				[](const Chromosome &p, const Chromosome &q)
                {
                    // If either p or q is infeasible
                    if (p.constraints != utils::Approx(q.constraints)) {
                        return p.constraints < q.constraints;
                    }	

                    return p.objective < q.objective;
                }
            );
		}

		void Update()
		{
			Select();
			Reproduce();

			#pragma omp parallel for 
			for (int i = 0; i < offspring.size(); ++i) {
				fitness_function(offspring[i]);
            }

			Replace();
		}

		// Returns top parent individual.
		Chromosome Top()
		{
			// No need to sort parents because of the merge in Replace().
			return parents[0];
		}

		// Returns top parent individual.
		Chromosome Top(Population solutions)
		{
			std::sort(solutions.begin(), solutions.end(),
				[](const Chromosome &p, const Chromosome &q)
                {
                    // If either p or q is infeasible
                    if (p.constraints != utils::Approx(q.constraints)) {
                        return p.constraints < q.constraints;
                    }	

                    return p.objective < q.objective;
                }
            );

			return std::move(solutions[0]);
		}
	};
}

#endif 