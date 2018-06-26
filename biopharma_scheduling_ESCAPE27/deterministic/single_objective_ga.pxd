from libcpp.vector cimport vector


cdef extern from "single_objective_ga.h" namespace "algorithms" nogil:
    cdef cppclass SingleObjectiveGA[Individual, FitnessFunctor]:
        SingleObjectiveGA()
        SingleObjectiveGA(FitnessFunctor, int seed, int num_threads)
        void Init(
            int popsize,
            int starting_length,
            double p_xo,
            double p_gene_swap,
            int num_products,
            double p_product_mut,
            double p_plus_batch_mut,
            double p_minus_batch_mut
        )
        void Update()
        Individual Top()
        Individual Top(vector[Individual])