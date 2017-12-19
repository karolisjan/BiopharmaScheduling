#include "single_objective_ga.h"
#include "fitness.h"


//------------------------------------------------------------------
//				Lakhdar2005 Example 1 Scheduling Model
//------------------------------------------------------------------

// Fitness function test with a known continous-time 
// base case solution.
// The calculated profit should be 517.
void Lakhdar2005Ex1_BaseCaseGlobalOptimumTest()
{
	std::vector<std::vector<int>> demand =
	{
		{ 0, 0, 0, 6, 0, 6 },
		{ 0, 0, 6, 0, 0, 0 },
		{ 0, 8, 0, 0, 8, 0 }
	};

	std::vector<int> days_per_period = { 60, 60, 60, 60, 60, 60 };

	int num_products = demand.size();
	int num_periods = demand[0].size();
	int num_usp_suites = 2, num_dsp_suites = 2;

	std::vector<double> usp_storage_cost = { 5, 5, 5 };
	std::vector<double> sales_price = { 20, 20, 20 };
	std::vector<double> production_cost = { 2, 2, 2 };
	std::vector<double> waste_disposal_cost = { 1, 1, 1 };
	std::vector<double> dsp_storage_cost = { 1, 1, 1 };
	std::vector<double> backlog_penalty = { 20, 20, 20 };
	std::vector<double> changeover_cost = { 1, 1, 1 };

	std::vector<double> usp_days = { 20, 22, 12.5 };
	std::vector<double> usp_lead_days = { 10, 10, 10 };
	std::vector<double> usp_shelf_life = { 60, 60, 60 };
	std::vector<double> usp_storage_cap = { 10, 10, 10 };

	std::vector<double> dsp_days = { 10, 10, 10 };
	std::vector<double> dsp_lead_days = { 10, 10, 12.5 };
	std::vector<double> dsp_shelf_life = { 180, 180, 180 };
	std::vector<double> dsp_storage_cap = { 40, 40, 40 };

	deterministic::Lakhdar2005Ex1Model lakhdar2005ex1_model(
		num_usp_suites,
		num_dsp_suites,
		demand,
		days_per_period,
		usp_storage_cost,
		sales_price,
		production_cost,
		waste_disposal_cost,
		dsp_storage_cost,
		backlog_penalty,
		changeover_cost,
		usp_days,
		usp_lead_days,
		usp_shelf_life,
		usp_storage_cap,
		dsp_days,
		dsp_lead_days,
		dsp_shelf_life,
		dsp_storage_cap);

	types::SingleObjectiveIndividual i;
	i.genes.resize(5);

	i.genes[0].usp_suite_num = 1;
	i.genes[0].product_num = 3;
	i.genes[0].num_batches = 11;

	i.genes[1].usp_suite_num = 1;
	i.genes[1].product_num = 1;
	i.genes[1].num_batches = 9;

	i.genes[2].usp_suite_num = 2;
	i.genes[2].product_num = 2;
	i.genes[2].num_batches = 6;

	i.genes[3].usp_suite_num = 2;
	i.genes[3].product_num = 1;
	i.genes[3].num_batches = 3;

	i.genes[4].usp_suite_num = 2;
	i.genes[4].product_num = 3;
	i.genes[4].num_batches = 5;

	lakhdar2005ex1_model(i);

	printf("%.2f profit", i.objective);

	auto usp_schedule = lakhdar2005ex1_model.CreateUSPSchedule(i);
	auto dsp_schedule = lakhdar2005ex1_model.CreateDSPSchedule(usp_schedule);

	std::vector<std::vector<int>> inventory, sold, backlog, dsp_waste;
	deterministic::Lakhdar2005Ex1Model::Objectives objectives = lakhdar2005ex1_model.CalculateObjectives(usp_schedule, dsp_schedule, inventory, sold, dsp_waste, backlog);

	printf(", (%.2f, %.2f)\n", objectives.profit, objectives.backlog_cost);
}

