from libcpp.vector cimport vector


cdef extern from "nsga_chromosome.h" namespace "types":
    cdef cppclass NSGAChromosome[Gene]:
        NSGAChromosome()
        vector[double] objectives
        double constraints