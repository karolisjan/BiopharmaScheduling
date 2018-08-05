'''
    Ubuntu tests
'''
import unittest
import platform

import pandas as pd
from biopharma_scheduling.single_site.deterministic import DetSingleSiteSimple, DetSingleSiteMultiSuite


class DetSingleSiteMultiSuiteTest(unittest.TestCase):
    def test_single_objective_problem1(self):
        '''
            Example 1 problem from Lakhdar et al. (2005)
        '''
        start_date = '2016-11-02'
        batch_demand = pd.read_csv('data/deterministic_single_site_multi_suite_ex1/batch_demand.csv', index_col='date')
        product_data = pd.read_csv('data/deterministic_single_site_multi_suite_ex1/product_data.csv')
        usp_changeover_days = pd.read_csv('data/deterministic_single_site_multi_suite_ex1/usp_changeover_days.csv')
        dsp_changeover_days = pd.read_csv('data/deterministic_single_site_multi_suite_ex1/dsp_changeover_days.csv')

        num_usp_suites = 2
        num_dsp_suites = 2

        ga_params = {
            'num_runs': 10,
            'popsize': 100,
            'num_gens': 100, 
            'starting_length': 1,
            'p_xo': 0.131266, 
            'p_product_mut': 0.131266,
            'p_usp_suite_mut': 0.131266,
            'p_plus_batch_mut': 0.131266,
            'p_minus_batch_mut': 0.131266,
            'p_gene_swap': 0.131266
        }

        objectives = {
            'total_profit': 1 # max
        }

        constraints = {
            'total_backlog_penalty': [-1, 0], # <= 0, i.e. less or equal than 0
        }

        for _ in range(3):
            model = DetSingleSiteMultiSuite(**ga_params, random_state=7, num_threads=-1).fit(
                start_date,
                objectives,
                num_usp_suites,
                num_dsp_suites,
                batch_demand,
                product_data,
                usp_changeover_days,
                dsp_changeover_days,
                constraints
            )

            self.assertAlmostEqual(model.schedules[0].objectives.total_profit[0], 518)
            self.assertAlmostEqual(model.schedules[0].objectives.total_backlog_penalty[0], 0.0)

    def test_single_objective_problem2(self):
        '''
            Modified Example 2 problem from Lakhdar et al. (2005)
        '''
        start_date = '2016-11-02'
        batch_demand = pd.read_csv('data/deterministic_single_site_multi_suite_ex2/batch_demand.csv', index_col='date')
        product_data = pd.read_csv('data/deterministic_single_site_multi_suite_ex2/product_data.csv')
        usp_changeover_days = pd.read_csv('data/deterministic_single_site_multi_suite_ex2/usp_changeover_days.csv')
        dsp_changeover_days = pd.read_csv('data/deterministic_single_site_multi_suite_ex2/dsp_changeover_days.csv')

        num_usp_suites = 2
        num_dsp_suites = 3

        ga_params = {
            'num_runs': 10,
            'popsize': 100,
            'num_gens': 100, 
            'starting_length': 1,
            'p_xo': 0.026776, 
            'p_product_mut': 0.004667,
            'p_usp_suite_mut': 0.015991,
            'p_plus_batch_mut': 0.896385,
            'p_minus_batch_mut': 0.853790,
            'p_gene_swap': 0.403328
        }

        objectives = {
            'total_profit': 1 # max
        }

        constraints = {
            'total_backlog_penalty': [-1, 0], # <= 0, i.e. less or equal than 0
        }

        for _ in range(3):
            model = DetSingleSiteMultiSuite(**ga_params, random_state=7, num_threads=-1).fit(
                start_date,
                objectives,
                num_usp_suites,
                num_dsp_suites,
                batch_demand,
                product_data,
                usp_changeover_days,
                dsp_changeover_days,
                constraints
            )

            self.assertAlmostEqual(model.schedules[0].objectives.total_profit[0], 793.0)
            self.assertAlmostEqual(model.schedules[0].objectives.total_backlog_penalty[0], 0.0)


