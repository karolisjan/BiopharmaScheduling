#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
    #pragma GCC diagnostic ignored "-Wreorder"
	#pragma GCC diagnostic ignored "-Wsign-compare"
#endif 

#ifndef  __FITNESS_H__
#define __FITNESS_H__

#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>
#include <stdio.h>
#include <iostream>
#include <cassert>
#include <unordered_map>
#include <queue>

#include "batch.h"
#include "campaign.h"
#include "single_objective_individual.h"
#include "nsga_individual.h"

namespace deterministic
{
	class Lakhdar2005Ex1Model
	{
		int num_products, num_periods, horizon;
		int num_usp_suites, num_dsp_suites;

		std::vector<std::vector<int>> demand;

		std::vector<int> days_per_period;

		std::vector<double> usp_storage_cost;
		std::vector<double> sales_price;
		std::vector<double> production_cost;
		std::vector<double> waste_disposal_cost;
		std::vector<double> dsp_storage_cost;
		std::vector<double> backlog_penalty;
		std::vector<double> changeover_cost;

		std::vector<double> usp_days;
		std::vector<double> usp_lead_days;
		std::vector<double> usp_shelf_life;
		std::vector<double> usp_storage_cap;

		std::vector<double> dsp_days;
		std::vector<double> dsp_lead_days;
		std::vector<double> dsp_shelf_life;
		std::vector<double> dsp_storage_cap;

		inline void add_first_usp_campaign(
			std::unordered_map<int, std::vector<types::Campaign>>& usp_schedule,
			types::Gene& gene
		)
		{
			types::Campaign cmpgn;
			cmpgn.suite = gene.usp_suite_num;
			cmpgn.product = gene.product_num;
			cmpgn.batches = gene.num_batches;
			cmpgn.start = usp_lead_days[cmpgn.product - 1];
			cmpgn.end = cmpgn.start + usp_days[cmpgn.product - 1] * cmpgn.batches;


			while (cmpgn.end > horizon && --cmpgn.batches > 0)
				cmpgn.end -= usp_days[cmpgn.product - 1];

			if (cmpgn.batches)
				usp_schedule[cmpgn.suite].push_back(cmpgn);

			gene.num_batches = cmpgn.batches;
		}


		template<class PriorityQueue>
		inline void add_first_dsp_campaign(
			int dsp_suite,
			std::unordered_map<int, std::vector<types::Campaign>>& dsp_schedule,
			PriorityQueue& dsp_campaigns,
			types::Campaign& usp_cmpgn
		)
		{
			types::Campaign dsp_cmpgn;
			dsp_cmpgn.suite = dsp_suite;
			dsp_cmpgn.product = usp_cmpgn.product;

			double usp_batch_fill_date = usp_cmpgn.start + usp_days[dsp_cmpgn.product - 1];

			dsp_cmpgn.start = (dsp_lead_days[dsp_cmpgn.product - 1] > usp_batch_fill_date) ?
				dsp_lead_days[dsp_cmpgn.product - 1] : usp_batch_fill_date;

			dsp_cmpgn.end = dsp_cmpgn.start + dsp_days[dsp_cmpgn.product - 1];

			if (dsp_cmpgn.end > horizon)
				return;

			dsp_cmpgn.batches = usp_cmpgn.batches;

			types::Batch dsp_batch;
			dsp_batch.product = dsp_cmpgn.product;
			dsp_batch.stored_at = dsp_cmpgn.end;
			dsp_batch.expires_at = dsp_batch.stored_at + dsp_shelf_life[dsp_cmpgn.product - 1];

			dsp_cmpgn.batches_list.push_back(dsp_batch);

			for (int batches = 1; batches < usp_cmpgn.batches; ++batches) {
				usp_batch_fill_date += usp_days[dsp_cmpgn.product - 1];
				dsp_cmpgn.end = usp_batch_fill_date + dsp_days[dsp_cmpgn.product - 1];

				if (dsp_cmpgn.end > horizon) {
					dsp_cmpgn.end -= dsp_days[dsp_cmpgn.product - 1];
					dsp_cmpgn.batches = batches;
					break;
				}

				types::Batch dsp_batch;
				dsp_batch.product = dsp_cmpgn.product;
				dsp_batch.stored_at = dsp_cmpgn.end;
				dsp_batch.expires_at = dsp_batch.stored_at + dsp_shelf_life[dsp_cmpgn.product - 1];

				dsp_cmpgn.batches_list.push_back(dsp_batch);
			}

			dsp_schedule[dsp_suite].push_back(dsp_cmpgn);
			dsp_campaigns.push(dsp_cmpgn);
		}

