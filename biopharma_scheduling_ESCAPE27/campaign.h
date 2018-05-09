#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
    #pragma GCC diagnostic ignored "-Wreorder"
	#pragma GCC diagnostic ignored "-Wsign-compare"
	#pragma GCC diagnostic ignored "-Wunused-variable"
#endif 

#ifndef __CAMPAIGN_H__
#define __CAMPAIGN_H__

#include <vector>

#include "batch.h"

namespace types
{
	struct Campaign
	{
		int	product;
		int suite;
		int	batches;
		int dsp_turnaround;
		int	usp_time;
		int	dsp_time;
		int	takt;
		int	start;
		int	harvest;
		int first_fill;
		int	end;

		std::vector<Batch> batches_list;
	};
}

#endif // !__CAMPAIGN_H__
