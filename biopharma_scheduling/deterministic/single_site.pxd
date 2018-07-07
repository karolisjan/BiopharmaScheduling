from libcpp.utility cimport pair
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.unordered_map cimport unordered_map

from ..schedule cimport SingleSiteSimpleSchedule, SingleSiteMultiSuiteSchedule


cdef extern from "../input_data.h" namespace "deterministic":
    cdef enum OBJECTIVES:
        TOTAL_KG_INVENTORY_DEFICIT
        TOTAL_KG_THROUGHPUT
        TOTAL_KG_BACKLOG
        TOTAL_KG_SUPPLY
        TOTAL_KG_WASTE

        TOTAL_BATCH_INVENTORY_DEFICIT
        TOTAL_BATCH_THROUGHPUT
        TOTAL_BATCH_BACKLOG
        TOTAL_BATCH_SUPPLY
        TOTAL_BATCH_WASTE

        TOTAL_INVENTORY_PENALTY
        TOTAL_CHANGEOVER_COST
        TOTAL_BACKLOG_PENALTY 
        TOTAL_PRODUCTION_COST
        TOTAL_STORAGE_COST
        TOTAL_WASTE_COST
        TOTAL_REVENUE
        TOTAL_PROFIT
        TOTAL_COST
        NUM_OBJECTIVES = TOTAL_COST 

    cdef cppclass SingleSiteSimpleInputData:
        SingleSiteSimpleInputData()

        SingleSiteSimpleInputData(
            unordered_map[OBJECTIVES, int] objectives,
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


    cdef cppclass SingleSiteMultiSuiteInputData:
        SingleSiteMultiSuiteInputData()

        SingleSiteMultiSuiteInputData(
            unordered_map[OBJECTIVES, int] objectives,

            int num_usp_suites,
            int num_dsp_suites,

            vector[vector[int]] demand,
            vector[int] days_per_period,
            
            vector[double] usp_days,
            vector[double] dsp_days,
            
            vector[int] shelf_life,
            vector[int] storage_cap,

            vector[double] sales_price,
            vector[double] storage_cost,
            vector[double] backlog_penalty,
            vector[double] waste_disposal_cost,
            vector[double] usp_production_cost,
            vector[double] dsp_production_cost,
            vector[double] usp_changeover_cost,
            vector[double] dsp_changeover_cost,

            vector[vector[double]] usp_changeovers,
            vector[vector[double]] dsp_changeovers,
 
            unordered_map[OBJECTIVES, pair[int, double]] *constraints
        )

        unordered_map[OBJECTIVES, int] objectives,
        int num_usp_suites,
        int num_dsp_suites,
        vector[vector[int]] demand,
        vector[int] days_per_period,
        vector[double] usp_days,
        vector[double] dsp_days,
        vector[int] shelf_life,
        vector[int] storage_cap,
        vector[double] sales_price,
        vector[double] storage_cost,
        vector[double] backlog_penalty,
        vector[double] waste_disposal_cost,
        vector[double] usp_production_cost,
        vector[double] dsp_production_cost,
        vector[double] usp_changeover_cost,
        vector[double] dsp_changeover_cost
        vector[vector[double]] usp_changeovers,
        vector[vector[double]] dsp_changeovers,
        unordered_map[OBJECTIVES, pair[int, double]] *constraints


cdef extern from "../scheduling_models.h" namespace "deterministic" nogil:
    cdef cppclass SingleSiteSimpleModel:
        SingleSiteSimpleModel()
        SingleSiteSimpleModel(SingleSiteSimpleInputData input_data)
        void CreateSchedule[Chromosome](Chromosome &chromosome, SingleSiteSimpleSchedule &schedule)


    cdef cppclass SingleSiteMultiSuiteModel:
        SingleSiteMultiSuiteModel()
        SingleSiteMultiSuiteModel(SingleSiteMultiSuiteInputData input_data)
        void CreateSchedule[Chromosome](Chromosome &chromosome, SingleSiteMultiSuiteSchedule &schedule)
