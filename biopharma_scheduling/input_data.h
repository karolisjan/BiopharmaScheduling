#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
 	// #pragma GCC diagnostic ignored "-Wreorder"
	// #pragma GCC diagnostic ignored "-Wunused-variable"
	#pragma GCC diagnostic ignored "-Wformat="
	#pragma GCC diagnostic ignored "-Wsign-compare"
#endif 

#ifndef __INPUT_DATA_H__
#define __INPUT_DATA_H__

#include <queue>
#include <vector>
#include <unordered_map>

#include "utils.h"


namespace stochastic
{
    enum OBJECTIVES 
    {
		MEAN_OBJECTIVES_START,
		TOTAL_KG_INVENTORY_DEFICIT_MEAN = MEAN_OBJECTIVES_START,
		TOTAL_KG_THROUGHPUT_MEAN,
		TOTAL_KG_BACKLOG_MEAN,
		TOTAL_KG_SUPPLY_MEAN,
		TOTAL_KG_WASTE_MEAN,

		TOTAL_INVENTORY_PENALTY_MEAN,
		TOTAL_BACKLOG_PENALTY_MEAN,
		TOTAL_PRODUCTION_COST_MEAN,
		TOTAL_STORAGE_COST_MEAN,
		TOTAL_WASTE_COST_MEAN,
		TOTAL_REVENUE_MEAN,
		TOTAL_PROFIT_MEAN,
		TOTAL_COST_MEAN,
		MEAN_OBJECTIVES_END = TOTAL_COST_MEAN + 1,

		TOTAL_CHANGEOVER_COST,

        NUM_OBJECTIVES = TOTAL_CHANGEOVER_COST + 1
    };

    struct SingleSiteSimpleInputData
	{
		SingleSiteSimpleInputData() {}
		
		SingleSiteSimpleInputData(
			int mc_seed,
			int num_mc_sims, 
			
			std::unordered_map<OBJECTIVES, int> objectives,
			std::vector<int> days_per_period,

			std::vector< std::vector<double>> kg_demand_min,
			std::vector< std::vector<double>> kg_demand_mode,
			std::vector< std::vector<double>> kg_demand_max,

			std::vector<double> kg_yield_per_batch_min,
			std::vector<double> kg_yield_per_batch_mode,
			std::vector<double> kg_yield_per_batch_max,

			std::vector<double> kg_opening_stock,
			std::vector<double> kg_storage_limits,
			
			std::vector<double> inventory_penalty_per_kg,
			std::vector<double> backlog_penalty_per_kg,
			std::vector<double> production_cost_per_kg,
			std::vector<double> storage_cost_per_kg,
			std::vector<double> waste_cost_per_kg,
			std::vector<double> sell_price_per_kg,

			std::vector<int> inoculation_days,
			std::vector<int> seed_days,
			std::vector<int> production_days,
			std::vector<int> usp_days,
			std::vector<int> dsp_days,
			std::vector<int> approval_days,
			std::vector<int> shelf_life_days,
			std::vector<int> min_batches_per_campaign,
			std::vector<int> max_batches_per_campaign,
			std::vector<int> batches_multiples_of_per_campaign,
			std::vector<std::vector<int>> changeover_days,

			// Optional
			std::vector<std::vector<double>> *kg_inventory_target = NULL,
			std::unordered_map<OBJECTIVES, std::pair<int, double>> *constraints = NULL
		) 
		{				
			this->kg_demand_min = kg_demand_min;
			this->kg_demand_mode = kg_demand_mode;
			this->kg_demand_max = kg_demand_max;

			this->mc_seed = mc_seed;
			this->num_mc_sims = num_mc_sims;

			this->kg_yield_per_batch_min = kg_yield_per_batch_min;
			this->kg_yield_per_batch_mode = kg_yield_per_batch_mode;
			this->kg_yield_per_batch_max = kg_yield_per_batch_max;			

			this->days_per_period = days_per_period;
			this->num_products = kg_demand_mode.size();
			this->num_periods = days_per_period.size();

			this->kg_opening_stock = kg_opening_stock;
			this->kg_storage_limits = kg_storage_limits;

			this->inventory_penalty_per_kg = inventory_penalty_per_kg;
			this->backlog_penalty_per_kg = backlog_penalty_per_kg;
			this->production_cost_per_kg = production_cost_per_kg;
			this->storage_cost_per_kg = storage_cost_per_kg;
			this->waste_cost_per_kg = waste_cost_per_kg;
			this->sell_price_per_kg = sell_price_per_kg;

			this->inoculation_days = inoculation_days;
			this->seed_days = seed_days;
			this->production_days = production_days;
			this->usp_days = usp_days; 
			this->dsp_days = dsp_days;
			this->approval_days = approval_days;
			this->shelf_life_days = shelf_life_days;
			this->changeover_days = changeover_days;
			this->min_batches_per_campaign = min_batches_per_campaign;
			this->max_batches_per_campaign = max_batches_per_campaign;
			this->batches_multiples_of_per_campaign = batches_multiples_of_per_campaign;

			int prev = 0;

			for (auto &days : days_per_period) {
				due_dates.push_back(days + prev);
				prev = due_dates.back();
			}

			horizon = due_dates.back();

			for (const auto &it : objectives) {
				this->objectives.push_back(std::make_pair(it.first, it.second));
			}

			if (kg_inventory_target) {
				this->kg_inventory_target = *kg_inventory_target;
			}

			if (constraints) {
				for (const auto &it : *constraints) {
					this->constraints.push_back(std::make_pair(it.first, it.second));
				}
			}

			if (mc_seed != -1) {
				std::vector<int> seed = { mc_seed };
				rng.init(seed);
			}
			else {
				rng.init();
			}
		}

