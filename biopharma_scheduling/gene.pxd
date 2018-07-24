cdef extern from "gene.h" namespace "types":
    cdef struct SingleSiteMultiSuiteGene:
        int product_num
        int suite_num
        int num_batches

    cdef struct SingleSiteSimpleGene:
        int product_num
        int num_batches