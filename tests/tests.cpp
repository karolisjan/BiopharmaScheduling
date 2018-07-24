#define CATCH_CONFIG_MAIN  

#include "catch.hpp"

#include <unordered_map>

#include "../biopharma_scheduling/gene.h"
#include "../biopharma_scheduling/nsgaii.h"
#include "../biopharma_scheduling/single_objective_ga.h"
#include "../biopharma_scheduling/scheduling_models.h"


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

SCENARIO("deterministic::SingleSiteSimpleModel known solution test")
{
	int seed = 7;
	int num_threads = -1;

	int num_runs = 20;
	int num_gens = 100;
	int popsize = 100;
	int starting_length = 1;

	double p_xo = 0.130878;
	double p_product_mut = 0.017718;
	double p_plus_batch_mut = 0.707202;
	double p_minus_batch_mut = 0.834735;
	double p_gene_swap = 0.531073;

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

	types::NSGAChromosome<types::SingleSiteSimpleGene> i;

	i.genes.resize(11);

	i.genes[0].product_num = 4;
	i.genes[0].num_batches = 15;
	
	i.genes[1].product_num = 3;
	i.genes[1].num_batches = 9;

	i.genes[2].product_num = 1;
	i.genes[2].num_batches = 28;

	i.genes[3].product_num = 2;
	i.genes[3].num_batches = 2;

	i.genes[4].product_num = 4;
	i.genes[4].num_batches = 15;

	i.genes[5].product_num = 3;
	i.genes[5].num_batches = 8;

	i.genes[6].product_num = 1;
	i.genes[6].num_batches = 10;

	i.genes[7].product_num = 3;
	i.genes[7].num_batches = 3;

	i.genes[8].product_num = 2;
	i.genes[8].num_batches = 2;

	i.genes[9].product_num = 1;
	i.genes[9].num_batches = 3;

	i.genes[10].product_num = 4;
	i.genes[10].num_batches = 29;

	deterministic::SingleSiteSimpleModel deterministic_fitness(input_data);
	types::SingleSiteSimpleSchedule schedule;
	
	deterministic_fitness(i);
	deterministic_fitness.CreateSchedule(i, schedule);

	REQUIRE( -i.objectives[0] == Approx(schedule.objectives[deterministic::TOTAL_KG_THROUGHPUT]) );
	REQUIRE( i.objectives[1] == Approx(schedule.objectives[deterministic::TOTAL_KG_INVENTORY_DEFICIT]) );
	REQUIRE( schedule.objectives[deterministic::TOTAL_KG_THROUGHPUT] == Approx(574.4) );
	REQUIRE( schedule.objectives[deterministic::TOTAL_KG_INVENTORY_DEFICIT] == Approx(194.6) );
	REQUIRE( schedule.objectives[deterministic::TOTAL_KG_BACKLOG] == Approx(0.0) );
	REQUIRE( schedule.objectives[deterministic::TOTAL_KG_WASTE] == Approx(0.0) );
}

SCENARIO("deterministic::SingleSiteMultiSuiteModel Single-Objective Example 1 test")
{
	int seed = 7;
	int num_threads = -1;
	int num_runs = 10;
	int num_gens = 100;
	int popsize = 100; 

	int starting_length = 1;

	double p_xo = 0.131266;
	double p_product_mut = 0.131266;
	double p_usp_suite_mut = 0.131266;
	double p_plus_batch_mut = 0.131266;
	double p_minus_batch_mut = 0.131266;
	double p_gene_swap = 0.131266;

	std::unordered_map<deterministic::OBJECTIVES, int> objectives;
 	objectives.emplace(deterministic::TOTAL_PROFIT, 1);
	
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
		dsp_changeovers
	);

 	deterministic::SingleSiteMultiSuiteModel single_site_multi_suite_model(input_data);

 	algorithms::SingleObjectiveGA<types::SingleObjectiveChromosome<types::SingleSiteMultiSuiteGene>, deterministic::SingleSiteMultiSuiteModel> simple_ga(
 		single_site_multi_suite_model,
 		seed,
 		num_threads
 	);

	std::vector<types::SingleObjectiveChromosome<types::SingleSiteMultiSuiteGene>> solutions;

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
 		}

 		solutions.push_back(simple_ga.Top());
 	}

	auto solution = simple_ga.Top(solutions);
	types::SingleSiteMultiSuiteSchedule schedule;
	single_site_multi_suite_model.CreateSchedule(solution, schedule);

	REQUIRE( -solution.objective == Approx(schedule.objectives[deterministic::TOTAL_PROFIT]) );		
	REQUIRE( schedule.objectives[deterministic::TOTAL_PROFIT] == Approx(518.0) );
	REQUIRE( schedule.objectives[deterministic::TOTAL_BATCH_BACKLOG] == Approx(0.0) );
	REQUIRE( schedule.objectives[deterministic::TOTAL_BATCH_WASTE] == Approx(0.0) );
}

