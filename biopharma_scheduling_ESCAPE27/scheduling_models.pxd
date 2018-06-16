from libcpp.vector cimport vector
from libcpp.algorithm cimport sort
from libcpp.unordered_map cimport unordered_map


cdef extern from "single_objective_ga.h" namespace "algorithms" nogil:
    cdef cppclass SingleObjectiveGA[Individual, FitnessFunctor]:
        SingleObjectiveGA()
        SingleObjectiveGA(FitnessFunctor fitness, int seed, int num_threads)
        void Init(
            int popsize,
            int starting_length,
            double p_xo,
            double p_gene_swap,
            
            int num_products,
            int num_usp_suites,
            
            double p_product_mut,
            double p_usp_suite_mut,
            double p_plus_batch_mut,
            double p_minus_batch_mut
        )
        void Update()
        Individual Top()
        Individual Top(vector[Individual])
        
        
cdef extern from "single_objective_individual.h" namespace "types":
    cdef struct SingleObjectiveIndividual:
        SingleObjectiveIndividual()
        double objective, constraint
        
        
cdef extern from "campaign.h" namespace "types":
    cdef struct Campaign:
        Campaign()
        int suite, product, batches
        double start, end
        
        
cdef extern from "fitness.h" namespace "deterministic" nogil:
    cdef cppclass Lakhdar2005Ex1Model:
        struct Objectives:
            Objectives()
            double profit
            double sales
            double backlog_cost
            double changeover_cost
            double production_cost
            double usp_storage_cost
            double dsp_storage_cost
            double usp_waste_cost
            double dsp_waste_cost

        Lakhdar2005Ex1Model()
        
        Lakhdar2005Ex1Model(
            int num_usp_suites,
            int num_dsp_suites,
            
            vector[vector[int]] demand,
            vector[int] days_per_period,
            
            vector[double] usp_storage_cost,
            vector[double] sales_price,
            vector[double] production_cost,
            vector[double] waste_disposal_cost,
            vector[double] dsp_storage_cost,
            vector[double] backlog_penalty,
            vector[double] changeover_cost,
            
            vector[double] usp_days,
            vector[double] usp_lead_days,
            vector[double] usp_shelf_life,
            vector[double] usp_storage_cap,
            
            vector[double] dsp_days,
            vector[double] dsp_lead_days,
            vector[double] dsp_shelf_life,
            vector[double] dsp_storage_cap
        )

        unordered_map[int, vector[Campaign]] CreateUSPSchedule(SingleObjectiveIndividual&)
        unordered_map[int, vector[Campaign]] CreateDSPSchedule(unordered_map[int, vector[Campaign]]& usp_schedule)
        
        Objectives CalculateObjectives(
            unordered_map[int, vector[Campaign]]& usp_schedule,
            unordered_map[int, vector[Campaign]]& dsp_schedule,
            vector[vector[int]]& inventory,
            vector[vector[int]]& sold,
            vector[vector[int]]& dsp_waste,
            vector[vector[int]]& backlog
        )