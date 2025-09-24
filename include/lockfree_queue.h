/**
 * @file lockfree_queue.h
 * @author HUANG He (hu4nghe@outlook.com)
 * @brief 
 * @version 1.0
 * @date 2025-09-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once

#include "slot.h"

#include <bit>
#include <vector>

namespace lockfree
{   
    template<class value_type>
    struct queue
    {
    private:
        using slot_type = slot<value_type>;
        using size_type = std::size_t;

        std::vector<slot_type>  buffer;
        size_type               capacity;
        size_type               mask;

        alignas(64) std::atomic<size_type> head;
        alignas(64) std::atomic<size_type> tail;
    public:

        /**
         * @brief Lock-free ring queue.
         *
         * @tparam value_type  Type of the elements stored in the queue.
         *
         * @note  Capacity handling:
         *        The constructor parameter `requested` is only a hint.
         *        The real internal capacity is automatically adjusted to
         *        the smallest power of two that is **>= requested** and
         *        is guaranteed to be at least 2.
         *        Example:
         *          requested = 0 or 1  -> actual capacity = 2
         *          requested = 3       -> actual capacity = 4
         *          requested = 8       -> actual capacity = 8
         *
         *        This is required so that the queue can use bitwise
         *        masking (`index & (capacity-1)`) instead of the slower
         *        modulo operator.
         */
        explicit queue(size_type requested)
            :   capacity(std::max<size_type>(2, std::bit_ceil(requested))),
                mask    (capacity - 1),
                buffer  (capacity),
                head    (0),
                tail    (0)
        {
            for(auto i = 0; i < capacity; i++)
                buffer[i].reset_seq(i);
        }

        /// Impossible to copy or move a lock-free queue.
        queue(const queue&)            = delete;
        queue(queue&&)                 = delete;
        queue& operator=(const queue&) = delete;
        queue& operator=(queue&&)      = delete;

        ~queue() 
        {
            // Destroy any remaining items
            value_type tmp;
            while(dequeue(tmp)){}
        }
        /**
         * @brief Try to enqueue a new element by constructing in place.
         *
         * @tparam Args Constructor arguments for value_type
         * @return true  if pushed
         * @return false if queue full
         */
        template<typename... Args>
        bool enqueue(Args&&... args) 
        {
            while(true)
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
                //else if cmp > 0 : tail is moved by other thread, retry.
            }
        }

         /**
         * @brief Try to pop an element
         * @param out where to store popped value
         * @return true  if popped
         * @return false if queue empty
         */
        bool dequeue(value_type& out) 
        {
            while(true)
            {
                size_type  current_head = head.load(std::memory_order_relaxed);
                slot_type& current_slot = buffer[head & mask];
                size_type  current_seq  = current_slot.seq.load(std::memory_order_acquire);

                auto cmp = (std::intptr_t)current_seq <=> (std::intptr_t)current_head;

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
                //else if cmp > 0 : head is moved by other thread, retry.
            }
        }
    };
}