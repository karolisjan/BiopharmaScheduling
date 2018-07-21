from typing import Union
from collections import OrderedDict

import numpy as np
import pandas as pd

import plotly.offline as opy
import plotly.graph_objs as go
import plotly.figure_factory as ff


class PySingleSiteSimpleSchedule:
    def __init__(
            self, 
            objectives: dict, 
            campaigns_table: list,
            batches_table: list=None,
            tasks_table: list=None,
            kg_inventory: list=None,
            kg_backlog: list=None,
            kg_supply: list=None,
            kg_waste: list=None,
        ):
        '''
            A Python helper class for encapsulating biopharma_scheduling solutions 
            and their attributes (e.g. objective values, campaigns list, batches list).

            PARAMETERS:

                objectives: dict
                    A Python dictionary of objective name and value pairs, e.g.:

                    {
                        'total_kg_inventory_deficit': float,
                        'total_kg_throughput': float,
                        'total_kg_backlog': float,
                        'total_kg_waste': float,
                        'total_profit': float,
                        'total_cost': float,
                    }

                campaigns_table: list of Union[dict, OrderedDict]
                    A Python list of either dict or OrderedDict, e.g.:

                    [
                        {
                            'Product': str, product label,
                            'Batches': int,
                            'Kg': float,
                            'Start': str, date in the '%Y-%m-%d' format,
                            'First Harvest': str, date in the '%Y-%m-%d' format,
                            'First Batch': str, date in the '%Y-%m-%d' format,
                            'Last Batch': str, date in the '%Y-%m-%d' format
                        },
                        ...
                    ]

                batches_table: list of Union[dict, OrderedDict], optional, default None
                    A Python list of either dict or OrderedDict, e.g.:

                    [
                        {
                            'Product': str, product label,
                            'Kg': float,
                            'Harvested on': str, date in the '%Y-%m-%d' format,
                            'Stored on': str, date in the '%Y-%m-%d' format,
                            'Expires on': str, date in the '%Y-%m-%d' format,
                            'Approved on': str, date in the '%Y-%m-%d' format
                        },
                        ...
                    ]

                tasks_table: list of Union[dict, OrderedDict], optional, default None
                    A Python list of either dict or OrderedDict, e.g.:

                    [
                        {
                            'Product': str, product label,
                            'Task': str, task name, e.g. { 'Inoculation', 'Seed', 'Production' },
                            'Start': str, date in the '%Y-%m-%d' format,
                            'Finish on': str, date in the '%Y-%m-%d' format
                        },
                        ...
                    ]

                kg_inventory: list, optional, default None
                    Example:

                    {
                        'date': str, date in the '%Y-%m-%d' format,
                        '<produt label1>': float,
                        '<produt label2>': float,
                        ...
                        '<produt labeln>': float,
                    }

                kg_backlog: list, optional, default None    
                    Example:

                    [
                        {
                            'date': str, date in the '%Y-%m-%d' format,
                            '<produt label1>': float,
                            '<produt label2>': float,
                            ...
                            '<produt labeln>': float,
                        },
                        ...
                    ]

                kg_supply: list, optional, default None
                    A Python list of 

                    [
                        {
                            'date': str, date in the '%Y-%m-%d' format,
                            '<produt label1>': float,
                            '<produt label2>': float,
                            ...
                            '<produt labeln>': float,
                        },
                        ...
                    ]

                kg_waste: list, optional, default None
                    Example:

                    [
                        {
                            'date': str, date in the '%Y-%m-%d' format,
                            '<produt label1>': float,
                            '<produt label2>': float,
                            ...
                            '<produt labeln>': float,
                        },
                        ...
                    ]
        '''
        self.__objectives = pd.DataFrame.from_records([objectives], index=['value'])
        self.__campaigns = pd.DataFrame.from_records(campaigns_table)
        self.__batches = pd.DataFrame.from_records(batches_table) if batches_table else None
        self.__tasks = pd.DataFrame.from_records(tasks_table) if tasks_table else None
        self.__kg_inventory = pd.DataFrame.from_records(kg_inventory) if kg_inventory else None
        self.__kg_backlog = pd.DataFrame.from_records(kg_backlog) if kg_backlog else None
        self.__kg_supply = pd.DataFrame.from_records(kg_supply) if kg_supply else None
        self.__kg_waste = pd.DataFrame.from_records(kg_waste) if kg_waste else None

        for df in [self.__kg_inventory, self.__kg_backlog, self.__kg_supply, self.__kg_waste]:
            if df is not None:
                df.index = pd.to_datetime(df['date'])
                del df['date']

    def campaigns_gantt(self, colors: dict=None, layout: dict=None):
        '''
            Creates a Gantt chart of the campaigns table.

            INPUT:
                colors: dict, optional, default None
                    A dictionary of product label and color code pairs (RGB or HEX)
                    for the Gantt chart.

                    {
                        'A': 'rgb(146, 208, 80)', 
                        'B': 'rgb(179, 129, 217)', 
                        'C': 'rgb(196, 189, 151)', 
                        'D': 'rgb(255, 0, 0)'
                    }

                layout: dict, optiona, default None
                    A dictionary of configuration parameters for the Gantt chart.
                    See https://plot.ly/python/gantt/.
        '''
        df = self.__campaigns.reset_index()

        df['Finish'] = df['Last Batch']
        df['Resource'] = df['Product']
        df['Task'] = df['Product']
        df = df.to_dict('records')
        
        gantt = ff.create_gantt(
            df, 
            colors=colors, 
            index_col='Resource', 
            group_tasks=True,
            showgrid_x=True, 
            showgrid_y=True
        )

        for gantt_row, campaign in zip(gantt['data'], df):
            text = '<br>'.join([
                '{}: {}'.format(key, val) 
                for key, val in campaign.items() 
                if key not in { 'index', 'Finish', 'Resource', 'Task' }
            ])
            gantt_row.update({'text': text})

        if layout is None:
            gantt['layout'].update({
                'title': '',
                'xaxis': {
                    'tickangle': -30,
                    'side': 'bottom'
                }
            })
        else:
            gantt['layout'].update(layout)
    
        return opy.iplot(gantt)

    def tasks_gantt(self, colors: dict=None, layout: dict=None):
        '''
            Creates a Gantt chart of the campaigns table.

            INPUT:
                colors: dict, optional, default None
                    A dictionary of product label and color code pairs (RGB or HEX)
                    for the Gantt chart.

                    {
                        'A': 'rgb(146, 208, 80)', 
                        'B': 'rgb(179, 129, 217)', 
                        'C': 'rgb(196, 189, 151)', 
                        'D': 'rgb(255, 0, 0)'
                    }

                title: str, optional, default ''
                    Title displayed at the top of the Gantt chart.
        '''
        df = self.__tasks.reset_index()

        df['Resource'] = df['Product']
        df = df.to_dict('records')
        
        gantt = ff.create_gantt(
            df, 
            colors=colors, 
            index_col='Resource', 
            group_tasks=True,
            showgrid_x=True, 
            showgrid_y=True,
            show_colorbar=True
        )

        for gantt_row, campaign in zip(gantt['data'], df):
            text = '<br>'.join([
                '{}: {}'.format(key, val) 
                for key, val in campaign.items() 
                if key not in {'index', 'Resource'}
            ])
            gantt_row.update({'text': text})

        if layout is None:
            gantt['layout'].update({
                'title': '',
                'xaxis': {
                    'tickangle': -30,
                    'side': 'bottom'
                }
            })
        else:
            gantt['layout'].update(layout)
    
        return opy.iplot(gantt)

    @property
    def objectives(self):
        return self.__objectives

    @property
    def campaigns(self):
        return self.__campaigns 

    @property
    def batches(self):
        return self.__batches

    @property
    def tasks(self):
        return self.__tasks

    @property
    def kg_inventory(self):
        return self.__kg_inventory

    @property
    def kg_backlog(self):
        return self.__kg_backlog

    @property
    def kg_supply(self):
        return self.__kg_supply

    @property
    def kg_waste(self):
        return self.__kg_waste