		struct oldest_batch_first
		{
			bool operator()(const types::Batch& b1, const types::Batch& b2)
			{
				return b1.expires_at > b2.expires_at;
			}
		};

		auto CreateInventoryProfile(
			std::unordered_map<int, std::vector<types::Campaign>>& dsp_schedule
		)
		{
			std::vector<std::vector<std::priority_queue<types::Batch, std::vector<types::Batch>, oldest_batch_first>>> inventory_profile(num_products,
				std::vector<std::priority_queue<types::Batch, std::vector<types::Batch>, oldest_batch_first>>(num_periods));

			for (int product = 0; product < num_products; ++product) {
				for (int period = 0; period < num_periods; ++period) {
					inventory_profile[product][period] = std::priority_queue<types::Batch, std::vector<types::Batch>, oldest_batch_first>();
				}
			}

			double prev_date, due_date;

			for (auto& it : dsp_schedule) {
				for (types::Campaign& dsp_cmpgn : it.second) {
					for (types::Batch& batch : dsp_cmpgn.batches_list) {
						prev_date = due_date = 0;

						for (int period = 0; period < num_periods; ++period) {
							prev_date = due_date;
							due_date += days_per_period[period];

							if (batch.stored_at > prev_date && batch.stored_at <= due_date)
								inventory_profile[batch.product - 1][period].push(batch);
						}
					}
				}
			}

			return inventory_profile;
		}

		template <class T, class S, class C>
		S& access_queue_container(std::priority_queue<T, S, C>& q) {
			struct hacked_queue : private std::priority_queue<T, S, C> {
				static S& access_queue_container(std::priority_queue<T, S, C>& q) {
					return q.*&hacked_queue::c;
				}
			};
			return hacked_queue::access_queue_container(q);
		}

		template<class PriorityQueue>
		void CreateOtherProfiles(
			std::vector<std::vector<PriorityQueue>>& inventory_profile,
			std::vector<std::vector<int>>& sold,
			std::vector<std::vector<int>>& dsp_waste,
			std::vector<std::vector<int>>& backlog
		)
		{
			sold = std::vector<std::vector<int>>(num_products, std::vector<int>(num_periods, 0));
			dsp_waste = std::vector<std::vector<int>>(num_products, std::vector<int>(num_periods, 0));
			backlog = std::vector<std::vector<int>>(num_products, std::vector<int>(num_periods, 0));

			int period;
			double due_date;

			for (int product = 0; product < num_products; ++product) {
				period = 0;
				due_date = days_per_period[period];

				while (!inventory_profile[product][period].empty() && inventory_profile[product][period].top().expires_at <= due_date) {
					inventory_profile[product][period].pop();
					++dsp_waste[product][period];
				}

				if (inventory_profile[product][period].size() >= demand[product][period]) {
					sold[product][period] = demand[product][period];
					for (int i = 0; i < sold[product][period]; ++i)
						inventory_profile[product][period].pop();
				}
				else {
					sold[product][period] = inventory_profile[product][period].size();
					inventory_profile[product][period] = PriorityQueue();
					backlog[product][period] = demand[product][period] - sold[product][period];
				}

				for (period = 1; period < num_periods; ++period) {
					due_date += days_per_period[period];

					for (auto& batch : access_queue_container(inventory_profile[product][period - 1]))
						inventory_profile[product][period].push(batch);

					while (!inventory_profile[product][period].empty() && inventory_profile[product][period].top().expires_at <= due_date) {
						inventory_profile[product][period].pop();
						++dsp_waste[product][period];
					}

					if (inventory_profile[product][period].size() >= demand[product][period]) {
						sold[product][period] = demand[product][period];
						for (int i = 0; i < sold[product][period]; ++i)
							inventory_profile[product][period].pop();

						if (backlog[product][period - 1]) {
							if (inventory_profile[product][period].size() >= backlog[product][period - 1]) {
								sold[product][period] += backlog[product][period - 1];
								backlog[product][period] = 0;
								for (int i = 0; i < backlog[product][period]; ++i)
									inventory_profile[product][period].pop();
							}
							else {
								sold[product][period] += inventory_profile[product][period].size();
								backlog[product][period] = backlog[product][period - 1] - inventory_profile[product][period].size();
								inventory_profile[product][period] = PriorityQueue();
							}
						}
					}
					else {
						sold[product][period] = inventory_profile[product][period].size();
						inventory_profile[product][period] = PriorityQueue();
						backlog[product][period] = backlog[product][period - 1] + demand[product][period] - sold[product][period];
					}
				}
			}
		}

