#ifndef  __NSGAII_H__
#define __NSGAII_H__

#include <omp.h>
#include <numeric>

#include "base_ga.h"
#include "utils.h"

namespace algorithms
{
	/*
		Multi-objective genetic algorithm based on NSGA-II 
		Deb, K., Pratap, A., Agarwal, S. and Meyarivan, T.A.M.T., 2002. A fast and elitist multiobjective genetic algorithm: NSGA-II. IEEE transactions on evolutionary computation, 6(2), pp.182-197.
		http://ieeexplore.ieee.org/document/996017/?reload=true
	*/
	template<class Individual, class FitnessFunctor>
	class NSGAII : public BaseGA<Individual, FitnessFunctor>
	{
		using BaseGA<Individual, FitnessFunctor>::BaseGA;
		using BaseGA<Individual, FitnessFunctor>::Select;
		using BaseGA<Individual, FitnessFunctor>::Reproduce;
		using BaseGA<Individual, FitnessFunctor>::fitness_functor;
		using BaseGA<Individual, FitnessFunctor>::indices;
		using BaseGA<Individual, FitnessFunctor>::parents;
		using BaseGA<Individual, FitnessFunctor>::offspring;

		typedef typename BaseGA<Individual, FitnessFunctor>::Population Population;

		Population top_front;

		bool Dominates(Individual& p, Individual& q)
		{
			if (p.constraint < q.constraint)
				return true;

			if (p.constraint > q.constraint)
				return false;

			bool dominates = false;

			for (int m = 0; m < p.objectives.size(); ++m) {
				if (p.objectives[m] < q.objectives[m])
					dominates = true;
				if (p.objectives[m] > q.objectives[m])
					return false;
			}

			return dominates;
		}

		Individual Tournament(Individual& p, Individual& q) override
		{
			if (Dominates(p, q))
				return p;
			if (Dominates(q, p))
				return q;

			if (p.d > q.d)
				return p;
			if (q.d < p.d)
				return q;

			if (utils::random() < 0.5)
				return p;
			return q;
		}

		void FastNonDominatedSort(Population& R, std::vector<Population>& fronts)
		{
			Population front;

			for (int p = 0; p < R.size(); ++p) {
				R[p].Sp.clear();
				R[p].n = 0;

				for (int q = 0; q < R.size(); ++q) {
					if (p == q)
						continue;

					if (Dominates(R[p], R[q]))
						R[p].Sp.push_back(q);
					else if (Dominates(R[q], R[p]))
						R[p].n++;
				}

				if (R[p].n == 0) {
					R[p].rank = 1;
					front.push_back(R[p]);
				}
			}

			fronts.push_back(front);
			front.clear();

			int i = 0;

			while (i < fronts.size()) {
				Population Q;

				for (int p = 0; p < fronts[i].size(); p++) {
					for (int q = 0; q < fronts[i][p].Sp.size(); q++) {
						int index = fronts[i][p].Sp[q];
						R[index].n--;

						if (R[index].n == 0) {
							R[index].rank = i + 2;
							Q.push_back(R[index]);
						}
					}
				}

				++i;

				if (Q.size())
					fronts.push_back(Q);
			}
		}

		void CalculateCrowdingDistance(Population &front)
		{
			for (int j = 0; j < front.size(); ++j) {
				front[j].d = 0;
			}

			if (front.size() == 1) {
				front[0].d = INT_MAX;
			}
			else if (front.size() == 2) {
				front[0].d = front[1].d = INT_MAX;
			}
			else if (front.size() > 2) {
				for (int m = 0; m < front[0].objectives.size(); ++m) {
					sort(front.begin(), front.end(),
						[&m](Individual i1, Individual i2) { return i1.objectives[m] < i2.objectives[m]; });

					float min = front[0].objectives[m],
						max = front.back().objectives[m];

					front[0].d = INT_MAX;
					front.back().d = INT_MAX;

					if (front[0].objectives[m] != front.back().objectives[m]) {
						for (int k = 1; k < front.size() - 1; ++k) {
							front[k].d += ((front[k + 1].objectives[m] - front[k - 1].objectives[m]) / (max - min));
						}
					}
				}
			}
		}

		Population Rank()
		{
			int popsize = parents.size();

			parents.insert(parents.end(), offspring.begin(), offspring.end());

			std::vector<Population> all_fronts;
			FastNonDominatedSort(parents, all_fronts);
			parents.clear();
			offspring.clear();

			int i = 0;
			for (; i < all_fronts.size(); i++) {
				CalculateCrowdingDistance(all_fronts[i]);

				if (parents.size() + all_fronts[i].size() > popsize) {
					break;
				}
				else {
					parents.insert(parents.end(), all_fronts[i].begin(), all_fronts[i].end());
				}
			}

			auto on_crowding_distance = [](Individual& i1, Individual& i2) { return i1.d > i2.d; };

			if (parents.size() < popsize) {
				std::sort(all_fronts[i].begin(), all_fronts[i].end(), on_crowding_distance);
				parents.insert(parents.end(), all_fronts[i].begin(), all_fronts[i].begin() + (popsize - parents.size()));
			}

			return all_fronts[0];
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
		}
		
		void Update() override
		{
			top_front = Rank();

			Select();
			Reproduce();

			#pragma omp parallel for 
			for (int i = 0; i < offspring.size(); ++i)
				fitness_functor(offspring[i]);
		}

		Population TopFront()
		{
			std::sort(top_front.begin(), top_front.end(),
				[](Individual& i1, Individual& i2) { return i1.objectives[0] > i2.objectives[0]; });
			
			auto duplicates_begin = unique(top_front.begin(), top_front.end(), 
				[](Individual& i1, Individual& i2) 
			{
				for (int m = 0; m < i1.objectives.size(); ++m) {
					if (abs(i1.objectives[m] - i2.objectives[m]) > 0.00001) {
						return false;
					}
				}

				return true;
			});
			
			if (duplicates_begin != top_front.end())
				top_front.erase(duplicates_begin, top_front.end());

			return top_front;
		}

		Population TopFront(Population R)
		{
			std::vector<Population> fronts;
			FastNonDominatedSort(R, fronts);

			top_front = fronts[0];

			return TopFront();
		}
	};
}

#endif //!__NSGAII_H__
