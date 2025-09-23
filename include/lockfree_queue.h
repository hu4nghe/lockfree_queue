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
#include <vector>

namespace lockfree
{   
    template<class value_type>
    class queue
    {
    private:
        using slot_type = slot<value_type>;
        using size_type = std::size_t;

        std::vector<slot_type>  buffer;
        size_type               capacity;
        size_type               musk;

        alignas(64) std::atomic<size_type> head;
        alignas(64) std::atomic<size_type> tail;
    public:

        /**
         * @brief Construct a new queue object
         * 
         * @param capacity queue capacity
         */
        explicit queue(size_type capacity)
            :   capacity(round_up_pow2(capacity)),
                musk    (capacity - 1),
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
            while(try_pop(tmp)){}
        }
    };
    
}