	public:
		Lakhdar2005Ex1Model() {}

		Lakhdar2005Ex1Model(
			int num_usp_suites,
			int num_dsp_suites,

			std::vector<std::vector<int>> demand,
			std::vector<int> days_per_period,

			std::vector<double> usp_storage_cost,
			std::vector<double> sales_price,
			std::vector<double> production_cost,
			std::vector<double> waste_disposal_cost,
			std::vector<double> dsp_storage_cost,
			std::vector<double> backlog_penalty,
			std::vector<double> changeover_cost,

			std::vector<double> usp_days,
			std::vector<double> usp_lead_days,
			std::vector<double> usp_shelf_life,
			std::vector<double> usp_storage_cap,

			std::vector<double> dsp_days,
			std::vector<double> dsp_lead_days,
			std::vector<double> dsp_shelf_life,
			std::vector<double> dsp_storage_cap
		) :
			num_usp_suites(num_usp_suites),
			num_dsp_suites(num_dsp_suites),

			demand(demand),
			days_per_period(days_per_period),

			usp_storage_cost(usp_storage_cost),
			sales_price(sales_price),
			production_cost(production_cost),
			waste_disposal_cost(waste_disposal_cost),
			dsp_storage_cost(dsp_storage_cost),
			backlog_penalty(backlog_penalty),
			changeover_cost(changeover_cost),

			usp_days(usp_days),
			usp_lead_days(usp_lead_days),
			usp_shelf_life(usp_shelf_life),
			usp_storage_cap(usp_storage_cap),

			dsp_days(dsp_days),
			dsp_lead_days(dsp_lead_days),
			dsp_shelf_life(dsp_shelf_life),
			dsp_storage_cap(dsp_storage_cap)
		{
			num_products = demand.size();
			num_periods = demand[0].size();

			horizon = 0;
			for (auto &val : days_per_period)
				horizon += val;
		}

		std::unordered_map<int, std::vector<types::Campaign>> CreateUSPSchedule(types::SingleObjectiveIndividual& c)
		{
			std::unordered_map<int, std::vector<types::Campaign>> usp_schedule;

			int i = 0;

			for (; i < c.genes.size(); ++i) {
				auto& gene = c.genes[i];

				if (usp_schedule.find(gene.usp_suite_num) == usp_schedule.end()) {
					add_first_usp_campaign(usp_schedule, gene);
					continue;
				}

				types::Campaign& prev_cmpgn = usp_schedule[gene.usp_suite_num].back();

				if (gene.product_num != prev_cmpgn.product) {
					types::Campaign cmpgn;
					cmpgn.suite = gene.usp_suite_num;
					cmpgn.product = gene.product_num;
					cmpgn.batches = gene.num_batches;
					cmpgn.start = prev_cmpgn.end + usp_lead_days[cmpgn.product - 1];

					if (cmpgn.start > horizon) {
						c.genes.erase(c.genes.begin() + i);
						continue;
					}

					cmpgn.end = cmpgn.start + usp_days[cmpgn.product - 1] * cmpgn.batches;

					while (cmpgn.end > horizon && --cmpgn.batches > 0)
						cmpgn.end -= usp_days[cmpgn.product - 1];

					if (cmpgn.batches) {
						gene.num_batches = cmpgn.batches;
						usp_schedule[cmpgn.suite].push_back(cmpgn);
					}
					else {
						c.genes.erase(c.genes.begin() + i);
					}
				}
				else {
					prev_cmpgn.batches += gene.num_batches;
					prev_cmpgn.end = prev_cmpgn.start + usp_days[prev_cmpgn.product - 1] * prev_cmpgn.batches;

					while (prev_cmpgn.end > horizon && --prev_cmpgn.batches > 0)
						prev_cmpgn.end -= usp_days[prev_cmpgn.product - 1];
				}
			}

			return usp_schedule;
		}