class PySingleSiteMultiSuiteSchedule:
    def __init__(
            self, 
            objectives: dict, 
            campaigns_table: list,
            batches_table: list=None,
            batch_inventory: list=None,
            batch_backlog: list=None,
            batch_supply: list=None,
            batch_waste: list=None,
        ):
        self.__objectives = pd.DataFrame.from_records([objectives], index=['value'])
        self.__campaigns = pd.DataFrame.from_records(campaigns_table)
        self.__batches = pd.DataFrame.from_records(batches_table) if batches_table else None
        self.__batch_inventory = pd.DataFrame.from_records(batch_inventory) if batch_inventory else None
        self.__batch_backlog = pd.DataFrame.from_records(batch_backlog) if batch_backlog else None
        self.__batch_supply = pd.DataFrame.from_records(batch_supply) if batch_supply else None
        self.__batch_waste = pd.DataFrame.from_records(batch_waste) if batch_waste else None

        for df in [
            self.__batch_inventory, 
            self.__batch_backlog,
            self.__batch_supply, 
            self.__batch_waste
        ]:
            if df is not None:
                df.index = pd.to_datetime(df['date'])
                del df['date']

    def campaigns_gantt(self, colors: dict=None, layout: dict=None):
        df = self.__campaigns.reset_index()

        df['Finish'] = df['End']
        df['Resource'] = df['Product']
        df['Task'] = df['Suite']
        df = df.to_dict('records')
        
        gantt = ff.create_gantt(
            df, 
            colors=colors, 
            index_col='Resource', 
            group_tasks=True,
            showgrid_x=True, 
            showgrid_y=True
        )
        
        gantt = ff.create_gantt(
            df, 
            colors=colors, 
            index_col='Resource', 
            group_tasks=True,
            showgrid_x=True, 
            showgrid_y=True
        )

        for gantt_row, campaign in zip(gantt['data'], df):
            text = '<br>'.join([
                '{}: {}'.format(key, val) 
                for key, val in campaign.items() 
                    if key not in {'index', 'Finish', 'Resource', 'Task'}
            ])
            gantt_row.update({'text': text})

        if layout is None:
            gantt['layout'].update({
                'title': '',
                'xaxis': {
                    'tickangle': -30,
                    'side': 'bottom'
                }
            })
        else:
            gantt['layout'].update(layout)
    
        return opy.iplot(gantt)

    @property
    def objectives(self):
        return self.__objectives

    @property
    def campaigns(self):
        return self.__campaigns

    @property
    def batches(self):
        return self.__batches

    @property
    def batch_inventory(self):
        return self.__batch_inventory

    @property
    def batch_backlog(self):
        return self.__batch_backlog

    @property
    def batch_supply(self):
        return self.__batch_supply

    @property
    def batch_waste(self):
        return self.__batch_waste