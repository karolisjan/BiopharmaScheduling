#ifndef  __SINGLE_OBJECTIVE_GA_H__
#define __SINGLE_OBJECTIVE_GA_H__

#include "base_ga.h"
#include "utils.h"

using namespace std;

namespace algorithms
{
	template<class Individual, class FitnessFunctor>
	class SingleObjectiveGA : public BaseGA<Individual, FitnessFunctor>
	{
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

			sort(offspring.begin(), offspring.end(), on_objective_and_constraint);
			Population combo(parents.size() + offspring.size());
			merge(parents.begin(), parents.end(), offspring.begin(), offspring.end(), combo.begin(), on_objective_and_constraint);
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

			if (random() < 0.50)
				return p;
			return q;
		}

	public:
		using BaseGA::BaseGA;

		// Creates new parent population.
		template<class... IndividualParams>
		void Init(
			int popsize,
			IndividualParams... params
		)
		{
			indices.resize(popsize);
			iota(indices.begin(), indices.end(), 0);

			parents.clear();
			while (popsize-- > 0)
				parents.push_back(Individual(params...));

			for (auto &p : parents)
				fitness_functor(p);

			// Sorts in an descending order of objective 
			// and ascending order of constraint values
			sort(parents.begin(), parents.end(),
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

		Individual Top()
		{
			fitness_functor(parents[0]);
			return parents[0];
		}

		Individual Top(Population solutions)
		{
			sort(solutions.begin(), solutions.end(),
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