// Fitness function test with a continuous-time solution
// to the problem of increased demand for product 1.
// The calculated profit should be 562.
void Lakhdar2005Ex1_IncreasedDemandGlobalOptimumTest()
{
	std::vector<std::vector<int>> demand =
	{
		{ 0, 0, 0, 6, 0, 9 },
		{ 0, 0, 6, 0, 0, 0 },
		{ 0, 8, 0, 0, 8, 0 }
	};

	std::vector<int> days_per_period = { 60, 60, 60, 60, 60, 60 };

	int num_products = demand.size();
	int num_periods = demand[0].size();
	int num_usp_suites = 2, num_dsp_suites = 2;

	std::vector<double> usp_storage_cost = { 5, 5, 5 };
	std::vector<double> sales_price = { 20, 20, 20 };
	std::vector<double> production_cost = { 2, 2, 2 };
	std::vector<double> waste_disposal_cost = { 1, 1, 1 };
	std::vector<double> dsp_storage_cost = { 1, 1, 1 };
	std::vector<double> backlog_penalty = { 20, 20, 20 };
	std::vector<double> changeover_cost = { 1, 1, 1 };

	std::vector<double> usp_days = { 20, 22, 12.5 };
	std::vector<double> usp_lead_days = { 10, 10, 10 };
	std::vector<double> usp_shelf_life = { 60, 60, 60 };
	std::vector<double> usp_storage_cap = { 10, 10, 10 };

	std::vector<double> dsp_days = { 10, 10, 10 };
	std::vector<double> dsp_lead_days = { 10, 10, 12.5 };
	std::vector<double> dsp_shelf_life = { 180, 180, 180 };
	std::vector<double> dsp_storage_cap = { 40, 40, 40 };

	deterministic::Lakhdar2005Ex1Model lakhdar2005ex1_model(
		num_usp_suites,
		num_dsp_suites,
		demand,
		days_per_period,
		usp_storage_cost,
		sales_price,
		production_cost,
		waste_disposal_cost,
		dsp_storage_cost,
		backlog_penalty,
		changeover_cost,
		usp_days,
		usp_lead_days,
		usp_shelf_life,
		usp_storage_cap,
		dsp_days,
		dsp_lead_days,
		dsp_shelf_life,
		dsp_storage_cap);

	types::SingleObjectiveIndividual i;
	i.genes.resize(7);

	i.genes[0].usp_suite_num = 1;
	i.genes[0].product_num = 3;
	i.genes[0].num_batches = 10;

	i.genes[1].usp_suite_num = 1;
	i.genes[1].product_num = 1;
	i.genes[1].num_batches = 10;

	i.genes[2].usp_suite_num = 2;
	i.genes[2].product_num = 2;
	i.genes[2].num_batches = 6;

	i.genes[3].usp_suite_num = 2;
	i.genes[3].product_num = 1;
	i.genes[3].num_batches = 2;

	i.genes[4].usp_suite_num = 2;
	i.genes[4].product_num = 3;
	i.genes[4].num_batches = 6;

	i.genes[5].usp_suite_num = 2;
	i.genes[5].product_num = 1;
	i.genes[5].num_batches = 3;

	i.genes[6].usp_suite_num = 2;
	i.genes[6].product_num = 1;
	i.genes[6].num_batches = 3;

	lakhdar2005ex1_model(i);

	printf("%.2f profit", i.objective);

	auto usp_schedule = lakhdar2005ex1_model.CreateUSPSchedule(i);
	auto dsp_schedule = lakhdar2005ex1_model.CreateDSPSchedule(usp_schedule);

	std::vector<std::vector<int>> inventory, sold, backlog, dsp_waste;
	deterministic::Lakhdar2005Ex1Model::Objectives objectives = lakhdar2005ex1_model.CalculateObjectives(usp_schedule, dsp_schedule, inventory, sold, dsp_waste, backlog);

	printf(", (%.2f, %.2f)\n", objectives.profit, objectives.backlog_cost);
}

