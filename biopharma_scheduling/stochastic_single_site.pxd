from libcpp.utility cimport pair
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.unordered_map cimport unordered_map

from schedule cimport SingleSiteSimpleSchedule


cdef extern from "input_data.h" namespace "stochastic":
    cdef enum OBJECTIVES:
        TOTAL_KG_INVENTORY_DEFICIT_MEAN
        TOTAL_KG_THROUGHPUT_MEAN
        TOTAL_KG_BACKLOG_MEAN
        TOTAL_KG_SUPPLY_MEAN
        TOTAL_KG_WASTE_MEAN

        TOTAL_INVENTORY_PENALTY_MEAN
        TOTAL_BACKLOG_PENALTY_MEAN
        TOTAL_PRODUCTION_COST_MEAN
        TOTAL_STORAGE_COST_MEAN
        TOTAL_WASTE_COST_MEAN
        TOTAL_REVENUE_MEAN
        TOTAL_PROFIT_MEAN
        TOTAL_COST_MEAN

        TOTAL_CHANGEOVER_COST

    cdef cppclass SingleSiteSimpleInputData:
        SingleSiteSimpleInputData()

        SingleSiteSimpleInputData(
            int mc_seed,
            int num_mc_sims,

            unordered_map[OBJECTIVES, int] objectives,
            vector[int] days_per_period,

            vector[vector[double]] kg_demand_min,
            vector[vector[double]] kg_demand_mode,
            vector[vector[double]] kg_demand_max,

            vector[double] kg_yield_per_batch_min,
            vector[double] kg_yield_per_batch_mode,
            vector[double] kg_yield_per_batch_max,

            vector[double] kg_opening_stock,
            vector[double] kg_storage_limits,

            vector[double] inventory_penalty_per_kg,
            vector[double] backlog_penalty_per_kg,
            vector[double] production_cost_per_kg,
            vector[double] storage_cost_per_kg,
            vector[double] waste_cost_per_kg,
            vector[double] sell_price_per_kg,

            vector[int] inoculation_days,
            vector[int] seed_days,
            vector[int] production_days,
            vector[int] usp_days,
            vector[int] dsp_days,
            vector[int] approval_days,
            vector[int] shelf_life_days,
            vector[int] min_batches_per_campaign,
            vector[int] max_batches_per_campaign,
            vector[int] batches_multiples_of_per_campaign,
            vector[vector[int]] changeover_days,

            vector[vector[double]] *kg_inventory_target,
            unordered_map[OBJECTIVES, pair[int, double]] *constraints
        )

        vector[vector[double]] kg_demand,
        vector[int] days_per_period,
        vector[double] kg_opening_stock,
        vector[double] kg_yield_per_batch,
        vector[double] kg_storage_limits,
        vector[double] inventory_penalty_per_kg,
        vector[double] backlog_penalty_per_kg,
        vector[double] production_cost_per_kg,
        vector[double] storage_cost_per_kg,
        vector[double] waste_cost_per_kg,
        vector[double] sell_price_per_kg,
        vector[int] inoculation_days,
        vector[int] seed_days,
        vector[int] production_days,
        vector[int] usp_days,
        vector[int] dsp_days,
        vector[int] approval_days,
        vector[int] shelf_life_days,
        vector[int] min_batches_per_campaign,
        vector[int] max_batches_per_campaign,
        vector[vector[int]] changeover_days,
        vector[vector[double]] *kg_inventory_target


cdef extern from "scheduling_models.h" namespace "stochastic" nogil:
    cdef cppclass SingleSiteSimpleModel:
        SingleSiteSimpleModel()
        SingleSiteSimpleModel(SingleSiteSimpleInputData input_data)
        void CreateSchedule[Chromosome](Chromosome &chromosome, SingleSiteSimpleSchedule &schedule)