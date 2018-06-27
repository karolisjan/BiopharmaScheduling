#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
    #pragma GCC diagnostic ignored "-Wreorder"
	#pragma GCC diagnostic ignored "-Wsign-compare"
#endif 

#ifndef  __SINGLE_OBJECTIVE_GA_H__
#define __SINGLE_OBJECTIVE_GA_H__

#include <utility>
#include <numeric>

#include "base_ga.h"


namespace algorithms
{
	template<class Individual, class FitnessFunctor>
	class SingleObjectiveGA : public BaseGA<Individual, FitnessFunctor>
	{
		using BaseGA<Individual, FitnessFunctor>::BaseGA;
		using BaseGA<Individual, FitnessFunctor>::Select;
		using BaseGA<Individual, FitnessFunctor>::Reproduce;
		using BaseGA<Individual, FitnessFunctor>::fitness_functor;
		using BaseGA<Individual, FitnessFunctor>::indices;
		using BaseGA<Individual, FitnessFunctor>::parents;
		using BaseGA<Individual, FitnessFunctor>::offspring;

		typedef typename BaseGA<Individual, FitnessFunctor>::Population Population;

		// Updates the parents with the best individuals from the offspring population.
		void Replace()
		{
			auto on_objective_and_constraints = [](const Individual &p, const Individual &q)
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

		inline bool Tournament(const Individual &p, const Individual &q) override
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
				fitness_functor(parents[i]);
			}

			// Sorts in an descending order of objective 
			// and ascending order of constraints values
			std::sort(parents.begin(), parents.end(),
				[](const Individual &p, const Individual &q)
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
				fitness_functor(offspring[i]);
            }

			Replace();
		}

		// Returns top parent individual.
		Individual Top()
		{
			// No need to sort parents because of the merge in Replace().
			return parents[0];
		}

		// Returns top parent individual.
		Individual Top(Population solutions)
		{
			std::sort(solutions.begin(), solutions.end(),
				[](const Individual &p, const Individual &q)
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