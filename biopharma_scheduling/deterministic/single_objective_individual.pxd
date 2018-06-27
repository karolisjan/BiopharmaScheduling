cdef extern from "../single_objective_individual.h" namespace "types":
    cdef cppclass SingleObjectiveIndividual[Gene]:
        SingleObjectiveIndividual()
        double objective
        double constraints