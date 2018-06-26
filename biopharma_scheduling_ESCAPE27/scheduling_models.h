#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
    #pragma GCC diagnostic ignored "-Wreorder"
	#pragma GCC diagnostic ignored "-Wsign-compare"
	#pragma GCC diagnostic ignored "-Wunused-variable"
	#pragma GCC diagnostic ignored "-Wformat="
#endif 

#ifndef __SCHEDULING_MODELS_H__
#define __SCHEDULING_MODELS_H__

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

#include "schedule.h"
#include "input_data.h"
#include "nsga_individual.h"
#include "single_objective_individual.h"


namespace deterministic
{
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
			// int period_num = utils::search(input_data.due_dates, new_batch.stored_at); 

			int period_num = -1;

			for (int t = 0; t != input_data.due_dates.size(); ++t) {
				if (new_batch.stored_at <= input_data.due_dates[t]) {
					period_num = t;
					break;
				}
			}

			if (period_num != -1) {
				schedule.inventory[new_batch.product_num - 1][period_num].push(new_batch);
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

			new_cmpgn.suite_num = gene.usp_suite_num;
			new_cmpgn.product_num = gene.product_num;
			new_cmpgn.num_batches = gene.num_batches;

			if (schedule.suites[new_cmpgn.suite_num - 1].size()) {
				types::Campaign &prev_cmpgn = schedule.suites[new_cmpgn.suite_num - 1].back();
				new_cmpgn.start = prev_cmpgn.end + input_data.usp_lead_days[new_cmpgn.product_num - 1];
			}
			// First campaign in this suite
			else {
				new_cmpgn.start = input_data.usp_lead_days[new_cmpgn.product_num - 1];
			}

			// Real campaign
			if (new_cmpgn.product_num != 0) { 
				new_cmpgn.end = new_cmpgn.start + input_data.usp_days[new_cmpgn.product_num - 1] * new_cmpgn.num_batches;

				while (new_cmpgn.end > input_data.horizon && new_cmpgn.num_batches > 0) {
					new_cmpgn.end -= input_data.usp_days[new_cmpgn.product_num - 1];
					--new_cmpgn.num_batches;
				}
			}
			// Dummy one
			else {
				new_cmpgn.end = new_cmpgn.start + new_cmpgn.num_batches;

				while (new_cmpgn.end > input_data.horizon && new_cmpgn.num_batches > 0) {
					--new_cmpgn.end;
					--new_cmpgn.num_batches;				
				}
			}

			gene.num_batches = new_cmpgn.num_batches;	

			if (new_cmpgn.num_batches) {
				schedule.suites[new_cmpgn.suite_num - 1].push_back(new_cmpgn);
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
			types::Campaign &prev_cmpgn = schedule.suites[gene.usp_suite_num - 1].back();

			prev_cmpgn.num_batches += gene.num_batches;

			if (prev_cmpgn.product_num != 0) {
				prev_cmpgn.end = prev_cmpgn.start + input_data.usp_days[prev_cmpgn.product_num - 1] * prev_cmpgn.num_batches;

				while (prev_cmpgn.end > input_data.horizon && prev_cmpgn.num_batches > 0) {
					prev_cmpgn.end -= input_data.usp_days[prev_cmpgn.product_num - 1];
					--prev_cmpgn.num_batches;
				}
			}
			else {
				prev_cmpgn.end = prev_cmpgn.start + prev_cmpgn.num_batches;

				while (prev_cmpgn.end > input_data.horizon && prev_cmpgn.num_batches > 0) {
					--prev_cmpgn.end;
					--prev_cmpgn.num_batches;
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

			int cmpgn_num = 0;

			AddNewUSPCampaign(cmpgn_num, individual, schedule);

			for (cmpgn_num = 1; cmpgn_num != individual.genes.size(); ++cmpgn_num) {
				if (
					individual.genes[cmpgn_num].product_num == individual.genes[cmpgn_num - 1].product_num && // different product
					individual.genes[cmpgn_num].usp_suite_num == individual.genes[cmpgn_num - 1].usp_suite_num && // same suite
					schedule.suites[individual.genes[cmpgn_num].usp_suite_num - 1].size()
				) {
					ContinuePreviousUSPCampaign(cmpgn_num, individual, schedule);
				}
				else {
					AddNewUSPCampaign(cmpgn_num, individual, schedule);
				}
			}

			// Ensure usp_schedule == chromosome
			auto earlier_usp_cmpgn_start = [](const auto &a, const auto &b) { return a.start > b.start; };
			std::priority_queue<types::Campaign, std::vector<types::Campaign>, decltype(earlier_usp_cmpgn_start)> usp_campaigns(earlier_usp_cmpgn_start);

			for (const auto &suite : schedule.suites) {
				for (const auto &cmpgn : suite) {
					usp_campaigns.push(cmpgn);
				}
			}

			cmpgn_num = 0;
			individual.genes.resize(usp_campaigns.size());

			while (!usp_campaigns.empty()) {
				individual.genes[cmpgn_num].usp_suite_num = usp_campaigns.top().suite_num;
				individual.genes[cmpgn_num].product_num = usp_campaigns.top().product_num;
				individual.genes[cmpgn_num].num_batches = usp_campaigns.top().num_batches;
				usp_campaigns.pop();
				++cmpgn_num;
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
			schedule.suites[dsp_suite - 1].push_back(dsp_cmpgn);
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

				if (!dsp_campaigns.empty()) {
					dsp_campaigns.pop();
				}

				if (!schedule.suites[dsp_suite - 1].size()) {
					AddFirstDSPCampaign(dsp_suite, schedule, dsp_campaigns, usp_cmpgn);
					continue;
				}

				types::Campaign dsp_cmpgn;
				types::Campaign &prev_dsp_cmpgn = schedule.suites[dsp_suite - 1].back();
				
				dsp_cmpgn.suite_num = dsp_suite;
				dsp_cmpgn.product_num = usp_cmpgn.product_num;

				auto usp_batch_fill_date = usp_cmpgn.start + input_data.usp_days[dsp_cmpgn.product_num - 1];

				dsp_cmpgn.start = (prev_dsp_cmpgn.end + input_data.dsp_lead_days[dsp_cmpgn.product_num - 1] > usp_batch_fill_date) ?
					prev_dsp_cmpgn.end + input_data.dsp_lead_days[dsp_cmpgn.product_num - 1] : usp_batch_fill_date;

				dsp_cmpgn.end = dsp_cmpgn.start + input_data.dsp_days[dsp_cmpgn.product_num - 1];

				if (dsp_cmpgn.end > input_data.horizon) {
					continue;
				}

				// TODO
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
					AddToInventory(schedule, dsp_batch);
				}

				dsp_campaigns.push(dsp_cmpgn);
				schedule.suites[dsp_suite - 1].push_back(dsp_cmpgn);
			}
		}

		inline void RemoveExcess(types::SingleSiteMultiSuiteSchedule &schedule, int product_num, int period_num) 
		{
			int batches_over = schedule.batch_inventory[product_num][period_num] - input_data.storage_cap[product_num];

			while (!schedule.inventory[product_num][period_num].empty() && batches_over > input_data.storage_cap[product_num]) {
				++schedule.batch_waste[product_num][period_num];
				schedule.inventory[product_num][period_num].pop();
				--batches_over;
			}
		}

		inline void RemoveExpired(types::SingleSiteMultiSuiteSchedule &schedule, int product_num, int period_num)
		{
			// Keep popping batches out of the queue as long as their expiry date is < due date of the current time period_num
			while (
				!schedule.inventory[product_num][period_num].empty() && 
				schedule.inventory[product_num][period_num].top().expires_at < input_data.due_dates[period_num]
			) {
				++schedule.batch_waste[product_num][period_num];
				schedule.inventory[product_num][period_num].pop();
			}
		}

		void CheckSupplyDemandBacklogInventory(
			types::SingleSiteMultiSuiteSchedule &schedule, 
			int product_num, 
			int period_num
		)
		{
			int batches_available = schedule.inventory[product_num][period_num].size();

			// No demand and backlog orders -> exit early
			if (period_num && !input_data.demand[product_num][period_num] && !schedule.batch_backlog[product_num][period_num - 1]) {
				schedule.batch_inventory[product_num][period_num] = batches_available;
				return;
			}

			// Check that there is indeed a demand for a given product
			if (input_data.demand[product_num][period_num]) {
				if (batches_available >= input_data.demand[product_num][period_num]) {
					schedule.batch_supply[product_num][period_num] = input_data.demand[product_num][period_num];
					batches_available -= input_data.demand[product_num][period_num];
				}
				else {
					schedule.batch_supply[product_num][period_num] = batches_available;
					schedule.batch_backlog[product_num][period_num] = input_data.demand[product_num][period_num] - batches_available;
					batches_available = 0;

					if (period_num) {
						schedule.batch_backlog[product_num][period_num] += schedule.batch_backlog[product_num][period_num - 1];
					}	
				}
			}

			// Check if there are any backlog orders that can be filled
			if (period_num && schedule.batch_backlog[product_num][period_num - 1] > 0 && batches_available) {
				if (batches_available >= schedule.batch_backlog[product_num][period_num - 1]) {
					schedule.batch_backlog[product_num][period_num] += schedule.batch_backlog[product_num][period_num - 1];
					batches_available -= schedule.batch_backlog[product_num][period_num - 1];
				}
				else {
					schedule.batch_supply[product_num][period_num] += batches_available;
					schedule.batch_backlog[product_num][period_num] += schedule.batch_backlog[product_num][period_num - 1];
				}
			}

			int batches_supplied = schedule.batch_supply[product_num][period_num];

			// Adjust the batch inventory according to the supplied
			while (!schedule.inventory[product_num][period_num].empty() && batches_supplied > 0) {
				schedule.inventory[product_num][period_num].pop();
				--batches_supplied;
			}

			schedule.batch_inventory[product_num][period_num] = batches_available;
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
					for (const auto &batch : utils::access_queue_container(schedule.inventory[product_num][period_num - 1])) {
						schedule.inventory[product_num][period_num].push(std::move(batch));
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

					schedule.objectives[TOTAL_CHANGEOVER_COST] += input_data.usp_changeover_cost[usp_cmpgn.product_num - 1];
					schedule.objectives[TOTAL_PRODUCTION_COST] += (usp_cmpgn.num_batches * input_data.usp_production_cost[usp_cmpgn.product_num - 1]);
				}
			}

			for (int dsp_suite = input_data.num_usp_suites; dsp_suite < schedule.suites.size(); ++dsp_suite) {
				for (const auto &dsp_cmpgn : schedule.suites[dsp_suite]) {
					if (dsp_cmpgn.product_num == 0) {
						continue;
					}

					schedule.objectives[TOTAL_CHANGEOVER_COST] += input_data.dsp_changeover_cost[dsp_cmpgn.product_num - 1];
					schedule.objectives[TOTAL_PRODUCTION_COST] += (dsp_cmpgn.num_batches * input_data.dsp_production_cost[dsp_cmpgn.product_num - 1]);
				}
			}

			for (int product_num = 0; product_num != input_data.num_products; ++product_num) {
				for (int period_num = 0; period_num != input_data.num_periods; ++period_num) {
					schedule.objectives[TOTAL_STORAGE_COST] += schedule.batch_inventory[product_num][period_num] * input_data.storage_cost[product_num];
					schedule.objectives[TOTAL_BACKLOG_PENALTY] += schedule.batch_backlog[product_num][period_num] * input_data.backlog_penalty[product_num];
					schedule.objectives[TOTAL_WASTE_COST] += schedule.batch_waste[product_num][period_num] * input_data.waste_disposal_cost[product_num];
					schedule.objectives[TOTAL_REVENUE] += schedule.batch_supply[product_num][period_num] * input_data.sales_price[product_num];
				}
			}
			schedule.objectives[TOTAL_COST] = (
				schedule.objectives[TOTAL_STORAGE_COST] + 
				schedule.objectives[TOTAL_BACKLOG_PENALTY] + 
				schedule.objectives[TOTAL_WASTE_COST] + 
				schedule.objectives[TOTAL_CHANGEOVER_COST] + 
				schedule.objectives[TOTAL_PRODUCTION_COST]
			);

			schedule.objectives[TOTAL_PROFIT] = schedule.objectives[TOTAL_REVENUE] - schedule.objectives[TOTAL_COST];
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

		void operator()(types::SingleObjectiveIndividual<types::SingleSiteMultiSuiteGene> &individual)
		{
			types::SingleSiteMultiSuiteSchedule schedule;
			CreateSchedule(individual, schedule);			
			
			for (const auto &it : input_data.objectives) {
				individual.objective = schedule.objectives[it.first] * it.second * -1;
				break;
			}

			// The smaller the constraint value the better
			individual.constraints = 0.0;

			if (!input_data.constraints.empty()) {
				for (auto &it : input_data.constraints) {
					// <= bound
					if (it.second.first == -1 && schedule.objectives[it.first] > it.second.second) {
						individual.constraints += std::fabs(schedule.objectives[it.first] - it.second.second);
					}
					// >= bound
					else if (it.second.first == 1 && schedule.objectives[it.first] < it.second.second) {
						individual.constraints += std::fabs(schedule.objectives[it.first] - it.second.second);
					}
				}
			}
		}
		
		void operator()(types::NSGAIndividual<types::SingleSiteMultiSuiteGene> &individual)
		{
			types::SingleSiteMultiSuiteSchedule schedule;
			CreateSchedule(individual, schedule);		

			individual.objectives.resize(0);

			for (auto &it : input_data.objectives) {
				individual.objectives.push_back(schedule.objectives[it.first] * it.second * -1);
			}

			// The smaller the constraint value the better
			individual.constraints = 0.0;
			
			if (!input_data.constraints.empty()) {
				for (auto &it : input_data.constraints) {
					// <= bound
					if (it.second.first == -1 && schedule.objectives[it.first] > it.second.second) {
						individual.constraints += std::fabs(schedule.objectives[it.first] - it.second.second);
					}
					// >= bound
					else if (it.second.first == 1 && schedule.objectives[it.first] < it.second.second) {
						individual.constraints += std::fabs(schedule.objectives[it.first] - it.second.second);
					}
				}
			}
		}
	};


	class SingleSiteSimpleModel
	{
		SingleSiteSimpleInputData input_data;

		/*
			Adds a batch to an inventory priority queue (oldest first) within an appropriate time
			bucket based on the approval date of the said batch.
		*/
		inline void AddToInventory(types::SingleSiteSimpleSchedule &schedule, types::Batch &&new_batch)
		{
			// Range based binary search for a time period to fit the batch in 
			// based on its approval date
			int period_num = utils::search(input_data.due_dates, new_batch.approved_at);

			if (period_num != -1) {
				schedule.inventory[new_batch.product_num - 1][period_num].push(std::move(new_batch));
			}
		}

		template<class Individual>
		inline bool IsOverHorizon(
			int cmpgn_num,
			Individual &individual, 
			types::Schedule &schedule,
			types::Campaign &new_cmpgn,
			types::Batch &new_batch,
			types::Batch &prev_batch
		)
		{
			if (new_batch.stored_at >= input_data.horizon) {
				new_cmpgn.num_batches = new_cmpgn.batches.size();					
				new_cmpgn.last_batch = prev_batch.stored_at;
				individual.genes[cmpgn_num].num_batches = new_cmpgn.num_batches;
				schedule.campaigns.push_back(std::move(new_cmpgn));
				return true; 
			}

			return false;
		}

		/*
			Adds the first campaign to the schedule. Returns false if the schedule 
			is at/over the horizon, true otherwise.
		*/
		template<class Individual>
		bool AddFirstCampaign(
			Individual &individual,
			types::Schedule &schedule
		) 
		{
			types::Campaign new_cmpgn;
			new_cmpgn.product_num = individual.genes[0].product_num;
			new_cmpgn.start = 0;
			new_cmpgn.first_harvest = new_cmpgn.start + input_data.usp_days[new_cmpgn.product_num - 1];
			new_cmpgn.first_batch = new_cmpgn.first_harvest + input_data.dsp_days[new_cmpgn.product_num - 1];

			if (new_cmpgn.first_batch  >= input_data.horizon) {
				return false; 
			}

			// First actual batch object of the current campaign
			types::Batch new_batch;
			new_batch.product_num = new_cmpgn.product_num;
			new_batch.kg = input_data.kg_yield_per_batch[new_cmpgn.product_num - 1];
			new_batch.start = new_cmpgn.start;
			new_batch.harvested_at = new_cmpgn.first_harvest;
			new_batch.stored_at = new_cmpgn.first_batch;
			new_batch.approved_at = new_batch.stored_at + input_data.approval_days[new_cmpgn.product_num - 1];
			new_batch.expires_at = new_batch.stored_at + input_data.shelf_life_days[new_cmpgn.product_num - 1];
			
			new_cmpgn.kg += new_batch.kg;
			new_cmpgn.batches.reserve(100); 
			new_cmpgn.batches.push_back(new_batch); 
			AddToInventory(schedule, std::move(new_batch));

			int num_batches = individual.genes[0].num_batches;		

			if (num_batches < input_data.min_batches_per_campaign[new_cmpgn.product_num - 1]) {
				num_batches = input_data.min_batches_per_campaign[new_cmpgn.product_num - 1];
			}

			while (num_batches % input_data.batches_multiples_of_per_campaign[new_cmpgn.product_num - 1] != 0) {
				++num_batches;
			}	

			if (num_batches > input_data.max_batches_per_campaign[new_cmpgn.product_num - 1]) {
				num_batches = input_data.max_batches_per_campaign[new_cmpgn.product_num - 1];

				while (num_batches % input_data.batches_multiples_of_per_campaign[new_cmpgn.product_num - 1] != 0) {
					--num_batches;
				}	
			}

			// Remaining batches of the first campaign
			for (int i = 1; i < num_batches; ++i) {
				types::Batch new_batch, &prev_batch = new_cmpgn.batches.back();
				new_batch.product_num = new_cmpgn.product_num;
				new_batch.kg = input_data.kg_yield_per_batch[new_cmpgn.product_num - 1];
				new_batch.harvested_at = prev_batch.stored_at;
				new_batch.start = new_batch.harvested_at - input_data.usp_days[new_cmpgn.product_num - 1];
				new_batch.stored_at = new_batch.harvested_at + input_data.dsp_days[new_cmpgn.product_num - 1];
				
				if (IsOverHorizon(0, individual, schedule, new_cmpgn, new_batch, prev_batch)) {
					return false;
				}

				new_batch.approved_at = new_batch.stored_at + input_data.approval_days[new_cmpgn.product_num - 1];
				new_batch.expires_at = new_batch.stored_at + input_data.shelf_life_days[new_cmpgn.product_num - 1];
				
				new_cmpgn.kg += new_batch.kg;
				new_cmpgn.batches.push_back(new_batch);
				AddToInventory(schedule, std::move(new_batch));
			}

			new_cmpgn.num_batches = new_cmpgn.batches.size();
			individual.genes[0].num_batches = new_cmpgn.num_batches;
			new_cmpgn.last_batch = new_cmpgn.batches.back().stored_at;
			schedule.campaigns.reserve(100); 
			schedule.campaigns.push_back(std::move(new_cmpgn));
			return true;
		}

		/*
			Adds a new manufacturing campaign of a different product. Returns false if 
			the schedule is at/over the horizon, true otherwise.
		*/
		template<class Individual>
		bool AddNewCampaign(
			int cmpgn_num,
			Individual &individual,
			types::Schedule &schedule
		)
		{		
			types::Campaign new_cmpgn, &prev_cmpgn = schedule.campaigns.back();
			new_cmpgn.product_num = individual.genes[cmpgn_num].product_num;
			new_cmpgn.first_harvest = prev_cmpgn.last_batch + input_data.changeover_days[prev_cmpgn.product_num - 1][new_cmpgn.product_num - 1];
			new_cmpgn.first_batch = new_cmpgn.first_harvest + input_data.dsp_days[new_cmpgn.product_num - 1];
			new_cmpgn.start = new_cmpgn.first_harvest - input_data.usp_days[new_cmpgn.product_num - 1];
			
			if (new_cmpgn.first_batch >= input_data.horizon) {
				return false; 
			}

			// First batch of the current campaign
			types::Batch new_batch;
			new_batch.product_num = new_cmpgn.product_num;
			new_batch.kg = input_data.kg_yield_per_batch[new_cmpgn.product_num - 1];
			new_batch.start = new_cmpgn.start;
			new_batch.harvested_at = new_cmpgn.first_harvest;
			new_batch.stored_at = new_cmpgn.first_batch;
			new_batch.approved_at = new_batch.stored_at + input_data.approval_days[new_cmpgn.product_num - 1];
			new_batch.expires_at = new_batch.stored_at + input_data.shelf_life_days[new_cmpgn.product_num - 1];
			
			new_cmpgn.kg += new_batch.kg;
			new_cmpgn.batches.reserve(100);
			new_cmpgn.batches.push_back(new_batch);
			AddToInventory(schedule, std::move(new_batch));

			int num_batches = individual.genes[cmpgn_num].num_batches;		

			if (num_batches < input_data.min_batches_per_campaign[new_cmpgn.product_num - 1]) {
				num_batches = input_data.min_batches_per_campaign[new_cmpgn.product_num - 1];
			}

			while (num_batches % input_data.batches_multiples_of_per_campaign[new_cmpgn.product_num - 1] != 0) {
				++num_batches;
			}	

			if (num_batches > input_data.max_batches_per_campaign[new_cmpgn.product_num - 1]) {
				num_batches = input_data.max_batches_per_campaign[new_cmpgn.product_num - 1];

				while (num_batches % input_data.batches_multiples_of_per_campaign[new_cmpgn.product_num - 1] != 0) {
					--num_batches;
				}	
			}

			// Remaining batches of the campaign
			for (int i = 1; i < num_batches; ++i) {
				types::Batch new_batch, &prev_batch = new_cmpgn.batches.back();
				new_batch.product_num = new_cmpgn.product_num;
				new_batch.kg = input_data.kg_yield_per_batch[new_cmpgn.product_num - 1];
				new_batch.harvested_at = prev_batch.stored_at;
				new_batch.start = new_batch.harvested_at - input_data.usp_days[new_cmpgn.product_num - 1];
				new_batch.stored_at = new_batch.harvested_at + input_data.dsp_days[new_cmpgn.product_num - 1];
				
				if (IsOverHorizon(cmpgn_num, individual, schedule, new_cmpgn, new_batch, prev_batch)) {
					return false;
				}
	
				new_batch.approved_at = new_batch.stored_at + input_data.approval_days[new_cmpgn.product_num - 1];
				new_batch.expires_at = new_batch.stored_at + input_data.shelf_life_days[new_cmpgn.product_num - 1];
				
				new_cmpgn.kg += new_batch.kg;
				new_cmpgn.batches.push_back(new_batch);
				AddToInventory(schedule, std::move(new_batch));
			}

			new_cmpgn.num_batches = new_cmpgn.batches.size();
			individual.genes[cmpgn_num].num_batches = new_cmpgn.num_batches;
			new_cmpgn.last_batch = new_cmpgn.batches.back().stored_at;
			schedule.campaigns.reserve(100);
			schedule.campaigns.push_back(std::move(new_cmpgn));
			return true;
		}

		template<class Individual>
		bool ContinuePreviousCampaign(
			int cmpgn_num,
			Individual &individual,
			types::Schedule &schedule
		)
		{
			types::Campaign &prev_cmpgn = schedule.campaigns.back();

			int i = 0, num_batches = individual.genes[cmpgn_num].num_batches;

			if ((prev_cmpgn.num_batches + num_batches) > input_data.max_batches_per_campaign[prev_cmpgn.product_num - 1]) {
				num_batches = input_data.max_batches_per_campaign[prev_cmpgn.product_num - 1] - prev_cmpgn.num_batches;
			}

			while ((prev_cmpgn.num_batches + num_batches) % input_data.batches_multiples_of_per_campaign[prev_cmpgn.product_num - 1] != 0) {
				--num_batches;
			}	

			for (; i < num_batches; ++i) {
				types::Batch new_batch, &prev_batch = prev_cmpgn.batches.back();
				new_batch.product_num = prev_cmpgn.product_num;
				new_batch.kg = input_data.kg_yield_per_batch[prev_cmpgn.product_num - 1];
				new_batch.harvested_at = prev_batch.stored_at;
				new_batch.start = new_batch.harvested_at - input_data.usp_days[prev_cmpgn.product_num - 1];
				new_batch.stored_at = new_batch.harvested_at + input_data.dsp_days[prev_cmpgn.product_num - 1];
				
				if (new_batch.stored_at >= input_data.horizon) {
					prev_cmpgn.num_batches = prev_cmpgn.batches.size();
					prev_cmpgn.last_batch = prev_batch.stored_at;
					individual.genes[cmpgn_num].num_batches = i;
					return false;
				}

				new_batch.approved_at = new_batch.stored_at + input_data.approval_days[prev_cmpgn.product_num - 1];
				new_batch.expires_at = new_batch.stored_at + input_data.shelf_life_days[prev_cmpgn.product_num - 1];
				
				prev_cmpgn.kg += new_batch.kg;
				prev_cmpgn.batches.push_back(new_batch);
				AddToInventory(schedule, std::move(new_batch));
			}

			prev_cmpgn.num_batches = prev_cmpgn.batches.size();
			individual.genes[cmpgn_num].num_batches = i;
			prev_cmpgn.last_batch = prev_cmpgn.batches.back().stored_at;
			return true;
		}

		inline void CreateOpeningStock(types::SingleSiteSimpleSchedule &schedule, int product_num, int period_num)
		{
			if (input_data.kg_opening_stock[product_num] > 0) {
				types::Batch opening_stock;
				opening_stock.kg = input_data.kg_opening_stock[product_num];
				opening_stock.harvested_at = -1;
				opening_stock.stored_at = 0;
				opening_stock.approved_at = 0;
				opening_stock.expires_at = input_data.shelf_life_days[product_num];
				schedule.inventory[product_num][0].push(std::move(opening_stock));
			}
		}
		
		inline void RemoveExcess(types::SingleSiteSimpleSchedule &schedule, int product_num, int period_num) 
		{
			double kg_over = schedule.kg_inventory[product_num][period_num] - input_data.kg_storage_limits[product_num];

			while (!schedule.inventory[product_num][period_num].empty() && kg_over > input_data.kg_storage_limits[product_num]) {
				if (kg_over >= schedule.inventory[product_num][period_num].top().kg) {
					schedule.kg_waste[product_num][period_num] += schedule.inventory[product_num][period_num].top().kg;
					schedule.objectives[TOTAL_KG_WASTE] += schedule.inventory[product_num][period_num].top().kg;
					schedule.objectives[TOTAL_WASTE_COST] += schedule.inventory[product_num][period_num].top().kg * input_data.waste_cost_per_kg[product_num];
					kg_over -= schedule.inventory[product_num][period_num].top().kg;
					schedule.inventory[product_num][period_num].pop();

					if (kg_over < utils::EPSILON) {
						kg_over = 0;
					}
				}
				else {
					schedule.kg_waste[product_num][period_num] += kg_over;
					schedule.objectives[TOTAL_KG_WASTE] += kg_over;
					schedule.objectives[TOTAL_WASTE_COST] += kg_over * input_data.waste_cost_per_kg[product_num];
					utils::access_queue_container(schedule.inventory[product_num][period_num])[0].kg -= kg_over;
					kg_over = 0;
				}
			}
		}

		inline void RemoveExpired(types::SingleSiteSimpleSchedule &schedule, int product_num, int period_num)
		{
			// Keep popping batches out of the queue as long as their expiry date is < due date of the current time period
			while (
				!schedule.inventory[product_num][period_num].empty() && 
				schedule.inventory[product_num][period_num].top().expires_at < input_data.due_dates[period_num]
			) {
				schedule.kg_waste[product_num][period_num] += schedule.inventory[product_num][period_num].top().kg;
				schedule.objectives[TOTAL_KG_WASTE] += schedule.inventory[product_num][period_num].top().kg;
				schedule.objectives[TOTAL_WASTE_COST] += schedule.inventory[product_num][period_num].top().kg * input_data.waste_cost_per_kg[product_num];
				schedule.inventory[product_num][period_num].pop();
			}
		}

		static inline double GetKgAvailable(types::SingleSiteSimpleSchedule &schedule, int product_num, int period_num)
		{
			// Access the queue by reference (special hack)
			auto &inventory = utils::access_queue_container(schedule.inventory[product_num][period_num]);

			if (!inventory.size()) {
				return 0;
			}

			return std::accumulate(
				inventory.cbegin(), 
				inventory.cend(), 
				0.0,
				[](double kg, const types::Batch &b){ return kg + b.kg; }
			);
		}

		inline void CheckSupplyDemandBacklogInventory(types::SingleSiteSimpleSchedule &schedule, int product_num, int period_num) 
		{
			double kg_available = GetKgAvailable(schedule, product_num, period_num);

			// No demand and backlog orders -> exit early
			if (period_num && !input_data.kg_demand[product_num][period_num] && !schedule.kg_backlog[product_num][period_num - 1]) {
				schedule.kg_inventory[product_num][period_num] = kg_available;
				return;
			}

			// Check that there is indeed a demand for a given product
			if (input_data.kg_demand[product_num][period_num]) {
				if (kg_available >= input_data.kg_demand[product_num][period_num]) {
					schedule.kg_supply[product_num][period_num] = input_data.kg_demand[product_num][period_num];
					kg_available -= input_data.kg_demand[product_num][period_num];
				}
				else {
					schedule.kg_supply[product_num][period_num] = kg_available;
					schedule.kg_backlog[product_num][period_num] = input_data.kg_demand[product_num][period_num] - kg_available;
					schedule.objectives[TOTAL_KG_BACKLOG] += schedule.kg_backlog[product_num][period_num];
					kg_available = 0;

					if (period_num) {
						schedule.kg_backlog[product_num][period_num] += schedule.kg_backlog[product_num][period_num - 1];
					}	
				}
			}

			// Check if there are any backlog orders that can be filled
			if (period_num && schedule.kg_backlog[product_num][period_num - 1] > 0 && kg_available) {
				if (kg_available >= schedule.kg_backlog[product_num][period_num - 1]) {
					schedule.kg_supply[product_num][period_num] += schedule.kg_backlog[product_num][period_num - 1];
					kg_available -= schedule.kg_backlog[product_num][period_num - 1];
				}
				else {
					schedule.kg_supply[product_num][period_num] += kg_available;
					schedule.kg_backlog[product_num][period_num] += schedule.kg_backlog[product_num][period_num - 1];
				}
			}

			double kg_supplied = schedule.kg_supply[product_num][period_num];

			// Adjust the batch inventory according to the kg_supplied
			while (!schedule.inventory[product_num][period_num].empty() && kg_supplied > 0) {
				if (kg_supplied >= schedule.inventory[product_num][period_num].top().kg) {
					kg_supplied -= schedule.inventory[product_num][period_num].top().kg;
					schedule.inventory[product_num][period_num].pop();

					if (kg_supplied < utils::EPSILON) {
						kg_supplied = 0;
					}
				}
				else {
					// Access the top of the queue by reference
					utils::access_queue_container(schedule.inventory[product_num][period_num])[0].kg -= kg_supplied;
					kg_supplied = 0;
				}
			}

			schedule.objectives[TOTAL_BACKLOG_PENALTY] += schedule.kg_backlog[product_num][period_num] * input_data.backlog_penalty_per_kg[product_num];
			schedule.objectives[TOTAL_KG_SUPPLY] += schedule.kg_supply[product_num][period_num];
			schedule.objectives[TOTAL_REVENUE] += schedule.kg_supply[product_num][period_num] * input_data.sell_price_per_kg[product_num];			
			schedule.kg_inventory[product_num][period_num] = kg_available;
		}

		inline void CheckInventoryTarget(types::SingleSiteSimpleSchedule &schedule, int product_num, int period_num)
		{
			if (input_data.kg_inventory_target) {
				double kg_inventory_target_ = (*input_data.kg_inventory_target)[product_num][period_num];

				if (schedule.kg_inventory[product_num][period_num] < kg_inventory_target_) {
					schedule.objectives[TOTAL_KG_INVENTORY_DEFICIT] += kg_inventory_target_ - schedule.kg_inventory[product_num][period_num];
					schedule.objectives[TOTAL_INVENTORY_PENALTY] += (kg_inventory_target_ - schedule.kg_inventory[product_num][period_num]) * input_data.inventory_penalty_per_kg[product_num];
				}
			}
		}

		/*
			Builds inventory, supply, backlog, and waste graphs and evaluates them.
		*/
		void EvaluateCampaigns(types::SingleSiteSimpleSchedule &schedule) 
		{		
			int product_num, period_num;
	
			for (product_num = 0; product_num < input_data.num_products; ++product_num) {

				period_num = 0;

				CreateOpeningStock(schedule, product_num, period_num);
				RemoveExpired(schedule, product_num, period_num);		
				CheckSupplyDemandBacklogInventory(schedule, product_num, period_num);
				RemoveExcess(schedule, product_num, period_num);
				CheckInventoryTarget(schedule, product_num, period_num);
			
				for (period_num = 1; period_num < input_data.num_periods; ++period_num) {
					// Add batches from the previous time period to the current one
					for (auto &batch : utils::access_queue_container(schedule.inventory[product_num][period_num - 1])) {
						schedule.inventory[product_num][period_num].push(std::move(batch));
					}
					
					RemoveExpired(schedule, product_num, period_num);		
					CheckSupplyDemandBacklogInventory(schedule, product_num, period_num);
					RemoveExcess(schedule, product_num, period_num);
					CheckInventoryTarget(schedule, product_num, period_num);
				}
			}
		} 

	public:
		SingleSiteSimpleModel() {}
		SingleSiteSimpleModel(SingleSiteSimpleInputData input_data) : input_data(input_data) {}

		template<class Individual>
		void CreateSchedule(
			Individual &individual,
			types::SingleSiteSimpleSchedule &schedule
		)
		{
			size_t cmpgn_num = 0;
			schedule.Init(input_data.num_products, input_data.num_periods, NUM_OBJECTIVES);

			if (AddFirstCampaign(individual, schedule)) {
				// Add remaining campaigns. Break early if the schedule is at/over the horizon.
				for (cmpgn_num = 1; cmpgn_num != individual.genes.size(); ++cmpgn_num) {	
					// Product-dependent changeover.	
					if (individual.genes[cmpgn_num].product_num != individual.genes[cmpgn_num - 1].product_num) {
						if (!AddNewCampaign(cmpgn_num, individual, schedule)) {
							break;
						}
					}
					else {
						if (!ContinuePreviousCampaign(cmpgn_num, individual, schedule)) {
							break;
						}
					}
				}
			}

			EvaluateCampaigns(schedule);

			// TODO: check the final throughput against the storage constraints
			for (const auto &cmpgn : schedule.campaigns) {
				for (const auto &batch : cmpgn.batches) {
					schedule.objectives[TOTAL_KG_THROUGHPUT] += batch.kg;
					schedule.objectives[TOTAL_PRODUCTION_COST] += batch.kg * input_data.production_cost_per_kg[batch.product_num - 1];
				}
			}

			for (auto &obj : schedule.objectives) {
				if (obj < utils::EPSILON) {
					obj = 0.0;
				}
			}

			schedule.objectives[TOTAL_COST] = (
				schedule.objectives[TOTAL_INVENTORY_PENALTY] + 
				schedule.objectives[TOTAL_BACKLOG_PENALTY] +
				schedule.objectives[TOTAL_PRODUCTION_COST] +
				schedule.objectives[TOTAL_STORAGE_COST] +
				schedule.objectives[TOTAL_WASTE_COST]
			);

			schedule.objectives[TOTAL_PROFIT] = schedule.objectives[TOTAL_REVENUE] - schedule.objectives[TOTAL_COST];

			for (cmpgn_num = 0; cmpgn_num != schedule.campaigns.size(); ++cmpgn_num) {
				individual.genes[cmpgn_num].product_num = schedule.campaigns[cmpgn_num].product_num;
				individual.genes[cmpgn_num].num_batches = schedule.campaigns[cmpgn_num].num_batches;
			}

			// Delete excess genes
			if (cmpgn_num < individual.genes.size()) {
				individual.genes.erase(individual.genes.begin() + cmpgn_num + 1, individual.genes.end());
			}
		}

		void operator()(types::SingleObjectiveIndividual<types::SingleSiteSimpleGene> &individual)
		{
			types::SingleSiteSimpleSchedule schedule;

			CreateSchedule(individual, schedule);

			for (auto &it : input_data.objectives) {
				individual.objective = schedule.objectives[it.first] * it.second * -1;
				break;
			}

			// The smaller the constraint value the better
			individual.constraints = 0.0;
			if (!input_data.constraints.empty()) {
				for (auto &it : input_data.constraints) {
					// <= bound
					if (it.second.first == -1 && schedule.objectives[it.first] > it.second.second) {
						individual.constraints += std::fabs(schedule.objectives[it.first] - it.second.second);
					}
					// >= bound
					else if (it.second.first == 1 && schedule.objectives[it.first] < it.second.second) {
						individual.constraints += std::fabs(schedule.objectives[it.first] - it.second.second);
					}
				}
			}
		}
		
		void operator()(types::NSGAIndividual<types::SingleSiteSimpleGene> &individual)
		{
			types::SingleSiteSimpleSchedule schedule;

			CreateSchedule(individual, schedule);

			individual.objectives.resize(0);
			for (auto &it : input_data.objectives) {
				individual.objectives.push_back(schedule.objectives[it.first] * it.second * -1);
			}

			// The smaller the constraint value the better
			individual.constraints = 0.0;
			if (!input_data.constraints.empty()) {
				for (auto &it : input_data.constraints) {
					// <= bound
					if (it.second.first == -1 && schedule.objectives[it.first] > it.second.second) {
						individual.constraints += std::fabs(schedule.objectives[it.first] - it.second.second);
					}
					// >= bound
					else if (it.second.first == 1 && schedule.objectives[it.first] < it.second.second) {
						individual.constraints += std::fabs(schedule.objectives[it.first] - it.second.second);
					}
				}
			}
		}
	};
}

#endif 
