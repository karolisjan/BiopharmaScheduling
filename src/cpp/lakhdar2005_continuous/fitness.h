#ifndef  __FITNESS_H__
#define __FITNESS_H__

#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <queue>

#include "batch.h"
#include "campaign.h"
#include "single_objective_individual.h"

using namespace std;
using namespace types;

class Fitness
{
	int num_products, num_periods, horizon;
	int num_usp_suites, num_dsp_suites;

	vector<vector<int>> demand;

	vector<int> days_per_period;

	vector<int> usp_storage_cost;
	vector<int> sales_price;
	vector<int> production_cost;
	vector<int> waste_disposal_cost;
	vector<int> dsp_storage_cost;
	vector<int> backlog_penalty;
	vector<int> changeover_cost;

	vector<double> usp_days;
	vector<double> usp_lead_days;
	vector<double> usp_shelf_life;
	vector<double> usp_storage_cap;

	vector<double> dsp_days;
	vector<double> dsp_lead_days;
	vector<double> dsp_shelf_life;
	vector<double> dsp_storage_cap;

	struct Objectives
	{
		Objectives() :
			profit(0),
			changeover_cost(0),
			production_cost(0),
			usp_storage_cost(0),
			dsp_storage_cost(0),
			usp_waste_cost(0),
			dsp_waste_cost(0),
			backlog_cost(0), 
			sales(0),
			total_usp_inventory(0)
		{}

		int profit;
		int changeover_cost;
		int production_cost;
		int usp_storage_cost;
		int dsp_storage_cost;
		int usp_waste_cost;
		int dsp_waste_cost;
		int backlog_cost;
		int sales;
		int total_usp_inventory;
	};

