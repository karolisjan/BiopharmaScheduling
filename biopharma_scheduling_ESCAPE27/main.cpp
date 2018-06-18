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

	single_site_multi_suite_model(i);

	printf("%.2f profit", i.objective);
}

int main()
{
    printf("Lakhdar2005 Example 1 Scheduling Model tests...\n");

	printf("A base case solution test: ");
	Lakhdar2005Ex1_BaseCaseGlobalOptimumTest();
}