		struct Objectives
		{
			Objectives() :
				profit(0),
				sales(0),
				backlog_cost(0),
				changeover_cost(0),
				production_cost(0),
				usp_storage_cost(0),
				dsp_storage_cost(0),
				usp_waste_cost(0),
				dsp_waste_cost(0)
			{}

			double profit;
			double sales;
			double backlog_cost;
			double changeover_cost;
			double production_cost;
			double usp_storage_cost;
			double dsp_storage_cost;
			double usp_waste_cost;
			double dsp_waste_cost;
		};

		std::unordered_map<int, std::vector<types::Campaign>> CreateDSPSchedule(
			std::unordered_map<int, std::vector<types::Campaign>>& usp_schedule
		)
		{
			std::unordered_map<int, std::vector<types::Campaign>> dsp_schedule;
			int dsp_suite = 1;
			bool over_horizon = false;

			auto earlier_usp_cmpgn_start = [](const types::Campaign& a, const types::Campaign& b) { return a.start > b.start; };
			std::priority_queue<types::Campaign, std::vector<types::Campaign>, decltype(earlier_usp_cmpgn_start)> usp_campaigns(earlier_usp_cmpgn_start);

			for (auto& it : usp_schedule) {
				for (auto& cmpgn : it.second) {
					usp_campaigns.push(cmpgn);
				}
			}

			auto earlier_dsp_cmpgn_end = [](const types::Campaign& a, const types::Campaign& b) { return a.end > b.end; };
			std::priority_queue<types::Campaign, std::vector<types::Campaign>, decltype(earlier_dsp_cmpgn_end)> dsp_campaigns(earlier_dsp_cmpgn_end);

			for (; dsp_suite <= num_dsp_suites; ++dsp_suite) {
				types::Campaign dummy_dsp;
				dummy_dsp.suite = dsp_suite + num_usp_suites;
				dummy_dsp.end = 0;
				dsp_campaigns.push(dummy_dsp);
			}

			while (!usp_campaigns.empty()) {
				types::Campaign usp_cmpgn = usp_campaigns.top();
				usp_campaigns.pop();

				dsp_suite = dsp_campaigns.top().suite;
				dsp_campaigns.pop();

				if (dsp_schedule.find(dsp_suite) == dsp_schedule.end()) {
					add_first_dsp_campaign(dsp_suite, dsp_schedule, dsp_campaigns, usp_cmpgn);
					continue;
				}

				types::Campaign& prev_dsp_cmpgn = dsp_schedule[dsp_suite].back();
				types::Campaign dsp_cmpgn;
				dsp_cmpgn.suite = dsp_suite;
				dsp_cmpgn.product = usp_cmpgn.product;

				double usp_batch_fill_date = usp_cmpgn.start + usp_days[dsp_cmpgn.product - 1];

				dsp_cmpgn.start = (prev_dsp_cmpgn.end + dsp_lead_days[dsp_cmpgn.product - 1] > usp_batch_fill_date) ?
					prev_dsp_cmpgn.end + dsp_lead_days[dsp_cmpgn.product - 1] :
					usp_batch_fill_date;

				dsp_cmpgn.end = dsp_cmpgn.start + dsp_days[dsp_cmpgn.product - 1];

				if (dsp_cmpgn.end > horizon)
					continue;

				dsp_cmpgn.batches = usp_cmpgn.batches;

				types::Batch dsp_batch;
				dsp_batch.product = dsp_cmpgn.product;
				dsp_batch.stored_at = dsp_cmpgn.end;
				dsp_batch.expires_at = dsp_batch.stored_at + dsp_shelf_life[dsp_cmpgn.product - 1];

				dsp_cmpgn.batches_list.push_back(dsp_batch);

				for (int batches = 1; batches < usp_cmpgn.batches; ++batches) {
					usp_batch_fill_date += usp_days[dsp_cmpgn.product - 1];
					dsp_cmpgn.end = usp_batch_fill_date + dsp_days[dsp_cmpgn.product - 1];

					if (dsp_cmpgn.end > horizon) {
						dsp_cmpgn.end -= dsp_days[dsp_cmpgn.product - 1];
						dsp_cmpgn.batches = batches;
						break;
					}

					types::Batch dsp_batch;
					dsp_batch.product = dsp_cmpgn.product;
					dsp_batch.stored_at = dsp_cmpgn.end;
					dsp_batch.expires_at = dsp_batch.stored_at + dsp_shelf_life[dsp_cmpgn.product - 1];

					dsp_cmpgn.batches_list.push_back(dsp_batch);
				}

				dsp_schedule[dsp_suite].push_back(dsp_cmpgn);
				dsp_campaigns.push(dsp_cmpgn);
			}

			return dsp_schedule;
		}