		std::vector<std::pair<OBJECTIVES, int>> objectives;
		std::vector<std::pair<OBJECTIVES, std::pair<int, double>>> constraints;

		int mc_seed;
		int num_mc_sims;
		int num_products;
        int num_periods;

        double horizon; 

		std::vector< std::vector<double>> kg_demand_min;
		std::vector< std::vector<double>> kg_demand_mode;
		std::vector< std::vector<double>> kg_demand_max;

		std::vector<double> kg_yield_per_batch_min;
		std::vector<double> kg_yield_per_batch_mode;
		std::vector<double> kg_yield_per_batch_max;

		std::vector< std::vector<int>> changeover_days;
        std::vector< std::vector<double>> kg_inventory_target;

		std::vector<double> kg_opening_stock;
        std::vector<double> kg_storage_limits;

		std::vector<double> inventory_penalty_per_kg;
        std::vector<double> backlog_penalty_per_kg;
        std::vector<double> production_cost_per_kg;
        std::vector<double> storage_cost_per_kg;
        std::vector<double> waste_cost_per_kg;
        std::vector<double> sell_price_per_kg;

		std::vector<int> inoculation_days;
        std::vector<int> seed_days;
        std::vector<int> production_days;
        std::vector<int> usp_days;
        std::vector<int> dsp_days;
        std::vector<int> approval_days;
        std::vector<int> shelf_life_days;
		std::vector<int> days_per_period;
        std::vector<int> due_dates;
		std::vector<int> min_batches_per_campaign;
        std::vector<int> max_batches_per_campaign;
        std::vector<int> batches_multiples_of_per_campaign;

		utils::CustomRandom<> rng;
	};
}

namespace deterministic
{
    enum OBJECTIVES 
    {
        TOTAL_KG_INVENTORY_DEFICIT,
        TOTAL_KG_THROUGHPUT,
        TOTAL_KG_BACKLOG,
        TOTAL_KG_SUPPLY,
        TOTAL_KG_WASTE,

		TOTAL_BATCH_INVENTORY_DEFICIT,
        TOTAL_BATCH_THROUGHPUT,
        TOTAL_BATCH_BACKLOG,
        TOTAL_BATCH_SUPPLY,
        TOTAL_BATCH_WASTE,

        TOTAL_INVENTORY_PENALTY,
        TOTAL_CHANGEOVER_COST,
        TOTAL_BACKLOG_PENALTY,
        TOTAL_PRODUCTION_COST,
        TOTAL_STORAGE_COST,
        TOTAL_WASTE_COST,
        TOTAL_REVENUE,
        TOTAL_PROFIT,
        TOTAL_COST,
        NUM_OBJECTIVES = TOTAL_COST + 1
    };


