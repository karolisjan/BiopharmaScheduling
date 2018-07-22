#include <chrono>
#include <stdio.h>
#include <iostream>
#include <climits>
#include <cassert>

#include "nsgaii.h"
#include "scheduling_models.h"
#include "single_objective_ga.h"


bool display_schedules = false;
int seed = 7, num_threads = -1;
int num_runs = 10, num_gens = 1000, popsize = 200; 

int starting_length = 1;

double p_xo = 0.131266;
double p_product_mut = 0.131266;
double p_usp_suite_mut = 0.131266;
double p_plus_batch_mut = 0.131266;
double p_minus_batch_mut = 0.131266;
double p_gene_swap = 0.131266;


void DisplaySchedule(types::SingleSiteMultiSuiteSchedule &schedule)
{
	printf("Total proft %.1f\n", schedule.objectives[deterministic::OBJECTIVES::TOTAL_PROFIT]);
	printf("Backlog penalty %.1f\n", schedule.objectives[deterministic::OBJECTIVES::TOTAL_BACKLOG_PENALTY]);
	printf("Production costs %.1f\n", schedule.objectives[deterministic::OBJECTIVES::TOTAL_PRODUCTION_COST]);
	printf("Changeover costs %.1f\n", schedule.objectives[deterministic::OBJECTIVES::TOTAL_CHANGEOVER_COST]);
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

	printf("\nInventory\n\n");
	for (const auto &row : schedule.batch_inventory) {
		for (const auto &val : row) {
			printf("%d  ", val);
		}
		printf("\n");
	}

	printf("\nBacklog\n\n");
	for (const auto &row : schedule.batch_backlog) {
		for (const auto &val : row) {
			printf("%d  ", val);
		}
		printf("\n");
	}

	printf("\nSold\n\n");
	for (const auto &row : schedule.batch_supply) {
		for (const auto &val : row) {
			printf("%d  ", val);
		}
		printf("\n");
	}

	printf("\nWaste\n\n");
	for (const auto &row : schedule.batch_waste) {
		for (const auto &val : row) {
			printf("%d  ", val);
		}
		printf("\n");
	}
}

 void Det_SingleSiteMultiSuite_BaseCaseTest()
{
	std::unordered_map<deterministic::OBJECTIVES, int> objectives;
 	objectives.emplace(deterministic::TOTAL_PROFIT, 1);
	
 	std::unordered_map<deterministic::OBJECTIVES, std::pair<int, double>> constraints;
 	constraints.emplace(deterministic::TOTAL_KG_BACKLOG, std::make_pair(-1, 0));

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
		dsp_changeovers,

 		&constraints
	);

 	deterministic::SingleSiteMultiSuiteModel single_site_multi_suite_model(input_data);

 	algorithms::SingleObjectiveGA<types::SingleObjectiveChromosome<types::SingleSiteMultiSuiteGene>, deterministic::SingleSiteMultiSuiteModel> simple_ga(
 		single_site_multi_suite_model,
 		seed,
 		num_threads
 	);

 	for (int run = 0; run != num_runs; ++run) {
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

 		for (int gen = 0; gen != num_gens; ++gen) {
 			simple_ga.Update();

 			printf(
 				"\rRun %d, Gen: %d, Best: %.1f, Constraint: %.1f, Length: %d",
 				run + 1, gen + 1, simple_ga.Top().objective, simple_ga.Top().constraints, simple_ga.Top().genes.size()
 			);

 			std::cout << std::flush;
 		}

 		auto best = simple_ga.Top();

 		types::SingleSiteMultiSuiteSchedule schedule;
 		single_site_multi_suite_model.CreateSchedule(best, schedule);

 		int total_num_usp_campaigns = 0;

 		for (int usp_suite = 0; usp_suite != num_usp_suites; ++usp_suite) {
 			total_num_usp_campaigns += schedule.suites[usp_suite].size();
 		}

 		printf(
 			", (%.1f, %.1f, %d)\n", 
 			schedule.objectives[deterministic::OBJECTIVES::TOTAL_PROFIT], 
 			schedule.objectives[deterministic::OBJECTIVES::TOTAL_BACKLOG_PENALTY],
 			total_num_usp_campaigns
 		);
 		std::cout << std::flush;

 		if (display_schedules) {
 			DisplaySchedule(schedule);
 		}
 	}
}

