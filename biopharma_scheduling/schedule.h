#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include <queue>

#include "campaign.h"


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

namespace types
{   
    struct SingleSiteMultiSuiteSchedule
    {       
        SingleSiteMultiSuiteSchedule() {}

        void Init(int num_products, int num_periods, int num_suites, int num_objectives) 
        {
            using queue = std::priority_queue<types::Batch, std::vector<types::Batch>, OldestBatchFirst>;

            // Reserve space for the queue (big performance boost)
            inventory.resize(num_products);

            for (auto &i : inventory) {
                i.resize(num_periods);

                for (auto &q : i) {
                    q = queue(oldest_batch_first, make_reserved<types::Batch>(100));
                }
            }

            suites.resize(num_suites);

            batch_inventory = std::vector<std::vector<int>>(
                num_products, std::vector<int>(num_periods, 0.0)
            );

            batch_supply = std::vector<std::vector<int>>(
                num_products, std::vector<int>(num_periods, 0.0)
            );

            batch_backlog = std::vector<std::vector<int>>(
                num_products, std::vector<int>(num_periods, 0.0)
            );
            
            batch_waste = std::vector<std::vector<int>>(
                num_products, std::vector<int>(num_periods, 0.0)
            );

            objectives = std::vector<double>(num_objectives, 0.0);
        }

        std::vector<double> objectives;

        std::vector<std::vector<types::Campaign>> suites; 

        std::vector<std::vector<int>> batch_inventory;
        std::vector<std::vector<int>> batch_supply;
        std::vector<std::vector<int>> batch_backlog;
        std::vector<std::vector<int>> batch_waste;  

        // Queues batches for each product in each "time period" based on the expiry date
        // so that the oldest batches can be sold first to minimize waste
        std::vector< 
            std::vector<
                std::priority_queue<
                    types::Batch, 
                    std::vector<types::Batch>,
                    OldestBatchFirst
                > 
            >
        > inventory;    
    };

    struct SingleSiteSimpleSchedule
    {       
        SingleSiteSimpleSchedule() {}

        void Init(int num_products, int num_periods, int num_objectives) 
        {
            using queue = std::priority_queue<types::Batch, std::vector<types::Batch>, OldestBatchFirst>;

            // Reserve space for the queue (big performance boost)
            inventory.resize(num_products);

            for (auto &i : inventory) {
                i.resize(num_periods);

                for (auto &q : i) {
                    q = queue(oldest_batch_first, make_reserved<types::Batch>(100));
                }
            }

            kg_inventory = std::vector<std::vector<double>>(
                num_products, std::vector<double>(num_periods, 0.0)
            );

            kg_supply = std::vector<std::vector<double>>(
                num_products, std::vector<double>(num_periods, 0.0)
            );

            kg_backlog = std::vector<std::vector<double>>(
                num_products, std::vector<double>(num_periods, 0.0)
            );
            
            kg_waste = std::vector<std::vector<double>>(
                num_products, std::vector<double>(num_periods, 0.0)
            );

            objectives = std::vector<double>(num_objectives, 0.0);
        }

        std::vector<double> objectives;

        std::vector<types::Campaign> campaigns; 

        std::vector<std::vector<double>> kg_inventory;  
        std::vector<std::vector<double>> kg_supply; 
        std::vector<std::vector<double>> kg_backlog, kg_waste;

        std::vector< 
            std::vector<
                std::priority_queue<
                    types::Batch, 
                    std::vector<types::Batch>,
                    OldestBatchFirst
                > 
            >
        > inventory;    
    };
}

#endif