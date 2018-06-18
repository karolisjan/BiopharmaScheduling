#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
    #pragma GCC diagnostic ignored "-Wreorder"
	#pragma GCC diagnostic ignored "-Wsign-compare"
	#pragma GCC diagnostic ignored "-Wunused-variable"
#endif 

#ifndef  __MODELS_H__
#define __MODELS_H__

#include <queue>
#include <cmath>
#include <vector>
#include <utility>
#include <numeric>
#include <cassert>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <unordered_map>

#include "batch.h"
#include "campaign.h"
#include "schedule.h"
#include "single_objective_individual.h"

namespace deterministic
{
	enum OBJECTIVES 
    {
		TOTAL_CHANGEOVER_COSTS,
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
			int num_usp_suites,
			int num_dsp_suites,

			std::vector<std::vector<int>> demand,
			std::vector<int> days_per_period,

			std::vector<double> usp_days,
			std::vector<double> dsp_days,
			std::vector<double> production_factor,

			std::vector<int> shelf_life,
			std::vector<int> storage_cap,

			std::vector<double> sales_prices,
			std::vector<double> sotrage_cost,
			std::vector<double> backlog_penalty,
			std::vector<double> waste_disposal_cost,
			std::vector<double> usp_production_cost,
			std::vector<double> dsp_production_cost,
			std::vector<double> usp_changeover_cost,
			std::vector<double> dsp_changeover_cost,

			std::vector<double> usp_lead_days,
			std::vector<double> dsp_lead_days
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
			storage_cost(sotrage_cost),
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

			for (const auto &days : days_per_period) {
				due_dates.push_back(days + prev);
				prev = due_dates.back();
			}

			horizon = due_dates.back();
		}

		int num_products, num_periods, horizon, num_usp_suites, num_dsp_suites;

		std::vector<std::vector<int>> demand;
		std::vector<int> days_per_period, due_dates;

		std::vector<double> usp_production_cost, dsp_production_cost;
		std::vector<double> usp_changeover_cost, dsp_changeover_cost;
		std::vector<double> sales_price, storage_cost, backlog_penalty, waste_disposal_cost;

		std::vector<double> production_factor;
		std::vector<double> usp_days, dsp_days;
		std::vector<double> usp_lead_days, dsp_lead_days;

