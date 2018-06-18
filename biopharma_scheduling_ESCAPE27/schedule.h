#if defined(__posix) || defined(__unix) || defined(__linux) || defined(__APPLE__)
    #pragma GCC diagnostic ignored "-Wreorder"
	#pragma GCC diagnostic ignored "-Wsign-compare"
#endif 

#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include <queue>

#include "campaign.h"


namespace types
{   
    struct OldestBatchFirst
    {
        bool operator()(const types::Batch &b1, const types::Batch &b2)
        {
            return b1.expires_at > b2.expires_at;
        }
    } oldest_batch_first;

    template <class T>
    std::vector<T> make_reserved(const std::size_t size)
    {
        std::vector<T> v;
        v.reserve(size);
        return std::move(v);
    }

    struct SingleSiteMultiSuiteSchedule
    {       
        SingleSiteMultiSuiteSchedule() {}

        void Init(int num_products, int num_periods, int num_suites, int num_objectives) 
        {
            using queue = std::priority_queue<types::Batch, std::vector<types::Batch>, OldestBatchFirst>;

            // Reserve space for the queue (big performance boost)
            batch_inventory.resize(num_products);
            for (auto &i : batch_inventory) {
                i.resize(num_periods);
                for (auto &q : i) {
                    q = queue(oldest_batch_first, make_reserved<types::Batch>(100));
                }
            }

            suites.resize(num_suites);

            inventory = std::vector<std::vector<int>>(
                num_products, std::vector<int>(num_periods, 0.0)
            );

            supply = std::vector<std::vector<int>>(
                num_products, std::vector<int>(num_periods, 0.0)
            );

            backlog = std::vector<std::vector<int>>(
                num_products, std::vector<int>(num_periods, 0.0)
            );
            
            waste = std::vector<std::vector<int>>(
                num_products, std::vector<int>(num_periods, 0.0)
            );

            objectives = std::vector<double>(num_objectives, 0.0);
        }

        std::vector<double> objectives;
        std::vector<std::vector<types::Campaign>> suites; 
        std::vector<std::vector<int>> inventory, supply, backlog, waste;  

        std::vector< 
            std::vector<
                std::priority_queue<
                    types::Batch, 
                    std::vector<types::Batch>,
                    OldestBatchFirst
                > 
            >
        > batch_inventory;    
    };
}

#endif