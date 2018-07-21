import unittest
import pandas as pd

from biopharma_scheduling.single_site.deterministic import DetSingleSiteSimple


class DetSingleSiteSimpleTest(unittest.TestCase):
    def setUp(self):
        self.start_date = '2016-12-01'
        self.kg_demand = pd.read_csv('data/deterministic_single_site_simple/kg_demand.csv', index_col='date')
        self.kg_inventory_target = pd.read_csv('data/deterministic_single_site_simple/kg_inventory_target.csv', index_col='date')
        self.product_data = pd.read_csv('data/deterministic_single_site_simple/product_data.csv')
        self.changeover_days = pd.read_csv('data/deterministic_single_site_simple/changeover_days.csv')
        
    def test_single_objective_problem1(self):
        '''
            Maximise total kg throughput.
            
            Keep total kg backlog and total kg waste at 0 kg.
        '''
        objectives = {
            'total_kg_throughput': 1 # maximisation, -1 for minimisation
        }

        constraints = {
            'total_kg_backlog': [-1, 0], # total_kg_backlog <= 0
            'total_kg_waste': [-1, 0] # total_kg_waste <= 0
        }

        for i in range(3):
            model = DetSingleSiteSimple(random_state=7, num_threads=-1).fit(
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
        objectives = {
            'total_kg_inventory_deficit': -1 # maximisation, -1 for minimisation
        }

        constraints = {
            'total_kg_backlog': [-1, 0], # total_kg_backlog <= 0
            'total_kg_waste': [-1, 0] # total_kg_waste <= 0
        }

        for i in range(3):
            model = DetSingleSiteSimple(random_state=7, num_threads=-1).fit(
                self.start_date,
                objectives,
                self.kg_demand,
                self.product_data,
                self.changeover_days,
                self.kg_inventory_target,
                constraints
            )

            self.assertAlmostEqual(model.schedules[0].objectives.total_kg_throughput[0], 538.7)
            self.assertAlmostEqual(model.schedules[0].objectives.total_kg_inventory_deficit[0], 194.7)
            self.assertAlmostEqual(model.schedules[0].objectives.total_kg_backlog[0], 0.0)
            self.assertAlmostEqual(model.schedules[0].objectives.total_kg_waste[0], 0.0)
    
    def test_multi_objective_problem1(self):
        '''
            Maximise total kg throughput.
            Minimise total kg inventory deficit.
            
            Keep total kg backlog and total kg waste at 0 kg.
        '''
        objectives = {
            'total_kg_throughput': 1,
            'total_kg_inventory_deficit': -1 # maximisation, -1 for minimisation
        }

        constraints = {
            'total_kg_backlog': [-1, 0], # total_kg_backlog <= 0
            'total_kg_waste': [-1, 0] # total_kg_waste <= 0
        }

        for i in range(3):
            model = DetSingleSiteSimple(random_state=7, num_threads=-1).fit(
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
            self.assertAlmostEqual(solution_x.objectives.total_kg_throughput[0], 573.8)
            self.assertAlmostEqual(solution_x.objectives.total_kg_inventory_deficit[0], 191.5)
            self.assertAlmostEqual(solution_x.objectives.total_kg_backlog[0], 0.0)
            self.assertAlmostEqual(solution_x.objectives.total_kg_waste[0], 0.0)

            # Solution Y
            solution_y = max(model.schedules, key=lambda s: s.objectives.total_kg_throughput[0])
            self.assertAlmostEqual(solution_y.objectives.total_kg_throughput[0], 630.4)
            self.assertAlmostEqual(solution_y.objectives.total_kg_inventory_deficit[0], 469.3)
            self.assertAlmostEqual(solution_y.objectives.total_kg_backlog[0], 0.0)
            self.assertAlmostEqual(solution_y.objectives.total_kg_waste[0], 0.0)

            
if __name__ == '__main__':
    unittest.main()
