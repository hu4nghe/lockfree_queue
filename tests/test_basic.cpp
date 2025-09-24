#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#include "lockfree_queue.h"

TEST_CASE("Single thread scenario test") 
{
    const int requested_capacity = 8;
    lockfree::queue<int> q(requested_capacity);
    for (int i = 0; i < requested_capacity; ++i)
        REQUIRE(q.enqueue(i));

    REQUIRE_FALSE(q.enqueue(42));

    for (int i = 0; i < requested_capacity; ++i) 
    {
        int out = -1;
        REQUIRE(q.dequeue(out));
        REQUIRE(out == i);
    }

    int out;
    REQUIRE_FALSE(q.dequeue(out));
}

#include <thread>
#include <vector>

TEST_CASE("Multi thread scenario test") 
{
    lockfree::queue<int> q(1024);
    constexpr int items_per_thread = 10000;
    constexpr int producers = 4;
    constexpr int consumers = 4;

    std::atomic<int> produced{0}, consumed{0};

    auto producer = [&] 
    {
        for (int i = 0; i < items_per_thread; ++i) 
        {
            while (!q.enqueue(i)) { /* spin until space */ }
            produced.fetch_add(1, std::memory_order_relaxed);
        }
    };

    auto consumer = [&] 
    {
        int value;
        while (consumed.load(std::memory_order_relaxed) < producers*items_per_thread) 
            if (q.dequeue(value))
                consumed.fetch_add(1, std::memory_order_relaxed);
        
    };

    std::vector<std::thread> threads;
    for (int i=0;i<producers;i++) threads.emplace_back(producer);
    for (int i=0;i<consumers;i++) threads.emplace_back(consumer);
    for (auto& t: threads) t.join();

    REQUIRE(produced.load() == consumers*items_per_thread);
    REQUIRE(produced.load() == consumed.load());
}