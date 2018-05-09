#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
    #pragma GCC diagnostic ignored "-Wreorder"
	#pragma GCC diagnostic ignored "-Wsign-compare"
#endif 

#ifndef  __SINGLE_OBJECTIVE_GA_H__
#define __SINGLE_OBJECTIVE_GA_H__

#include <numeric>

#include "base_ga.h"
#include "utils.h"

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
			auto on_objective_and_constraint = [](Individual& c1, Individual& c2)
			{
				if (c1.constraint && c2.constraint)
					return c1.constraint < c2.constraint;
				if (!c1.constraint && !c2.constraint)
					return c1.objective > c2.objective;
				if (!c1.constraint && c2.constraint)
					return true;
				return false;
			};

			std::sort(offspring.begin(), offspring.end(), on_objective_and_constraint);
			Population combo(parents.size() + offspring.size());
			std::merge(parents.begin(), parents.end(), offspring.begin(), offspring.end(), combo.begin(), on_objective_and_constraint);
			parents = Population(combo.begin(), combo.begin() + parents.size());
		}

		Individual Tournament(Individual& p, Individual& q) override
		{
			if (p.constraint < q.constraint)
				return p;
			if (p.constraint > q.constraint)
				return q;

			if (p.objective > q.objective)
				return p;
			if (p.objective < q.objective)
				return q;

			if (utils::random() < 0.50)
				return p;
			return q;
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

			parents.clear();
			while (popsize-- > 0)
				parents.push_back(Individual(params...));

			for (auto &p : parents)
				fitness_functor(p);

			// Sorts in an descending order of objective 
			// and ascending order of constraint values
			std::sort(parents.begin(), parents.end(),
				[](Individual& c1, Individual& c2)
			{
				if (c1.constraint && c2.constraint)
					return c1.constraint < c2.constraint;
				if (!c1.constraint && !c2.constraint)
					return c1.objective > c2.objective;
				if (!c1.constraint && c2.constraint)
					return true;
				return false;
			});
		}

		void Update() override
		{
			Select();
			Reproduce();

			#pragma omp parallel for 
			for (int i = 0; i < offspring.size(); ++i)
				fitness_functor(offspring[i]);

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
				[](Individual& c1, Individual& c2)
			{
				if (c1.constraint && c2.constraint)
					return c1.constraint < c2.constraint;
				if (!c1.constraint && !c2.constraint)
					return c1.objective > c2.objective;
				if (!c1.constraint && c2.constraint)
					return true;
				return false;
			});

			return solutions[0];
		}
	};
}

#endif //!__SINGLE_OBJECTIVE_GA_H__

