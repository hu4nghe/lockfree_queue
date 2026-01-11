/**
 * @file lockfree_queue.h
 * @author HUANG He (hu4nghe@outlook.com)
 * @brief
 *  A MPMC queue after D.Vyukov's algorithm.
 * @version 1.0.1
 * @date 2026-01-11
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "slot.h"

#include <bit>
#include <ranges>
#include <vector>

namespace lockfree
{
    template <class value_type> struct queue
    {
        private:

        using slot_type = slot<value_type>;
        using size_type = std::size_t;

        size_type capacity;
        size_type mask;

        std::vector<slot_type> buffer;

        alignas(64) std::atomic<size_type> head;
        alignas(64) std::atomic<size_type> tail;

        public:

        /**
         * @brief
         *  Lock-free ring queue.
         *
         * @tparam value_type
         *  Type of the elements stored.
         *
         * @note
         *  Capacity handling:
         *
         *  The constructor parameter `requested` is only a hint.
         *  The real internal capacity is automatically adjusted to
         *  the smallest power of two that is grater than requested
         *  and is guaranteed to be at least 2 so the queue can use
         *  bitwise masking instead of "&".
         *
         *  Example:
         *  requested = 0 or 1  -> real capacity = 2
         *  requested = 3       -> real capacity = 4
         *  requested = 8       -> real capacity = 8
         *
         */
        explicit queue(size_type requested)
            : capacity(
                  std::max<size_type>(
                      2,
                      std::bit_ceil(requested))),
              mask(capacity - 1),
              buffer(capacity),
              head(0),
              tail(0)
        {
            for (auto [i, item] : buffer | std::views::enumerate) item.set_seq(i);
        }

        /* Impossible to copy or move a lock-free queue. */
        queue(const queue&)            = delete;
        queue(queue&&)                 = delete;
        queue& operator=(const queue&) = delete;
        queue& operator=(queue&&)      = delete;

        /**
         * @brief
         *  Destroy the queue object.
         *  This will destroy all remaining items.
         *
         */
        ~queue()
        {
            for (auto& item : buffer) item.destroy();
        }

        /**
         * @brief
         *  Try to enqueue a new element by constructing in place.
         *
         * @tparam
         *  Args Constructor arguments for value_type
         *
         * @return true
         *  If item is successfully pushed.
         * @return false
         *  If the queue is full.
         *
         */
        template <typename... Args> bool enqueue(Args&&... args)
        {
            while (true)
            {
                size_type  current_tail = tail.load(std::memory_order_relaxed);
                slot_type& current_slot = buffer[current_tail & mask];
                size_type  current_seq  = current_slot.seq.load(std::memory_order_acquire);

                auto cmp = (std::intptr_t)current_seq <=> (std::intptr_t)current_tail;

                if (cmp == 0)
                {
                    if (tail.compare_exchange_weak(
                            current_tail,
                            current_tail + 1,
                            std::memory_order_acq_rel,
                            std::memory_order_relaxed))
                    {
                        current_slot.construct(std::forward<Args>(args)...);
                        current_slot.seq.store(current_tail + 1, std::memory_order_release);
                        return true;
                    }
                }
                else if (cmp < 0) return false; // Full
                // else if cmp > 0 : tail is moved by other thread, retry.
            }
        }

        /**
         * @brief
         *  Try to pop an element
         *
         * @param out
         *  The variable used to receive the popped item.
         *
         * @return true
         *  If the item is successfully popped.
         * @return false
         *  If the queue is empty.
         *
         */
        bool dequeue(value_type& out)
        {
            while (true)
            {
                size_type  current_head = head.load(std::memory_order_relaxed);
                slot_type& current_slot = buffer[current_head & mask];
                size_type  current_seq  = current_slot.seq.load(std::memory_order_acquire);

                auto cmp = (std::intptr_t)current_seq <=> (std::intptr_t)current_head + 1;

                if (cmp == 0)
                {
                    // Try claim this slot
                    if (head.compare_exchange_weak(
                            current_head,
                            current_head + 1,
                            std::memory_order_acq_rel,
                            std::memory_order_relaxed))
                    {
                        out = std::move(current_slot.get());
                        current_slot.destroy();
                        current_slot.seq.store(current_head + capacity, std::memory_order_release);
                        return true;
                    }
                }
                else if (cmp < 0) return false; // Empty
                // else if cmp > 0 : head is moved by other thread, retry.
            }
        }
    };
} // namespace lockfree