	inline void add_first_usp_campaign(
		unordered_map<int, vector<Campaign>>& usp_schedule,
		Gene& gene
	)
	{
		Campaign cmpgn;
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

	template<class Individual>
	unordered_map<int, vector<Campaign>> CreateUSPSchedule(Individual& c)
	{
		unordered_map<int, vector<Campaign>> usp_schedule;

		int i = 0;

		for (; i < c.genes.size(); ++i) {
			auto& gene = c.genes[i];

			if (usp_schedule.find(gene.usp_suite_num) == usp_schedule.end()) {
				add_first_usp_campaign(usp_schedule, gene);
				continue;
			}

			Campaign& prev_cmpgn = usp_schedule[gene.usp_suite_num].back();

			if (gene.product_num != prev_cmpgn.product) {
				Campaign cmpgn;
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

	template<class PriorityQueue>
	inline void add_first_dsp_campaign(
		int dsp_suite,
		unordered_map<int, vector<Campaign>>& dsp_schedule,
		PriorityQueue& dsp_campaigns,
		Campaign& usp_cmpgn)
	{
		Campaign dsp_cmpgn;
		dsp_cmpgn.suite = dsp_suite;
		dsp_cmpgn.product = usp_cmpgn.product;

		double usp_batch_fill_date = usp_cmpgn.start + usp_days[dsp_cmpgn.product - 1];

		dsp_cmpgn.start = (dsp_lead_days[dsp_cmpgn.product - 1] > usp_batch_fill_date) ?
			dsp_lead_days[dsp_cmpgn.product - 1] : usp_batch_fill_date;

		dsp_cmpgn.end = dsp_cmpgn.start + dsp_days[dsp_cmpgn.product - 1];

		if (dsp_cmpgn.end > horizon)
			return;

		dsp_cmpgn.batches = usp_cmpgn.batches;

		Batch dsp_batch;
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

			Batch dsp_batch;
			dsp_batch.product = dsp_cmpgn.product;
			dsp_batch.stored_at = dsp_cmpgn.end;
			dsp_batch.expires_at = dsp_batch.stored_at + dsp_shelf_life[dsp_cmpgn.product - 1];

			dsp_cmpgn.batches_list.push_back(dsp_batch);
		}

		dsp_schedule[dsp_suite].push_back(dsp_cmpgn);
		dsp_campaigns.push(dsp_cmpgn);
	}

	unordered_map<int, vector<Campaign>> CreateDSPSchedule(
		unordered_map<int, vector<Campaign>>& usp_schedule
	)
	{
		unordered_map<int, vector<Campaign>> dsp_schedule;
		int dsp_suite = 1;
		bool over_horizon = false;

		auto earlier_usp_cmpgn_start = [](const Campaign& a, const Campaign& b) { return a.start > b.start; };
		priority_queue<Campaign, vector<Campaign>, decltype(earlier_usp_cmpgn_start)> usp_campaigns(earlier_usp_cmpgn_start);

		for (auto& it : usp_schedule) {
			for (auto& cmpgn : it.second) {
				usp_campaigns.push(cmpgn);
			}
		}

		auto earlier_dsp_cmpgn_end = [](const Campaign& a, const Campaign& b) { return a.end > b.end; };
		priority_queue<Campaign, vector<Campaign>, decltype(earlier_dsp_cmpgn_end)> dsp_campaigns(earlier_dsp_cmpgn_end);

		for (; dsp_suite <= num_dsp_suites; ++dsp_suite) {
			Campaign dummy_dsp;
			dummy_dsp.suite = dsp_suite + num_usp_suites;
			dummy_dsp.end = 0;
			dsp_campaigns.push(dummy_dsp);
		}

		while (!usp_campaigns.empty()) {
			Campaign usp_cmpgn = usp_campaigns.top();
			usp_campaigns.pop();

			dsp_suite = dsp_campaigns.top().suite;
			dsp_campaigns.pop();

			if (dsp_schedule.find(dsp_suite) == dsp_schedule.end()) {
				add_first_dsp_campaign(dsp_suite, dsp_schedule, dsp_campaigns, usp_cmpgn);
				continue;
			}

			Campaign& prev_dsp_cmpgn = dsp_schedule[dsp_suite].back();
			Campaign dsp_cmpgn;
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

			Batch dsp_batch;
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

				Batch dsp_batch;
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

	struct oldest_batch_first
	{
		bool operator()(const Batch& b1, const Batch& b2)
		{
			return b1.expires_at > b2.expires_at;
		}
	};

	auto CreateInventoryProfile(
		unordered_map<int, vector<Campaign>>& dsp_schedule
	)
	{
		vector<vector<priority_queue<Batch, vector<Batch>, oldest_batch_first>>> inventory_profile(num_products,
			vector<priority_queue<Batch, vector<Batch>, oldest_batch_first>>(num_periods));

		for (int product = 0; product < num_products; ++product) {
			for (int period = 0; period < num_periods; ++period) {
				inventory_profile[product][period] = priority_queue<Batch, vector<Batch>, oldest_batch_first>();
			}
		}

		double prev_date, due_date;

		for (auto& it : dsp_schedule) {
			for (Campaign& dsp_cmpgn : it.second) {
				for (Batch& batch : dsp_cmpgn.batches_list) {
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
	S& access_queue_container(priority_queue<T, S, C>& q) {
		struct hacked_queue : private priority_queue<T, S, C> {
			static S& access_queue_container(priority_queue<T, S, C>& q) {
				return q.*&hacked_queue::c;
			}
		};
		return hacked_queue::access_queue_container(q);
	}

	template<class PriorityQueue>
	void CreateOtherProfiles(
		vector<vector<PriorityQueue>>& inventory_profile,
		vector<vector<int>>& sold,
		vector<vector<int>>& dsp_waste,
		vector<vector<int>>& backlog
	)
	{
		sold = vector<vector<int>>(num_products, vector<int>(num_periods, 0));
		dsp_waste = vector<vector<int>>(num_products, vector<int>(num_periods, 0));
		backlog= vector<vector<int>>(num_products, vector<int>(num_periods, 0));

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

	template<class PriorityQueue>
	void CalculateObjectives(
		Objectives& objectives, 
		unordered_map<int, vector<Campaign>>& usp_schedule,
		unordered_map<int, vector<Campaign>>& dsp_schedule,
		vector<vector<PriorityQueue>>& inventory_profile,
		vector<vector<int>>& sold,
		vector<vector<int>>& dsp_waste,
		vector<vector<int>>& backlog
		)
	{
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
	}

public:
	Fitness(
		int num_usp_suites,
		int num_dsp_suites,

		vector<vector<int>> demand,
		vector<int> days_per_period,

		vector<int> usp_storage_cost,
		vector<int> sales_price,
		vector<int> production_cost,
		vector<int> waste_disposal_cost,
		vector<int> dsp_storage_cost,
		vector<int> backlog_penalty,
		vector<int> changeover_cost,

		vector<double> usp_days,
		vector<double> usp_lead_days,
		vector<double> usp_shelf_life,
		vector<double> usp_storage_cap,

		vector<double> dsp_days,
		vector<double> dsp_lead_days,
		vector<double> dsp_shelf_life,
		vector<double> dsp_storage_cap
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

	void operator()(SingleObjectiveIndividual& c)
	{
		unordered_map<int, vector<Campaign>> usp_schedule = CreateUSPSchedule(c);
		unordered_map<int, vector<Campaign>> dsp_schedule = CreateDSPSchedule(usp_schedule);
		
		auto inventory_profile = CreateInventoryProfile(dsp_schedule);

		vector<vector<int>> sold, dsp_waste, backlog;
		CreateOtherProfiles(inventory_profile, sold, dsp_waste, backlog);

		Objectives objectives;
		CalculateObjectives(objectives, usp_schedule, dsp_schedule, inventory_profile, sold, dsp_waste, backlog);
	
		c.objective = objectives.profit;
		c.constraint = objectives.backlog_cost;
	}
};


#endif //!__FITNESS_H__