		std::vector<int> shelf_life, storage_cap;
	};

	class SingleSiteMultiSuiteModel
	{
		SingleSiteMultiSuiteInputData input_data;

		/*
			Adds a batch to an inventory priority queue (oldest first) within an appropriate time
			bucket based on the approval date of the said batch.
		*/
		inline void AddToInventory(types::SingleSiteMultiSuiteSchedule &schedule, types::Batch &new_batch)
		{
			// Range based binary search for a time period_num to fit the batch in 
			// based on its approval date
			int period_num = utils::search(input_data.due_dates, new_batch.stored_at); 

			if (period_num != -1) {
				schedule.batch_inventory[new_batch.product_num - 1][period_num].push(new_batch);
			}
		}

		template<class Individual>
		inline void AddNewUSPCampaign(
			int cmpgn_num,
			Individual &individual,
			types::SingleSiteMultiSuiteSchedule &schedule
		)
		{
			auto &gene = individual.genes[cmpgn_num];

			types::Campaign new_cmpgn;
			types::Campaign &prev_cmpgn = schedule.suites[gene.usp_suite_num].back();

			new_cmpgn.suite_num = gene.usp_suite_num;
			new_cmpgn.product_num = gene.product_num;
			new_cmpgn.num_batches = gene.num_batches;
			new_cmpgn.start = prev_cmpgn.end + input_data.usp_lead_days[new_cmpgn.product_num - 1];

			// Real campaign
			if (new_cmpgn.product_num != 0) { 
				new_cmpgn.end = new_cmpgn.start + input_data.usp_days[new_cmpgn.product_num - 1] * new_cmpgn.num_batches;

				while (new_cmpgn.end > input_data.horizon && --new_cmpgn.num_batches > 0) {
					new_cmpgn.end -= input_data.usp_days[new_cmpgn.product_num - 1];
				}
			}
			// Dummy one
			else {
				new_cmpgn.end = new_cmpgn.start + new_cmpgn.num_batches;

				while (new_cmpgn.end > input_data.horizon && --new_cmpgn.num_batches > 0) {
					--new_cmpgn.end;
				}
			}

			gene.num_batches = new_cmpgn.num_batches;

			if (new_cmpgn.num_batches) {
				schedule.suites[new_cmpgn.suite_num].push_back(new_cmpgn);
			}
		}

		template<class Individual>
		inline void ContinuePreviousUSPCampaign(
			int cmpgn_num,
			Individual &individual,
			types::SingleSiteMultiSuiteSchedule &schedule
		)
		{
			auto &gene = individual.genes[cmpgn_num];
			types::Campaign &prev_cmpgn = schedule.suites[gene.usp_suite_num].back();

			prev_cmpgn.num_batches += gene.num_batches;

			if (prev_cmpgn.product_num != 0) {
				prev_cmpgn.end = prev_cmpgn.start + input_data.usp_days[prev_cmpgn.product_num - 1] * prev_cmpgn.num_batches;

				while (prev_cmpgn.end > input_data.horizon && --prev_cmpgn.num_batches > 0) {
					prev_cmpgn.end -= input_data.usp_days[prev_cmpgn.product_num - 1];
				}
			}
			else {
				prev_cmpgn.end = prev_cmpgn.start + prev_cmpgn.num_batches;

				while (prev_cmpgn.end > input_data.horizon && --prev_cmpgn.num_batches > 0) {
					--prev_cmpgn.end;
				}
			}
		}

		template<class Individual>
		void CreateUSPSchedule(
			Individual &individual,
			types::SingleSiteMultiSuiteSchedule &schedule
		)
		{
			schedule.Init(input_data.num_products, input_data.num_periods, input_data.num_usp_suites + input_data.num_dsp_suites, NUM_OBJECTIVES);

			for (int cmpgn_num = 0; cmpgn_num != individual.genes.size(); ++cmpgn_num) {
				if (
					individual.genes[cmpgn_num].product_num != individual.genes[cmpgn_num - 1].product_num && // different product
					individual.genes[cmpgn_num].usp_suite_num == individual.genes[cmpgn_num - 1].usp_suite_num // same suite
				) {
					AddNewUSPCampaign(cmpgn_num, individual, schedule);
				}
				else {
					ContinuePreviousUSPCampaign(cmpgn_num, individual, schedule);
				}
			}
		}

		template<class PriorityQueue>
		inline void AddFirstDSPCampaign(
			int dsp_suite,
			types::SingleSiteMultiSuiteSchedule &schedule,
			PriorityQueue &dsp_campaigns,
			types::Campaign &usp_cmpgn
		)
		{
			types::Campaign dsp_cmpgn;
			dsp_cmpgn.suite_num = dsp_suite;
			dsp_cmpgn.product_num = usp_cmpgn.product_num;

			double usp_batch_fill_date = usp_cmpgn.start + input_data.usp_days[dsp_cmpgn.product_num - 1];

			dsp_cmpgn.start = (input_data.dsp_lead_days[dsp_cmpgn.product_num - 1] > usp_batch_fill_date) ?
				input_data.dsp_lead_days[dsp_cmpgn.product_num - 1] : usp_batch_fill_date;

			dsp_cmpgn.end = dsp_cmpgn.start + input_data.dsp_days[dsp_cmpgn.product_num - 1];

			if (dsp_cmpgn.end > input_data.horizon) {
				return;
			}

			dsp_cmpgn.num_batches = usp_cmpgn.num_batches;

			types::Batch dsp_batch;
			dsp_batch.product_num = dsp_cmpgn.product_num;
			dsp_batch.stored_at = dsp_cmpgn.end;
			dsp_batch.expires_at = dsp_batch.stored_at + input_data.shelf_life[dsp_cmpgn.product_num - 1];
			
			dsp_cmpgn.batches.push_back(dsp_batch);
			AddToInventory(schedule, dsp_batch);

			for (int num_batches = 1; num_batches != usp_cmpgn.num_batches; ++num_batches) {
				usp_batch_fill_date += input_data.usp_days[dsp_cmpgn.product_num - 1];
				dsp_cmpgn.end = usp_batch_fill_date + input_data.dsp_days[dsp_cmpgn.product_num - 1];

				if (dsp_cmpgn.end > input_data.horizon) {
					dsp_cmpgn.end -= input_data.dsp_days[dsp_cmpgn.product_num - 1];
					dsp_cmpgn.num_batches = num_batches;
					break;
				}

				types::Batch dsp_batch;
				dsp_batch.product_num = dsp_cmpgn.product_num;
				dsp_batch.stored_at = dsp_cmpgn.end;
				dsp_batch.expires_at = dsp_batch.stored_at + input_data.shelf_life[dsp_cmpgn.product_num - 1];
				
				dsp_cmpgn.batches.push_back(dsp_batch);
				AddToInventory(schedule, dsp_batch);
			}

			dsp_campaigns.push(dsp_cmpgn);
			schedule.suites[dsp_suite].push_back(dsp_cmpgn);
		}

		template<class Individual>
		void CreateDSPSchedule(
			Individual &individual,
			types::SingleSiteMultiSuiteSchedule &schedule
		)
		{
			int dsp_suite = 1;
			bool over_horizon = false;

			// Priority queue for usp campaigns with the earliest start dates
			auto earlier_usp_cmpgn_start = [](const auto &a, const auto &b) { return a.start > b.start; };
			std::priority_queue<types::Campaign, std::vector<types::Campaign>, decltype(earlier_usp_cmpgn_start)> usp_campaigns(earlier_usp_cmpgn_start);

			for (const auto &suite : schedule.suites) {
				for (const auto &cmpgn : suite) {
					usp_campaigns.push(cmpgn);
				}
			}

			// Priority queue for dsp campaigns with the earliest end dates
			auto earlier_dsp_cmpgn_end = [](const auto &a, const auto &b) { return a.end > b.end; };
			std::priority_queue<types::Campaign, std::vector<types::Campaign>, decltype(earlier_dsp_cmpgn_end)> dsp_campaigns(earlier_dsp_cmpgn_end);

			for (; dsp_suite <= input_data.num_dsp_suites; ++dsp_suite) {
				types::Campaign dummy_dsp;
				dummy_dsp.suite_num = dsp_suite + input_data.num_usp_suites;
				dummy_dsp.end = 0;
				dsp_campaigns.push(dummy_dsp);
			}

			while (!usp_campaigns.empty()) {
				auto usp_cmpgn = usp_campaigns.top();
				usp_campaigns.pop();

				if (usp_cmpgn.product_num == 0) {
					continue;
				}

				dsp_suite = dsp_campaigns.top().suite_num;
				dsp_campaigns.pop();

				if (!schedule.suites[dsp_suite].size()) {
					AddFirstDSPCampaign(dsp_suite, schedule, dsp_campaigns, usp_cmpgn);
					continue;
				}

				types::Campaign dsp_cmpgn;
				types::Campaign &prev_dsp_cmpgn = schedule.suites[dsp_suite].back();
				
				dsp_cmpgn.suite_num = dsp_suite;
				dsp_cmpgn.product_num = usp_cmpgn.product_num;

				auto usp_batch_fill_date = usp_cmpgn.start + input_data.usp_days[dsp_cmpgn.product_num - 1];

				dsp_cmpgn.start = (prev_dsp_cmpgn.end + input_data.dsp_lead_days[dsp_cmpgn.product_num - 1] > usp_batch_fill_date) ?
					prev_dsp_cmpgn.end + input_data.dsp_lead_days[dsp_cmpgn.product_num - 1] : usp_batch_fill_date;

				dsp_cmpgn.end = dsp_cmpgn.start + input_data.dsp_days[dsp_cmpgn.product_num - 1];

				if (dsp_cmpgn.end > input_data.horizon) {
					continue;
				}

				// Production factor doesn't make sense from the biomanufacturing perspective:
				// For example, if for every 1 USP batch 2 DSP batches are produced, how is the DSP processing time affected?..
				dsp_cmpgn.num_batches = usp_cmpgn.num_batches; // input_data.production_factor[usp_cmpgn.product_num - 1];

				types::Batch dsp_batch;
				dsp_batch.product_num = dsp_cmpgn.product_num;
				dsp_batch.stored_at = dsp_cmpgn.end;
				dsp_batch.expires_at = dsp_batch.stored_at + input_data.shelf_life[dsp_cmpgn.product_num - 1];

				dsp_cmpgn.batches.push_back(dsp_batch);
				AddToInventory(schedule, dsp_batch);

				for (int num_batches = 1; num_batches != dsp_cmpgn.num_batches; ++num_batches) {
					usp_batch_fill_date += input_data.usp_days[dsp_cmpgn.product_num - 1];
					dsp_cmpgn.end = usp_batch_fill_date + input_data.dsp_days[dsp_cmpgn.product_num - 1];

					if (dsp_cmpgn.end > input_data.horizon) {
						dsp_cmpgn.end -= input_data.dsp_days[dsp_cmpgn.product_num - 1];
						dsp_cmpgn.num_batches = num_batches;
						break;
					}

					types::Batch dsp_batch;
					dsp_batch.product_num = dsp_cmpgn.product_num;
					dsp_batch.stored_at = dsp_cmpgn.end;
					dsp_batch.expires_at = dsp_batch.stored_at + input_data.shelf_life[dsp_cmpgn.product_num - 1];

					dsp_cmpgn.batches.push_back(dsp_batch);
				}

				dsp_campaigns.push(dsp_cmpgn);
				schedule.suites[dsp_suite].push_back(dsp_cmpgn);
			}
		}

		inline void RemoveExcess(types::SingleSiteMultiSuiteSchedule &schedule, int product_num, int period_num) 
		{
			int batches_over = schedule.inventory[product_num][period_num] - input_data.storage_cap[product_num];

			while (!schedule.batch_inventory[product_num][period_num].empty() && batches_over > input_data.storage_cap[product_num]) {
				++schedule.waste[product_num][period_num];
				schedule.batch_inventory[product_num][period_num].pop();
				--batches_over;
			}
		}

		inline void RemoveExpired(types::SingleSiteMultiSuiteSchedule &schedule, int product_num, int period_num)
		{
			// Keep popping batches out of the queue as long as their expiry date is < due date of the current time period_num
			while (
				!schedule.batch_inventory[product_num][period_num].empty() && 
				schedule.batch_inventory[product_num][period_num].top().expires_at < input_data.due_dates[period_num]
			) {
				++schedule.waste[product_num][period_num];
				schedule.batch_inventory[product_num][period_num].pop();
			}
		}

		void CheckSupplyDemandBacklogInventory(
			types::SingleSiteMultiSuiteSchedule &schedule, 
			int product_num, 
			int period_num
		)
		{
			int batches_available = schedule.batch_inventory[product_num][period_num].size();

			// No demand and backlog orders -> exit early
			if (period_num && !input_data.demand[product_num][period_num] && !schedule.backlog[product_num][period_num - 1]) {
				schedule.inventory[product_num][period_num] = batches_available;
				return;
			}

			// Check that there is indeed a demand for a given product
			if (input_data.demand[product_num][period_num]) {
				if (batches_available >= input_data.demand[product_num][period_num]) {
					schedule.supply[product_num][period_num] = input_data.demand[product_num][period_num];
					batches_available -= input_data.demand[product_num][period_num];
				}
				else {
					schedule.supply[product_num][period_num] = batches_available;
					schedule.backlog[product_num][period_num] = input_data.demand[product_num][period_num] - batches_available;
					batches_available = 0;

					if (period_num) {
						schedule.backlog[product_num][period_num] += schedule.backlog[product_num][period_num - 1];
					}	
				}
			}

			// Check if there are any backlog orders that can be filled
			if (period_num && schedule.backlog[product_num][period_num - 1] > 0 && batches_available) {
				if (batches_available >= schedule.backlog[product_num][period_num - 1]) {
					schedule.supply[product_num][period_num] += schedule.backlog[product_num][period_num - 1];
					batches_available -= schedule.backlog[product_num][period_num - 1];
				}
				else {
					schedule.supply[product_num][period_num] += batches_available;
					schedule.backlog[product_num][period_num] += schedule.backlog[product_num][period_num - 1];
				}
			}

			int batches_supplied = schedule.supply[product_num][period_num];

			// Adjust the batch inventory according to the supplied
			while (!schedule.batch_inventory[product_num][period_num].empty() && batches_supplied > 0) {
				schedule.batch_inventory[product_num][period_num].pop();
				--batches_supplied;
			}

			schedule.inventory[product_num][period_num] = batches_available;
		}

		void EvaluateCampaigns(
			types::SingleSiteMultiSuiteSchedule &schedule
		)
		{
			int product_num, period_num;
	
			for (product_num = 0; product_num < input_data.num_products; ++product_num) {

				period_num = 0;

				RemoveExpired(schedule, product_num, period_num);		
				CheckSupplyDemandBacklogInventory(schedule, product_num, period_num);
				RemoveExcess(schedule, product_num, period_num);
			
				for (period_num = 1; period_num < input_data.num_periods; ++period_num) {
					for (const auto &batch : utils::access_queue_container(schedule.batch_inventory[product_num][period_num - 1])) {
						schedule.batch_inventory[product_num][period_num].push(std::move(batch));
					}
					
					RemoveExpired(schedule, product_num, period_num);		
					CheckSupplyDemandBacklogInventory(schedule, product_num, period_num);
					RemoveExcess(schedule, product_num, period_num);
				}
			}

		}

		void CalculateObjectiveFunction(
			types::SingleSiteMultiSuiteSchedule &schedule
		)
		{
			for (int usp_suite = 0; usp_suite != input_data.num_usp_suites; ++usp_suite) {
				for (const auto &usp_cmpgn : schedule.suites[usp_suite]) {
					if (usp_cmpgn.product_num == 0) {
						continue;
					}

					schedule.objectives[TOTAL_CHANGEOVER_COSTS] += input_data.usp_changeover_cost[usp_cmpgn.product_num - 1];
					schedule.objectives[TOTAL_PRODUCTION_COST] += (usp_cmpgn.num_batches * input_data.usp_production_cost[usp_cmpgn.product_num - 1]);
				}
			}

			for (int dsp_suite = input_data.num_usp_suites; dsp_suite != schedule.suites.size(); ++dsp_suite) {
				for (const auto &dsp_cmpgn : schedule.suites[dsp_suite]) {
					if (dsp_cmpgn.product_num == 0) {
						continue;
					}

					schedule.objectives[TOTAL_CHANGEOVER_COSTS] += input_data.dsp_changeover_cost[dsp_cmpgn.product_num - 1];
					schedule.objectives[TOTAL_PRODUCTION_COST] += (dsp_cmpgn.num_batches * input_data.dsp_production_cost[dsp_cmpgn.product_num - 1]);
				}
			}

			for (int product_num = 0; product_num != input_data.num_products; ++product_num) {
				for (int period_num = 0; period_num != input_data.num_periods; ++period_num) {
					schedule.objectives[TOTAL_STORAGE_COST] += schedule.inventory[product_num][period_num] * input_data.storage_cost[product_num];
					schedule.objectives[TOTAL_BACKLOG_PENALTY] += schedule.backlog[product_num][period_num] * input_data.backlog_penalty[product_num];
					schedule.objectives[TOTAL_WASTE_COST] += schedule.waste[product_num][period_num] * input_data.waste_disposal_cost[product_num];
					schedule.objectives[TOTAL_REVENUE] += schedule.supply[product_num][period_num] * input_data.sales_price[product_num];
				}
			}
			schedule.objectives[TOTAL_COST] = (
				schedule.objectives[TOTAL_STORAGE_COST] + 
				schedule.objectives[TOTAL_BACKLOG_PENALTY] + 
				schedule.objectives[TOTAL_WASTE_COST] + 
				schedule.objectives[TOTAL_CHANGEOVER_COSTS] + 
				schedule.objectives[TOTAL_PRODUCTION_COST]
			);

			schedule.objectives[TOTAL_PROFIT] = schedule.objectives[TOTAL_REVENUE] - schedule.objectives[TOTAL_WASTE_COST];
		}

	public:
		SingleSiteMultiSuiteModel(const SingleSiteMultiSuiteInputData &input_data) : input_data(input_data) {}

		template<class Individual>
		void CreateSchedule(
			Individual &individual,
			types::SingleSiteMultiSuiteSchedule &schedule
		)
		{
			CreateUSPSchedule(individual, schedule);
			CreateDSPSchedule(individual, schedule);
			EvaluateCampaigns(schedule);
			CalculateObjectiveFunction(schedule);
		}

		void operator()(types::SingleObjectiveIndividual &individual)
		{
			types::SingleSiteMultiSuiteSchedule schedule;
			CreateSchedule(individual, schedule);
		}
	};
}

#endif 