// Fitness function test with a continuous-time solution
// to the problem of doubled demand profile, i.e. 2 years.
// The calculated profit should be 1007.
void Lakhdar2005Ex1_DoubledDemandProfileGlobalOptimumTest()
{
	std::vector<std::vector<int>> demand =
	{
		{ 0, 0, 0, 6, 0, 6, 0, 0, 0, 6, 0, 6 },
		{ 0, 0, 6, 0, 0, 0, 0, 0, 6, 0, 0, 0 },
		{ 0, 8, 0, 0, 8, 0, 0, 8, 0, 0, 8, 0 }
	};

	std::vector<int> days_per_period = { 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60 };

	int num_products = demand.size();
	int num_periods = demand[0].size();
	int num_usp_suites = 2, num_dsp_suites = 2;

	std::vector<double> usp_storage_cost = { 5, 5, 5 };
	std::vector<double> sales_price = { 20, 20, 20 };
	std::vector<double> production_cost = { 2, 2, 2 };
	std::vector<double> waste_disposal_cost = { 1, 1, 1 };
	std::vector<double> dsp_storage_cost = { 1, 1, 1 };
	std::vector<double> backlog_penalty = { 20, 20, 20 };
	std::vector<double> changeover_cost = { 1, 1, 1 };

	std::vector<double> usp_days = { 20, 22, 12.5 };
	std::vector<double> usp_lead_days = { 10, 10, 10 };
	std::vector<double> usp_shelf_life = { 60, 60, 60 };
	std::vector<double> usp_storage_cap = { 10, 10, 10 };

	std::vector<double> dsp_days = { 10, 10, 10 };
	std::vector<double> dsp_lead_days = { 10, 10, 12.5 };
	std::vector<double> dsp_shelf_life = { 180, 180, 180 };
	std::vector<double> dsp_storage_cap = { 40, 40, 40 };

	deterministic::Lakhdar2005Ex1Model lakhdar2005ex1_model(
		num_usp_suites,
		num_dsp_suites,
		demand,
		days_per_period,
		usp_storage_cost,
		sales_price,
		production_cost,
		waste_disposal_cost,
		dsp_storage_cost,
		backlog_penalty,
		changeover_cost,
		usp_days,
		usp_lead_days,
		usp_shelf_life,
		usp_storage_cap,
		dsp_days,
		dsp_lead_days,
		dsp_shelf_life,
		dsp_storage_cap);

	types::SingleObjectiveIndividual i;
	i.genes.resize(11);

	i.genes[0].usp_suite_num = 1;
	i.genes[0].product_num = 2;
	i.genes[0].num_batches = 6;

	i.genes[1].usp_suite_num = 1;
	i.genes[1].product_num = 1;
	i.genes[1].num_batches = 9;

	i.genes[2].usp_suite_num = 1;
	i.genes[2].product_num = 2;
	i.genes[2].num_batches = 6;

	i.genes[3].usp_suite_num = 1;
	i.genes[3].product_num = 1;
	i.genes[3].num_batches = 2;

	i.genes[4].usp_suite_num = 1;
	i.genes[4].product_num = 3;
	i.genes[4].num_batches = 1;

	i.genes[5].usp_suite_num = 2;
	i.genes[5].product_num = 3;
	i.genes[5].num_batches = 9;

	i.genes[6].usp_suite_num = 2;
	i.genes[6].product_num = 1;
	i.genes[6].num_batches = 3;

	i.genes[7].usp_suite_num = 2;
	i.genes[7].product_num = 3;
	i.genes[7].num_batches = 15;

	i.genes[8].usp_suite_num = 2;
	i.genes[8].product_num = 1;
	i.genes[8].num_batches = 4;

	i.genes[9].usp_suite_num = 2;
	i.genes[9].product_num = 3;
	i.genes[9].num_batches = 7;

	i.genes[10].usp_suite_num = 2;
	i.genes[10].product_num = 1;
	i.genes[10].num_batches = 6;

	lakhdar2005ex1_model(i);

	printf("%.2f profit", i.objective);

	auto usp_schedule = lakhdar2005ex1_model.CreateUSPSchedule(i);
	auto dsp_schedule = lakhdar2005ex1_model.CreateDSPSchedule(usp_schedule);

	std::vector<std::vector<int>> inventory, sold, backlog, dsp_waste;
	deterministic::Lakhdar2005Ex1Model::Objectives objectives = lakhdar2005ex1_model.CalculateObjectives(usp_schedule, dsp_schedule, inventory, sold, dsp_waste, backlog);

	printf(", (%.2f, %.2f)\n", objectives.profit, objectives.backlog_cost);
}

