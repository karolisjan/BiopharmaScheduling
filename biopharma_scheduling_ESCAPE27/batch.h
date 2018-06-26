#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
    #pragma GCC diagnostic ignored "-Wreorder"
	#pragma GCC diagnostic ignored "-Wsign-compare"
	#pragma GCC diagnostic ignored "-Wunused-variable"
#endif 

#ifndef __BATCH_H__
#define __BATCH_H__


namespace types
{
	struct BaseBatch
	{

	};
	
	struct Batch
	{
		Batch() : 
			product_num(-1),
            kg(-1),
			start(-1),
			harvested_at(-1),
			stored_at(-1),
			expires_at(-1),
			approved_at(-1) 
		{}

		int product_num;

		double kg;
		double start;
        double harvested_at;
        double stored_at; 
        double expires_at;
		double approved_at;
	};

	struct Batch
	{
		Batch() : 
			product_num(-1),
            kg(-1),
			start(-1),
			harvested_at(-1),
			stored_at(-1),
			expires_at(-1),
			approved_at(-1) 
		{}

		int product_num;
		
		double kg;
		double start;
        double harvested_at;
        double stored_at; 
        double expires_at;
		double approved_at;
	};
}

#endif 