SCENARIO("deterministic::SingleSiteMultiSuiteModel Single-Objective Example 2 test")
{
	int seed = 7;
	int num_threads = -1;
	int num_runs = 10;
	int num_gens = 100;
	int popsize = 100; 

	int starting_length = 1;

	double p_xo = 0.026776;
	double p_product_mut = 0.004667;
	double p_usp_suite_mut = 0.015991;
	double p_plus_batch_mut = 0.896385;
	double p_minus_batch_mut = 0.853790;
	double p_gene_swap = 0.403328;

	std::unordered_map<deterministic::OBJECTIVES, int> objectives;
 	objectives.emplace(deterministic::TOTAL_PROFIT, 1);
	
 	std::unordered_map<deterministic::OBJECTIVES, std::pair<int, double>> constraints;
	constraints.emplace(deterministic::TOTAL_BACKLOG_PENALTY, std::make_pair(-1, 0));

 	std::vector<std::vector<int>> demand =
 	{
		{  0,0,0,6,0,4,0,0,4  },
		{  0,4,0,0,0,0,4,0,0  },
		{  0,0,0,0,10,0,0,0,0  },
		{  0,6,0,8,0,0,0,0,10  },
 	};

 	std::vector<int> days_per_period = { 60, 60, 60, 60, 60, 60, 60, 60, 60 };

    int num_usp_suites = 2, num_dsp_suites = 3, num_products = demand.size();

	std::vector<double> sales_price = { 25.0, 20.0,	17.0, 17.0 };
	std::vector<double> usp_production_cost = { 5.0, 2.0, 1.0, 1.0 };
	std::vector<double> dsp_production_cost = { 5.0, 2.0, 1.0, 1.0 };
	std::vector<double> waste_disposal_cost = { 5.0, 5.0, 5.0, 5.0 };
	std::vector<double> storage_cost = { 1.0, 1.0, 1.0, 1.0 };
	std::vector<double> backlog_penalty = { 20.0, 20.0, 20.0, 20.0 };
	std::vector<double> usp_changeover_cost = { 1.0, 1.0, 1.0, 1.0 };
	std::vector<double> dsp_changeover_cost = { 1.0, 1.0, 1.0, 1.0 };
 
	std::vector<double> usp_days = { 20.0, 22.2222, 12.5, 12.5 };
	std::vector<double> dsp_days = { 10.0, 10.0, 10.0, 10.0 };

	std::vector<std::vector<double>> usp_changeovers = {
		{  10.0,10.0,10.0,10.0  },
		{  10.0,10.0,10.0,10.0  },
		{  10.0,10.0,10.0,10.0  },
		{  10.0,10.0,10.0,10.0  },
	};

	std::vector<std::vector<double>> dsp_changeovers = {
		{  10.0,10.0,10.0,10.0  },
		{  10.0,10.0,10.0,10.0  },
		{  12.5,12.5,12.5,12.5  },
		{  12.5,12.5,12.5,12.5  },
	};

	std::vector<int> shelf_life = { 180, 180, 180, 180 };
	std::vector<int> storage_cap = { 40, 40, 40, 40 };

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

	std::vector<types::SingleObjectiveChromosome<types::SingleSiteMultiSuiteGene>> solutions;

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
 		}

 		solutions.push_back(simple_ga.Top());
 	}

	auto solution = simple_ga.Top(solutions);
	types::SingleSiteMultiSuiteSchedule schedule;
	single_site_multi_suite_model.CreateSchedule(solution, schedule);

	REQUIRE( schedule.objectives[deterministic::TOTAL_PROFIT] == Approx(795.0) );
	REQUIRE( schedule.objectives[deterministic::TOTAL_BATCH_BACKLOG] == Approx(0.0) );
	REQUIRE( schedule.objectives[deterministic::TOTAL_BATCH_WASTE] == Approx(0.0) );
}

