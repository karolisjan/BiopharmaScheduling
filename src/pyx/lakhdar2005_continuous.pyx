import numpy as np
import pandas as pd
from tqdm import tqdm

from libcpp.vector cimport vector
from libcpp.algorithm cimport sort
from libcpp.unordered_map cimport unordered_map


cdef extern from "../cpp/lakhdar2005_continuous/single_objective_ga.h" namespace "algorithms" nogil:
    cdef cppclass SingleObjectiveGA[Individual, FitnessFunctor]:
        SingleObjectiveGA()
        SingleObjectiveGA(FitnessFunctor fitness, int seed)
        void Init(
            int popsize,
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
        
        
cdef extern from "../cpp/lakhdar2005_continuous/single_objective_individual.h" namespace "types":
    cdef struct SingleObjectiveIndividual:
        SingleObjectiveIndividual()
        double objective, constraint
        
        
cdef extern from "../cpp/lakhdar2005_continuous/campaign.h" namespace "types":
    cdef struct Campaign:
        Campaign()
        int suite, product, batches
        double start, end
        
        
cdef extern from "../cpp/lakhdar2005_continuous/fitness.h" nogil:
    cdef cppclass Fitness:
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
        Fitness()
        Fitness(
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
               
class Base:
    def __init__(
            self,
            int num_runs=10,
            int num_gens=1000,
            int popsize=100,
            double p_xo=0.131266,
            double p_product_mut=0.131266,
            double p_usp_suite_mut=0.131266,
            double p_plus_batch_mut=0.131266,
            double p_minus_batch_mut=0.131266,
            double p_gene_swap=0.131266,
            int seed=-1
    ):

        self._num_runs = num_runs
        self._num_gens = num_gens
        self._popsize = popsize
        self._p_xo = p_xo
        self._p_product_mut = p_product_mut
        self._p_usp_suite_mut = p_usp_suite_mut
        self._p_plus_batch_mut = p_plus_batch_mut
        self._p_minus_batch_mut = p_minus_batch_mut
        self._p_gene_swap = p_gene_swap
        self._seed = seed
        
        
class Example1Model(Base): 
    def fit(
        self,
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
        ):

        cdef:
            int gen, run, num_products = demand.size(), num_periods = demand[0].size()
            
            vector[SingleObjectiveIndividual] solutions

            Fitness fitness_functor = Fitness(
                num_usp_suites,
                num_dsp_suites,
                
                demand,
                days_per_period,
                
                usp_storage_cost,
                sales_price,
                production_cost,
                waste_disposal_cost,
                dsp_storage_cost,
                backlog_penalty,
                changeover_cost,
                
                usp_days,
                usp_lead_days,
                usp_shelf_life,
                usp_storage_cap,
                
                dsp_days,
                dsp_lead_days,
                dsp_shelf_life,
                dsp_storage_cap
            )

            SingleObjectiveGA[SingleObjectiveIndividual, Fitness] single_objective_ga = \
                SingleObjectiveGA[SingleObjectiveIndividual, Fitness](
                    fitness_functor,
                    self._seed
            )

        with tqdm(total=self._num_runs * self._num_gens) as pbar:
            for run in range(self._num_runs):
                single_objective_ga.Init(
                    self._popsize,

                    self._p_xo,
                    self._p_gene_swap,
                    
                    num_products,
                    num_usp_suites,
                    
                    self._p_product_mut,
                    self._p_usp_suite_mut,
                    self._p_plus_batch_mut,
                    self._p_minus_batch_mut,                    
                )
                
                for gen in range(self._num_gens):
                    single_objective_ga.Update()
                    pbar.update()
                    
                tqdm.write("Run: %d, Best: %.2f" % (run + 1, single_objective_ga.Top().objective));
                solutions.push_back(single_objective_ga.Top())
                
        cdef:
            int i, best = 0
            double max_ = solutions[0].objective

        for i in range(1, solutions.size()):
            if solutions[i].objective > max_:
                best = i
                max_ = solutions[i].objective
                
        tqdm.write("Best solution found -> Profit: %.2f, Backlog penalty: %.2f" 
            % (solutions[best].objective, solutions[best].constraint))

        cdef:
            unordered_map[int, vector[Campaign]] usp_schedule = fitness_functor.CreateUSPSchedule(solutions[best])
            unordered_map[int, vector[Campaign]] dsp_schedule = fitness_functor.CreateDSPSchedule(usp_schedule)
            vector[vector[int]] inventory, sold, dsp_waste, backlog
            Fitness.Objectives objectives = fitness_functor.CalculateObjectives(
                usp_schedule,
                dsp_schedule,
                inventory,
                sold,
                dsp_waste,
                backlog
            )
                      
        self.__schedule = []        
        for i in range(1, num_usp_suites + 1):
            for j in range(usp_schedule[i].size()):
                self.__schedule.append(
                    dict(
                        Task='USP Suite i%d' % i,
                        Resource='Product p%d' % usp_schedule[i][j].product,
                        Batches=usp_schedule[i][j].batches,
                        Start=usp_schedule[i][j].start,
                        Finish=usp_schedule[i][j].end
                    ))
                    
        for i in range(num_usp_suites + 1, num_usp_suites + num_dsp_suites + 1):
            for j in range(dsp_schedule[i].size()):
                self.__schedule.append(
                    dict(
                        Task='DSP Suite i%d' % (i - num_usp_suites),
                        Resource='Product p%d' % dsp_schedule[i][j].product,
                        Batches=dsp_schedule[i][j].batches,
                        Start=dsp_schedule[i][j].start,
                        Finish=dsp_schedule[i][j].end
                    ))
                    
        self.__objectives = dict(
            profit=objectives.profit,
            sales=objectives.sales,
            backlog_cost=objectives.backlog_cost,
            changeover_cost=objectives.changeover_cost,
            production_cost=objectives.production_cost,
            dsp_storage_cost=objectives.dsp_storage_cost,
            dsp_waste_cost=objectives.dsp_waste_cost
            )
            
        self.__inventory_profile = np.array([[inventory[i][j] for j in range(num_periods)] for i in range(num_products)]) 
        self.__sales_profile = np.array([[sold[i][j] for j in range(num_periods)] for i in range(num_products)]) 
        self.__backlog_profile = np.array([[backlog[i][j] for j in range(num_periods)] for i in range(num_products)]) 
        self.__waste_profile = np.array([[dsp_waste[i][j] for j in range(num_periods)] for i in range(num_products)]) 

        return self

    @property
    def schedule(self):
        df = pd.DataFrame(self.__schedule)
        df.index = df['Task']
        del df['Task']  
        return df[['Resource', 'Batches', 'Start', 'Finish']] 
        
    @property
    def objectives(self):
        return self.__objectives
        
    @property
    def inventory_profile(self):
        return self.__inventory_profile
        
    @property
    def sales_profile(self):
        return self.__sales_profile

    @property
    def backlog_profile(self):
        return self.__backlog_profile
           
    @property
    def waste_profile(self):
        return self.__waste_profile   
        

        