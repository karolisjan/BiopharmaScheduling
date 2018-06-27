from libcpp.vector cimport vector

from batch cimport Batch

cdef extern from "campaign.h" namespace "types":
    cdef struct Campaign:
        int product_num
        int num_batches
        double kg
        int start
        int first_harvest
        int first_batch
        int last_batch
        vector[Batch] batches