SCENARIO("deterministic::SingleSiteSimpleModel Single-Objective test")
{
	int seed = 7;
	int num_threads = -1;

	int num_runs = 20;
	int num_gens = 100;
	int popsize = 100;
	int starting_length = 1;

	double p_xo = 0.108198;
	double p_product_mut = 0.041373;
	double p_plus_batch_mut = 0.608130;
	double p_minus_batch_mut = 0.765819;
	double p_gene_swap = 0.471346;

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
	
	algorithms::SingleObjectiveGA<types::SingleObjectiveChromosome<types::SingleSiteSimpleGene>, deterministic::SingleSiteSimpleModel> ga(
		deterministic_fitness,
		seed,
		num_threads
	);

	std::vector<types::SingleObjectiveChromosome<types::SingleSiteSimpleGene>> solutions;

	for (int run = 0; run < num_runs; ++run) {
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

		solutions.push_back(ga.Top());	
	}
	
	auto solution = ga.Top(solutions);
	types::SingleSiteSimpleSchedule schedule;
	deterministic_fitness.CreateSchedule(solution, schedule);

	REQUIRE( -solution.objective == Approx(schedule.objectives[deterministic::TOTAL_KG_THROUGHPUT]) );		
	REQUIRE( schedule.objectives[deterministic::TOTAL_KG_THROUGHPUT] == Approx(630.4) );
	REQUIRE( schedule.objectives[deterministic::TOTAL_KG_INVENTORY_DEFICIT] == Approx(472.2) );
	REQUIRE( schedule.objectives[deterministic::TOTAL_KG_BACKLOG] == Approx(0.0) );
	REQUIRE( schedule.objectives[deterministic::TOTAL_KG_WASTE] == Approx(0.0) );
}