		template<class PriorityQueue>
		Objectives CalculateObjectives(
			std::unordered_map<int, std::vector<types::Campaign>>& usp_schedule,
			std::unordered_map<int, std::vector<types::Campaign>>& dsp_schedule,
			std::vector<std::vector<PriorityQueue>>& inventory_profile,
			std::vector<std::vector<int>>& sold,
			std::vector<std::vector<int>>& dsp_waste,
			std::vector<std::vector<int>>& backlog
		)
		{
			Objectives objectives;

			for (auto& it : usp_schedule) {
				objectives.changeover_cost += it.second.size();

				for (auto& usp_cmpgn : it.second)
					objectives.production_cost += (usp_cmpgn.batches * production_cost[usp_cmpgn.product - 1]);
			}

			for (auto& it : dsp_schedule) {
				objectives.changeover_cost += it.second.size();

				for (auto& dsp_cmpgn : it.second)
					objectives.production_cost += (dsp_cmpgn.batches * production_cost[dsp_cmpgn.product - 1]);
			}

			for (int product = 0; product < num_products; ++product) {
				for (int period = 0; period < num_periods; ++period) {
					objectives.dsp_storage_cost += (inventory_profile[product][period].size() * dsp_storage_cost[product]);
					objectives.backlog_cost += (backlog[product][period] * backlog_penalty[product]);
					objectives.dsp_waste_cost += (dsp_waste[product][period] * waste_disposal_cost[product]);
					objectives.sales += (sold[product][period] * sales_price[product]);
				}
			}

			objectives.profit = objectives.sales -
				(objectives.production_cost + objectives.changeover_cost + objectives.dsp_storage_cost + objectives.backlog_cost + objectives.dsp_waste_cost);

			return objectives;
		}

		Objectives CalculateObjectives(
			std::unordered_map<int, std::vector<types::Campaign>>& usp_schedule,
			std::unordered_map<int, std::vector<types::Campaign>>& dsp_schedule,
			std::vector<std::vector<int>>& inventory,
			std::vector<std::vector<int>>& sold,
			std::vector<std::vector<int>>& dsp_waste,
			std::vector<std::vector<int>>& backlog
		)
		{
			auto inventory_profile = CreateInventoryProfile(dsp_schedule);
			CreateOtherProfiles(inventory_profile, sold, dsp_waste, backlog);

			inventory = std::vector<std::vector<int>>(num_products, std::vector<int>(num_periods, 0));
			for (int product = 0; product < num_products; ++product) {
				for (int period = 0; period < num_periods; ++period) {
					inventory[product][period] = inventory_profile[product][period].size();
				}
			}

			return CalculateObjectives(usp_schedule, dsp_schedule, inventory_profile, sold, dsp_waste, backlog);
		}

