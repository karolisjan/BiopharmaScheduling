#include <queue>
#include <vector>
#include <unordered_map>


namespace deterministic
{
    enum OBJECTIVES 
    {
        TOTAL_KG_INVENTORY_DEFICIT,
        TOTAL_KG_THROUGHPUT,
        TOTAL_KG_BACKLOG,
        TOTAL_KG_SUPPLY,
        TOTAL_KG_WASTE,

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
			std::vector<double> production_factor,

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

			std::vector<double> usp_lead_days,
			std::vector<double> dsp_lead_days,

            // Optional
			std::unordered_map<OBJECTIVES, std::pair<int, double>> *constraints = NULL
		) :
			num_usp_suites(num_usp_suites),
			num_dsp_suites(num_dsp_suites),

			demand(demand),
			days_per_period(days_per_period),

			num_products(demand.size()),
			num_periods(days_per_period.size()),

			usp_days(usp_days),
			dsp_days(dsp_days),
			production_factor(production_factor),
			
			shelf_life(shelf_life),
			storage_cap(storage_cap),

			sales_price(sales_price),
			storage_cost(storage_cost),
			backlog_penalty(backlog_penalty),
			waste_disposal_cost(waste_disposal_cost),
			usp_production_cost(usp_production_cost),
			dsp_production_cost(dsp_production_cost),
			usp_changeover_cost(usp_changeover_cost),
			dsp_changeover_cost(dsp_changeover_cost),

			usp_lead_days(usp_lead_days),
			dsp_lead_days(dsp_lead_days)
		{
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

		std::vector<std::pair<OBJECTIVES, int>> objectives;
		std::vector<std::pair<OBJECTIVES, std::pair<int, double>>> constraints;

		int num_products, num_periods, horizon, num_usp_suites, num_dsp_suites;

		std::vector<std::vector<int>> demand;
		std::vector<int> days_per_period, due_dates;

		std::vector<double> usp_production_cost;
        std::vector<double> dsp_production_cost;
		std::vector<double> usp_changeover_cost;
        std::vector<double> dsp_changeover_cost;
		std::vector<double> sales_price;
        std::vector<double> storage_cost;
        std::vector<double> backlog_penalty;
        std::vector<double> waste_disposal_cost;

		std::vector<double> usp_days;
        std::vector<double> dsp_days;
		std::vector<double> usp_lead_days;
        std::vector<double> dsp_lead_days;
        std::vector<double> production_factor;

		std::vector<int> shelf_life;
        std::vector<int> storage_cap;
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
		) :
			kg_demand(kg_demand),
			kg_inventory_target(kg_inventory_target),
			days_per_period(days_per_period),
			num_products(kg_demand.size()),
			num_periods(days_per_period.size()),

			kg_opening_stock(kg_opening_stock),
			kg_yield_per_batch(kg_yield_per_batch),
			kg_storage_limits(kg_storage_limits),

			inventory_penalty_per_kg(inventory_penalty_per_kg),
			backlog_penalty_per_kg(backlog_penalty_per_kg),
			production_cost_per_kg(production_cost_per_kg),
			storage_cost_per_kg(storage_cost_per_kg),
			waste_cost_per_kg(waste_cost_per_kg),
			sell_price_per_kg(sell_price_per_kg),

			inoculation_days(inoculation_days),
			seed_days(seed_days),
			production_days(production_days),
			usp_days(usp_days), 
			dsp_days(dsp_days),
			approval_days(approval_days),
			shelf_life_days(shelf_life_days),
			changeover_days(changeover_days),
			min_batches_per_campaign(min_batches_per_campaign),
			max_batches_per_campaign(max_batches_per_campaign),
			batches_multiples_of_per_campaign(batches_multiples_of_per_campaign)
		{	
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

		std::vector<std::pair<OBJECTIVES, int>> objectives;
		std::vector<std::pair<OBJECTIVES, std::pair<int, double>>> constraints;

		int num_products;
        int num_periods;

        double horizon; 

		std::vector< std::vector<int>> changeover_days;
		std::vector< std::vector<double>> kg_demand;
        std::vector< std::vector<double>> *kg_inventory_target;

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
		std::vector<int> days_per_period;
        std::vector<int> due_dates;
		std::vector<int> min_batches_per_campaign;
        std::vector<int> max_batches_per_campaign;
        std::vector<int> batches_multiples_of_per_campaign;
	};
}