void Lakhdar2005Ex1_BaseCaseTest()
{
	int runs = 10, gens = 200, popsize = 100;

	int seed = 0;
	double p_xo = 0.131266;
	double p_product_mut = 0.131266;
	double p_usp_suite_mut = 0.131266;
	double p_dsp_suite_mut = 0.131266;
	double p_plus_batch_mut = 0.131266;
	double p_minus_batch_mut = 0.131266;
	double p_gene_swap = 0.131266;

	std::vector<std::vector<int>> demand =
	{
		{ 0, 0, 0, 6, 0, 6 },
		{ 0, 0, 6, 0, 0, 0 },
		{ 0, 8, 0, 0, 8, 0 }
	};

	std::vector<int> days_per_period = { 60, 60, 60, 60, 60, 60 };

	int num_products = demand.size();
	int num_periods = demand[0].size();
	int num_usp_suites = 2, num_dsp_suites = 2;

	std::vector<double> usp_storage_cost = { 5, 5, 5 };
	std::vector<double> sales_price = { 20, 20, 20 };
	std::vector<double> production_cost = { 2, 2, 2 };
	std::vector<double> waste_disposal_cost = { 1, 1, 1 };
	std::vector<double> dsp_storage_cost = { 1, 1, 1 };
	std::vector<double> backlog_penalty = { 20, 20, 20 };
	std::vector<double> changeover_cost = { 1, 1, 1 };

	std::vector<double> usp_days = { 20, 22, 12.5 };
	std::vector<double> usp_lead_days = { 10, 10, 10 };
	std::vector<double> usp_shelf_life = { 60, 60, 60 };
	std::vector<double> usp_storage_cap = { 10, 10, 10 };

	std::vector<double> dsp_days = { 10, 10, 10 };
	std::vector<double> dsp_lead_days = { 10, 10, 12.5 };
	std::vector<double> dsp_shelf_life = { 180, 180, 180 };
	std::vector<double> dsp_storage_cap = { 40, 40, 40 };

	deterministic::Lakhdar2005Ex1Model lakhdar2005ex1_model(
		num_usp_suites,
		num_dsp_suites,
		demand,
		days_per_period,
		usp_storage_cost,
		sales_price,
		production_cost,
		waste_disposal_cost,
		dsp_storage_cost,
		backlog_penalty,
		changeover_cost,
		usp_days,
		usp_lead_days,
		usp_shelf_life,
		usp_storage_cap,
		dsp_days,
		dsp_lead_days,
		dsp_shelf_life,
		dsp_storage_cap);

	algorithms::SingleObjectiveGA<types::SingleObjectiveIndividual, deterministic::Lakhdar2005Ex1Model> simple_ga(
		lakhdar2005ex1_model,
		seed
	);

	for (size_t run = 0; run < 10; ++run) {
		simple_ga.Init(
			popsize,
			p_xo,
			p_gene_swap,
			num_products,
			num_usp_suites,
			p_product_mut,
			p_usp_suite_mut,
			p_plus_batch_mut,
			p_minus_batch_mut
		);

		for (size_t gen = 0; gen < 1000; ++gen) {
			simple_ga.Update();

			printf("\rRun %d, Gen: %d, Best: %.2f, Constraint: %.2f",
				run + 1, gen + 1, simple_ga.Top().objective, simple_ga.Top().constraint);
		}

		auto best = simple_ga.Top();
		auto usp_schedule = lakhdar2005ex1_model.CreateUSPSchedule(best);
		auto dsp_schedule = lakhdar2005ex1_model.CreateDSPSchedule(usp_schedule);

		std::vector<std::vector<int>> inventory, sold, backlog, dsp_waste;
		deterministic::Lakhdar2005Ex1Model::Objectives objectives = lakhdar2005ex1_model.CalculateObjectives(usp_schedule, dsp_schedule, inventory, sold, dsp_waste, backlog);

		printf(", (%.2f, %.2f)", objectives.profit, objectives.backlog_cost);

		printf("\n");
	}
}

