cdef extern from "single_objective_individual.h" namespace "types":
    cdef struct SingleObjectiveIndividual:
        SingleObjectiveIndividual()
        double objective
        double constraints