    struct SingleSiteMultiSuiteInputData
	{
		SingleSiteMultiSuiteInputData() {}

		SingleSiteMultiSuiteInputData(
			std::unordered_map<OBJECTIVES, int> objectives,

			int num_usp_suites,
			int num_dsp_suites,

			std::vector<std::vector<int>> demand,
			std::vector<int> days_per_period,

			std::vector<double> usp_days,
			std::vector<double> dsp_days,

			std::vector<int> shelf_life,
			std::vector<int> storage_cap,

			std::vector<double> sales_price,
			std::vector<double> storage_cost,
			std::vector<double> backlog_penalty,
			std::vector<double> waste_disposal_cost,
			std::vector<double> usp_production_cost,
			std::vector<double> dsp_production_cost,
			std::vector<double> usp_changeover_cost,
			std::vector<double> dsp_changeover_cost,

			std::vector<std::vector<double>> usp_changeovers,
			std::vector<std::vector<double>> dsp_changeovers,

            // Optional
			std::unordered_map<OBJECTIVES, std::pair<int, double>> *constraints = NULL
		) 
		{
			this->num_usp_suites = num_usp_suites;
			this->num_dsp_suites = num_dsp_suites;

			this->demand = demand;
			this->days_per_period = days_per_period;

			this->num_products = demand.size();
			this->num_periods = days_per_period.size();

			this->usp_days = usp_days;
			this->dsp_days = dsp_days;
			
			this->shelf_life = shelf_life;
			this->storage_cap = storage_cap;

			this->sales_price = sales_price;
			this->storage_cost = storage_cost;
			this->backlog_penalty = backlog_penalty;
			this->waste_disposal_cost = waste_disposal_cost;
			this->usp_production_cost = usp_production_cost;
			this->dsp_production_cost = dsp_production_cost;
			this->usp_changeover_cost = usp_changeover_cost;
			this->dsp_changeover_cost = dsp_changeover_cost;

			this->usp_changeovers = usp_changeovers;
			this->dsp_changeovers = dsp_changeovers;

			int prev = 0;

			for (auto &days : days_per_period) {
				due_dates.push_back(days + prev);
				prev = due_dates.back();
			}

			horizon = due_dates.back();

			for (const auto &it : objectives) {
				this->objectives.push_back(std::make_pair(it.first, it.second));
			}

			if (constraints) {
				for (const auto &it : *constraints) {
					this->constraints.push_back(std::make_pair(it.first, it.second));
				}
			}
		}

		int num_usp_suites, num_dsp_suites;

		std::vector<std::vector<int>> demand;
		std::vector<int> days_per_period;

		std::vector<double> usp_days;
        std::vector<double> dsp_days;

		int num_products, num_periods;

		std::vector<double> usp_production_cost;
        std::vector<double> dsp_production_cost;
		std::vector<double> usp_changeover_cost;
        std::vector<double> dsp_changeover_cost;
		std::vector<double> sales_price;
        std::vector<double> storage_cost;
        std::vector<double> backlog_penalty;
        std::vector<double> waste_disposal_cost;

		std::vector<int> shelf_life;
        std::vector<int> storage_cap;

		std::vector<std::vector<double>> usp_changeovers;
		std::vector<std::vector<double>> dsp_changeovers;

		std::vector<int> due_dates;

		std::vector<std::pair<OBJECTIVES, int>> objectives;
		std::vector<std::pair<OBJECTIVES, std::pair<int, double>>> constraints;

		int horizon;
	};


    struct SingleSiteSimpleInputData
	{
		SingleSiteSimpleInputData() {}
		
