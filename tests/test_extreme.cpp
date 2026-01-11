#include <catch2/catch_all.hpp>

#include <thread>

#include "lockfree_queue.h"

TEST_CASE("Extreme contention performance test")
{
    using test_type = std::uint64_t;

    constexpr std::size_t producers          = 10;
    constexpr std::size_t consumers          = 10;
    constexpr std::size_t items_per_producer = 1000000;
    constexpr std::size_t capacity           = 2;

    lockfree::queue<test_type> q(capacity);

    constexpr std::size_t total_items = producers * items_per_producer;

    std::atomic<std::size_t> produced = 0;
    std::atomic<std::size_t> consumed = 0;

    // Per-thread checksum to avoid shared state
    std::vector<test_type> consumer_sums(consumers, 0);

    auto producer = [&]
    {
        for (std::size_t i = 0; i < items_per_producer; ++i)
        {
            test_type v = 1; // value doesn't matter
            while (!q.enqueue(v))
            {
                // bounded spin
                for (std::size_t k = 0; k < 32; ++k) _mm_pause();
            }
            produced.fetch_add(1, std::memory_order_relaxed);
        }
    };

    auto consumer = [&](std::size_t idx)
    {
        test_type v;
        while (true)
        {
            if (q.dequeue(v))
            {
                consumer_sums[idx] += v;
                if (consumed.fetch_add(1, std::memory_order_relaxed) + 1 >= total_items) break;
            }
            else
            {
                if (consumed.load(std::memory_order_relaxed) >= total_items) break;
                for (std::size_t k = 0; k < 32; ++k) _mm_pause();
            }
        }
    };

    auto start = std::chrono::steady_clock::now();

    std::vector<std::thread> threads;
    for (std::size_t i = 0; i < producers; ++i) threads.emplace_back(producer);
    for (std::size_t i = 0; i < consumers; ++i) threads.emplace_back(consumer, i);

    for (auto& t : threads) t.join();

    auto end = std::chrono::steady_clock::now();

    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    REQUIRE(produced.load() == total_items);
    REQUIRE(consumed.load() == total_items);

    test_type sum = 0;
    for (auto s : consumer_sums) sum += s;

    REQUIRE(sum == static_cast<test_type>(total_items));
}