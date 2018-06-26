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
	template<class Batch>
	struct BaseCampaign 
	{
		int product_num;
		int num_batches;

		double start;
		double end;

		std::vector<Batch> batches;
	};

	struct SingleSiteSimpleCampaign : public BaseCampaign<Batch>
	{
		using BaseCampaign::product_num;
		using BaseCampaign::num_batches;
		using BaseCampaign::start;
		using BaseCampaign::end;
		using BaseCampaign::batches;

		SingleSiteSimpleCampaign()
		{
			product_num = -1;
			start = -1;
			end = -1;
			kg = -1;
		}

		double kg;
	};

	struct SingleSiteMultiSuiteCampaign  : public BaseCampaign<Batch>
	{
		using BaseCampaign::product_num;
		using BaseCampaign::num_batches;
		using BaseCampaign::start;
		using BaseCampaign::end;
		using BaseCampaign::batches;

		SingleSiteMultiSuiteCampaign() 
		{
			product_num = -1;
			start = -1;
			end = -1;
			suite_num = -1;
		}

		int	suite_num;
	};
}

#endif 