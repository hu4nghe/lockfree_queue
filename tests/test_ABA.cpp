
#include <catch2/catch_all.hpp>
#include <thread>
#include <vector>
#include <atomic>
#include <unordered_set>
#include <mutex>

#include "lockfree_queue.h"

TEST_CASE("Stress test: high contention to provoke ABA") 
{
    using Id = std::uint64_t;

    constexpr int producers = 10;
    constexpr int consumers = 10;
    constexpr int items_per_producer = 10000; 
    const std::size_t capacity = 2; // Low capacity to force slot reutilisation 

    lockfree::queue<Id> q(capacity);

    std::atomic<Id> global_id{0};
    std::atomic<int> produced{0};
    std::atomic<int> consumed{0};

    // 用于检测重复/丢失：使用 mutex 保护的 unordered_set
    std::unordered_set<Id> seen;
    std::mutex seen_mtx;

    auto producer = [&] 
    {
        for (int i = 0; i < items_per_producer; ++i) 
        {
            Id id = global_id.fetch_add(1, std::memory_order_relaxed);
            
            while (!q.enqueue(id)) 
            {
                std::this_thread::yield();
            }
            produced.fetch_add(1, std::memory_order_relaxed);
        }
    };

    auto consumer = [&] 
    {
        Id v;
        while(true)
        {
            if (q.dequeue(v)) 
            {
                {
                    std::lock_guard<std::mutex> lk(seen_mtx);
                    // check repeate item
                    if (seen.find(v) != seen.end()) 
                    {
                        INFO("Duplicate id consumed: " << v);
                        REQUIRE_FALSE(true);
                    }
                    seen.insert(v);
                }
                consumed.fetch_add(1, std::memory_order_relaxed);

                // all items consumed
                if (consumed.load(std::memory_order_relaxed) >= producers * items_per_producer)
                    break;
            } 
            else 
            {
                std::this_thread::yield();
                if (consumed.load(std::memory_order_relaxed) >= producers * items_per_producer)
                    break;
            }
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < producers; ++i) threads.emplace_back(producer);
    for (int i = 0; i < consumers; ++i) threads.emplace_back(consumer);

    for (auto &t : threads) t.join();

    const int total_expected = producers * items_per_producer;

    // Count check
    REQUIRE(produced.load(std::memory_order_relaxed) == total_expected);
    REQUIRE(consumed.load(std::memory_order_relaxed) == total_expected);

    std::lock_guard<std::mutex> lk(seen_mtx);
    
    REQUIRE(static_cast<int>(seen.size()) == total_expected);

    for (Id id = 0; id < static_cast<Id>(total_expected); ++id) 
        if (seen.find(id) == seen.end()) 
            FAIL("Missing id: " << id);
}