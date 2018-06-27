from libcpp.vector cimport vector


cdef extern from "../nsga_individual.h" namespace "types":
    cdef cppclass NSGAIndividual[Gene]:
        NSGAIndividual()
        vector[double] objectives
        double constraints