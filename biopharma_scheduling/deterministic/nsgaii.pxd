from libcpp.vector cimport vector


cdef extern from "../nsgaii.h" namespace "algorithms" nogil:
    cdef cppclass NSGAII[Individual, FitnessFunctor]:
        NSGAII()
        NSGAII(FitnessFunctor, int seed, int num_threads)

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

        void Init(
            int popsize,
            int starting_length,
            double p_xo,
            double p_gene_swap,
            int num_products,
            int num_usp_suites,
            double p_product_mut,
            double p_usp_suite_mut,
            double p_plus_batch_mut,
            double p_minus_batch_mut
        )

        void Update()
        vector[Individual] TopFront()
        vector[Individual] TopFront(vector[Individual])