void Lakhdar2005Ex1_IncreasedDemandTest()
{
	int runs = 10, gens = 1000, popsize = 100;

	int seed = 0;
	double p_xo = 0.131266;
	double p_product_mut = 0.131266;
	double p_usp_suite_mut = 0.131266;
	double p_dsp_suite_mut = 0.131266;
	double p_plus_batch_mut = 0.131266;
	double p_minus_batch_mut = 0.131266;
	double p_gene_swap = 0.131266;

	std::vector<std::vector<int>> demand =
	{
		{ 0, 0, 0, 6, 0, 9 },
		{ 0, 0, 6, 0, 0, 0 },
		{ 0, 8, 0, 0, 8, 0 }
	};

	std::vector<int> days_per_period = { 60, 60, 60, 60, 60, 60 };

	int num_products = demand.size();
	int num_periods = demand[0].size();
	int num_usp_suites = 2, num_dsp_suites = 2;

	std::vector<double> usp_storage_cost = { 5, 5, 5 };
	std::vector<double> sales_price = { 20, 20, 20 };
	std::vector<double> production_cost = { 2, 2, 2 };
	std::vector<double> waste_disposal_cost = { 1, 1, 1 };
	std::vector<double> dsp_storage_cost = { 1, 1, 1 };
	std::vector<double> backlog_penalty = { 20, 20, 20 };
	std::vector<double> changeover_cost = { 1, 1, 1 };

	std::vector<double> usp_days = { 20, 22, 12.5 };
	std::vector<double> usp_lead_days = { 10, 10, 10 };
	std::vector<double> usp_shelf_life = { 60, 60, 60 };
	std::vector<double> usp_storage_cap = { 10, 10, 10 };

	std::vector<double> dsp_days = { 10, 10, 10 };
	std::vector<double> dsp_lead_days = { 10, 10, 12.5 };
	std::vector<double> dsp_shelf_life = { 180, 180, 180 };
	std::vector<double> dsp_storage_cap = { 40, 40, 40 };

	deterministic::Lakhdar2005Ex1Model lakhdar2005ex1_model(
		num_usp_suites,
		num_dsp_suites,
		demand,
		days_per_period,
		usp_storage_cost,
		sales_price,
		production_cost,
		waste_disposal_cost,
		dsp_storage_cost,
		backlog_penalty,
		changeover_cost,
		usp_days,
		usp_lead_days,
		usp_shelf_life,
		usp_storage_cap,
		dsp_days,
		dsp_lead_days,
		dsp_shelf_life,
		dsp_storage_cap);

	algorithms::SingleObjectiveGA<types::SingleObjectiveIndividual, deterministic::Lakhdar2005Ex1Model> simple_ga(
		lakhdar2005ex1_model,
		seed
	);

	for (size_t run = 0; run < 10; ++run) {
		simple_ga.Init(
			popsize,
			p_xo,
			p_gene_swap,
			num_products,
			num_usp_suites,
			p_product_mut,
			p_usp_suite_mut,
			p_plus_batch_mut,
			p_minus_batch_mut
		);

		for (size_t gen = 0; gen < 1000; ++gen) {
			simple_ga.Update();

			printf("\rRun %d, Gen: %d, Best: %.2f, Constraint: %.2f",
				run + 1, gen + 1, simple_ga.Top().objective, simple_ga.Top().constraint);
		}

		auto best = simple_ga.Top();
		auto usp_schedule = lakhdar2005ex1_model.CreateUSPSchedule(best);
		auto dsp_schedule = lakhdar2005ex1_model.CreateDSPSchedule(usp_schedule);

		std::vector<std::vector<int>> inventory, sold, backlog, dsp_waste;
		deterministic::Lakhdar2005Ex1Model::Objectives objectives = lakhdar2005ex1_model.CalculateObjectives(usp_schedule, dsp_schedule, inventory, sold, dsp_waste, backlog);

		printf(", (%.2f, %.2f)", objectives.profit, objectives.backlog_cost);

		printf("\n");
	}
}