		SingleSiteSimpleInputData(
			std::unordered_map<OBJECTIVES, int> objectives,

			std::vector< std::vector<double>> kg_demand,
			std::vector<int> days_per_period,

			std::vector<double> kg_opening_stock,
			std::vector<double> kg_yield_per_batch,
			std::vector<double> kg_storage_limits,
			
			std::vector<double> inventory_penalty_per_kg,
			std::vector<double> backlog_penalty_per_kg,
			std::vector<double> production_cost_per_kg,
			std::vector<double> storage_cost_per_kg,
			std::vector<double> waste_cost_per_kg,
			std::vector<double> sell_price_per_kg,

			std::vector<int> inoculation_days,
			std::vector<int> seed_days,
			std::vector<int> production_days,
			std::vector<int> usp_days,
			std::vector<int> dsp_days,
			std::vector<int> approval_days,
			std::vector<int> shelf_life_days,
			std::vector<int> min_batches_per_campaign,
			std::vector<int> max_batches_per_campaign,
			std::vector<int> batches_multiples_of_per_campaign,
			std::vector<std::vector<int>> changeover_days,

			// Optional
			std::vector<std::vector<double>> *kg_inventory_target = NULL,
			std::unordered_map<OBJECTIVES, std::pair<int, double>> *constraints = NULL
		) 
		{	
			this->kg_demand = kg_demand;
			this->days_per_period = days_per_period;
			this->num_products = kg_demand.size();
			this->num_periods = days_per_period.size();

			this->kg_opening_stock = kg_opening_stock;
			this->kg_yield_per_batch = kg_yield_per_batch;
			this->kg_storage_limits = kg_storage_limits;

			this->inventory_penalty_per_kg = inventory_penalty_per_kg;
			this->backlog_penalty_per_kg = backlog_penalty_per_kg;
			this->production_cost_per_kg = production_cost_per_kg;
			this->storage_cost_per_kg = storage_cost_per_kg;
			this->waste_cost_per_kg = waste_cost_per_kg;
			this->sell_price_per_kg = sell_price_per_kg;

			this->inoculation_days = inoculation_days;
			this->seed_days = seed_days;
			this->production_days = production_days;
			this->usp_days = usp_days; 
			this->dsp_days = dsp_days;
			this->approval_days = approval_days;
			this->shelf_life_days = shelf_life_days;
			this->changeover_days = changeover_days;
			this->min_batches_per_campaign = min_batches_per_campaign;
			this->max_batches_per_campaign = max_batches_per_campaign;
			this->batches_multiples_of_per_campaign = batches_multiples_of_per_campaign;

			int prev = 0;

			for (auto &days : days_per_period) {
				due_dates.push_back(days + prev);
				prev = due_dates.back();
			}

			horizon = due_dates.back();

			for (const auto &it : objectives) {
				this->objectives.push_back(std::make_pair(it.first, it.second));
			}

			if (kg_inventory_target) {
				this->kg_inventory_target = *kg_inventory_target;
			}

			if (constraints) {
				for (const auto &it : *constraints) {
					this->constraints.push_back(std::make_pair(it.first, it.second));
				}
			}
		}

		std::vector< std::vector<double>> kg_demand;
		std::vector< std::vector<double>> kg_inventory_target;
		std::vector<int> days_per_period;

		int num_products;
        int num_periods;

		std::vector<double> kg_opening_stock;
        std::vector<double> kg_yield_per_batch;
        std::vector<double> kg_storage_limits;

		std::vector<double> inventory_penalty_per_kg;
        std::vector<double> backlog_penalty_per_kg;
        std::vector<double> production_cost_per_kg;
        std::vector<double> storage_cost_per_kg;
        std::vector<double> waste_cost_per_kg;
        std::vector<double> sell_price_per_kg;

		std::vector<int> inoculation_days;
        std::vector<int> seed_days;
        std::vector<int> production_days;
        std::vector<int> usp_days;
        std::vector<int> dsp_days;
        std::vector<int> approval_days;
        std::vector<int> shelf_life_days;
		std::vector< std::vector<int>> changeover_days;
		std::vector<int> min_batches_per_campaign;
        std::vector<int> max_batches_per_campaign;
        std::vector<int> batches_multiples_of_per_campaign;

		std::vector<int> due_dates;

		std::vector<std::pair<OBJECTIVES, int>> objectives;
		std::vector<std::pair<OBJECTIVES, std::pair<int, double>>> constraints;

		double horizon; 
	};
}

#endif