		void operator()(types::SingleObjectiveIndividual& c)
		{
			std::unordered_map<int, std::vector<types::Campaign>> usp_schedule = CreateUSPSchedule(c);
			std::unordered_map<int, std::vector<types::Campaign>> dsp_schedule = CreateDSPSchedule(usp_schedule);

			auto inventory_profile = CreateInventoryProfile(dsp_schedule);

			std::vector<std::vector<int>> sold, dsp_waste, backlog;
			CreateOtherProfiles(inventory_profile, sold, dsp_waste, backlog);

			Objectives objectives = CalculateObjectives(usp_schedule, dsp_schedule, inventory_profile, sold, dsp_waste, backlog);

			c.objective = objectives.profit;
			c.constraint = objectives.backlog_cost;
		}
	};

	class EliLillyModel
	{
		int num_products, num_periods, horizon;
		std::vector<std::vector<double>> demand, inventory_safety_levels;
		std::vector<double> opening_stock;
		std::vector<double> yield;
		std::vector<int> days_per_period;
		std::vector<int> usp_days, dsp_days, takt_days, approval_days;
		std::vector<std::vector<int>> dsp_turnaround;

	protected:
		struct Objectives
		{
			Objectives() :
				total_kg_deficit(0),
				total_pre_approved_kg(0),
				total_pre_approved_batches(0),
				total_approved_kg(0),
				total_approved_batches(0),
				total_kg_backlog(0) {}

			double total_kg_deficit;
			double total_pre_approved_kg;
			double total_pre_approved_batches;
			double total_approved_kg;
			double total_approved_batches;
			double total_kg_backlog;
		};

		void EvaluateApprovedBatchesProfile(
			Objectives &objectives, 
			std::vector<types::Campaign>& schedule,
			std::vector<int>& approved_batches_profile
			)
		{
			int product, period, batch, due_date, prev_date;
			int fill_date_preapproved, fill_date_approved;
			int idx;

			for (types::Campaign &cmpgn : schedule) {
				product = cmpgn.product - 1;
				due_date = 0;

				for (period = 0; period < num_periods; ++period) {
					prev_date = due_date;
					due_date += days_per_period[period];
					idx = product * num_periods + period;

					for (batch = 0; batch < cmpgn.batches; ++batch) {
						fill_date_preapproved = cmpgn.first_fill + cmpgn.dsp_time * batch;
						fill_date_approved = fill_date_preapproved + approval_days[product];

						if (fill_date_preapproved >= prev_date &&
							fill_date_preapproved < due_date) {
							++objectives.total_pre_approved_batches;
							objectives.total_pre_approved_kg += yield[product];
						}

						if (fill_date_approved >= prev_date &&
							fill_date_approved < due_date) {
							++approved_batches_profile[idx];
							++objectives.total_approved_batches;
						}

					}
				}
			}
		}
		
		void EvaluateInventoryProfile(Objectives &objectives, std::vector<int>& approved_batches_profile)
		{
			int product = 0, period = 0, idx;
			double prev_inventory, inventory;

			for (; product < num_products; ++product) {
				prev_inventory = opening_stock[product];

				for (period = 0; period < num_periods; ++period) {
					idx = product * num_periods + period;

					inventory = prev_inventory + approved_batches_profile[idx] * yield[product] - demand[product][period];
					prev_inventory = inventory;

					if (inventory < 0) {
						objectives.total_kg_backlog -= inventory;
						prev_inventory = 0;
					}

					if (inventory_safety_levels[product][period] > inventory)
						objectives.total_kg_deficit += (inventory_safety_levels[product][period] - inventory);
				}

			}
		}

	public:
		EliLillyModel() {};

		EliLillyModel(
			std::vector<std::vector<double>> demand,
			std::vector<std::vector<double>> inventory_safety_levels,
			std::vector<int> days_per_period,
			std::vector<double> opening_stock,
			std::vector<double> yield,
			std::vector<int> usp_days,
			std::vector<int> dsp_days,
			std::vector<int> takt_days,
			std::vector<int> approval_time,
			std::vector<std::vector<int>> dsp_turnaround
		) :
			demand(demand),
			inventory_safety_levels(inventory_safety_levels),
			days_per_period(days_per_period),
			opening_stock(opening_stock),
			yield(yield),
			usp_days(usp_days),
			dsp_days(dsp_days),
			takt_days(takt_days),
			approval_days(approval_time),
			dsp_turnaround(dsp_turnaround)
		{
			num_products = demand.size();
			num_periods = demand[0].size();
			horizon = std::accumulate(days_per_period.begin(), days_per_period.end(), 0);	
		}

