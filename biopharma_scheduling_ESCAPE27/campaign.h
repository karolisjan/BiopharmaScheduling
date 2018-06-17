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
		Campaign() :
			product_num(-1),
			num_batches(0),
			suite_num(0),
			start(-1),
			end(-1) {}

		int	product_num;
		int suite_num;
		int	num_batches;
		int	start;
		int	end;

		std::vector<Batch> batches;
	};
}

#endif // !__CAMPAIGN_H__
