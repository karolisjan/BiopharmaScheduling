#include <stdio.h>
#include <iostream>

#include "models.h"
#include "single_objective_ga.h"


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
    std::vector<double> production_factor = { 1, 1, 1 };

    std::vector<double> usp_lead_days = { 10, 10, 10 };
	std::vector<double> dsp_lead_days = { 10, 10, 12.5 };

	std::vector<int> shelf_life = { 180, 180, 180 };
	std::vector<int> storage_cap = { 40, 40, 40 };

    deterministic::SingleSiteMultiSuiteInputData input_data(
        num_usp_suites,
        num_dsp_suites,

        demand,
        days_per_period,

        usp_days,
        dsp_days,
        production_factor,
        
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

        usp_lead_days,
        dsp_lead_days
    );

	deterministic::SingleSiteMultiSuiteModel single_site_multi_suite_model(input_data);

	types::SingleObjectiveIndividual i;

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

	printf("Total proft %.1f\n", schedule.objectives[deterministic::OBJECTIVES::TOTAL_PROFIT]);
	printf("Backlog penalty %.1f\n", schedule.objectives[deterministic::OBJECTIVES::TOTAL_BACKLOG_PENALTY]);
	printf("Production costs %.1f\n", schedule.objectives[deterministic::OBJECTIVES::TOTAL_PRODUCTION_COST]);
	printf("Changeover costs %.1f\n", schedule.objectives[deterministic::OBJECTIVES::TOTAL_CHANGEOVER_COSTS]);
	printf("Storage costs %.1f\n", schedule.objectives[deterministic::OBJECTIVES::TOTAL_STORAGE_COST]);
	printf("Waste costs %.1f\n\n", schedule.objectives[deterministic::OBJECTIVES::TOTAL_WASTE_COST]);

	for (const auto &suite : schedule.suites) {
		for (const auto &cmpgn : suite) {
			printf(
				"Suite: %d, p%d, %d, start: %.1f, end: %.1f\n", 
				cmpgn.suite_num,
				cmpgn.product_num, 
				cmpgn.num_batches,
				cmpgn.start,
				cmpgn.end
			);
		}
	}

	// printf("\nFinal p3 batches\n\n");
	// for (int dsp_suite = num_usp_suites; dsp_suite != schedule.suites.size(); ++dsp_suite) {
	// 	for (const auto &cmpgn : schedule.suites[dsp_suite]) {
	// 		if (cmpgn.product_num != 3) {
	// 			continue;
	// 		}
	// 		for (const auto &batch : cmpgn.batches) {
	// 			printf("Stored at: %.1f, expires at: %.1f\n", batch.stored_at, batch.expires_at);
	// 		}
	// 	}
	// }

	printf("\nInventory\n\n");
	for (const auto &row : schedule.inventory) {
		for (const auto &val : row) {
			printf("%d  ", val);
		}
		printf("\n");
	}

	printf("\nBacklog\n\n");
	for (const auto &row : schedule.backlog) {
		for (const auto &val : row) {
			printf("%d  ", val);
		}
		printf("\n");
	}

	printf("\nSold\n\n");
	for (const auto &row : schedule.supply) {
		for (const auto &val : row) {
			printf("%d  ", val);
		}
		printf("\n");
	}

	printf("\nWaste\n\n");
	for (const auto &row : schedule.waste) {
		for (const auto &val : row) {
			printf("%d  ", val);
		}
		printf("\n");
	}
}

void Lakhdar2005Ex1_BaseCaseTest()
{
	int seed = 0, num_threads = -1;
	int runs = 10, gens = 200, popsize = 100; 

	int starting_length = 1;
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

    int num_usp_suites = 2, num_dsp_suites = 2, num_products = demand.size();

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
    std::vector<double> production_factor = { 1, 1, 1 };

    std::vector<double> usp_lead_days = { 10, 10, 10 };
	std::vector<double> dsp_lead_days = { 10, 10, 12.5 };

	std::vector<int> shelf_life = { 180, 180, 180 };
	std::vector<int> storage_cap = { 40, 40, 40 };

    deterministic::SingleSiteMultiSuiteInputData input_data(
        num_usp_suites,
        num_dsp_suites,

        demand,
        days_per_period,

        usp_days,
        dsp_days,
        production_factor,
        
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

        usp_lead_days,
        dsp_lead_days
    );

	deterministic::SingleSiteMultiSuiteModel single_site_multi_suite_model(input_data);

	algorithms::SingleObjectiveGA<types::SingleObjectiveIndividual, deterministic::SingleSiteMultiSuiteModel> simple_ga(
		single_site_multi_suite_model,
		seed,
		num_threads
	);

	for (int run = 0; run != 10; ++run) {
		simple_ga.Init(
			popsize,
			starting_length,
			p_xo,
			p_gene_swap,
			num_products,
			num_usp_suites,
			p_product_mut,
			p_usp_suite_mut,
			p_plus_batch_mut,
			p_minus_batch_mut
		);

		for (int gen = 0; gen < 1000; ++gen) {
			simple_ga.Update();

			printf(
				"\rRun %d, Gen: %d, Best: %.2f, Constraint: %.2f",
				run + 1, gen + 1, simple_ga.Top().objective, simple_ga.Top().constraints
			);

			std::cout << std::flush;
		}

		auto best = simple_ga.Top();

		types::SingleSiteMultiSuiteSchedule schedule;
		single_site_multi_suite_model.CreateSchedule(best, schedule);

		printf(", (%.2f, %.2f)\n", schedule.objectives[deterministic::OBJECTIVES::TOTAL_PROFIT], schedule.objectives[deterministic::OBJECTIVES::TOTAL_BACKLOG_PENALTY]);
		std::cout << std::flush;
	}
}

int main()
{
	printf("Lakhdar2005 Example 1 (base case) model test...\n\n");
	Lakhdar2005Ex1_BaseCaseGlobalOptimumTest();

	printf("\nLakhdar2005 Example 1 (base case) GA test...\n\n");
	Lakhdar2005Ex1_BaseCaseTest();
}