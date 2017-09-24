#include "single_objective_ga.h"
#include "fitness.h"

using namespace types;
using namespace algorithms;

// Fitness function test with a known continous-time 
// base case solution.
// The calculated profit should be 517.
void BaseCaseGlobalOptimumTest()
{
	vector<vector<int>> demand =
	{
		{ 0, 0, 0, 6, 0, 6 },
		{ 0, 0, 6, 0, 0, 0 },
		{ 0, 8, 0, 0, 8, 0 }
	};

	vector<int> days_per_period = { 60, 60, 60, 60, 60, 60 };

	int num_products = demand.size();
	int num_periods = demand[0].size();
	int num_usp_suites = 2, num_dsp_suites = 2;

	vector<int> usp_storage_cost = { 5, 5, 5 };
	vector<int> sales_price = { 20, 20, 20 };
	vector<int> production_cost = { 2, 2, 2 };
	vector<int> waste_disposal_cost = { 1, 1, 1 };
	vector<int> dsp_storage_cost = { 1, 1, 1 };
	vector<int> backlog_penalty = { 20, 20, 20 };
	vector<int> changeover_cost = { 1, 1, 1 };

	vector<double> usp_days = { 20, 22, 12.5 };
	vector<double> usp_lead_days = { 10, 10, 10 };
	vector<double> usp_shelf_life = { 60, 60, 60 };
	vector<double> usp_storage_cap = { 10, 10, 10 };

	vector<double> dsp_days = { 10, 10, 10 };
	vector<double> dsp_lead_days = { 10, 10, 12.5 };
	vector<double> dsp_shelf_life = { 180, 180, 180 };
	vector<double> dsp_storage_cap = { 40, 40, 40 };

	Fitness fitness(
		num_usp_suites,
		num_dsp_suites,
		demand, days_per_period,
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

	SingleObjectiveIndividual i;
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

	fitness(i);

	printf("%.2f profit\n", i.objective);
}

// Fitness function test with a continuous-time solution
// to the problem of increased demand for product 1.
// The calculated profit should be 562.
void IncreasedDemandGlobalOptimumTest()
{
	vector<vector<int>> demand =
	{
		{ 0, 0, 0, 6, 0, 9 },
		{ 0, 0, 6, 0, 0, 0 },
		{ 0, 8, 0, 0, 8, 0 }
	};

	vector<int> days_per_period = { 60, 60, 60, 60, 60, 60 };

	int num_products = demand.size();
	int num_periods = demand[0].size();
	int num_usp_suites = 2, num_dsp_suites = 2;

	vector<int> usp_storage_cost = { 5, 5, 5 };
	vector<int> sales_price = { 20, 20, 20 };
	vector<int> production_cost = { 2, 2, 2 };
	vector<int> waste_disposal_cost = { 1, 1, 1 };
	vector<int> dsp_storage_cost = { 1, 1, 1 };
	vector<int> backlog_penalty = { 20, 20, 20 };
	vector<int> changeover_cost = { 1, 1, 1 };

	vector<double> usp_days = { 20, 22, 12.5 };
	vector<double> usp_lead_days = { 10, 10, 10 };
	vector<double> usp_shelf_life = { 60, 60, 60 };
	vector<double> usp_storage_cap = { 10, 10, 10 };

	vector<double> dsp_days = { 10, 10, 10 };
	vector<double> dsp_lead_days = { 10, 10, 12.5 };
	vector<double> dsp_shelf_life = { 180, 180, 180 };
	vector<double> dsp_storage_cap = { 40, 40, 40 };

	Fitness fitness(
		num_usp_suites,
		num_dsp_suites,
		demand, days_per_period,
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

	SingleObjectiveIndividual i;
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

	fitness(i);

	printf("%.2f profit\n", i.objective);
}

// Fitness function test with a continuous-time solution
// to the problem of doubled demand profile, i.e. 2 years.
// The calculated profit should be 1007.
void DoubledDemandProfileGlobalOptimumTest()
{
	vector<vector<int>> demand =
	{
		{ 0, 0, 0, 6, 0, 6, 0, 0, 0, 6, 0, 6 },
		{ 0, 0, 6, 0, 0, 0, 0, 0, 6, 0, 0, 0 },
		{ 0, 8, 0, 0, 8, 0, 0, 8, 0, 0, 8, 0 }
	};

	vector<int> days_per_period = { 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60 };

	int num_products = demand.size();
	int num_periods = demand[0].size();
	int num_usp_suites = 2, num_dsp_suites = 2;

	vector<int> usp_storage_cost = { 5, 5, 5 };
	vector<int> sales_price = { 20, 20, 20 };
	vector<int> production_cost = { 2, 2, 2 };
	vector<int> waste_disposal_cost = { 1, 1, 1 };
	vector<int> dsp_storage_cost = { 1, 1, 1 };
	vector<int> backlog_penalty = { 20, 20, 20 };
	vector<int> changeover_cost = { 1, 1, 1 };

	vector<double> usp_days = { 20, 22, 12.5 };
	vector<double> usp_lead_days = { 10, 10, 10 };
	vector<double> usp_shelf_life = { 60, 60, 60 };
	vector<double> usp_storage_cap = { 10, 10, 10 };

	vector<double> dsp_days = { 10, 10, 10 };
	vector<double> dsp_lead_days = { 10, 10, 12.5 };
	vector<double> dsp_shelf_life = { 180, 180, 180 };
	vector<double> dsp_storage_cap = { 40, 40, 40 };

	Fitness fitness(
		num_usp_suites,
		num_dsp_suites,
		demand, days_per_period,
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

	SingleObjectiveIndividual i;
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

	fitness(i);

	printf("%.2f profit\n", i.objective);
}

void BaseCaseTest()
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

	vector<vector<int>> demand =
	{
		{ 0, 0, 0, 6, 0, 6 },
		{ 0, 0, 6, 0, 0, 0 },
		{ 0, 8, 0, 0, 8, 0 }
	};

	vector<int> days_per_period = { 60, 60, 60, 60, 60, 60 };

	int num_products = demand.size();
	int num_periods = demand[0].size();
	int num_usp_suites = 2, num_dsp_suites = 2;

	vector<int> usp_storage_cost = { 5, 5, 5 };
	vector<int> sales_price = { 20, 20, 20 };
	vector<int> production_cost = { 2, 2, 2 };
	vector<int> waste_disposal_cost = { 1, 1, 1 };
	vector<int> dsp_storage_cost = { 1, 1, 1 };
	vector<int> backlog_penalty = { 20, 20, 20 };
	vector<int> changeover_cost = { 1, 1, 1 };

	vector<double> usp_days = { 20, 22, 12.5 };
	vector<double> usp_lead_days = { 10, 10, 10 };
	vector<double> usp_shelf_life = { 60, 60, 60 };
	vector<double> usp_storage_cap = { 10, 10, 10 };

	vector<double> dsp_days = { 10, 10, 10 };
	vector<double> dsp_lead_days = { 10, 10, 12.5 };
	vector<double> dsp_shelf_life = { 180, 180, 180 };
	vector<double> dsp_storage_cap = { 40, 40, 40 };

	Fitness fitness(
		num_usp_suites,
		num_dsp_suites,
		demand, days_per_period,
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


	SingleObjectiveGA<SingleObjectiveIndividual, Fitness> simple_ga(
		fitness,
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

		printf("\n");
	}
}

void IncreasedDemandTest()
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

	vector<vector<int>> demand =
	{
		{ 0, 0, 0, 6, 0, 9 },
		{ 0, 0, 6, 0, 0, 0 },
		{ 0, 8, 0, 0, 8, 0 }
	};

	vector<int> days_per_period = { 60, 60, 60, 60, 60, 60 };

	int num_products = demand.size();
	int num_periods = demand[0].size();
	int num_usp_suites = 2, num_dsp_suites = 2;

	vector<int> usp_storage_cost = { 5, 5, 5 };
	vector<int> sales_price = { 20, 20, 20 };
	vector<int> production_cost = { 2, 2, 2 };
	vector<int> waste_disposal_cost = { 1, 1, 1 };
	vector<int> dsp_storage_cost = { 1, 1, 1 };
	vector<int> backlog_penalty = { 20, 20, 20 };
	vector<int> changeover_cost = { 1, 1, 1 };

	vector<double> usp_days = { 20, 22, 12.5 };
	vector<double> usp_lead_days = { 10, 10, 10 };
	vector<double> usp_shelf_life = { 60, 60, 60 };
	vector<double> usp_storage_cap = { 10, 10, 10 };

	vector<double> dsp_days = { 10, 10, 10 };
	vector<double> dsp_lead_days = { 10, 10, 12.5 };
	vector<double> dsp_shelf_life = { 180, 180, 180 };
	vector<double> dsp_storage_cap = { 40, 40, 40 };

	Fitness fitness(
		num_usp_suites,
		num_dsp_suites,
		demand, days_per_period,
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


	SingleObjectiveGA<SingleObjectiveIndividual, Fitness> simple_ga(
		fitness,
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

		printf("\n");
	}
}

void DoubledDemandProfileTest()
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

	vector<vector<int>> demand =
	{
		{ 0, 0, 0, 6, 0, 6, 0, 0, 0, 6, 0, 6 },
		{ 0, 0, 6, 0, 0, 0, 0, 0, 6, 0, 0, 0 },
		{ 0, 8, 0, 0, 8, 0, 0, 8, 0, 0, 8, 0 }
	};

	vector<int> days_per_period = { 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60 };

	int num_products = demand.size();
	int num_periods = demand[0].size();
	int num_usp_suites = 2, num_dsp_suites = 2;

	vector<int> usp_storage_cost = { 5, 5, 5 };
	vector<int> sales_price = { 20, 20, 20 };
	vector<int> production_cost = { 2, 2, 2 };
	vector<int> waste_disposal_cost = { 1, 1, 1 };
	vector<int> dsp_storage_cost = { 1, 1, 1 };
	vector<int> backlog_penalty = { 20, 20, 20 };
	vector<int> changeover_cost = { 1, 1, 1 };

	vector<double> usp_days = { 20, 22, 12.5 };
	vector<double> usp_lead_days = { 10, 10, 10 };
	vector<double> usp_shelf_life = { 60, 60, 60 };
	vector<double> usp_storage_cap = { 10, 10, 10 };

	vector<double> dsp_days = { 10, 10, 10 };
	vector<double> dsp_lead_days = { 10, 10, 12.5 };
	vector<double> dsp_shelf_life = { 180, 180, 180 };
	vector<double> dsp_storage_cap = { 40, 40, 40 };

	Fitness fitness(
		num_usp_suites,
		num_dsp_suites,
		demand, days_per_period,
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


	SingleObjectiveGA<SingleObjectiveIndividual, Fitness> simple_ga(
		fitness,
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

		printf("\n");
	}
}


int main()
{
	printf("A base case solution test: ");
	BaseCaseGlobalOptimumTest();

	printf("Increased demand for product 1 solution test: ");
	IncreasedDemandGlobalOptimumTest();

	printf("Doubled demand profile solution test: ");
	DoubledDemandProfileGlobalOptimumTest();

	printf("\nGA solution for the base case:\n");
	BaseCaseTest();

	printf("\nGA solution for the case of increased demand of product 1:\n");
	IncreasedDemandTest();

	printf("\nGA solution for the case of doubled demand profile:\n");
	DoubledDemandProfileTest();

	printf("\nDone!\n");
	cin.get();

	return 0;
}