SCENARIO("deterministic::SingleSiteSimpleModel Multi-Objective test")
{
	int seed = 7;
	int num_threads = -1;

	int num_runs = 20;
	int num_gens = 100;
	int popsize = 100;

	int starting_length = 1;

	double p_xo = 0.108198;
	double p_product_mut = 0.041373;
	double p_plus_batch_mut = 0.608130;
	double p_minus_batch_mut = 0.765819;
	double p_gene_swap = 0.471346;

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
	
	algorithms::NSGAII<types::NSGAChromosome<types::SingleSiteSimpleGene>, deterministic::SingleSiteSimpleModel> nsgaii(
		deterministic_fitness,
		seed,
		num_threads
	);

	std::vector<types::NSGAChromosome<types::SingleSiteSimpleGene>> solutions;

	for (int run = 0; run < num_runs; ++run) {
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

		auto top_front = nsgaii.TopFront();
		solutions.insert(solutions.end(), top_front.begin(), top_front.end());
	}

	solutions = nsgaii.TopFront(solutions);
	types::SingleSiteSimpleSchedule schedule_x, schedule_y;
	deterministic_fitness.CreateSchedule(solutions[0], schedule_x);
	deterministic_fitness.CreateSchedule(solutions.back(), schedule_y);

	REQUIRE( -solutions[0].objectives[0] == Approx(schedule_x.objectives[deterministic::TOTAL_KG_THROUGHPUT]) );
	REQUIRE( solutions[0].objectives[1] == Approx(schedule_x.objectives[deterministic::TOTAL_KG_INVENTORY_DEFICIT]) );
	REQUIRE( schedule_x.objectives[deterministic::TOTAL_KG_THROUGHPUT] == Approx(596.4) );
	REQUIRE( schedule_x.objectives[deterministic::TOTAL_KG_INVENTORY_DEFICIT] == Approx(326.3) );
	REQUIRE( schedule_x.objectives[deterministic::TOTAL_KG_BACKLOG] == Approx(1.0) );
	REQUIRE( schedule_x.objectives[deterministic::TOTAL_KG_WASTE] == Approx(0.0) );

	REQUIRE( -solutions.back().objectives[0] == Approx(schedule_y.objectives[deterministic::TOTAL_KG_THROUGHPUT]) );
	REQUIRE( solutions.back().objectives[1] == Approx(schedule_y.objectives[deterministic::TOTAL_KG_INVENTORY_DEFICIT]) );
	REQUIRE( schedule_y.objectives[deterministic::TOTAL_KG_THROUGHPUT] == Approx(614.7) );
	REQUIRE( schedule_y.objectives[deterministic::TOTAL_KG_INVENTORY_DEFICIT] == Approx(479.9) );
	REQUIRE( schedule_y.objectives[deterministic::TOTAL_KG_BACKLOG] == Approx(1.0) );
	REQUIRE( schedule_y.objectives[deterministic::TOTAL_KG_WASTE] == Approx(0.0) );	
}

SCENARIO("stochastic::SingleSiteSimpleModel::CreateSchedule test") 
{
	int mc_seed = 7;
	int num_mc_sims = 100;

	int seed = 7;

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
	
	types::NSGAChromosome<types::SingleSiteSimpleGene> i;

	i.genes.resize(11);

	i.genes[0].product_num = 4;
	i.genes[0].num_batches = 15;
	
	i.genes[1].product_num = 3;
	i.genes[1].num_batches = 9;

	i.genes[2].product_num = 1;
	i.genes[2].num_batches = 28;

	i.genes[3].product_num = 2;
	i.genes[3].num_batches = 2;

	i.genes[4].product_num = 4;
	i.genes[4].num_batches = 15;

	i.genes[5].product_num = 3;
	i.genes[5].num_batches = 8;

	i.genes[6].product_num = 1;
	i.genes[6].num_batches = 10;

	i.genes[7].product_num = 3;
	i.genes[7].num_batches = 3;

	i.genes[8].product_num = 2;
	i.genes[8].num_batches = 2;

	i.genes[9].product_num = 1;
	i.genes[9].num_batches = 3;

	i.genes[10].product_num = 4;
	i.genes[10].num_batches = 29;

	stochastic::SingleSiteSimpleModel single_site_simple_model(input_data);
	types::SingleSiteSimpleSchedule schedule;
	
	single_site_simple_model(i);
	single_site_simple_model.CreateSchedule(i, schedule);

	REQUIRE( -i.objectives[0] == Approx(schedule.objectives[stochastic::TOTAL_KG_THROUGHPUT_MEAN]) );
	REQUIRE( i.objectives[1] == Approx(schedule.objectives[stochastic::TOTAL_KG_INVENTORY_DEFICIT_MEAN]) );
	REQUIRE( schedule.objectives[stochastic::TOTAL_KG_THROUGHPUT_MEAN] == Approx(574.4) );
	REQUIRE( schedule.objectives[stochastic::TOTAL_KG_INVENTORY_DEFICIT_MEAN] == Approx(194.6) );
	REQUIRE( schedule.objectives[stochastic::TOTAL_KG_BACKLOG_MEAN] == Approx(0.0) );
	REQUIRE( schedule.objectives[stochastic::TOTAL_KG_WASTE_MEAN] == Approx(0.0) );
}