class DetSingleSiteSimpleTest(unittest.TestCase):
    def setUp(self):
        self.start_date = '2016-12-01'
        self.kg_demand = pd.read_csv('data/deterministic_single_site_simple/kg_demand.csv', index_col='date')
        self.kg_inventory_target = pd.read_csv('data/deterministic_single_site_simple/kg_inventory_target.csv', index_col='date')
        self.product_data = pd.read_csv('data/deterministic_single_site_simple/product_data.csv')
        self.changeover_days = pd.read_csv('data/deterministic_single_site_simple/changeover_days.csv')
        
    def test_known_solution(self):
        known_solution = pd.DataFrame({
            'Product': ['D', 'C', 'A', 'B', 'D', 'C', 'A', 'C', 'B', 'A', 'D'],
            'Batches': [15, 9, 28, 2, 15, 8, 10, 3, 2, 3, 29]
        })

        objectives = {
            'total_kg_throughput': 1,
            'total_kg_inventory_deficit': -1 # maximisation, -1 for minimisation
        }

        constraints = {
            'total_kg_backlog': [-1, 0], # total_kg_backlog <= 0
            'total_kg_waste': [-1, 0] # total_kg_waste <= 0
        }

        model = DetSingleSiteSimple(random_state=7, num_threads=-1).fit(
            self.start_date,
            objectives,
            self.kg_demand,
            self.product_data,
            self.changeover_days,
            self.kg_inventory_target,
            constraints
        )

        schedule = model.create_schedule(known_solution)

        self.assertAlmostEqual(schedule.objectives.total_kg_throughput[0], 574.4)
        self.assertAlmostEqual(schedule.objectives.total_kg_inventory_deficit[0], 194.6)
        self.assertAlmostEqual(schedule.objectives.total_kg_backlog[0], 0.0)
        self.assertAlmostEqual(schedule.objectives.total_kg_waste[0], 0.0)

    def test_single_objective_problem1(self):
        '''
            Maximise total kg throughput.
            
            Keep total kg backlog and total kg waste at 0 kg.
        '''
        ga_params = {
            'num_runs': 20,
            'num_gens': 100,
            'popsize': 100,
            'starting_length': 1,
            'p_xo': 0.108198,
            'p_product_mut': 0.041373,
            'p_plus_batch_mut': 0.608130,
            'p_minus_batch_mut': 0.765819,
            'p_gene_swap': 0.471346,
        }

        objectives = {
            'total_kg_throughput': 1 # maximisation, -1 for minimisation
        }

        constraints = {
            'total_kg_backlog': [-1, 0], # total_kg_backlog <= 0
            'total_kg_waste': [-1, 0] # total_kg_waste <= 0
        }

        for _ in range(3):
            model = DetSingleSiteSimple(**ga_params, random_state=7, num_threads=-1).fit(
                self.start_date,
                objectives,
                self.kg_demand,
                self.product_data,
                self.changeover_days,
                self.kg_inventory_target,
                constraints
            )

            self.assertAlmostEqual(model.schedules[0].objectives.total_kg_throughput[0], 630.4)
            self.assertAlmostEqual(model.schedules[0].objectives.total_kg_inventory_deficit[0], 472.2)
            self.assertAlmostEqual(model.schedules[0].objectives.total_kg_backlog[0], 0.0)
            self.assertAlmostEqual(model.schedules[0].objectives.total_kg_waste[0], 0.0)

    def test_single_objective_problem2(self):
        '''
            Minimise total kg inventory deficit.
            
            Keep total kg backlog and total kg waste at 0 kg.
        '''
        ga_params = {
            'num_runs': 20,
            'num_gens': 100,
            'popsize': 100,
            'starting_length': 1,
            'p_xo': 0.108198,
            'p_product_mut': 0.041373,
            'p_plus_batch_mut': 0.608130,
            'p_minus_batch_mut': 0.765819,
            'p_gene_swap': 0.471346,
        }

        objectives = {
            'total_kg_inventory_deficit': -1 # maximisation, -1 for minimisation
        }

        constraints = {
            'total_kg_backlog': [-1, 0], # total_kg_backlog <= 0
            'total_kg_waste': [-1, 0] # total_kg_waste <= 0
        }

        for i in range(3):
            model = DetSingleSiteSimple(**ga_params, random_state=7, num_threads=-1).fit(
                self.start_date,
                objectives,
                self.kg_demand,
                self.product_data,
                self.changeover_days,
                self.kg_inventory_target,
                constraints
            )

            self.assertAlmostEqual(model.schedules[0].objectives.total_kg_throughput[0], 503.2)
            self.assertAlmostEqual(model.schedules[0].objectives.total_kg_inventory_deficit[0], 192.5)
            self.assertAlmostEqual(model.schedules[0].objectives.total_kg_backlog[0], 0.0)
            self.assertAlmostEqual(model.schedules[0].objectives.total_kg_waste[0], 0.0)
    
    def test_multi_objective_problem1(self):
        '''
            Maximise total kg throughput.
            Minimise total kg inventory deficit.
            
            Keep total kg backlog and total kg waste at 0 kg.
        '''
        ga_params = {
            'num_runs': 20,
            'num_gens': 100,
            'popsize': 100,
            'starting_length': 1,
            'p_xo': 0.108198,
            'p_product_mut': 0.041373,
            'p_plus_batch_mut': 0.608130,
            'p_minus_batch_mut': 0.765819,
            'p_gene_swap': 0.471346,
        }

        objectives = {
            'total_kg_throughput': 1,
            'total_kg_inventory_deficit': -1 # maximisation, -1 for minimisation
        }

        constraints = {
            'total_kg_backlog': [-1, 0], # total_kg_backlog <= 0
            'total_kg_waste': [-1, 0] # total_kg_waste <= 0
        }

        for _ in range(3):
            model = DetSingleSiteSimple(**ga_params, random_state=7, num_threads=-1).fit(
                self.start_date,
                objectives,
                self.kg_demand,
                self.product_data,
                self.changeover_days,
                self.kg_inventory_target,
                constraints
            )
            
            # Solution X
            solution_x = min(model.schedules, key=lambda s: s.objectives.total_kg_throughput[0])
            self.assertAlmostEqual(solution_x.objectives.total_kg_throughput[0], 577.9)
            self.assertAlmostEqual(solution_x.objectives.total_kg_inventory_deficit[0], 193.4)
            self.assertAlmostEqual(solution_x.objectives.total_kg_backlog[0], 0.0)
            self.assertAlmostEqual(solution_x.objectives.total_kg_waste[0], 0.0)

            # Solution Y
            solution_y = max(model.schedules, key=lambda s: s.objectives.total_kg_throughput[0])
            self.assertAlmostEqual(solution_y.objectives.total_kg_throughput[0], 630.4)
            self.assertAlmostEqual(solution_y.objectives.total_kg_inventory_deficit[0], 469.3)
            self.assertAlmostEqual(solution_y.objectives.total_kg_backlog[0], 0.0)
            self.assertAlmostEqual(solution_y.objectives.total_kg_waste[0], 0.0)

            
if __name__ == '__main__':
    assert platform.system() == 'Linux', "Only Linux tests are available at the moment."
    unittest.main()