void Det_SingleSiteSimple_SingleObjective_Test()
{
	seed = 7;

	num_runs = 20;
	num_gens = 100;
	popsize = 100;

	// p_xo = 0.130878;
	// p_product_mut = 0.017718;	
	// p_plus_batch_mut = 0.707202;
	// p_minus_batch_mut = 0.834735;
	// p_gene_swap = 0.531073;

	p_xo = 0.223077;
	p_product_mut = 0.010324;
	p_plus_batch_mut = 0.800127;
	p_minus_batch_mut = 0.787252;
	p_gene_swap = 0.322800;

	std::unordered_map<deterministic::OBJECTIVES, int> objectives;
	objectives.emplace(deterministic::TOTAL_KG_THROUGHPUT, 1);

	std::unordered_map<deterministic::OBJECTIVES, std::pair<int, double>> constraints;
	constraints.emplace(deterministic::TOTAL_KG_BACKLOG, std::make_pair(-1, 0));
	constraints.emplace(deterministic::TOTAL_KG_WASTE, std::make_pair(-1, 0));

	// Kg demand
	std::vector<std::vector<double>> kg_demand = { 
		{ 0.0,0.0,3.1,0.0,0.0,3.1,0.0,3.1,3.1,3.1,0.0,6.2,6.2,3.1,6.2,0.0,3.1,9.3,0.0,6.2,6.2,0.0,6.2,9.3,0.0,9.3,6.2,3.1,6.2,3.1,0.0,9.3,6.2,9.3,6.2,0.0 },
		{ 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,6.2,0.0,0.0,0.0,0.0,0.0,6.2,0.0,0.0,0.0,0.0,0.0,0.0,6.2 },
		{ 0.0,0.0,0.0,0.0,0.0,0.0,4.9,4.9,0.0,0.0,0.0,9.8,4.9,0.0,4.9,0.0,0.0,4.9,9.8,0.0,0.0,0.0,4.9,4.9,0.0,9.8,0.0,0.0,4.9,9.8,9.8,0.0,4.9,9.8,4.9,0.0 },
		{ 0.0,5.5,5.5,0.0,5.5,5.5,5.5,5.5,5.5,0.0,11.0,5.5,0.0,5.5,5.5,11.0,5.5,5.5,0.0,5.5,5.5,5.5,11.0,5.5,0.0,11.0,0.0,11.0,5.5,5.5,0.0,11.0,11.0,0.0,5.5,5.5 }
	};
	
	int num_products = kg_demand.size();

	// 6-month kg inventoy safety levels
	std::vector<std::vector<double>> kg_inventory_target = {
		{ 6.2,6.2,9.3,9.3,12.4,12.4,15.5,21.7,21.7,24.8,21.7,24.8,27.9,21.7,24.8,24.8,24.8,27.9,27.9,27.9,31.0,31.0,34.1,34.1,27.9,27.9,27.9,27.9,34.1,34.1,31.0,31.0,21.7,15.5,6.2,0.0 },
		{ 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2 },
		{ 0.0,4.9,9.8,9.8,9.8,9.8,19.6,19.6,14.7,19.6,19.6,19.6,14.7,19.6,19.6,14.7,14.7,19.6,19.6,9.8,19.6,19.6,19.6,19.6,24.5,34.3,24.5,29.4,39.2,39.2,29.4,19.6,19.6,14.7,4.9,0.0 },
		{ 22.0,27.5,27.5,27.5,27.5,33.0,33.0,27.5,27.5,27.5,38.5,33.0,33.0,33.0,33.0,33.0,27.5,33.0,33.0,33.0,38.5,33.0,38.5,33.0,33.0,33.0,33.0,44.0,33.0,33.0,33.0,33.0,22.0,11.0,11.0,5.5 },
	};

	std::vector<int> days_per_period = std::vector<int>{ 
		31,31,28,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30
	};

	std::vector<double> kg_yield_per_batch = { 3.1, 6.2, 4.9, 5.5 };
	std::vector<double> kg_storage_limits = { 250, 250, 250, 250 }; // set high to ignore
	std::vector<double> kg_opening_stock = { 18.6, 0, 19.6, 32.0 };

	std::vector<double> inventory_penalty_per_kg = { 1, 1, 1, 1 };
	std::vector<double> backlog_penalty_per_kg = { 1, 1, 1, 1 };
	std::vector<double> production_cost_per_kg = { 1, 1, 1, 1 };
	std::vector<double> storage_cost_per_kg = { 1, 1, 1, 1 };
	std::vector<double> waste_cost_per_kg = { 1, 1, 1, 1 };
	std::vector<double> sell_price_per_kg = { 1, 1, 1, 1 };

	std::vector<int> inoculation_days = { 20, 15, 20, 26 };
	std::vector<int> seed_days = { 11, 7, 11, 9 };
	std::vector<int> production_days = { 14, 14, 14, 14 };
	std::vector<int> usp_days = { 45, 36, 45, 49 }; //
	std::vector<int> dsp_days = { 7, 11, 7, 7 };
	std::vector<int> shelf_life_days = { 730, 730, 730, 730 }; // set high to ignore
	std::vector<int> approval_days = { 90, 90, 90, 90 };
	std::vector<int> min_batches_per_campaign = { 2, 2, 2, 3 };
	std::vector<int> max_batches_per_campaign = { 50, 50, 50, 30 };
	std::vector<int> batches_multiples_of_per_campaign = { 1, 1, 1, 3 };

	std::vector<std::vector<int>> changeover_days = {
		{ 0,  10, 16, 20 },
		{ 16,  0, 16, 20 },
		{ 16, 10,  0, 20 },
		{ 18, 10, 18,  0 }
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

	deterministic::SingleSiteSimpleModel deterministic_fitness(input_data);
	
	printf("\nRunning Single-Objective GA...\n");
	algorithms::SingleObjectiveGA<types::SingleObjectiveChromosome<types::SingleSiteSimpleGene>, deterministic::SingleSiteSimpleModel> ga(
		deterministic_fitness,
		seed,
		num_threads
	);

	std::vector<types::SingleObjectiveChromosome<types::SingleSiteSimpleGene>> solutions;

	double mean_time = 0.0;

	for (int run = 0; run < num_runs; ++run) {

		auto start = std::chrono::steady_clock::now();

		ga.Init(
			popsize,

			//Individual Params + GeneParams
			starting_length,
			p_xo,
			p_gene_swap,

			//GeneParams 
			num_products,
			p_product_mut,
			p_plus_batch_mut,
			p_minus_batch_mut
		);

		for (int gen = 0; gen < num_gens; ++gen) {
			ga.Update();
		}			

		auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
		mean_time += elapsed_time;

		auto solution = ga.Top();
		solutions.push_back(solution);

		types::SingleSiteSimpleSchedule schedule;
		deterministic_fitness.CreateSchedule(solution, schedule);
	}

	if (solutions.size()) {
		auto solution = ga.Top(solutions);
		types::SingleSiteSimpleSchedule schedule;;
		deterministic_fitness.CreateSchedule(solution, schedule);

		std::cout << "\n######################## After " << num_runs << " num_runs, mean elapsed time: " << mean_time / num_runs << " ms ########################\n" << std::endl;

		printf(
			"Top Solution:\nTotal kg throughput: %.2f (%.2f)\nTotal kg inventory deficit: %.2f\nTotal kg backlog: %.2f\nTotal kg waste: %.2f\n\n",
			solution.objective, schedule.objectives[deterministic::TOTAL_KG_THROUGHPUT],
			schedule.objectives[deterministic::TOTAL_KG_INVENTORY_DEFICIT],
			schedule.objectives[deterministic::TOTAL_KG_BACKLOG],
			schedule.objectives[deterministic::TOTAL_KG_WASTE]
		);
	}
}

void Det_SingleSiteSimple_MultiObjective_Test()
{
	seed = 7;
	num_threads = -1;

	num_runs = 10;
	num_gens = 100;
	popsize = 100;

	// p_xo = 0.130878;
	// p_product_mut = 0.017718;
	// p_plus_batch_mut = 0.707202;
	// p_minus_batch_mut = 0.834735;
	// p_gene_swap = 0.531073;

	p_xo = 0.223077;
	p_product_mut = 0.010324;
	p_plus_batch_mut = 0.800127;
	p_minus_batch_mut = 0.787252;
	p_gene_swap = 0.322800;

	std::unordered_map<deterministic::OBJECTIVES, int> objectives;
	objectives.emplace(deterministic::TOTAL_KG_INVENTORY_DEFICIT, -1);
	objectives.emplace(deterministic::TOTAL_KG_THROUGHPUT, 1);

	std::unordered_map<deterministic::OBJECTIVES, std::pair<int, double>> constraints;
	constraints.emplace(deterministic::TOTAL_KG_BACKLOG, std::make_pair(-1, 0));
	constraints.emplace(deterministic::TOTAL_KG_WASTE, std::make_pair(-1, 0));

	// Kg demand
	std::vector<std::vector<double>> kg_demand = { 
		{ 0.0,0.0,3.1,0.0,0.0,3.1,0.0,3.1,3.1,3.1,0.0,6.2,6.2,3.1,6.2,0.0,3.1,9.3,0.0,6.2,6.2,0.0,6.2,9.3,0.0,9.3,6.2,3.1,6.2,3.1,0.0,9.3,6.2,9.3,6.2,0.0 },
		{ 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,6.2,0.0,0.0,0.0,0.0,0.0,6.2,0.0,0.0,0.0,0.0,0.0,0.0,6.2 },
		{ 0.0,0.0,0.0,0.0,0.0,0.0,4.9,4.9,0.0,0.0,0.0,9.8,4.9,0.0,4.9,0.0,0.0,4.9,9.8,0.0,0.0,0.0,4.9,4.9,0.0,9.8,0.0,0.0,4.9,9.8,9.8,0.0,4.9,9.8,4.9,0.0 },
		{ 0.0,5.5,5.5,0.0,5.5,5.5,5.5,5.5,5.5,0.0,11.0,5.5,0.0,5.5,5.5,11.0,5.5,5.5,0.0,5.5,5.5,5.5,11.0,5.5,0.0,11.0,0.0,11.0,5.5,5.5,0.0,11.0,11.0,0.0,5.5,5.5 }
	};
	
	int num_products = kg_demand.size();

	// 6-month kg inventoy safety levels
	std::vector<std::vector<double>> kg_inventory_target = {
		{ 6.2,6.2,9.3,9.3,12.4,12.4,15.5,21.7,21.7,24.8,21.7,24.8,27.9,21.7,24.8,24.8,24.8,27.9,27.9,27.9,31.0,31.0,34.1,34.1,27.9,27.9,27.9,27.9,34.1,34.1,31.0,31.0,21.7,15.5,6.2,0.0 },
		{ 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2 },
		{ 0.0,4.9,9.8,9.8,9.8,9.8,19.6,19.6,14.7,19.6,19.6,19.6,14.7,19.6,19.6,14.7,14.7,19.6,19.6,9.8,19.6,19.6,19.6,19.6,24.5,34.3,24.5,29.4,39.2,39.2,29.4,19.6,19.6,14.7,4.9,0.0 },
		{ 22.0,27.5,27.5,27.5,27.5,33.0,33.0,27.5,27.5,27.5,38.5,33.0,33.0,33.0,33.0,33.0,27.5,33.0,33.0,33.0,38.5,33.0,38.5,33.0,33.0,33.0,33.0,44.0,33.0,33.0,33.0,33.0,22.0,11.0,11.0,5.5 },
	};

	std::vector<int> days_per_period = std::vector<int>{ 
		31,31,28,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30
	};

	std::vector<double> kg_yield_per_batch = { 3.1, 6.2, 4.9, 5.5 };
	std::vector<double> kg_storage_limits = { 250, 250, 250, 250 }; // set high to ignore
	std::vector<double> kg_opening_stock = { 18.6, 0, 19.6, 32.0 };

	std::vector<double> inventory_penalty_per_kg = { 1, 1, 1, 1 };
	std::vector<double> backlog_penalty_per_kg = { 1, 1, 1, 1 };
	std::vector<double> production_cost_per_kg = { 1, 1, 1, 1 };
	std::vector<double> storage_cost_per_kg = { 1, 1, 1, 1 };
	std::vector<double> waste_cost_per_kg = { 1, 1, 1, 1 };
	std::vector<double> sell_price_per_kg = { 1, 1, 1, 1 };

	std::vector<int> inoculation_days = { 20, 15, 20, 26 };
	std::vector<int> seed_days = { 11, 7, 11, 9 };
	std::vector<int> production_days = { 14, 14, 14, 14 };
	std::vector<int> usp_days = { 45, 36, 45, 49 }; //
	std::vector<int> dsp_days = { 7, 11, 7, 7 };
	std::vector<int> shelf_life_days = { 730, 730, 730, 730 }; // set high to ignore
	std::vector<int> approval_days = { 90, 90, 90, 90 };
	std::vector<int> min_batches_per_campaign = { 2, 2, 2, 3 };
	std::vector<int> max_batches_per_campaign = { 50, 50, 50, 30 };
	std::vector<int> batches_multiples_of_per_campaign = { 1, 1, 1, 3 };

	std::vector<std::vector<int>> changeover_days = {
		{ 0,  10, 16, 20 },
		{ 16,  0, 16, 20 },
		{ 16, 10,  0, 20 },
		{ 18, 10, 18,  0 }
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

	deterministic::SingleSiteSimpleModel deterministic_fitness(input_data);
	
	printf("\nRunning Multi-Objective GA (displaying Pareto front boundary solutions X and Y only)...\n");
	algorithms::NSGAII<types::NSGAChromosome<types::SingleSiteSimpleGene>, deterministic::SingleSiteSimpleModel> nsgaii(
		deterministic_fitness,
		seed,
		num_threads
	);

	std::vector<types::NSGAChromosome<types::SingleSiteSimpleGene>> solutions;

	double mean_time = 0.0;

	for (int run = 0; run < num_runs; ++run) {

		auto start = std::chrono::steady_clock::now();

		nsgaii.Init(
			popsize,

			//Individual Params + GeneParams
			starting_length,
			p_xo,
			p_gene_swap,

			//GeneParams 
			num_products,
			p_product_mut,
			p_plus_batch_mut,
			p_minus_batch_mut
		);

		for (int gen = 0; gen < num_gens; ++gen) {
			nsgaii.Update();
		}			

		auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
		mean_time += elapsed_time;

		auto top_front = nsgaii.TopFront();
		solutions.insert(solutions.end(), top_front.begin(), top_front.end());

		types::SingleSiteSimpleSchedule schedule_x, schedule_y;
		deterministic_fitness.CreateSchedule(top_front[0], schedule_x);
		deterministic_fitness.CreateSchedule(top_front.back(), schedule_y);

		// std::cout << "\n######################## Run: " << run + 1 << ", no. solutions: " << top_front.size() << ", elapsed time: " << elapsed_time << " ms ########################\n" << std::endl;

		// printf(
		// 	"Solution X:\nTotal kg throughput: %.2f (%.2f)\nTotal kg inventory deficit: %.2f (%.2f)\nTotal kg backlog: %.2f\nTotal kg waste: %.2f\n\n",
		// 	top_front[0].objectives[0], schedule_x.objectives[deterministic::TOTAL_KG_THROUGHPUT],
		// 	top_front[0].objectives[1], schedule_x.objectives[deterministic::TOTAL_KG_INVENTORY_DEFICIT],
		// 	schedule_x.objectives[deterministic::TOTAL_KG_BACKLOG],
		// 	schedule_x.objectives[deterministic::TOTAL_KG_WASTE]
		// );

		// printf(
		// 	"Solution Y:\nTotal kg throughput: %.2f (%.2f)\nTotal kg inventory deficit: %.2f (%.2f)\nTotal kg backlog: %.2f\nTotal kg waste: %.2f\n\n",
		// 	top_front.back().objectives[0], schedule_y.objectives[deterministic::TOTAL_KG_THROUGHPUT],
		// 	top_front.back().objectives[1], schedule_y.objectives[deterministic::TOTAL_KG_INVENTORY_DEFICIT],
		// 	schedule_y.objectives[deterministic::TOTAL_KG_BACKLOG],
		// 	schedule_y.objectives[deterministic::TOTAL_KG_WASTE]
		// );

		// std::cout << std::flush;
	}

	if (solutions.size()) {
		solutions = nsgaii.TopFront(solutions);
		types::SingleSiteSimpleSchedule schedule_x, schedule_y;
		deterministic_fitness.CreateSchedule(solutions[0], schedule_x);
		deterministic_fitness.CreateSchedule(solutions.back(), schedule_y);

		std::cout << "\n######################## After " << num_runs << " num_runs, no. best solutions: " << solutions.size() << ", mean elapsed time: " << mean_time / num_runs << " ms ########################\n" << std::endl;

		printf(
			"Solution X:\nTotal kg throughput: %.2f (%.2f)\nTotal kg inventory deficit: %.2f (%.2f)\nTotal kg backlog: %.2f\nTotal kg waste: %.2f\n\n",
			solutions[0].objectives[0], schedule_x.objectives[deterministic::TOTAL_KG_THROUGHPUT],
			solutions[0].objectives[1], schedule_x.objectives[deterministic::TOTAL_KG_INVENTORY_DEFICIT],
			schedule_x.objectives[deterministic::TOTAL_KG_BACKLOG],
			schedule_x.objectives[deterministic::TOTAL_KG_WASTE]
		);

		printf(
			"Solution Y:\nTotal kg throughput: %.2f (%.2f)\nTotal kg inventory deficit: %.2f (%.2f)\nTotal kg backlog: %.2f\nTotal kg waste: %.2f\n\n",
			solutions.back().objectives[0], schedule_y.objectives[deterministic::TOTAL_KG_THROUGHPUT],
			solutions.back().objectives[1], schedule_y.objectives[deterministic::TOTAL_KG_INVENTORY_DEFICIT],
			schedule_y.objectives[deterministic::TOTAL_KG_BACKLOG],
			schedule_y.objectives[deterministic::TOTAL_KG_WASTE]
		);
	}
}

void Stoch_SingleSiteSimple_Test()
{
	int mc_seed = 7;
	int num_mc_sims = 1;

	seed = 7;
	num_threads = -1;

	num_runs = 10;
	num_gens = 100;
	popsize = 100;

	p_xo = 0.223077;
	p_product_mut = 0.010324;
	p_plus_batch_mut = 0.800127;
	p_minus_batch_mut = 0.787252;
	p_gene_swap = 0.322800;

	std::unordered_map<stochastic::OBJECTIVES, int> objectives;
	objectives.emplace(stochastic::TOTAL_KG_INVENTORY_DEFICIT_MEAN, -1);
	objectives.emplace(stochastic::TOTAL_KG_THROUGHPUT_MEAN, 1);

	std::unordered_map<stochastic::OBJECTIVES, std::pair<int, double>> constraints;
	constraints.emplace(stochastic::TOTAL_KG_BACKLOG_MEAN, std::make_pair(-1, 0));
	constraints.emplace(stochastic::TOTAL_KG_WASTE_MEAN, std::make_pair(-1, 0));

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

	int num_products = kg_demand_mode.size();

	// 6-month kg inventoy safety levels
	std::vector<std::vector<double>> kg_inventory_target = {
		{ 6.2,6.2,9.3,9.3,12.4,12.4,15.5,21.7,21.7,24.8,21.7,24.8,27.9,21.7,24.8,24.8,24.8,27.9,27.9,27.9,31.0,31.0,34.1,34.1,27.9,27.9,27.9,27.9,34.1,34.1,31.0,31.0,21.7,15.5,6.2,0.0 },
		{ 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2,6.2 },
		{ 0.0,4.9,9.8,9.8,9.8,9.8,19.6,19.6,14.7,19.6,19.6,19.6,14.7,19.6,19.6,14.7,14.7,19.6,19.6,9.8,19.6,19.6,19.6,19.6,24.5,34.3,24.5,29.4,39.2,39.2,29.4,19.6,19.6,14.7,4.9,0.0 },
		{ 22.0,27.5,27.5,27.5,27.5,33.0,33.0,27.5,27.5,27.5,38.5,33.0,33.0,33.0,33.0,33.0,27.5,33.0,33.0,33.0,38.5,33.0,38.5,33.0,33.0,33.0,33.0,44.0,33.0,33.0,33.0,33.0,22.0,11.0,11.0,5.5 },
	};

	std::vector<int> days_per_period = std::vector<int>{ 
		31,31,28,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30
	};

	std::vector<double> kg_yield_per_batch_min = { 3.1, 6.2, 4.9, 5.5 };
	std::vector<double> kg_yield_per_batch_mode = { 3.1, 6.2, 4.9, 5.5 };
	std::vector<double> kg_yield_per_batch_max = { 3.1, 6.2, 4.9, 5.5 };

	std::vector<double> kg_storage_limits = { 250, 250, 250, 250 }; // set high to ignore
	std::vector<double> kg_opening_stock = { 18.6, 0, 19.6, 32.0 };

	std::vector<double> inventory_penalty_per_kg = { 1, 1, 1, 1 };
	std::vector<double> backlog_penalty_per_kg = { 1, 1, 1, 1 };
	std::vector<double> production_cost_per_kg = { 1, 1, 1, 1 };
	std::vector<double> storage_cost_per_kg = { 1, 1, 1, 1 };
	std::vector<double> waste_cost_per_kg = { 1, 1, 1, 1 };
	std::vector<double> sell_price_per_kg = { 1, 1, 1, 1 };

	std::vector<int> inoculation_days = { 20, 15, 20, 26 };
	std::vector<int> seed_days = { 11, 7, 11, 9 };
	std::vector<int> production_days = { 14, 14, 14, 14 };
	std::vector<int> usp_days = { 45, 36, 45, 49 }; //
	std::vector<int> dsp_days = { 7, 11, 7, 7 };
	std::vector<int> shelf_life_days = { 730, 730, 730, 730 }; // set high to ignore
	std::vector<int> approval_days = { 90, 90, 90, 90 };
	std::vector<int> min_batches_per_campaign = { 2, 2, 2, 3 };
	std::vector<int> max_batches_per_campaign = { 50, 50, 50, 30 };
	std::vector<int> batches_multiples_of_per_campaign = { 1, 1, 1, 3 };
	std::vector<std::vector<int>> changeover_days = {
		{ 0,  10, 16, 20 },
		{ 16,  0, 16, 20 },
		{ 16, 10,  0, 20 },
		{ 18, 10, 18,  0 }
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

	stochastic::SingleSiteSimpleModel stochastic_fitness(input_data);
	
	printf("\nRunning Multi-Objective GA (displaying Pareto front boundary solutions X and Y only)...\n");
	algorithms::NSGAII<types::NSGAChromosome<types::SingleSiteSimpleGene>, stochastic::SingleSiteSimpleModel> nsgaii(
		stochastic_fitness,
		seed,
		num_threads
	);

	std::vector<types::NSGAChromosome<types::SingleSiteSimpleGene>> solutions;

	double mean_time = 0.0;

	for (int run = 0; run < num_runs; ++run) {

		auto start = std::chrono::steady_clock::now();

		nsgaii.Init(
			popsize,

			//Individual Params + GeneParams
			starting_length,
			p_xo,
			p_gene_swap,

			//GeneParams 
			num_products,
			p_product_mut,
			p_plus_batch_mut,
			p_minus_batch_mut
		);

		for (int gen = 0; gen < num_gens; ++gen) {
			nsgaii.Update();
		}			

		auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
		mean_time += elapsed_time;

		auto top_front = nsgaii.TopFront();
		solutions.insert(solutions.end(), top_front.begin(), top_front.end());
	}

	if (solutions.size()) {
		solutions = nsgaii.TopFront(solutions);
		types::SingleSiteSimpleSchedule schedule_x, schedule_y;
		stochastic_fitness.CreateSchedule(solutions[0], schedule_x);
		stochastic_fitness.CreateSchedule(solutions.back(), schedule_y);

		std::cout << "\n######################## After " << num_runs << " num_runs, no. best solutions: " << solutions.size() << ", mean elapsed time: " << mean_time / num_runs << " ms ########################\n" << std::endl;

		printf(
			"Solution X:\nTotal kg throughput: %.2f (%.2f)\nTotal kg inventory deficit: %.2f (%.2f)\nTotal kg backlog: %.2f\nTotal kg waste: %.2f\n\n",
			solutions[0].objectives[0], schedule_x.objectives[stochastic::TOTAL_KG_THROUGHPUT_MEAN],
			solutions[0].objectives[1], schedule_x.objectives[stochastic::TOTAL_KG_INVENTORY_DEFICIT_MEAN],
			schedule_x.objectives[stochastic::TOTAL_KG_BACKLOG_MEAN],
			schedule_x.objectives[stochastic::TOTAL_KG_WASTE_MEAN]
		);

		printf(
			"Solution Y:\nTotal kg throughput: %.2f (%.2f)\nTotal kg inventory deficit: %.2f (%.2f)\nTotal kg backlog: %.2f\nTotal kg waste: %.2f\n\n",
			solutions.back().objectives[0], schedule_y.objectives[stochastic::TOTAL_KG_THROUGHPUT_MEAN],
			solutions.back().objectives[1], schedule_y.objectives[stochastic::TOTAL_KG_INVENTORY_DEFICIT_MEAN],
			schedule_y.objectives[stochastic::TOTAL_KG_BACKLOG_MEAN],
			schedule_y.objectives[stochastic::TOTAL_KG_WASTE_MEAN]
		);
	}
}


int main()
{
	printf("\nDeterministic SingleSiteMultiSuite (base case) GA test...\n\n");
	Det_SingleSiteMultiSuite_BaseCaseTest();

	printf("\nDeterministic SingleSiteSimple Single-Objective GA test\n\n");
	Det_SingleSiteSimple_SingleObjective_Test();	

	printf("\nDeterministic SingleSiteSimple Multi-Objective GA test\n\n");
	Det_SingleSiteSimple_MultiObjective_Test();	

	printf("\nStochastic SingleSiteSimple GA test\n\n");
	Stoch_SingleSiteSimple_Test();

	printf("\n");

	#if defined(_WIN32) || defined(_WIN64)
		system("pause");
	#endif

	return 0;
}