		template<class Individual>
		std::vector<types::Campaign> CreateSchedule(Individual& c)
		{
			std::vector<types::Campaign> schedule;

			int cmpgn_count = 1,
				product = c.genes[0].product_num - 1,
				batches = c.genes[0].num_batches;

			types::Campaign new_cmpgn;

			new_cmpgn.product = product + 1;
			new_cmpgn.batches = batches;
			new_cmpgn.start = 0;
			new_cmpgn.dsp_turnaround = 0;
			new_cmpgn.usp_time = usp_days[product];
			new_cmpgn.dsp_time = dsp_days[product];
			new_cmpgn.takt = takt_days[product];

			new_cmpgn.harvest = new_cmpgn.start + new_cmpgn.usp_time;
			new_cmpgn.first_fill = new_cmpgn.harvest + new_cmpgn.dsp_time;

			if (new_cmpgn.takt < new_cmpgn.dsp_time) {
				new_cmpgn.end = new_cmpgn.first_fill + new_cmpgn.dsp_time * (batches - 1);
			}
			else {
				new_cmpgn.end = new_cmpgn.first_fill + new_cmpgn.takt * (batches - 1);
			}

			schedule.push_back(new_cmpgn);
			types::Campaign *prev_cmpgn = &schedule[0];

			for (; cmpgn_count < c.genes.size(); ++cmpgn_count) {
				new_cmpgn = types::Campaign();

				product = c.genes[cmpgn_count].product_num - 1;
				batches = c.genes[cmpgn_count].num_batches;

				new_cmpgn.product = product + 1;

				if (prev_cmpgn->product != new_cmpgn.product) {

					new_cmpgn.batches = batches;
					new_cmpgn.usp_time = usp_days[product];
					new_cmpgn.dsp_time = dsp_days[product];
					new_cmpgn.takt = takt_days[product];
					new_cmpgn.dsp_turnaround = dsp_turnaround[prev_cmpgn->product - 1][new_cmpgn.product - 1];
					new_cmpgn.harvest = prev_cmpgn->end + new_cmpgn.dsp_turnaround;

					if (new_cmpgn.takt <= new_cmpgn.dsp_time)
						new_cmpgn.first_fill = new_cmpgn.harvest + new_cmpgn.dsp_time;
					else
						new_cmpgn.first_fill = new_cmpgn.harvest + new_cmpgn.takt;

					new_cmpgn.start = new_cmpgn.harvest - new_cmpgn.usp_time;

					if (new_cmpgn.first_fill > horizon)
						break;

					if (new_cmpgn.takt <= new_cmpgn.dsp_time) {
						new_cmpgn.end = new_cmpgn.first_fill + new_cmpgn.dsp_time * (batches - 1);

						if (new_cmpgn.end > horizon) {
							int fill = new_cmpgn.first_fill + new_cmpgn.dsp_time;
							for (int b = 2; b <= batches; b++) {
								if (fill > horizon) {
									new_cmpgn.end = fill - new_cmpgn.dsp_time;
									new_cmpgn.batches = b - 1;
									break;
								}
								fill += new_cmpgn.dsp_time;
							}

							schedule.push_back(new_cmpgn);
							prev_cmpgn = &schedule.back();
							break;
						}
					}
					else {
						new_cmpgn.end = new_cmpgn.first_fill + new_cmpgn.takt * (batches - 1);

						if (new_cmpgn.end > horizon) {
							int fill = new_cmpgn.first_fill + new_cmpgn.takt;
							for (int b = 2; b <= batches; b++) {
								if (fill > horizon) {
									new_cmpgn.end = fill - new_cmpgn.takt;
									new_cmpgn.batches = b - 1;
									break;
								}
								fill += new_cmpgn.takt;
							}

							schedule.push_back(new_cmpgn);
							prev_cmpgn = &schedule.back();
							break;
						}
					}

					schedule.push_back(new_cmpgn);
					prev_cmpgn = &schedule.back();
				}
				else {
					prev_cmpgn->batches += batches;

					batches = prev_cmpgn->batches;

					if (prev_cmpgn->takt <= prev_cmpgn->dsp_time) {
						prev_cmpgn->end = prev_cmpgn->first_fill + prev_cmpgn->dsp_time * (batches - 1);

						if (prev_cmpgn->end > horizon) {
							int fill = prev_cmpgn->first_fill + prev_cmpgn->dsp_time;
							for (int b = 2; b <= batches; b++) {
								if (fill > horizon) {
									prev_cmpgn->end = fill - prev_cmpgn->dsp_time;
									prev_cmpgn->batches = b - 1;
									break;
								}
								fill += prev_cmpgn->dsp_time;
							}

							break;
						}
					}
					else {
						prev_cmpgn->end = prev_cmpgn->first_fill + prev_cmpgn->takt * (batches - 1);

						if (prev_cmpgn->end > horizon) {
							int fill = prev_cmpgn->first_fill + prev_cmpgn->takt;
							for (int b = 2; b <= batches; b++) {
								if (fill > horizon) {
									prev_cmpgn->end = fill - prev_cmpgn->takt;
									prev_cmpgn->batches = b - 1;
									break;
								}
								fill += prev_cmpgn->takt;
							}

							break;
						}
					}
				}
			}

			if (c.genes.size() > cmpgn_count) {
				c.genes.erase(c.genes.begin() + cmpgn_count + 1, c.genes.end());
			}

			for (cmpgn_count = 1; cmpgn_count < c.genes.size(); ++cmpgn_count) {
				if (c.genes[cmpgn_count].product_num == c.genes[cmpgn_count - 1].product_num) {
					c.genes[cmpgn_count - 1].num_batches += c.genes[cmpgn_count].num_batches;
					c.genes.erase(c.genes.begin() + cmpgn_count);
				}
			}

			return schedule;
		}

