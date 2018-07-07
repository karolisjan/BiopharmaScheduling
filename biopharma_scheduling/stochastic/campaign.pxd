from libcpp.vector cimport vector

from batch cimport Batch


cdef extern from "../campaign.h" namespace "types":
    cdef struct Campaign:
        int product_num
        int num_batches
        int suite_num

        double kg
        double start
        double first_harvest
        double first_batch
        double last_batch
        double end
        
        vector[Batch] batches