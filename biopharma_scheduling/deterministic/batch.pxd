cdef extern from "../batch.h" namespace "types":
    cdef struct Batch:
        int product_num
        double kg
        int start
        int harvested_at
        int stored_at
        int expires_at
        int approved_at