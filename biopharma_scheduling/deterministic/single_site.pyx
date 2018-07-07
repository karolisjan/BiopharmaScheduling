import numpy as np
import pandas as pd
from tqdm import tqdm
from collections import OrderedDict

from libcpp.utility cimport pair
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.algorithm cimport sort
from libcpp.unordered_map cimport unordered_map

from ..nsgaii cimport NSGAII
from ..nsgaii_chromosome cimport NSGAChromosome
from ..single_objective_ga cimport SingleObjectiveGA
from ..single_objective_chromosome cimport SingleObjectiveChromosome
from ..gene cimport SingleSiteSimpleGene, SingleSiteMultiSuiteGene

from single_site cimport (
    OBJECTIVES, 
    SingleSiteSimpleInputData, 
    SingleSiteMultiSuiteInputData,
    SingleSiteSimpleModel,
    SingleSiteMultiSuiteModel
)

from ..schedule cimport SingleSiteSimpleSchedule, SingleSiteMultiSuiteSchedule

from ..pyhv import hypervolume
from ..pyschedule import PySingleSiteSimpleSchedule, PySingleSiteMultiSuiteSchedule


cdef class SingleSiteSimple:
    '''
        Continuous-time capacity planning of a single multi-product
        biopharmaceutical facility using a Deterministic Single/Multi-Objective 
        Genetic Algorithm.

        See 'SingleSiteSimple.AVAILABLE_OBJECTIVES' for the objectives
        and constraints available in this model.

        Constraints take the priority over the objectives.
    '''
    cdef:
        SingleSiteSimpleInputData input_data
        SingleSiteSimpleModel single_site_simple

        object history
        object schedules
        object start_date
        object product_labels
        object due_dates
        object objectives
        object objectives_coefficients_list

        int num_runs
        int num_gens
        int popsize
        int starting_length
        int num_threads
        int random_state
        int verbose
        int save_history

        double p_xo
        double p_product_mut
        double p_plus_batch_mut
        double p_minus_batch_mut
        double p_gene_swap

    AVAILABLE_OBJECTIVES = {
        'total_kg_inventory_deficit',
        'total_kg_throughput',
        'total_kg_backlog',
        'total_kg_supply',
        'total_kg_waste',
        'total_inventory_penalty',
        'total_backlog_penalty',
        'total_production_cost',
        'total_storage_cost',
        'total_waste_cost',
        'total_revenue',
        'total_profit',
        'total_cost',
    }

    def __init__(
        self,
        num_runs: int=10,
        num_gens: int=1000,
        popsize: int=100,
        starting_length: int=1,
        p_xo: float=0.820769,
        p_product_mut: float=0.141214,
        p_plus_batch_mut: float=0.121224,
        p_minus_batch_mut: float=0.213939,
        p_gene_swap: float=0.766782,
        num_threads: int=1,
        random_state: int=None,
        verbose: bool=False,
        save_history: bool=False,
    ):
        '''
            PARAMETERS:

                num_runs: int, default 10
                    Number of times the genetic algorithm will be run with different seeds
                    for the random number generator. The population is re-generated before 
                    the start of each run.

                num_gens: int, default 1000
                    Number of generations of the genetic algorithm for a single run.

                popsize: int, default 100
                    Number of candidate chromosomes.

                starting_length: int, default 1
                    Starting length of candidate chromosome, i.e. number of genes.

                p_xo: float, default 0.820769
                    Uniform crossover probability [0.0 - 1.0].

                p_product_mut: float, default 0.141214
                    Product label mutation probability (per campaign) [0.0 - 1.0].

                p_plus_batch_mut: float, default 0.121224
                    Probability (per campaign) of increasing the number of batches by 1 [0.0 - 1.0].

                p_minus_batch_mut: float, default 0.213939
                    Probability (per campaign) of decreasing the number of batches by 1 [0.0 - 1.0].

                p_gene_swap: float, default 0.766782
                    Probability (per chromosome) of swapping two genes within the chromosome 
                    at random [0.0 - 1.0].

                num_threads: int, optional, default 1
                    Number of threads to use for evaluating the chromosome SingleSiteSimple. 
                    If num_threads = -1, all CPUs are used. If num_threads = 0 or 1, 1 CPU is used.

                random_state, int, optional, default None
                    If int, random_state is the seed used by the random number generator.
                
                verbose: bool, default False
                    If True, will print out the progress of the algorithm and display a progress bar.

                save_history: bool, default False
                    If True, will save best solution(s) from each GA run.

        '''
        assert num_runs >= 1, "'num_runs' needs to be a positive integer number." 
        self.num_runs = num_runs

        assert num_gens >= 1, "'num_gens' needs to be a positive integer number." 
        self.num_gens = num_gens

        assert popsize >= 1, "'popsize' needs to be a positive integer number." 
        self.popsize = popsize

        assert starting_length >= 1, "'starting_length' needs to be a positive integer number." 
        self.starting_length = starting_length

        assert p_xo >= 0.0 and p_xo <= 1.0, \
        "'p_xo' needs to be a positive floating point number in range [0.0 - 1.0]."
        self.p_xo = p_xo

        assert p_product_mut >= 0.0 and p_product_mut <= 1.0, \
        "'p_product_mut' needs to be a positive floating point number in range [0.0 - 1.0]."
        self.p_product_mut = p_product_mut

        assert p_plus_batch_mut >= 0.0 and p_plus_batch_mut <= 1.0, \
        "'p_plus_batch_mut' needs to be a positive floating point number in range [0.0 - 1.0]."
        self.p_plus_batch_mut = p_plus_batch_mut

        assert p_minus_batch_mut >= 0.0 and p_minus_batch_mut <= 1.0, \
        "'p_minus_batch_mut' needs to be a positive floating point number in range [0.0 - 1.0]."
        self.p_minus_batch_mut = p_minus_batch_mut

        assert p_gene_swap >= 0.0 and p_gene_swap <= 1.0, \
        "'p_gene_swap' needs to be a positive floating point number in range [0.0 - 1.0]."
        self.p_gene_swap = p_gene_swap

        self.num_threads = num_threads
        self.random_state = random_state if random_state else -1
        self.verbose = verbose
        self.save_history = save_history

        self.objectives = {
            'total_kg_inventory_deficit': OBJECTIVES.TOTAL_KG_INVENTORY_DEFICIT,
            'total_kg_throughput': OBJECTIVES.TOTAL_KG_THROUGHPUT,
            'total_kg_backlog': OBJECTIVES.TOTAL_KG_BACKLOG,
            'total_kg_supply': OBJECTIVES.TOTAL_KG_SUPPLY,
            'total_kg_waste': OBJECTIVES.TOTAL_KG_WASTE,
            'total_inventory_penalty': OBJECTIVES.TOTAL_INVENTORY_PENALTY,
            'total_backlog_penalty': OBJECTIVES.TOTAL_BACKLOG_PENALTY,
            'total_production_cost': OBJECTIVES.TOTAL_PRODUCTION_COST,
            'total_storage_cost': OBJECTIVES.TOTAL_STORAGE_COST,
            'total_waste_cost': OBJECTIVES.TOTAL_WASTE_COST,
            'total_revenue': OBJECTIVES.TOTAL_REVENUE,
            'total_profit': OBJECTIVES.TOTAL_PROFIT,
            'total_cost': OBJECTIVES.TOTAL_COST
        }

    def fit(
        self,
        start_date: str,
        objectives: dict,
        kg_demand: pd.core.frame.DataFrame,
        product_data: pd.core.frame.DataFrame,
        changeover_days: pd.core.frame.DataFrame,
        kg_inventory_target: pd.core.frame.DataFrame=None,
        constraints: dict=None,
    ):
        '''
            Runs the deterministic multi-objective genetic algorithm and generates optimal schedule(s)
            based on the given objectives, constraints, and input data. Returns an instance of 
            'SingleSiteSimple'

            INPUT:

                start_date: str
                    Start date of the schedule. Must be in the '%Y-%d-%m' format and earlier than the
                    earliest product demand due date.

                objectives: dict
                    Objectives to optimise with a corresponding coefficient indicating whether to 
                    maximise (1) or minimise (-1). Choose from AVAILABLE_OBJECTIVES.

                    If 'total_kg_inventory_deficit' is passed as an objective then 
                    'kg_inventory_target' needs to be provided as well.

                kg_demand: pd.core.frame.DataFrame
                    The DataFrame should have an index named as 'date' with due dates 
                    in the '%Y-%d-%m' format. The remaining columns should be named after the 
                    product and contain the amount in kg due at the corresponding date from 
                    the 'Dates' column. For example:

                          date    A    B    C     D
                    2017-01-01  0.0  0.0  0.0   0.0
                    2017-02-01  0.0  0.0  0.0   5.5
                    2017-03-01  3.1  0.0  0.0   5.5
                    2017-04-01  0.0  0.0  0.0   0.0
                    2017-05-01  0.0  0.0  0.0   5.5 
                    ...

                product_data: pd.core.frame.DataFrame
                    Pandas DataFrame containing the following columns:
                        'product': product labels (have to match the 'kg_demand' and 
                            'kg_inventory_target').
                        'production_cost_per_kg'
                        'sell_price_per_kg'
                        'inventory_penalty_per_kg': penalty applied whenever inventory level
                            is below 'kg_inventory_target'.
                        'backlog_penalty_per_kg'
                        'storage_cost_per_kg'
                        'waste_cost_per_kg'
                        'inoculation_days': number of days to prepare inoculum for 1 batch.
                        'seed_days': number of days to prepare seed for 1 batch.
                        'production_days': number of days at the main bioreactor.
                        'usp_days': number of USP days per 1 batch.
                        'dsp_days': number of DSP days per 1 batch.
                        'approval_days': number of days it takes to approve 1 batch.
                        'shelf_life_days': shelf-life of 1 batch.
                        'min_batches_per_campaign'
                        'max_batches_per_campaign'
                        'batches_multiples_of_per_campaign': constraint the campaign to produce
                            batches in multiples of a given number
                        'kg_yield_per_batch': kg yield per 1 batch.
                        'kg_storage_limits': max kg of a specific product that can be stored in 
                            the facility. 
                        'kg_opening_stock': available kg before the start of the schedule.

                    For example:

                    product  price_per_g  cog  usp_days  dsp_days  approval_days  
                          A            2   10        45         7             90
                          B            2   12        36        11             90
                          C            2   11        45         7             90
                          D            2    9        49         7             90

                    shelf_life_days  kg_yield_per_batch  kg_storage_limits  kg_opening_stock
                               1000                 3.1               1000              18.6
                               1000                 6.2               1000               0.0
                               1000                 4.9               1000              19.6
                               1000                 5.5               1000             110.0 
                    ...
                               
                changeover_days: pd.core.frame.DataFrame
                    Pandas DataFrame containing table product-dependent changeover days. 
                    The DataFrame needs to contain a column labelled 'product' with product 
                    labels indicating a switch FROM product. The remaining columns have to be
                    named after the products indicating a switch TO product. For example:

                    product   A   B   C   D
                          A   0  20  20  20
                          B  20   0  20  20
                          C  20  20   0  20
                          D  20  20  20   0

                kg_inventory_target: pd.core.frame.DataFrame, optional
                    Only used when 'total_kg_inventory_deficit' was specified in either 'objectives' 
                    or  'constraints'. Needs to have the same format as 'kg_demand', i.e. needs to
                    have an index named as 'date' with due dates in the '%Y-%d-%m' format. The 
                    remaining  columns should be named after the product and contain the amount in kg. 
                    The 'date' column and needs to match the 'date' column from the 'kg_demand'
                    DataFrame. The product labels need also to be the same as in the 'kg_demand'.
                    For example:

                          date     A    B     C     D
                    2017-01-01   6.2  0.0   0.0  22.0
                    2017-02-01   6.2  0.0   4.9  27.5
                    2017-03-01   9.3  0.0   9.8  27.5
                    2017-04-01   9.3  0.0   9.8  27.5
                    2017-05-01  12.4  0.0   9.8  27.5     

                constraints: dict, optional
                    Constraints take the priority over the objectives. Choose from AVAILABLE_OBJECTIVES.
                    
                    Example:
                    
                    'total_kg_backlog <= 0' (total kg backlog must not exceed 0)

                    {
                        'total_kg_backlog': [-1, 0] 
                    }        

                    i.e. 'constraint': [-1 if <= or 1 if >=, bound]
        '''
        self.__validate_input(
            objectives,
            constraints,
            kg_demand,
            kg_inventory_target,
            product_data, 
            changeover_days
        )

        days_per_period = self.__count_days(start_date, kg_demand.index.values.tolist())

        self.objectives_coefficients_list = []
        cdef unordered_map[OBJECTIVES, int] cpp_objectives
        for obj, coef in objectives.items():
            cpp_objectives[self.objectives[obj]] = coef
            self.objectives_coefficients_list.append((obj, coef))

        cdef pair[int, double] p
        cdef unordered_map[OBJECTIVES, pair[int, double]] cpp_constraints
        if constraints:
            for cons, [coef, bound] in constraints.items():
                p.first = coef
                p.second = bound
                cpp_constraints[self.objectives[cons]] = p 

        cdef vector[vector[double]] cpp_kg_inventory_target
        if kg_inventory_target is not None:
            cpp_kg_inventory_target = kg_inventory_target.fillna(0).transpose().values.tolist()

        self.input_data = SingleSiteSimpleInputData(
            cpp_objectives,
            kg_demand.fillna(0).transpose().values.tolist(),
            days_per_period,

            product_data.kg_opening_stock.fillna(0).values.tolist(),
            product_data.kg_yield_per_batch.fillna(0).values,
            product_data.kg_storage_limits.fillna(0).values.tolist(),

            product_data.inventory_penalty_per_kg.fillna(0).values.tolist(),
            product_data.backlog_penalty_per_kg.fillna(0).values.tolist(),
            product_data.production_cost_per_kg.fillna(0).values.tolist(),
            product_data.storage_cost_per_kg.fillna(0).values.tolist(),
            product_data.waste_cost_per_kg.fillna(0).values.tolist(),
            product_data.sell_price_per_kg.fillna(0).values.tolist(),

            product_data.inoculation_days.fillna(0).values.tolist(),
            product_data.seed_days.fillna(0).values.tolist(),
            product_data.production_days.fillna(0).values.tolist(),
            product_data.usp_days.fillna(0).values.tolist(),
            product_data.dsp_days.fillna(0).values.tolist(),
            product_data.approval_days.fillna(0).values.tolist(),
            product_data.shelf_life_days.fillna(0).values.tolist(),
            product_data.min_batches_per_campaign.fillna(0).values.tolist(),
            product_data.max_batches_per_campaign.fillna(0).values.tolist(),
            product_data.batches_multiples_of_per_campaign.fillna(0).values.tolist(),
            changeover_days.drop('product', axis=1).fillna(0).values.tolist(),

            &cpp_kg_inventory_target if kg_inventory_target is not None else NULL,
            &cpp_constraints if constraints is not None else NULL 
        )

        self.single_site_simple = SingleSiteSimpleModel(self.input_data)

        if len(objectives) == 1:
            self.__run_single_objective_ga()
        else:
            self.__run_nsgaii()

        return self

    def __validate_input(
        self,
        objectives: dict, 
        constraints: dict, 
        kg_demand: pd.core.frame.DataFrame, 
        kg_inventory_target: pd.core.frame.DataFrame, 
        product_data: pd.core.frame.DataFrame, 
        changeover_days: pd.core.frame.DataFrame,
    ):
        assert type(objectives) is dict, \
        "'objectives' needs to be a 'dict', is a '{}'.".format(type(objectives))
        for obj in objectives:
            assert obj in self.AVAILABLE_OBJECTIVES, \
            "'{}' is not allowed as an objective.".format(obj)
            assert objectives[obj] in [-1, 1], \
            "Objective coefficient can only be -1 or 1."

        if constraints is not None:
            assert type(constraints) is dict, \
            "'constraints' needs to be a 'dict', is a '{}'.".format(type(constraints))
            for cons in constraints:
                assert cons in self.AVAILABLE_OBJECTIVES, \
                "'{}' is not allowed as a constraint.".format(cons)
                assert type(constraints[cons]) is list and len(constraints[cons]) == 2, \
                "'constraints' are expected to hold a coeffcient and a bound."
                assert constraints[cons][0] in [-1, 1], \
                "Constraint coefficient can only be -1 or 1."

        for df in [kg_demand, product_data, changeover_days]:
            assert type(df) is pd.core.frame.DataFrame, \
            "Input data must be a 'pd.core.frame.DataFrame', is a '{}'".format(type(df))

        assert 'date' == kg_demand.index.name, \
        "'kg_demand' must have a 'date' index." 

        if kg_inventory_target is not None:
            assert type(kg_inventory_target) is pd.core.frame.DataFrame, \
            "Input data must be a 'pd.core.frame.DataFrame', is a '{}'".format(type(kg_inventory_target))
            assert 'date' == kg_inventory_target.index.name, \
            "'kg_inventory_target' must have a 'date' index." 
            assert kg_demand.index.all() == kg_inventory_target.index.all(), \
            "'date' indices from 'kg_demand' and 'kg_inventory_target' do not match."
            assert set(kg_demand.columns) == set(kg_inventory_target.columns) and \
                   len(kg_demand.columns) == len(kg_inventory_target.columns), \
                   "Product labels from 'kg_demand' and 'kg_inventory_target' do not match."

        for col in [
            'product', 
            'inventory_penalty_per_kg', 
            'backlog_penalty_per_kg',
            'production_cost_per_kg',
            'storage_cost_per_kg',
            'waste_cost_per_kg',
            'sell_price_per_kg',
            'inoculation_days',
            'seed_days',
            'production_days',
            'usp_days',
            'dsp_days',
            'approval_days',
            'shelf_life_days',
            'kg_yield_per_batch',
            'kg_storage_limits',
            'kg_opening_stock',
            'min_batches_per_campaign',
            'max_batches_per_campaign',
            'batches_multiples_of_per_campaign'
        ]:
            assert col in product_data, "'product_data' is missing '{}' column.".format(col)
        
        self.product_labels = kg_demand.columns.values.tolist()

        assert len(self.product_labels) == len(product_data['product']) and \
               set(self.product_labels) == set(product_data['product']), \
               "Product labels from 'kg_demand' and 'product_data' do not match."

        assert 'product' in changeover_days, "'product' column is missing in 'changeover_days'."
        assert len(self.product_labels) == len(changeover_days['product']) and \
               set(self.product_labels) == set(changeover_days['product']), \
               "Product labels from 'kg_demand' and 'changeover_days' do not match."

        changeover_days_product_columns = changeover_days.columns.values.tolist()
        changeover_days_product_columns.remove('product')

        assert len(self.product_labels) == len(changeover_days_product_columns) and \
               set(self.product_labels) == set(changeover_days_product_columns), \
               "Product labels in 'product' column do not match with the actual product columns in 'changeover_days_product_columns'."

    def __count_days(self, start_date: str, due_dates: list):
        self.start_date = start_date
        self.due_dates = due_dates
        start_date = pd.to_datetime(start_date)
        due_dates = pd.to_datetime(due_dates)
        days_per_period = [(due_dates[0] - start_date).days]
        for i in range(1, len(due_dates), 1):
            days_per_period.append((due_dates[i] - due_dates[i - 1]).days)
        return days_per_period

    def __run_single_objective_ga(self):
        cdef:
            SingleSiteSimpleSchedule schedule
            SingleObjectiveChromosome[SingleSiteSimpleGene] top_solution
            vector[SingleObjectiveChromosome[SingleSiteSimpleGene]] solutions 
            
            SingleObjectiveGA[SingleObjectiveChromosome[SingleSiteSimpleGene], SingleSiteSimpleModel] ga = \
                SingleObjectiveGA[SingleObjectiveChromosome[SingleSiteSimpleGene], SingleSiteSimpleModel](
                self.single_site_simple,
                self.random_state,
                self.num_threads   
            )

        if self.verbose: 
            pbar = tqdm(total=self.num_runs * self.num_gens)

        for run in range(self.num_runs):
            if self.verbose: 
                pbar.set_description('GA is running %d/%d' % (run + 1, self.num_runs))

            ga.Init(
                self.popsize,
                self.starting_length,
                self.p_xo,
                self.p_gene_swap,
                len(self.product_labels),
                self.p_product_mut,
                self.p_plus_batch_mut,
                self.p_minus_batch_mut,
            )

            for gen in range(self.num_gens):
                ga.Update()

                if self.verbose: 
                    pbar.update()

            top_solution = ga.Top()
            solutions.push_back(top_solution)

        if self.verbose and self.save_history:
            pbar.set_description('Processing history')

        if self.save_history:
            self.history = []
            for solution in solutions:
                schedule = SingleSiteSimpleSchedule()
                self.single_site_simple.CreateSchedule(solution, schedule)
                self.history.append(self.__make_pyschedule(schedule))

        if self.verbose:
            pbar.set_description('Collecting schedules')

        self.schedules = []
        top_solution = ga.Top(solutions)
        solutions.resize(1)
        solutions[0] = top_solution
        for solution in solutions:
            schedule = SingleSiteSimpleSchedule()
            self.single_site_simple.CreateSchedule(solution, schedule)
            self.schedules.append(self.__make_pyschedule(schedule))

        if self.verbose: 
            pbar.set_description('Done')
            pbar.close()

    def __run_nsgaii(self):
        cdef:
            SingleSiteSimpleSchedule schedule
            vector[vector[NSGAChromosome[SingleSiteSimpleGene]]] history
            vector[NSGAChromosome[SingleSiteSimpleGene]] solutions, top_front
            
            NSGAII[NSGAChromosome[SingleSiteSimpleGene], SingleSiteSimpleModel] nsgaii = \
                NSGAII[NSGAChromosome[SingleSiteSimpleGene], SingleSiteSimpleModel](
                self.single_site_simple,
                self.random_state,
                self.num_threads   
            )

        if self.verbose: 
            pbar = tqdm(total=self.num_runs * self.num_gens)

        for run in range(self.num_runs):
            if self.verbose: 
                pbar.set_description('GA is running %d/%d' % (run + 1, self.num_runs))

            nsgaii.Init(
                self.popsize,
                self.starting_length,
                self.p_xo,
                self.p_gene_swap,
                len(self.product_labels),
                self.p_product_mut,
                self.p_plus_batch_mut,
                self.p_minus_batch_mut,
            )

            for gen in range(self.num_gens):
                nsgaii.Update()

                if self.verbose: 
                    pbar.update()

            top_front = nsgaii.TopFront()
            solutions.insert(solutions.end(), top_front.begin(), top_front.end())

            if self.save_history:
                history.push_back(top_front)

        if self.verbose and self.save_history:
            pbar.set_description('Processing history')

        if self.save_history:
            self.history = []
            for front in history:
                self.history.append([])
                for solution in front:
                    schedule = SingleSiteSimpleSchedule()
                    self.single_site_simple.CreateSchedule(solution, schedule)
                    self.history[-1].append(self.__make_pyschedule(schedule))

        if self.verbose:
            pbar.set_description('Collecting schedules')

        self.schedules = []
        solutions = nsgaii.TopFront(solutions)
        for solution in solutions:
            schedule = SingleSiteSimpleSchedule()
            self.single_site_simple.CreateSchedule(solution, schedule)
            self.schedules.append(self.__make_pyschedule(schedule))

        if self.verbose: 
            pbar.set_description('Done')
            pbar.close()

    cdef __make_pyschedule(self, SingleSiteSimpleSchedule &schedule):

        def get_date_of(delta):
            return pd.Timedelta('%d days' % delta) + pd.to_datetime(self.start_date).date()

        campaigns_table = []
        batches_table = []
        tasks_table = []
        
        for campaign in schedule.campaigns: 
            campaigns_table.append(OrderedDict([
                ('Product', self.product_labels[campaign.product_num - 1]),
                ('Batches', campaign.num_batches),
                ('Kg', campaign.kg),
                ('Start', get_date_of(campaign.start)),
                ('First Harvest', get_date_of(campaign.first_harvest)),
                ('First Batch', get_date_of(campaign.first_batch)),
                ('Last Batch', get_date_of(campaign.last_batch))
            ])) 

            for batch in campaign.batches:
                batches_table.append(OrderedDict([
                    ('Product', self.product_labels[batch.product_num - 1]),
                    ('Kg', batch.kg),
                    ('Start', get_date_of(batch.start)),
                    ('Harvested on', get_date_of(batch.harvested_at)),
                    ('Stored on', get_date_of(batch.stored_at)),
                    ('Expires on', get_date_of(batch.expires_at)),
                    ('Approved on', get_date_of(batch.approved_at))
                ]))

                tasks_table.append(OrderedDict([
                    ('Product', self.product_labels[campaign.product_num - 1]),
                    ('Task', 'Inoculation'),
                    ('Start', batches_table[-1]['Start']),
                    (
                        'Finish', 
                        batches_table[-1]['Start'] + pd.Timedelta(
                            '%d days' % 
                            self.input_data.inoculation_days[batch.product_num - 1]
                        )
                    )
                ]))
                tasks_table.append(OrderedDict([
                    ('Product', self.product_labels[campaign.product_num - 1]),
                    ('Task', 'Seed'),
                    ('Start', tasks_table[-1]['Finish']),
                    (
                        'Finish', 
                        tasks_table[-1]['Finish'] + pd.Timedelta(
                            '%d days' % 
                            self.input_data.seed_days[batch.product_num - 1]
                        )
                    )                
                ]))
                tasks_table.append(OrderedDict([
                    ('Product', self.product_labels[campaign.product_num - 1]),
                    ('Task', 'Production'),
                    ('Start', tasks_table[-1]['Finish']),
                    (
                        'Finish', 
                        tasks_table[-1]['Finish'] + pd.Timedelta(
                            '%d days' % 
                            self.input_data.production_days[batch.product_num - 1]
                        )
                    )                
                ]))
                tasks_table.append(OrderedDict([
                    ('Product', self.product_labels[campaign.product_num - 1]),
                    ('Task', 'DSP'),
                    ('Start', tasks_table[-1]['Finish']),
                    (
                        'Finish', 
                        tasks_table[-1]['Finish'] + pd.Timedelta(
                            '%d days' % 
                            self.input_data.dsp_days[batch.product_num - 1]
                        )
                    )                
                ]))

        kg_inventory, kg_backlog, kg_supply, kg_waste = [], [], [], []

        for i, due_date in enumerate(self.due_dates):
            kg_inventory.append({
                product_label: schedule.kg_inventory[j][i] 
                for j, product_label in enumerate(self.product_labels)
            })
            kg_inventory[-1].update({'date': due_date})

            kg_backlog.append({
                product_label: schedule.kg_backlog[j][i] 
                for j, product_label in enumerate(self.product_labels)
            })
            kg_backlog[-1].update({'date': due_date})

            kg_supply.append({
                product_label: schedule.kg_supply[j][i] 
                for j, product_label in enumerate(self.product_labels)
            })
            kg_supply[-1].update({'date': due_date})

            kg_waste.append({
                product_label: schedule.kg_waste[j][i] 
                for j, product_label in enumerate(self.product_labels)
            })
            kg_waste[-1].update({'date': due_date})

        return PySingleSiteSimpleSchedule(
            {
                obj: schedule.objectives[self.objectives[obj]] 
                for obj in self.AVAILABLE_OBJECTIVES
            }, 
            campaigns_table,
            batches_table,
            tasks_table,
            kg_inventory,
            kg_backlog,
            kg_supply,
            kg_waste
        )

    @property
    def schedules(self):
        return self.schedules

    @property
    def history(self):
        return self.history

    def score(self, schedules: list, ref_point: dict=None, ideal_point: dict=None):
        '''
            When the number of objectives >= 2, estimates the hypervolume 
            of the top non-dominated front, otherwise - returns the max 
            objective function value of 'num_runs'.

            Utilised 'hypervolume' benchmark utility from 'deap - Distributed 
            Evolutionary Algorithms in Python' (https://github.com/DEAP/deap).

            INPUT:

                schedules: list
                    A list of PySingleSiteSimpleSchedule objects obtain from DeterministicSingleSite.schedules 
                    once the model is fit. 

                ref_point: dict, optional, default None
                    A dictionary of objective name and value pairs. It is used
                    as a reference point for hypervolume estimation. If 'ref_point'
                    is None then, the worst value for each objective +1 is used.

                    For example:

                    {
                        'total_kg_inventory_deficit': 2007.7,
                        'total_kg_throughput': 106.9
                    }

                ideal_point: dict, optional, default None
                    A dictionary of objective name and value pairs. If 'ideal_point'
                    is not None, then it is used to normalise the hypervolume in
                    range 0.0 - 1.0. 'ideal_point' is ignored if the number of 
                    objectives is 1. 
        '''
        assert type(schedules) is list and len(schedules) > 0

        if len(self.objectives_coefficients_list) == 1:
            return schedules[0].objectives[self.objectives_coefficients_list[0][0]]

        points = np.array([
            [schedule.objectives[obj].values[0] * coef * -1 for (obj, coef) in self.objectives_coefficients_list]
            for schedule in schedules
        ])

        if ref_point is not None:
            ref_point = [ref_point[obj] * coef * -1 for (obj, coef) in self.objectives_coefficients_list]
        else:
            ref_point = (np.max(points, axis=0) + 1).tolist()

        hv = hypervolume(points, ref_point)

        if ideal_point is not None:
            ideal_point = np.array([[ideal_point[obj] * coef * -1 for (obj, coef) in self.objectives_coefficients_list]])
            hv /= hypervolume(ideal_point, ref_point)

        return hv        



