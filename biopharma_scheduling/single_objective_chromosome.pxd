cdef extern from "single_objective_chromosome.h" namespace "types":
    cdef cppclass SingleObjectiveChromosome[Gene]:
        SingleObjectiveChromosome()
        double objective
        double constraints