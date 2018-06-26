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
			num_batches(-1),
			suite_num(-1),
			start(-1),
			end(-1)
		{}


		int product_num;
		int num_batches;
		int	suite_num;

		double start;
		double end;

		std::vector<Batch> batches;
	};
}

#endif 