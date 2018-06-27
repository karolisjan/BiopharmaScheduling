from libcpp.vector cimport vector

cdef extern from "nsga_individual.h" namespace "types":
    cdef struct NSGAIndividual:
        NSGAIndividual()
        vector[double] objectives
        double constraints