void Lakhdar2005Ex1_DoubledDemandProfileTest()
{
	int runs = 10, gens = 1000, popsize = 100;

	int seed = 0;
	double p_xo = 0.131266;
	double p_product_mut = 0.131266;
	double p_usp_suite_mut = 0.131266;
	double p_dsp_suite_mut = 0.131266;
	double p_plus_batch_mut = 0.131266;
	double p_minus_batch_mut = 0.131266;
	double p_gene_swap = 0.131266;

	std::vector<std::vector<int>> demand =
	{
		{ 0, 0, 0, 6, 0, 6, 0, 0, 0, 6, 0, 6 },
		{ 0, 0, 6, 0, 0, 0, 0, 0, 6, 0, 0, 0 },
		{ 0, 8, 0, 0, 8, 0, 0, 8, 0, 0, 8, 0 }
	};

	std::vector<int> days_per_period = { 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60 };

	int num_products = demand.size();
	int num_periods = demand[0].size();
	int num_usp_suites = 2, num_dsp_suites = 2;

	std::vector<double> usp_storage_cost = { 5, 5, 5 };
	std::vector<double> sales_price = { 20, 20, 20 };
	std::vector<double> production_cost = { 2, 2, 2 };
	std::vector<double> waste_disposal_cost = { 1, 1, 1 };
	std::vector<double> dsp_storage_cost = { 1, 1, 1 };
	std::vector<double> backlog_penalty = { 20, 20, 20 };
	std::vector<double> changeover_cost = { 1, 1, 1 };

	std::vector<double> usp_days = { 20, 22, 12.5 };
	std::vector<double> usp_lead_days = { 10, 10, 10 };
	std::vector<double> usp_shelf_life = { 60, 60, 60 };
	std::vector<double> usp_storage_cap = { 10, 10, 10 };

	std::vector<double> dsp_days = { 10, 10, 10 };
	std::vector<double> dsp_lead_days = { 10, 10, 12.5 };
	std::vector<double> dsp_shelf_life = { 180, 180, 180 };
	std::vector<double> dsp_storage_cap = { 40, 40, 40 };

	deterministic::Lakhdar2005Ex1Model lakhdar2005ex1_model(
		num_usp_suites,
		num_dsp_suites,

		demand,
		days_per_period,

		usp_storage_cost,
		sales_price,
		production_cost,
		waste_disposal_cost,
		dsp_storage_cost,
		backlog_penalty,
		changeover_cost,

		usp_days,
		usp_lead_days,
		usp_shelf_life,
		usp_storage_cap,

		dsp_days,
		dsp_lead_days,
		dsp_shelf_life,
		dsp_storage_cap);

	algorithms::SingleObjectiveGA<types::SingleObjectiveIndividual, deterministic::Lakhdar2005Ex1Model> simple_ga(
		lakhdar2005ex1_model,
		seed
	);

	for (size_t run = 0; run < 10; ++run) {
		simple_ga.Init(
			popsize,
			p_xo,
			p_gene_swap,
			num_products,
			num_usp_suites,
			p_product_mut,
			p_usp_suite_mut,
			p_plus_batch_mut,
			p_minus_batch_mut
		);

		for (size_t gen = 0; gen < 1000; ++gen) {
			simple_ga.Update();

			printf("\rRun %d, Gen: %d, Best: %.2f, Constraint: %.2f",
				run + 1, gen + 1, simple_ga.Top().objective, simple_ga.Top().constraint);
		}

		auto best = simple_ga.Top();
		auto usp_schedule = lakhdar2005ex1_model.CreateUSPSchedule(best);
		auto dsp_schedule = lakhdar2005ex1_model.CreateDSPSchedule(usp_schedule);

		std::vector<std::vector<int>> inventory, sold, backlog, dsp_waste;
		deterministic::Lakhdar2005Ex1Model::Objectives objectives = lakhdar2005ex1_model.CalculateObjectives(usp_schedule, dsp_schedule, inventory, sold, dsp_waste, backlog);

		printf(", (%.2f, %.2f)", objectives.profit, objectives.backlog_cost);

		printf("\n");
	}
}

// Eli Lilly problem solutio using continuous time model based on Lakhdar et al. (2005)
void EliLillyTest()
{

}

//------------------------------------------------------------------
//				Lakhdar2005 Example 1 Scheduling Model
//------------------------------------------------------------------

int main()
{
	printf("Lakhdar2005 Example 1 Scheduling Model tests...\n");

	printf("A base case solution test: ");
	Lakhdar2005Ex1_BaseCaseGlobalOptimumTest();

	printf("Increased demand for product 1 solution test: ");
	Lakhdar2005Ex1_IncreasedDemandGlobalOptimumTest();

	printf("Doubled demand profile solution test: ");
	Lakhdar2005Ex1_DoubledDemandProfileGlobalOptimumTest();

	printf("\nGA solution for the base case:\n");
	Lakhdar2005Ex1_BaseCaseTest();

	printf("\nGA solution for the case of increased demand of product 1:\n");
	Lakhdar2005Ex1_IncreasedDemandTest();

	printf("\nGA solution for the case of doubled demand profile:\n");
	Lakhdar2005Ex1_DoubledDemandProfileTest();

	printf("\nDone!\n");
	std::cin.get();

	return 0;
}