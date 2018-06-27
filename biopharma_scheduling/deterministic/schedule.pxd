from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.unordered_map cimport unordered_map

from campaign cimport Campaign


cdef extern from "schedule.h" namespace "types":
    cdef struct SingleSiteSimpleSchedule:
        Schedule()
        vector[double] objectives
        vector[Campaign] campaigns
        vector[vector[double]] kg_inventory
        vector[vector[double]] kg_supply
        vector[vector[double]] kg_backlog
        vector[vector[double]] kg_waste

    cdef struct SingleSiteMultiSuiteSchedule:
        Schedule()
        vector[double] objectives
        vector[vector[Campaign]] suites
        vector[vector[double]] batch_inventory
        vector[vector[double]] batch_supply
        vector[vector[double]] batch_backlog
        vector[vector[double]] batch_waste
