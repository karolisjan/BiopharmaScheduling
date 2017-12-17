#ifndef __CAMPAIGN_H__
#define __CAMPAIGN_H__

#include <vector>
#include "batch.h"

using namespace std;

namespace types
{
	struct Campaign
	{
		Campaign() :
			suite(0),
			product(0),
			batches(0),
			start(0),
			end(0)
		{}

		int suite;
		int product;
		int batches;
		double start;
		double end;

		vector<Batch> batches_list;
	};
}

#endif // !__CAMPAIGN_H__