		std::vector<std::vector<double>> GetInventoryProfile(types::NSGAIndividual& c)
		{
			int product = 0, period = 0, idx;
			double prev_inventory, backlog = 0;

			std::vector<types::Campaign> schedule = CreateSchedule(c);
			Objectives objectives;
			std::vector<int> approved_batches_profile(num_products * num_periods, 0);
			std::vector<std::vector<double>> inventory_profile(num_products);

			EvaluateApprovedBatchesProfile(objectives, schedule, approved_batches_profile);

			for (; product < num_products; ++product) {
				inventory_profile[product] = std::vector<double>(num_periods);
				prev_inventory = opening_stock[product];

				for (period = 0; period < num_periods; ++period) {
					idx = product * num_periods + period;

					inventory_profile[product][period] = prev_inventory + approved_batches_profile[idx] * yield[product] - demand[product][period];
					prev_inventory = inventory_profile[product][period];

					if (inventory_profile[product][period] < 0) {
						backlog += inventory_profile[product][period];
						prev_inventory = 0;
					}

				}
			}

			return inventory_profile;
		}

		void operator()(types::SingleObjectiveIndividual& c)
		{
			std::vector<types::Campaign> schedule = CreateSchedule(c);
			Objectives objectives;
			std::vector<int> approved_batches_profile(num_products * num_periods, 0);
			EvaluateApprovedBatchesProfile(objectives, schedule, approved_batches_profile);
			EvaluateInventoryProfile(objectives, approved_batches_profile);

			c.objective = objectives.total_pre_approved_kg;
			c.constraint = floor(objectives.total_kg_backlog + 0.5);
		}

		void operator()(types::NSGAIndividual& c)
		{
			std::vector<types::Campaign> schedule = CreateSchedule(c);
			Objectives objectives;
			std::vector<int> approved_batches_profile(num_products * num_periods, 0);
			EvaluateApprovedBatchesProfile(objectives, schedule, approved_batches_profile);
			EvaluateInventoryProfile(objectives, approved_batches_profile);

			c.objectives = { -objectives.total_pre_approved_kg, objectives.total_kg_deficit };
			c.constraint = floor(objectives.total_kg_backlog + 0.5);
		}
	};
}

#endif //!__FITNESS_H__
