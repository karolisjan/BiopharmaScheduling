#define CATCH_CONFIG_MAIN  
#include "catch.hpp"

#include <unordered_map>

#include "gene.h"
#include "nsgaii.h"
#include "single_objective_ga.h"
#include "scheduling_models.h"


struct Chromosome
{
	Chromosome() {}
	Chromosome(std::vector<double> objectives, std::vector<double> constraints) : objectives(objectives), constraints(constraints) {}

	std::vector<double> objectives, constraints;
	std::vector<int> S;
	int rank, n;
	double d;
};

template<class Chromosome>
static inline int CheckDominance(const Chromosome &p, const Chromosome &q)
{
	// Constraints are expected to be normalised in 0.0 - 1.0 range
	double p_constraints = 0.0, q_constraints = 0.0;

	for (const auto &constraint_val : p.constraints) {
		p_constraints += constraint_val;
	}

	for (const auto &constraint_val : q.constraints) {
		q_constraints += constraint_val;
	}

	// If either p or q is infeasible
	if (p_constraints != utils::Approx(q_constraints)) { // Checks for floating point 'equality'
		return (p_constraints < q_constraints) ? 1 : -1;
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

SCENARIO("Dominance test")
{
	Chromosome i1({ -580.5, 500.0 }, { 34.1 });
	Chromosome i2({ -359.9, 934.4 }, { 66.4 });

	REQUIRE( CheckDominance(i1, i2) == 1 );
	REQUIRE( CheckDominance(i2, i1) == -1 );

	Chromosome i3({ -568.9, 217.4 }, { 0.0 });
	Chromosome i4({ -596.4, 193.4 }, { 0.0 });

	REQUIRE( CheckDominance(i3, i4) == -1 );
	REQUIRE( CheckDominance(i4, i3) == 1 );


	Chromosome i5({ -628.6, 454.6 }, { 0.0 });
	Chromosome i6({ -625.5, 477.1 }, { 0.0 });

	REQUIRE( CheckDominance(i5, i6) == 1 );
	REQUIRE( CheckDominance(i6, i5) == -1 );
}

template<class Population>
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

SCENARIO("Non-dominated sort test (unconstrained)")
{
	Chromosome i1({ -1, 1 }, { 0 });
	Chromosome i2({ -1, 2 }, { 0 });
	Chromosome i3({ -1, 3 }, { 0 });
	Chromosome i4({ -2, 1 }, { 0 });
	Chromosome i5({ -2, 2 }, { 0 });
	Chromosome i6({ -2, 3 }, { 0 });
	Chromosome i7({ -3, 1 }, { 0 });
	Chromosome i8({ -3, 2 }, { 0 });
	Chromosome i9({ -3, 3 }, { 0 });
	Chromosome i10({ -4, 1 }, { 0 });
	Chromosome i11({ -4, 2 }, { 0 });
	Chromosome i12({ -4, 3 }, { 0 });

	std::vector<Chromosome> R = { i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12 };
	std::vector<std::vector<Chromosome>> F;

	NonDominatedSort(R, F);

	REQUIRE( F.size() == 6 );
	REQUIRE( F[0].size() == 1 );
	REQUIRE( F[1].size() == 2 );
	REQUIRE( F[2].size() == 3 );
	REQUIRE( F[3].size() == 3 );
	REQUIRE( F[4].size() == 2 );
	REQUIRE( F[5].size() == 1 );
	REQUIRE( (F[5][0].objectives[0] == -1 && F[5][0].objectives[1] == 3) == true );
}

SCENARIO("Non-dominated sort test (constrained)")
{
	Chromosome i1({ -1, 1 }, { 0 });
	Chromosome i2({ -1, 2 }, { 3.0 });
	Chromosome i3({ -1, 3 }, { 0 });
	Chromosome i4({ -2, 1 }, { 1.1 });
	Chromosome i5({ -2, 2 }, { 0 });
	Chromosome i6({ -2, 3 }, { 0.5, 0.4 });
	Chromosome i7({ -3, 1 }, { 0.0 });
	Chromosome i8({ -3, 2 }, { 2.0 });
	Chromosome i9({ -3, 3 }, { 0 });
	Chromosome i10({ -4, 1 }, { 1.1 });
	Chromosome i11({ -4, 2 }, { 0 });
	Chromosome i12({ -4, 3 }, { 1.0, 0.1 });

	std::vector<Chromosome> R = { i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12 };
	std::vector<std::vector<Chromosome>> F;

	NonDominatedSort(R, F);

	REQUIRE( F.size() == 8 );
	REQUIRE( F[0].size() == 2 );
	REQUIRE( F[1].size() == 3 );
	REQUIRE( F[2].size() == 1 );
	REQUIRE( F[3].size() == 1 );
	REQUIRE( F[4].size() == 1 );
	REQUIRE( F[5].size() == 2 );
	REQUIRE (F[6].size() == 1 );
	REQUIRE (F[7].size() == 1 );
	REQUIRE( (F[7][0].objectives[0] == -1 && F[7][0].objectives[1] == 2 && F[7][0].constraints[0] == Approx(3.0)) == true );
}

template<typename T>
void MinMaxScaler(std::vector<T> &v)
{
	auto less_than = [](const auto &i1, const auto &i2){ return i1 < i2; };
	
	double min, max;
	auto normalise = [&min, &max](auto &i){ i = (i - min) / (max - min); };

	max = *std::max_element(v.begin(), v.end(), less_than);
	min = *std::min_element(v.begin(), v.end(), less_than);

	for (auto &i : v) {
		normalise(i);
	}
}

SCENARIO("Min-max scaler test")
{
	std::vector<double> v(100);
	for (auto &i : v) {
		i = (double)utils::random_int(1, 100);
	}
	
	MinMaxScaler(v);

	double max = *std::max_element(v.begin(), v.end());
	double min = *std::min_element(v.begin(), v.end());

	REQUIRE( max == utils::Approx(1.0) );
	REQUIRE( min == utils::Approx(0.0) );
}

template<typename T>
void SortInAscendingOrder(std::vector<T> &v)
{
	std::sort(v.begin(), v.end(), [](const auto &i1, const auto &i2) { return i1 < i2; });
}

SCENARIO("Sorting in ascending order test")
{
	std::vector<int> v(100);
	for (auto &i : v) {
		i = utils::random_int(0, 1000);
	}

	SortInAscendingOrder(v);

	REQUIRE(v[0] == *std::min_element(v.begin(), v.end()));
	REQUIRE(v.back() == *std::max_element(v.begin(), v.end()));
}

struct DummyChromosome
{
	std::vector<double> constraints;
};

inline void NormaliseConstraints(std::vector<DummyChromosome> &parents, std::vector<DummyChromosome> &offspring)
{
	auto num_constraints = parents.back().constraints.size();

	if (num_constraints <= 1) {
		return;
	}

	size_t i = 0;
	auto less_than = [&i](const auto &i1, const auto &i2) { return i1.constraints[i] < i2.constraints[i]; };

	double min, max;
	auto normalise = [&i, &min, &max](auto &ind) {
		ind.constraints[i] = (ind.constraints[i] - min) / (max - min);
	};

	if (offspring.empty()) {
		for (; i < num_constraints; ++i) {
			max = (*std::max_element(parents.begin(), parents.end(), less_than)).constraints[i];
			min = (*std::min_element(parents.begin(), parents.end(), less_than)).constraints[i];

			if (abs(max - min) < utils::EPSILON) {
				continue;
			}

			for (auto &p : parents) {
				normalise(p);
			}
		}
	}
	else {
		for (; i < num_constraints; ++i) {
			max = std::max(
				*std::max_element(parents.begin(), parents.end(), less_than),
				*std::max_element(offspring.begin(), offspring.end(), less_than),
				less_than
			).constraints[i];

			min = std::min(
				*std::min_element(parents.begin(), parents.end(), less_than),
				*std::min_element(offspring.begin(), offspring.end(), less_than),
				less_than
			).constraints[i];

			if (abs(max - min) < utils::EPSILON) {
				continue;
			}

			for (auto &p : parents) {
				normalise(p);
			}

			for (auto &o : offspring) {
				normalise(o);
			}
		}
	}
}

SCENARIO("Constraints normalisation test")
{
	std::vector<DummyChromosome> parents(100), offspring;

	for (auto &i : parents) {
		i.constraints = { (double)utils::random_int(1, 100), (double)utils::random_int(100, 1000) };
	}

	NormaliseConstraints(parents, offspring);

	int i;
	auto less_than = [&i](const auto &i1, const auto &i2) { return i1.constraints[i] < i2.constraints[i]; };

	for (i = 0; i < parents.back().constraints.size(); ++i) {
		auto max = (*std::max_element(parents.begin(), parents.end(), less_than)).constraints[i];
		REQUIRE(max == utils::Approx(1.0));

		auto min = (*std::min_element(parents.begin(), parents.end(), less_than)).constraints[i];
		REQUIRE(min == utils::Approx(0.0));
	}

	offspring.resize(100);
	for (auto &i : offspring) {
		i.constraints = { (double)utils::random_int(101, 200), (double)utils::random_int(10001, 100000) };
	}

	NormaliseConstraints(parents, offspring);

	for (i = 0; i < parents.back().constraints.size(); ++i) {
		auto max = (*std::max_element(offspring.begin(), offspring.end(), less_than)).constraints[i];
		REQUIRE(max == utils::Approx(1.0));

		auto min = (*std::min_element(parents.begin(), parents.end(), less_than)).constraints[i];
		REQUIRE(min == utils::Approx(0.0));
	}
}

SCENARIO("Range-based binary search for test")
{
	GIVEN("A mock vector of due dates: { 10, 25, 30, 50, 60, 100, 101}")
	{
		std::vector<int> due_dates = { 10, 25, 30, 50, 60, 100, 101};

		WHEN("The value is 24")
		{
			THEN("utils::search should return 1") 
			{
				REQUIRE( utils::search(due_dates, 24) == 1 );
			}
		}

		WHEN("The value is 25")
		{
			THEN("utils::search should return 1") 
			{
				REQUIRE( utils::search(due_dates, 25) == 2 );
			}
		}

		WHEN("The value is 52")
		{
			THEN("utils::search should return 6") 
			{
				REQUIRE( utils::search(due_dates, 52) == 4 );
			}
		}

		WHEN("The value is 101")
		{
			THEN("utils::search should return 6") 
			{
				REQUIRE( utils::search(due_dates, 101) == -1 );
			}
		}

		WHEN("The value is 1")
		{
			THEN("utils::search should return 0") 
			{
				REQUIRE( utils::search(due_dates, 1) == 0 );
			}
		}

		WHEN("The value is 110")

		{
			THEN("utils::search should return -1") 
			{
				REQUIRE( utils::search(due_dates, 110) == -1 );
			}
		}
	}
}

SCENARIO("deterministic::SingleSiteSimpleModel::CreateSchedule test without additional storage limits and shelf-life constraints.") 
{
	std::unordered_map<deterministic::OBJECTIVES, int> objectives;
	objectives.emplace(deterministic::TOTAL_KG_INVENTORY_DEFICIT, -1);
	objectives.emplace(deterministic::TOTAL_KG_THROUGHPUT, 1);

	std::unordered_map<deterministic::OBJECTIVES, std::pair<int, double>> constraints;
	constraints.emplace(deterministic::TOTAL_KG_BACKLOG, std::make_pair(-1, 0));

	std::vector<std::vector<double>> kg_demand = { 
		{ 0,0,3.1,0,0,3.1,0,3.1,3.1,3.1,0,6.2,6.2,3.1,6.2,0,3.1,9.3,0,6.2,6.2,0,6.2,9.3,0,9.3,6.2,3.1,6.2,3.1,0,9.3,6.2,9.3,6.2,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6.2,0,0,0,0,0,6.2,0,0,0,0,0,0,6.2 },
		{ 0,0,0,0,0,0,4.9,4.9,0,0,0,9.8,4.9,0,4.9,0,0,4.9,9.8,0,0,0,4.9,4.9,0,9.8,0,0,4.9,9.8,9.8,0,4.9,9.8,4.9,0 },
		{ 0,5.5,5.5,0,5.5,5.5,5.5,5.5,5.5,0,11,5.5,0,5.5,5.5,11,5.5,5.5,0,5.5,5.5,5.5,11,5.5,0,11,0,11,5.5,5.5,0,11,11,0,5.5,5.5 },
	};

	// 6-month kg inventoy target
	size_t num_months = 6, p = 0, t;
	std::vector< std::vector<double> > kg_inventory_target(kg_demand.size());
	for (; p < kg_demand.size(); ++p) {		
		for (t = 0; t < kg_demand[p].size() - num_months; ++t) {
			kg_inventory_target[p].push_back(std::accumulate(kg_demand[p].begin() + t, kg_demand[p].begin() + t + num_months, 0.0));
		}
		for (; t < kg_demand[p].size(); ++t) {
			kg_inventory_target[p].push_back(kg_inventory_target[p].back());
		}
	}

	std::vector<int> days_per_period = std::vector<int>{ 
		31,31,28,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31,31,29,31,30,31,30,31,31,30,31,30
	};

	std::vector<double> kg_yield_per_batch = { 3.1, 6.2, 4.9, 5.5 };
	std::vector<double> kg_storage_limits = { 3000, 3000, 3000, 3000 }; // set high to ignore
	std::vector<double> kg_opening_stock = { 18.6, 0, 19.6, 110 };

	std::vector<double> inventory_penalty_per_kg = { 0, 0, 0, 0 };
	std::vector<double> backlog_penalty_per_kg = { 0, 0, 0, 0 };
	std::vector<double> production_cost_per_kg = { 0, 0, 0, 0 };
	std::vector<double> storage_cost_per_kg = { 0, 0, 0, 0 };
	std::vector<double> waste_cost_per_kg = { 0, 0, 0, 0 };
	std::vector<double> sell_price_per_kg = { 0, 0, 0, 0 };

	std::vector<int> inoculation_days = { 20, 15, 20, 26 };
	std::vector<int> seed_days = { 11, 7, 11, 9 };
	std::vector<int> production_days = { 14, 14, 14, 14 };
	std::vector<int> usp_days = { 45, 36, 45, 49 };
	std::vector<int> dsp_days = { 7, 11, 7, 7 };
	std::vector<int> shelf_life_days = { 3000, 3000, 3000, 3000 }; // set high to ignore
	std::vector<int> approval_days = { 90, 90, 90, 90 };
	std::vector<int> min_batches_per_campaign = { 1, 1, 1, 1 };
	std::vector<int> max_batches_per_campaign = { 1000, 1000, 1000, 1000 };
	std::vector<int> batches_multiples_of_per_campaign = { 1, 1, 1, 1 };
	std::vector<std::vector<int>> changeover_days = {
		{ 0,  20, 20, 20 },
		{ 20,  0, 20, 20 },
		{ 20, 20,  0, 20 },
		{ 20, 20, 20,  0 }
	};


	deterministic::SingleSiteSimpleInputData input_data(
		objectives,
		kg_demand,
		days_per_period,

		kg_opening_stock,
		kg_yield_per_batch,
		kg_storage_limits,

		inventory_penalty_per_kg,
		backlog_penalty_per_kg,
		production_cost_per_kg,
		storage_cost_per_kg,
		waste_cost_per_kg,
		sell_price_per_kg,		

		inoculation_days,
		seed_days,
		production_days,
		usp_days,
		dsp_days,
		approval_days,
		shelf_life_days,
		min_batches_per_campaign,
		max_batches_per_campaign,
		batches_multiples_of_per_campaign,
		changeover_days,

		&kg_inventory_target,
		&constraints
	);

	deterministic::SingleSiteSimpleModel single_site_simple_model(input_data);
	
	GIVEN("Known solutions.")
	{
		WHEN("The solution has 8 genes and is known to correspond to a schedule with 8 different " +
		"campaigns with a total throughput of 570 kg, inventory deficit of 34 kg, and 0 kg backlog.")
		{
			types::NSGAChromosome<types::SingleSiteSimpleGene> known_solution;

			known_solution.genes.resize(7);

			known_solution.genes[0].product_num = 3;
			known_solution.genes[0].num_batches = 10;
			known_solution.genes[1].product_num = 1;
			known_solution.genes[1].num_batches = 21;
			known_solution.genes[2].product_num = 2;
			known_solution.genes[2].num_batches = 3;
			known_solution.genes[3].product_num = 4;
			known_solution.genes[3].num_batches = 20;
			known_solution.genes[4].product_num = 1;
			known_solution.genes[4].num_batches = 16;
			known_solution.genes[5].product_num = 3;
			known_solution.genes[5].num_batches = 16;
			known_solution.genes[6].product_num = 1;
			known_solution.genes[6].num_batches = 45;

			types::SingleSiteSimpleSchedule schedule;
			single_site_simple_model.CreateSchedule(known_solution, schedule);

			THEN("CreateSchedule creates a schedule with 8 different campaigns and the correct total kg throughput, inventory deficit, and backlog.")
			{
				REQUIRE( schedule.campaigns.size() == known_solution.genes.size() );
				REQUIRE( schedule.objectives[deterministic::TOTAL_KG_THROUGHPUT] == utils::Approx(510.2) );
				REQUIRE( schedule.objectives[deterministic::TOTAL_KG_INVENTORY_DEFICIT] == utils::Approx(14.2) );
				REQUIRE( schedule.objectives[deterministic::TOTAL_KG_BACKLOG] == utils::Approx(0.0) );
				
				for (size_t i = 0; i < known_solution.genes.size(); ++i) {
					REQUIRE( schedule.campaigns[i].product_num == known_solution.genes[i].product_num );
					REQUIRE( schedule.campaigns[i].num_batches == known_solution.genes[i].num_batches );
				}
			}
		}

		WHEN("The solution has 8 genes. 7 of the genes correspond to 7 valid campaigns but the last gene is known to be beyond the horizon.")
		{
			types::NSGAChromosome<types::SingleSiteSimpleGene> input, output;

			input.genes.resize(8);

			input.genes[0].product_num = 3;
			input.genes[0].num_batches = 11;
			input.genes[1].product_num = 1;
			input.genes[1].num_batches = 31;
			input.genes[2].product_num = 4;
			input.genes[2].num_batches = 18;
			input.genes[3].product_num = 2;
			input.genes[3].num_batches = 4;
			input.genes[4].product_num = 3;
			input.genes[4].num_batches = 12;
			input.genes[5].product_num = 1;
			input.genes[5].num_batches = 17;
			input.genes[6].product_num = 4;
			input.genes[6].num_batches = 37;
			input.genes[7].product_num = 1;
			input.genes[7].num_batches = 37;

			output.genes.resize(7);

			output.genes[0].product_num = 3;
			output.genes[0].num_batches = 11;
			output.genes[1].product_num = 1;
			output.genes[1].num_batches = 31;
			output.genes[2].product_num = 4;
			output.genes[2].num_batches = 18;
			output.genes[3].product_num = 2;
			output.genes[3].num_batches = 4;
			output.genes[4].product_num = 3;
			output.genes[4].num_batches = 12;
			output.genes[5].product_num = 1;
			output.genes[5].num_batches = 17;
			output.genes[6].product_num = 4;
			output.genes[6].num_batches = 37;

			types::SingleSiteSimpleSchedule schedule;
			single_site_simple_model.CreateSchedule(input, schedule);

			THEN("CreateSchedule correctly creates a schedule with 7 different campaigns cutting off the last one.")
			{
				REQUIRE( schedule.campaigns.size() == output.genes.size() );
				
				for (size_t i = 0; i < schedule.campaigns.size(); ++i) {
					REQUIRE( schedule.campaigns[i].product_num == output.genes[i].product_num );
					REQUIRE( schedule.campaigns[i].num_batches == output.genes[i].num_batches );
				}
			}
		}

		WHEN("The solution has 8 genes but two consecutive genes have the same product number.")
		{
			types::NSGAChromosome<types::SingleSiteSimpleGene> input, output;

			input.genes.resize(8);

			input.genes[0].product_num = 3;
			input.genes[0].num_batches = 11;
			input.genes[1].product_num = 1;
			input.genes[1].num_batches = 31;
			input.genes[2].product_num = 4;
			input.genes[2].num_batches = 9;
			input.genes[3].product_num = 4;
			input.genes[3].num_batches = 9;
			input.genes[4].product_num = 2;
			input.genes[4].num_batches = 4;
			input.genes[5].product_num = 3;
			input.genes[5].num_batches = 12;
			input.genes[6].product_num = 1;
			input.genes[6].num_batches = 17;
			input.genes[7].product_num = 4;
			input.genes[7].num_batches = 37;

			output.genes.resize(7);

			output.genes[0].product_num = 3;
			output.genes[0].num_batches = 11;
			output.genes[1].product_num = 1;
			output.genes[1].num_batches = 31;
			output.genes[2].product_num = 4;
			output.genes[2].num_batches = 18;
			output.genes[3].product_num = 2;
			output.genes[3].num_batches = 4;
			output.genes[4].product_num = 3;
			output.genes[4].num_batches = 12;
			output.genes[5].product_num = 1;
			output.genes[5].num_batches = 17;
			output.genes[6].product_num = 4;
			output.genes[6].num_batches = 37;

			types::SingleSiteSimpleSchedule schedule;
			single_site_simple_model.CreateSchedule(input, schedule);

			THEN("CreateSchedule correctly creates a schedule with 7 different campaigns concatenating the two campaigns with the same product number.")
			{
				REQUIRE( schedule.campaigns.size() == output.genes.size() );
				
				for (size_t i = 0; i < schedule.campaigns.size(); ++i) {
					REQUIRE( schedule.campaigns[i].product_num == output.genes[i].product_num );
					REQUIRE( schedule.campaigns[i].num_batches == output.genes[i].num_batches );
				}
			}
		}

		WHEN("The solution with 14 genes has multiple consecutive one with the same product number and some beyond the horizon.")
		{
			types::NSGAChromosome<types::SingleSiteSimpleGene> input, output;

			input.genes.resize(13);

			input.genes[0].product_num = 3;
			input.genes[0].num_batches = 5;
			input.genes[1].product_num = 3;
			input.genes[1].num_batches = 6;
			input.genes[2].product_num = 1;
			input.genes[2].num_batches = 31;
			input.genes[3].product_num = 4;
			input.genes[3].num_batches = 9;
			input.genes[4].product_num = 4;
			input.genes[4].num_batches = 9;
			input.genes[5].product_num = 2;
			input.genes[5].num_batches = 4;
			input.genes[6].product_num = 3;
			input.genes[6].num_batches = 12;
			input.genes[7].product_num = 1;
			input.genes[7].num_batches = 15;
			input.genes[8].product_num = 1;
			input.genes[8].num_batches = 2;
			input.genes[9].product_num = 4;
			input.genes[9].num_batches = 20;
			input.genes[10].product_num = 4;
			input.genes[10].num_batches = 10;
			input.genes[11].product_num = 4;
			input.genes[11].num_batches = 37;
			input.genes[12].product_num = 1;
			input.genes[12].num_batches = 1;

			output.genes.resize(7);

			output.genes[0].product_num = 3;
			output.genes[0].num_batches = 11;
			output.genes[1].product_num = 1;
			output.genes[1].num_batches = 31;
			output.genes[2].product_num = 4;
			output.genes[2].num_batches = 18;
			output.genes[3].product_num = 2;
			output.genes[3].num_batches = 4;
			output.genes[4].product_num = 3;
			output.genes[4].num_batches = 12;
			output.genes[5].product_num = 1;
			output.genes[5].num_batches = 17;
			output.genes[6].product_num = 4;
			output.genes[6].num_batches = 37;

			types::SingleSiteSimpleSchedule schedule;
			single_site_simple_model.CreateSchedule(input, schedule);

			THEN("CreateSchedule correctly creates a schedule with 7 different campaigns concatenating the campaigns with the same product number and removing the ones beyond the horizon.")
			{
				REQUIRE( schedule.campaigns.size() == output.genes.size() );

				for (size_t i = 0; i < schedule.campaigns.size(); ++i) {
					REQUIRE( schedule.campaigns[i].product_num == output.genes[i].product_num );
					REQUIRE( schedule.campaigns[i].num_batches == output.genes[i].num_batches );
				}
			}
		}
	}
}

SCENARIO("deterministic::SingleSiteMultiSuiteModel::CreateSchedule test")
{
	GIVEN("Example 1 base case data and a known solution")
	{
		THEN("deterministic::SingleSiteMultiSuiteModel::CreateSchedule creates a schedule with a total profit of 518")
		{
			std::unordered_map<deterministic::OBJECTIVES, int> objectives;
			objectives.emplace(deterministic::TOTAL_PROFIT, -1);
			
			std::vector<std::vector<int>> demand =
			{
				{ 0, 0, 0, 6, 0, 6 },
				{ 0, 0, 6, 0, 0, 0 },
				{ 0, 8, 0, 0, 8, 0 }
			};

			std::vector<int> days_per_period = { 60, 60, 60, 60, 60, 60 };

			int num_usp_suites = 2, num_dsp_suites = 2;

			std::vector<double> sales_price = { 20, 20, 20 };
			std::vector<double> usp_production_cost = { 2, 2, 2 };
			std::vector<double> dsp_production_cost = { 2, 2, 2 };
			std::vector<double> waste_disposal_cost = { 1, 1, 1 };
			std::vector<double> storage_cost = { 1, 1, 1 };
			std::vector<double> backlog_penalty = { 20, 20, 20 };
			std::vector<double> usp_changeover_cost = { 1, 1, 1 };
			std::vector<double> dsp_changeover_cost = { 1, 1, 1 };
		
			std::vector<double> usp_days = { 20, 22, 12.5 };
			std::vector<double> dsp_days = { 10, 10, 10 };

			std::vector<std::vector<double>> usp_changeovers = { 
				{ 10, 10, 10  },
				{ 10, 10, 10 },
				{ 10, 10, 10 }
			};

			std::vector<std::vector<double>> dsp_changeovers = { 
				{ 10,   10,   10   },
				{ 10,   10,   10   },
				{ 12.5, 12.5, 12.5 }
			};

			std::vector<int> shelf_life = { 180, 180, 180 };
			std::vector<int> storage_cap = { 40, 40, 40 };

			deterministic::SingleSiteMultiSuiteInputData input_data(
				objectives,

				num_usp_suites,
				num_dsp_suites,

				demand,
				days_per_period,

				usp_days,
				dsp_days,
				
				shelf_life,
				storage_cap,

				sales_price,
				storage_cost,
				backlog_penalty,
				waste_disposal_cost,
				usp_production_cost,
				dsp_production_cost,
				usp_changeover_cost,
				dsp_changeover_cost,

				usp_changeovers,
				dsp_changeovers
			);

			deterministic::SingleSiteMultiSuiteModel single_site_multi_suite_model(input_data);

			types::SingleObjectiveChromosome<types::SingleSiteMultiSuiteGene> i;

			i.genes.resize(5);

			i.genes[0].usp_suite_num = 1;
			i.genes[0].product_num = 2;
			i.genes[0].num_batches = 6;

			i.genes[1].usp_suite_num = 1;
			i.genes[1].product_num = 1;
			i.genes[1].num_batches = 2;

			i.genes[2].usp_suite_num = 1;
			i.genes[2].product_num = 3;
			i.genes[2].num_batches = 7;

			i.genes[3].usp_suite_num = 2;
			i.genes[3].product_num = 3;
			i.genes[3].num_batches = 9;

			i.genes[4].usp_suite_num = 2;
			i.genes[4].product_num = 1;
			i.genes[4].num_batches = 10;

			types::SingleSiteMultiSuiteSchedule schedule;
			single_site_multi_suite_model.CreateSchedule(i, schedule);

			REQUIRE(schedule.objectives[deterministic::TOTAL_PROFIT] == Approx(518.0));
		}
	}

	GIVEN("Increased demand for p1 during last time period, i.e. 9 batches due by the end of t6")
	{
		THEN("deterministic::SingleSiteMultiSuiteModel::CreateSchedule creates a schedule with a total profit of 563")
		{
			std::unordered_map<deterministic::OBJECTIVES, int> objectives;
			objectives.emplace(deterministic::TOTAL_PROFIT, -1);
			
			std::vector<std::vector<int>> demand =
			{
				{ 0, 0, 0, 6, 0, 9 },
				{ 0, 0, 6, 0, 0, 0 },
				{ 0, 8, 0, 0, 8, 0 }
			};

			std::vector<int> days_per_period = { 60, 60, 60, 60, 60, 60 };

			int num_usp_suites = 2, num_dsp_suites = 2;

			std::vector<double> sales_price = { 20, 20, 20 };
			std::vector<double> usp_production_cost = { 2, 2, 2 };
			std::vector<double> dsp_production_cost = { 2, 2, 2 };
			std::vector<double> waste_disposal_cost = { 1, 1, 1 };
			std::vector<double> storage_cost = { 1, 1, 1 };
			std::vector<double> backlog_penalty = { 20, 20, 20 };
			std::vector<double> usp_changeover_cost = { 1, 1, 1 };
			std::vector<double> dsp_changeover_cost = { 1, 1, 1 };
		
			std::vector<double> usp_days = { 20, 22, 12.5 };
			std::vector<double> dsp_days = { 10, 10, 10 };

			std::vector<std::vector<double>> usp_changeovers = {
				{ 10, 10, 10 },
				{ 10, 10, 10 },
				{ 10, 10, 10 }
			};

			std::vector<std::vector<double>> dsp_changeovers = {
				{ 10,   10,   10 },
				{ 10,   10,   10 },
				{ 12.5, 12.5, 12.5 }
			};

			std::vector<int> shelf_life = { 180, 180, 180 };
			std::vector<int> storage_cap = { 40, 40, 40 };

			deterministic::SingleSiteMultiSuiteInputData input_data(
				objectives, 

				num_usp_suites,
				num_dsp_suites,

				demand,
				days_per_period,

				usp_days,
				dsp_days,
				
				shelf_life,
				storage_cap,

				sales_price,
				storage_cost,
				backlog_penalty,
				waste_disposal_cost,
				usp_production_cost,
				dsp_production_cost,
				usp_changeover_cost,
				dsp_changeover_cost,

				usp_changeovers,
				dsp_changeovers
			);

			deterministic::SingleSiteMultiSuiteModel single_site_multi_suite_model(input_data);

			types::SingleObjectiveChromosome<types::SingleSiteMultiSuiteGene> i;

			i.genes.resize(6);

			i.genes[0].usp_suite_num = 1;
			i.genes[0].product_num = 3;
			i.genes[0].num_batches = 1;

			i.genes[1].usp_suite_num = 1;
			i.genes[1].product_num = 2;
			i.genes[1].num_batches = 6;

			i.genes[2].usp_suite_num = 1;
			i.genes[2].product_num = 3;
			i.genes[2].num_batches = 8;

			i.genes[3].usp_suite_num = 1;
			i.genes[3].product_num = 1;
			i.genes[3].num_batches = 3;

			i.genes[4].usp_suite_num = 2;
			i.genes[4].product_num = 3;
			i.genes[4].num_batches = 7;

			i.genes[5].usp_suite_num = 2;
			i.genes[5].product_num = 1;
			i.genes[5].num_batches = 12;

			types::SingleSiteMultiSuiteSchedule schedule;
			single_site_multi_suite_model.CreateSchedule(i, schedule);

			REQUIRE(schedule.objectives[deterministic::TOTAL_PROFIT] == Approx(563.0));
		}
	}
}

SCENARIO("stochastic::SingleSiteSimpleModel::CreateSchedule test with deterministic input and without additional storage limits and shelf-life constraints.") 
{
	int mc_seed = 7;
	int num_mc_sims = 100;

	std::unordered_map<stochastic::OBJECTIVES, int> objectives;
	objectives.emplace(stochastic::TOTAL_KG_INVENTORY_DEFICIT_MEAN, -1);
	objectives.emplace(stochastic::TOTAL_KG_THROUGHPUT_MEAN, 1);

	std::unordered_map<stochastic::OBJECTIVES, std::pair<int, double>> constraints;
	constraints.emplace(stochastic::TOTAL_KG_BACKLOG_MEAN, std::make_pair(-1, 0));

	// Kg demand
	std::vector<std::vector<double>> kg_demand_min = {
		{ 0,0,3.1,0,0,3.1,0,3.1,3.1,3.1,0,6.2,6.2,3.1,6.2,0,3.1,9.3,0,6.2,6.2,0,6.2,9.3,0,9.3,6.2,3.1,6.2,3.1,0,9.3,6.2,9.3,6.2,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6.2,0,0,0,0,0,6.2,0,0,0,0,0,0,6.2 },
		{ 0,0,0,0,0,0,4.9,4.9,0,0,0,9.8,4.9,0,4.9,0,0,4.9,9.8,0,0,0,4.9,4.9,0,9.8,0,0,4.9,9.8,9.8,0,4.9,9.8,4.9,0 },
		{ 0,5.5,5.5,0,5.5,5.5,5.5,5.5,5.5,0,11,5.5,0,5.5,5.5,11,5.5,5.5,0,5.5,5.5,5.5,11,5.5,0,11,0,11,5.5,5.5,0,11,11,0,5.5,5.5 },
	};

	std::vector<std::vector<double>> kg_demand_mode = {
		{ 0,0,3.1,0,0,3.1,0,3.1,3.1,3.1,0,6.2,6.2,3.1,6.2,0,3.1,9.3,0,6.2,6.2,0,6.2,9.3,0,9.3,6.2,3.1,6.2,3.1,0,9.3,6.2,9.3,6.2,0 },
	    { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6.2,0,0,0,0,0,6.2,0,0,0,0,0,0,6.2 },
		{ 0,0,0,0,0,0,4.9,4.9,0,0,0,9.8,4.9,0,4.9,0,0,4.9,9.8,0,0,0,4.9,4.9,0,9.8,0,0,4.9,9.8,9.8,0,4.9,9.8,4.9,0 },
		{ 0,5.5,5.5,0,5.5,5.5,5.5,5.5,5.5,0,11,5.5,0,5.5,5.5,11,5.5,5.5,0,5.5,5.5,5.5,11,5.5,0,11,0,11,5.5,5.5,0,11,11,0,5.5,5.5 },
	};

	std::vector<std::vector<double>> kg_demand_max = {
		{ 0,0,3.1,0,0,3.1,0,3.1,3.1,3.1,0,6.2,6.2,3.1,6.2,0,3.1,9.3,0,6.2,6.2,0,6.2,9.3,0,9.3,6.2,3.1,6.2,3.1,0,9.3,6.2,9.3,6.2,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6.2,0,0,0,0,0,6.2,0,0,0,0,0,0,6.2 },
		{ 0,0,0,0,0,0,4.9,4.9,0,0,0,9.8,4.9,0,4.9,0,0,4.9,9.8,0,0,0,4.9,4.9,0,9.8,0,0,4.9,9.8,9.8,0,4.9,9.8,4.9,0 },
		{ 0,5.5,5.5,0,5.5,5.5,5.5,5.5,5.5,0,11,5.5,0,5.5,5.5,11,5.5,5.5,0,5.5,5.5,5.5,11,5.5,0,11,0,11,5.5,5.5,0,11,11,0,5.5,5.5 },
	};


	// 6-month kg inventoy target
	size_t num_months = 6, p = 0, t;
	std::vector< std::vector<double> > kg_inventory_target(kg_demand_mode.size());
	for (; p < kg_demand_mode.size(); ++p) {		
		for (t = 0; t < kg_demand_mode[p].size() - num_months; ++t) {
			kg_inventory_target[p].push_back(std::accumulate(kg_demand_mode[p].begin() + t, kg_demand_mode[p].begin() + t + num_months, 0.0));
		}
		for (; t < kg_demand_mode[p].size(); ++t) {
			kg_inventory_target[p].push_back(kg_inventory_target[p].back());
		}
	}

	std::vector<int> days_per_period = std::vector<int>{ 
		31,31,28,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31,31,29,31,30,31,30,31,31,30,31,30
	};

	std::vector<double> kg_yield_per_batch_min = { 3.1, 6.2, 4.9, 5.5 };
	std::vector<double> kg_yield_per_batch_mode = { 3.1, 6.2, 4.9, 5.5 };
	std::vector<double> kg_yield_per_batch_max = { 3.1, 6.2, 4.9, 5.5 };

	std::vector<double> kg_storage_limits = { 3000, 3000, 3000, 3000 }; // set high to ignore
	std::vector<double> kg_opening_stock = { 18.6, 0, 19.6, 110 };

	std::vector<double> inventory_penalty_per_kg = { 0, 0, 0, 0 };
	std::vector<double> backlog_penalty_per_kg = { 0, 0, 0, 0 };
	std::vector<double> production_cost_per_kg = { 0, 0, 0, 0 };
	std::vector<double> storage_cost_per_kg = { 0, 0, 0, 0 };
	std::vector<double> waste_cost_per_kg = { 0, 0, 0, 0 };
	std::vector<double> sell_price_per_kg = { 0, 0, 0, 0 };

	std::vector<int> inoculation_days = { 20, 15, 20, 26 };
	std::vector<int> seed_days = { 11, 7, 11, 9 };
	std::vector<int> production_days = { 14, 14, 14, 14 };
	std::vector<int> usp_days = { 45, 36, 45, 49 };
	std::vector<int> dsp_days = { 7, 11, 7, 7 };
	std::vector<int> shelf_life_days = { 3000, 3000, 3000, 3000 }; // set high to ignore
	std::vector<int> approval_days = { 90, 90, 90, 90 };
	std::vector<int> min_batches_per_campaign = { 1, 1, 1, 1 };
	std::vector<int> max_batches_per_campaign = { 1000, 1000, 1000, 1000 };
	std::vector<int> batches_multiples_of_per_campaign = { 1, 1, 1, 1 };
	std::vector<std::vector<int>> changeover_days = {
		{ 0,  20, 20, 20 },
		{ 20,  0, 20, 20 },
		{ 20, 20,  0, 20 },
		{ 20, 20, 20,  0 }
	};


	stochastic::SingleSiteSimpleInputData input_data(
		mc_seed,
		num_mc_sims,

		objectives,
		days_per_period,

		kg_demand_min,
		kg_demand_mode,
		kg_demand_max,

		kg_yield_per_batch_min,
		kg_yield_per_batch_mode,
		kg_yield_per_batch_max,

		kg_opening_stock,
		kg_storage_limits,

		inventory_penalty_per_kg,
		backlog_penalty_per_kg,
		production_cost_per_kg,
		storage_cost_per_kg,
		waste_cost_per_kg,
		sell_price_per_kg,		

		inoculation_days,
		seed_days,
		production_days,
		usp_days,
		dsp_days,
		approval_days,
		shelf_life_days,
		min_batches_per_campaign,
		max_batches_per_campaign,
		batches_multiples_of_per_campaign,
		changeover_days,

		&kg_inventory_target,
		&constraints
	);

	stochastic::SingleSiteSimpleModel single_site_simple_model(input_data);
	
	GIVEN("Known solution.")
	{
		WHEN("The solution has 8 genes and is known to correspond to a schedule with 8 different " +
		"campaigns with a total throughput of 570 kg, inventory deficit of 34 kg, and 0 kg backlog.")
		{
			types::NSGAChromosome<types::SingleSiteSimpleGene> known_solution;

			known_solution.genes.resize(7);

			known_solution.genes[0].product_num = 3;
			known_solution.genes[0].num_batches = 10;
			known_solution.genes[1].product_num = 1;
			known_solution.genes[1].num_batches = 21;
			known_solution.genes[2].product_num = 2;
			known_solution.genes[2].num_batches = 3;
			known_solution.genes[3].product_num = 4;
			known_solution.genes[3].num_batches = 20;
			known_solution.genes[4].product_num = 1;
			known_solution.genes[4].num_batches = 16;
			known_solution.genes[5].product_num = 3;
			known_solution.genes[5].num_batches = 16;
			known_solution.genes[6].product_num = 1;
			known_solution.genes[6].num_batches = 45;

			types::SingleSiteSimpleSchedule schedule;
			single_site_simple_model.CreateSchedule(known_solution, schedule);

			THEN("CreateSchedule creates a schedule with 8 different campaigns and the correct total kg throughput, inventory deficit, and backlog.")
			{
				REQUIRE( schedule.campaigns.size() == known_solution.genes.size() );
				REQUIRE( schedule.objectives[stochastic::TOTAL_KG_THROUGHPUT_MEAN] == utils::Approx(510.2) );
				REQUIRE( schedule.objectives[stochastic::TOTAL_KG_INVENTORY_DEFICIT_MEAN] == utils::Approx(14.2) );
				REQUIRE( schedule.objectives[stochastic::TOTAL_KG_BACKLOG_MEAN] == utils::Approx(0.0) );
				
				for (size_t i = 0; i < known_solution.genes.size(); ++i) {
					REQUIRE( schedule.campaigns[i].product_num == known_solution.genes[i].product_num );
					REQUIRE( schedule.campaigns[i].num_batches == known_solution.genes[i].num_batches );
				}
			}
		}		
	}
}