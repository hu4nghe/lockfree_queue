/**
 * @file slot.h
 * @author HUANG He (hu4nghe@outlook.com)
 * @brief Base element of lockfree_queue
 * @version 1.0
 * @date 2025-09-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <atomic>

namespace lockfree
{
    template<class value_type>
    struct alignas(64) slot
    {
        std::atomic<size_t> seq;
    private:
        // Reserve memory for value
        alignas(value_type) std::byte storage[sizeof(value_type)];
        // Value status flag
        bool constructed;

    public:
        /**
         * @brief Default constructor
         * 
         */
        slot() noexcept : seq(0), constructed(false){}

        /// You don't want to move a slot in multithread queue. ///
        slot(const slot&)            = delete;
        slot(slot&&)                 = delete;
        slot& operator=(const slot&) = delete;
        slot& operator=(slot&&)      = delete;

        /**
         * @brief Late construct the value_type object at push
         * 
         * @tparam Args Parameter pack type
         * @param args Forwarding value
         */
        template<typename... Args>
        void construct(Args&&... args) 
        {
            //Placement new with forwarding
            new (&storage) value_type(std::forward<Args>(args)...);
            constructed = true;
        }

        /**
         * @brief Destroy a element at pop
         * 
         */
        void destroy() noexcept 
        {
            if (constructed) 
            {
                reinterpret_cast<value_type*>(&storage)->~value_type();
                constructed = false;
            }
        }
        
        /**
         * @brief reset slot sequence number
         * 
         * @param new_seq new sequence number value
         */
        void reset_seq(size_t new_seq) noexcept 
        {
            seq.store(new_seq, std::memory_order_release);
        }
    };
}
 