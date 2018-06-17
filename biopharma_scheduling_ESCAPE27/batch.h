#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
    #pragma GCC diagnostic ignored "-Wreorder"
	#pragma GCC diagnostic ignored "-Wsign-compare"
	#pragma GCC diagnostic ignored "-Wunused-variable"
#endif 

#ifndef __BATCH_H__
#define __BATCH_H__

namespace types
{
	struct Batch
	{
		Batch() : 
			product_num(-1),
			stored_at(0),
			expires_at(0)
		{}

		int product_num;
		double stored_at, expires_at;
	};
}

#endif // !__BATCH_H__