cdef class SingleSiteMultiSuite:
    cdef:
        SingleSiteMultiSuiteInputData input_data
        SingleSiteMultiSuiteModel single_site_multi_suite

        object history
        object schedules
        object start_date
        object product_labels
        object num_usp_suites
        object num_dsp_suites
        object due_dates
        object objectives
        object objectives_coefficients_list

        int num_runs
        int num_gens
        int popsize
        int starting_length
        int num_threads
        int random_state
        int verbose
        int save_history

        double p_xo
        double p_product_mut
        double p_usp_suite_mut
        double p_plus_batch_mut
        double p_minus_batch_mut
        double p_gene_swap

    AVAILABLE_OBJECTIVES = {
        'total_batch_throughput',
        'total_batch_backlog',
        'total_batch_supply',
        'total_batch_waste',

        'total_backlog_penalty',
        'total_changeover_cost',
        'total_production_cost',
        'total_storage_cost',
        'total_waste_cost',
        'total_revenue',
        'total_profit',
        'total_cost',
    }

    def __init__(
        self,
        num_runs: int=10,
        num_gens: int=1000,
        popsize: int=100,
        starting_length: int=1,
        p_xo: float=0.131266,
        p_product_mut: float=0.131266,
        p_usp_suite_mut: float=0.131266,
        p_plus_batch_mut: float=0.131266,
        p_minus_batch_mut: float=0.131266,
        p_gene_swap: float=0.131266,
        num_threads: int=1,
        random_state: int=None,
        verbose: bool=False,
        save_history: bool=False,
    ):
        assert num_runs >= 1, "'num_runs' needs to be a positive integer number." 
        self.num_runs = num_runs

        assert num_gens >= 1, "'num_gens' needs to be a positive integer number." 
        self.num_gens = num_gens

        assert popsize >= 1, "'popsize' needs to be a positive integer number." 
        self.popsize = popsize

        assert starting_length >= 1, "'starting_length' needs to be a positive integer number." 
        self.starting_length = starting_length

        assert p_xo >= 0.0 and p_xo <= 1.0, \
        "'p_xo' needs to be a positive floating point number in range [0.0 - 1.0]."
        self.p_xo = p_xo

        assert p_product_mut >= 0.0 and p_product_mut <= 1.0, \
        "'p_product_mut' needs to be a positive floating point number in range [0.0 - 1.0]."
        self.p_product_mut = p_product_mut

        assert p_usp_suite_mut >= 0.0 and p_usp_suite_mut <= 1.0, \
        "'p_usp_suite_mut' needs to be a positive floating point number in range [0.0 - 1.0]."
        self.p_usp_suite_mut = p_usp_suite_mut

        assert p_plus_batch_mut >= 0.0 and p_plus_batch_mut <= 1.0, \
        "'p_plus_batch_mut' needs to be a positive floating point number in range [0.0 - 1.0]."
        self.p_plus_batch_mut = p_plus_batch_mut

        assert p_minus_batch_mut >= 0.0 and p_minus_batch_mut <= 1.0, \
        "'p_minus_batch_mut' needs to be a positive floating point number in range [0.0 - 1.0]."
        self.p_minus_batch_mut = p_minus_batch_mut

        assert p_gene_swap >= 0.0 and p_gene_swap <= 1.0, \
        "'p_gene_swap' needs to be a positive floating point number in range [0.0 - 1.0]."
        self.p_gene_swap = p_gene_swap

        self.num_threads = num_threads
        self.random_state = random_state if random_state else -1
        self.verbose = verbose
        self.save_history = save_history

        self.objectives = {
            'total_batch_throughput': OBJECTIVES.TOTAL_BATCH_THROUGHPUT,
            'total_batch_backlog': OBJECTIVES.TOTAL_BATCH_BACKLOG,
            'total_batch_supply': OBJECTIVES.TOTAL_BATCH_SUPPLY,
            'total_batch_waste': OBJECTIVES.TOTAL_BATCH_WASTE,

            'total_changeover_cost': OBJECTIVES.TOTAL_CHANGEOVER_COST,
            'total_backlog_penalty': OBJECTIVES.TOTAL_BACKLOG_PENALTY,
            'total_production_cost': OBJECTIVES.TOTAL_PRODUCTION_COST,
            'total_storage_cost': OBJECTIVES.TOTAL_STORAGE_COST,
            'total_waste_cost': OBJECTIVES.TOTAL_WASTE_COST,
            'total_revenue': OBJECTIVES.TOTAL_REVENUE,
            'total_profit': OBJECTIVES.TOTAL_PROFIT,
            'total_cost': OBJECTIVES.TOTAL_COST
        }

    def fit(
        self,
        start_date: str,
        objectives: dict,
        num_usp_suites: int,
        num_dsp_suites: int,
        batch_demand: pd.core.frame.DataFrame,
        product_data: pd.core.frame.DataFrame,
        usp_changeover_days: pd.core.frame.DataFrame,
        dsp_changeover_days: pd.core.frame.DataFrame,
        constraints: dict=None,
    ):
        self.__validate_input(
            objectives,
            constraints,
            batch_demand,
            product_data, 
            usp_changeover_days,
            dsp_changeover_days
        )

        self.num_usp_suites = num_usp_suites
        self.num_dsp_suites = num_dsp_suites

        days_per_period = self.__count_days(start_date, batch_demand.index.values.tolist())

        self.objectives_coefficients_list = []
        cdef unordered_map[OBJECTIVES, int] cpp_objectives
        for obj, coef in objectives.items():
            cpp_objectives[self.objectives[obj]] = coef
            self.objectives_coefficients_list.append((obj, coef))

        cdef pair[int, double] p
        cdef unordered_map[OBJECTIVES, pair[int, double]] cpp_constraints
        if constraints:
            for cons, [coef, bound] in constraints.items():
                p.first = coef
                p.second = bound
                cpp_constraints[self.objectives[cons]] = p 

        self.input_data = SingleSiteMultiSuiteInputData(
            cpp_objectives,
            
            num_usp_suites,
            num_dsp_suites,
            
            batch_demand.fillna(0).transpose().values.tolist(),
            days_per_period,

            product_data.usp_days.fillna(0).values.tolist(),
            product_data.dsp_days.fillna(0).values.tolist(),
            
            product_data.shelf_life_days.fillna(0).values.tolist(),
            product_data.batch_storage_limits.fillna(0).values.tolist(),

            product_data.sell_price_per_batch.fillna(0).values.tolist(),
            product_data.storage_cost_per_batch.fillna(0).values.tolist(),
            product_data.backlog_penalty_per_batch.fillna(0).values.tolist(),
            product_data.waste_cost_per_batch.fillna(0).values.tolist(),
            product_data.usp_production_cost_per_batch.fillna(0).values.tolist(),
            product_data.dsp_production_cost_per_batch.fillna(0).values.tolist(),
            product_data.usp_changeover_cost.fillna(0).values.tolist(),
            product_data.dsp_changeover_cost.fillna(0).values.tolist(),

            usp_changeover_days.drop('product', axis=1).fillna(0).values.tolist(),
            dsp_changeover_days.drop('product', axis=1).fillna(0).values.tolist(),

            &cpp_constraints if constraints is not None else NULL 
        )

        self.single_site_multi_suite = SingleSiteMultiSuiteModel(self.input_data)

        if len(objectives) == 1:
            self.__run_single_objective_ga()
        else:
            self.__run_nsgaii()

        return self

    def __validate_input(
        self,
        objectives: dict, 
        constraints: dict, 
        batch_demand: pd.core.frame.DataFrame, 
        product_data: pd.core.frame.DataFrame, 
        usp_changeover_days: pd.core.frame.DataFrame,
        dsp_changeover_days: pd.core.frame.DataFrame
    ):
        assert type(objectives) is dict, \
        "'objectives' needs to be a 'dict', is a '{}'.".format(type(objectives))
        for obj in objectives:
            assert obj in self.AVAILABLE_OBJECTIVES, \
            "'{}' is not allowed as an objective.".format(obj)
            assert objectives[obj] in [-1, 1], \
            "Objective coefficient can only be -1 or 1."

        if constraints is not None:
            assert type(constraints) is dict, \
            "'constraints' needs to be a 'dict', is a '{}'.".format(type(constraints))
            for cons in constraints:
                assert cons in self.AVAILABLE_OBJECTIVES, \
                "'{}' is not allowed as a constraint.".format(cons)
                assert type(constraints[cons]) is list and len(constraints[cons]) == 2, \
                "'constraints' are expected to hold a coeffcient and a bound."
                assert constraints[cons][0] in [-1, 1], \
                "Constraint coefficient can only be -1 or 1."

        for df in [batch_demand, product_data, usp_changeover_days, dsp_changeover_days]:
            assert type(df) is pd.core.frame.DataFrame, \
            "Input data must be a 'pd.core.frame.DataFrame', is a '{}'".format(type(df))

        assert 'date' == batch_demand.index.name, \
        "'batch_demand' must have a 'date' index." 

        for col in [
            'product', 
            'usp_days',
            'dsp_days',
            'shelf_life_days',
            'batch_storage_limits',
            'sell_price_per_batch',
            'usp_production_cost_per_batch',
            'dsp_production_cost_per_batch',
            'storage_cost_per_batch',
            'waste_cost_per_batch',
            'backlog_penalty_per_batch',
            'usp_changeover_cost',
            'dsp_changeover_cost'
        ]:
            assert col in product_data, "'product_data' is missing '{}' column.".format(col)
        
        self.product_labels = batch_demand.columns.values.tolist()

        assert len(self.product_labels) == len(product_data['product']) and \
               set(self.product_labels) == set(product_data['product']), \
               "Product labels from 'batch_demand' and 'product_data' do not match."

        assert 'product' in usp_changeover_days, "'product' column is missing in 'usp_changeover_days'."
        assert len(self.product_labels) == len(usp_changeover_days['product']) and \
               set(self.product_labels) == set(usp_changeover_days['product']), \
               "Product labels from 'batch_demand' and 'usp_changeover_days' do not match."

        assert 'product' in dsp_changeover_days, "'product' column is missing in 'dsp_changeover_days'."
        assert len(self.product_labels) == len(dsp_changeover_days['product']) and \
               set(self.product_labels) == set(dsp_changeover_days['product']), \
               "Product labels from 'batch_demand' and 'dsp_changeover_days' do not match."

        usp_changeover_days_product_columns = usp_changeover_days.columns.values.tolist()
        usp_changeover_days_product_columns.remove('product')

        assert len(self.product_labels) == len(usp_changeover_days_product_columns) and \
               set(self.product_labels) == set(usp_changeover_days_product_columns), \
               "Product labels in 'product' column do not match with the actual product columns in 'usp_changeover_days_product_columns'."

        dsp_changeover_days_product_columns = dsp_changeover_days.columns.values.tolist()
        dsp_changeover_days_product_columns.remove('product')

        assert len(self.product_labels) == len(dsp_changeover_days_product_columns) and \
               set(self.product_labels) == set(dsp_changeover_days_product_columns), \
               "Product labels in 'product' column do not match with the actual product columns in 'dsp_changeover_days_product_columns'."


    def __count_days(self, start_date: str, due_dates: list):
        self.start_date = start_date
        self.due_dates = due_dates
        start_date = pd.to_datetime(start_date)
        due_dates = pd.to_datetime(due_dates)
        days_per_period = [(due_dates[0] - start_date).days]
        for i in range(1, len(due_dates), 1):
            days_per_period.append((due_dates[i] - due_dates[i - 1]).days)
        return days_per_period

    def __run_single_objective_ga(self):
        cdef:
            SingleSiteMultiSuiteSchedule schedule
            SingleObjectiveChromosome[SingleSiteMultiSuiteGene] top_solution
            vector[SingleObjectiveChromosome[SingleSiteMultiSuiteGene]] solutions 
            
            SingleObjectiveGA[SingleObjectiveChromosome[SingleSiteMultiSuiteGene], SingleSiteMultiSuiteModel] ga = \
                SingleObjectiveGA[SingleObjectiveChromosome[SingleSiteMultiSuiteGene], SingleSiteMultiSuiteModel](
                self.single_site_multi_suite,
                self.random_state,
                self.num_threads   
            )

        if self.verbose: 
            pbar = tqdm(total=self.num_runs * self.num_gens)

        for run in range(self.num_runs):
            if self.verbose: 
                pbar.set_description('GA is running %d/%d' % (run + 1, self.num_runs))

            ga.Init(
                self.popsize,
                self.starting_length,
                self.p_xo,
                self.p_gene_swap,
                len(self.product_labels),
                self.num_usp_suites,
                self.p_product_mut,
                self.p_usp_suite_mut,
                self.p_plus_batch_mut,
                self.p_minus_batch_mut,
            )

            for gen in range(self.num_gens):
                ga.Update()

                if self.verbose: 
                    pbar.update()

            top_solution = ga.Top()
            solutions.push_back(top_solution)

        if self.verbose and self.save_history:
            pbar.set_description('Processing history')

        if self.save_history:
            self.history = []
            for solution in solutions:
                schedule = SingleSiteMultiSuiteSchedule()
                self.single_site_multi_suite.CreateSchedule(solution, schedule)
                self.history.append(self.__make_pyschedule(schedule))

        if self.verbose:
            pbar.set_description('Collecting schedules')

        self.schedules = []
        top_solution = ga.Top(solutions)
        solutions.resize(1)
        solutions[0] = top_solution
        for solution in solutions:
            schedule = SingleSiteMultiSuiteSchedule()
            self.single_site_multi_suite.CreateSchedule(solution, schedule)
            self.schedules.append(self.__make_pyschedule(schedule))

        if self.verbose: 
            pbar.set_description('Done')
            pbar.close()

    def __run_nsgaii(self):
        cdef:
            SingleSiteMultiSuiteSchedule schedule
            vector[vector[NSGAChromosome[SingleSiteMultiSuiteGene]]] history
            vector[NSGAChromosome[SingleSiteMultiSuiteGene]] solutions, top_front
            
            NSGAII[NSGAChromosome[SingleSiteMultiSuiteGene], SingleSiteMultiSuiteModel] nsgaii = \
                NSGAII[NSGAChromosome[SingleSiteMultiSuiteGene], SingleSiteMultiSuiteModel](
                self.single_site_multi_suite,
                self.random_state,
                self.num_threads   
            )

        if self.verbose: 
            pbar = tqdm(total=self.num_runs * self.num_gens)

        for run in range(self.num_runs):
            if self.verbose: 
                pbar.set_description('GA is running %d/%d' % (run + 1, self.num_runs))

            nsgaii.Init(
                self.popsize,
                self.starting_length,
                self.p_xo,
                self.p_gene_swap,
                len(self.product_labels),
                self.num_usp_suites,
                self.p_product_mut,
                self.p_usp_suite_mut,
                self.p_plus_batch_mut,
                self.p_minus_batch_mut,
            )

            for gen in range(self.num_gens):
                nsgaii.Update()

                if self.verbose: 
                    pbar.update()

            top_front = nsgaii.TopFront()
            solutions.insert(solutions.end(), top_front.begin(), top_front.end())

            if self.save_history:
                history.push_back(top_front)

        if self.verbose and self.save_history:
            pbar.set_description('Processing history')

        if self.save_history:
            self.history = []
            for front in history:
                self.history.append([])
                for solution in front:
                    schedule = SingleSiteMultiSuiteSchedule()
                    self.single_site_multi_suite.CreateSchedule(solution, schedule)
                    self.history[-1].append(self.__make_pyschedule(schedule))

        if self.verbose:
            pbar.set_description('Collecting schedules')

        self.schedules = []
        solutions = nsgaii.TopFront(solutions)
        for solution in solutions:
            schedule = SingleSiteMultiSuiteSchedule()
            self.single_site_multi_suite.CreateSchedule(solution, schedule)
            self.schedules.append(self.__make_pyschedule(schedule))

        if self.verbose: 
            pbar.set_description('Done')
            pbar.close()

    cdef __make_pyschedule(self, SingleSiteMultiSuiteSchedule &schedule):

        def get_date_of(delta):
            return pd.Timedelta('%d days' % delta) + pd.to_datetime(self.start_date).date()

        campaigns_table = []
        batches_table = []

        for suite in schedule.suites: 
            for campaign in suite:

                campaigns_table.append(OrderedDict([
                    ('Product', self.product_labels[campaign.product_num - 1]),
                    ('Suite', 'USP%d' % campaign.suite_num if campaign.suite_num <= self.num_usp_suites else 'DSP%d' % (campaign.suite_num - self.num_usp_suites)),
                    ('Batches', campaign.num_batches),
                    ('Start', get_date_of(campaign.start)),
                    ('End', get_date_of(campaign.end))
                ])) 

                for batch in campaign.batches:

                    batches_table.append(OrderedDict([
                        ('Product', self.product_labels[batch.product_num - 1]),
                        ('Suite', 'USP%d' % campaign.suite_num if campaign.suite_num <= self.num_usp_suites else 'DSP%d' % (campaign.suite_num - self.num_usp_suites)),
                        ('Start', get_date_of(batch.start)),
                        ('Stored on', get_date_of(batch.stored_at)),
                        ('Expires on', get_date_of(batch.expires_at)),
                    ]))

        batch_inventory, batch_backlog, batch_supply, batch_waste = [], [], [], []

        for i, due_date in enumerate(self.due_dates):
            batch_inventory.append({
                product_label: schedule.batch_inventory[j][i] 
                for j, product_label in enumerate(self.product_labels)
            })
            batch_inventory[-1].update({'date': due_date})

            batch_backlog.append({
                product_label: schedule.batch_backlog[j][i] 
                for j, product_label in enumerate(self.product_labels)
            })
            batch_backlog[-1].update({'date': due_date})

            batch_supply.append({
                product_label: schedule.batch_supply[j][i] 
                for j, product_label in enumerate(self.product_labels)
            })
            batch_supply[-1].update({'date': due_date})

            batch_waste.append({
                product_label: schedule.batch_waste[j][i] 
                for j, product_label in enumerate(self.product_labels)
            })
            batch_waste[-1].update({'date': due_date})

        return PySingleSiteMultiSuiteSchedule(
            {
                obj: schedule.objectives[self.objectives[obj]] 
                for obj in self.AVAILABLE_OBJECTIVES
            }, 
            campaigns_table,
            batches_table,
            batch_inventory,
            batch_backlog,
            batch_supply,
            batch_waste
        )

    @property
    def schedules(self):
        return self.schedules

    @property
    def history(self):
        return self.history

    def score(self, schedules: list, ref_point: dict=None, ideal_point: dict=None):
        '''
            When the number of objectives >= 2, estimates the hypervolume 
            of the top non-dominated front, otherwise - returns the max 
            objective function value of 'num_runs'.

            Utilised 'hypervolume' benchmark utility from 'deap - Distributed 
            Evolutionary Algorithms in Python' (https://github.com/DEAP/deap).

            INPUT:

                schedules: list
                    A list of PySingleSiteMultiSuiteSchedule objects obtain from SingleSiteMultiSuite.schedules 
                    once the model is fit. 

                ref_point: dict, optional, default None
                    A dictionary of objective name and value pairs. It is used
                    as a reference point for hypervolume estimation. If 'ref_point'
                    is None then, the worst value for each objective +1 is used.

                    For example:

                    {
                        'total_kg_inventory_deficit': 2007.7,
                        'total_kg_throughput': 106.9
                    }

                ideal_point: dict, optional, default None
                    A dictionary of objective name and value pairs. If 'ideal_point'
                    is not None, then it is used to normalise the hypervolume in
                    range 0.0 - 1.0. 'ideal_point' is ignored if the number of 
                    objectives is 1. 
        '''
        assert type(schedules) is list and len(schedules) > 0

        if len(self.objectives_coefficients_list) == 1:
            return schedules[0].objectives[self.objectives_coefficients_list[0][0]]

        points = np.array([
            [schedule.objectives[obj].values[0] * coef * -1 for (obj, coef) in self.objectives_coefficients_list]
            for schedule in schedules
        ])

        if ref_point is not None:
            ref_point = [ref_point[obj] * coef * -1 for (obj, coef) in self.objectives_coefficients_list]
        else:
            ref_point = (np.max(points, axis=0) + 1).tolist()

        hv = hypervolume(points, ref_point)

        if ideal_point is not None:
            ideal_point = np.array([[ideal_point[obj] * coef * -1 for (obj, coef) in self.objectives_coefficients_list]])
            hv /= hypervolume(ideal_point, ref_point)

        return hv        