from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.unordered_map cimport unordered_map

from campaign cimport Campaign


cdef extern from "schedule.h" namespace "types":
    cdef struct Schedule:
        Schedule()
        vector[double] objectives
        vector[Campaign] campaigns
        vector[vector[double]] kg_inventory
        vector[vector[double]] kg_supply
        vector[vector[double]] kg_backlog
        vector[vector[double]] kg_waste
