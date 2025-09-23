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
    struct alignas(64) slot{
        
        std::atomic<size_t> seq;

    private:
        // Reserve memory for value
        alignas(value_type) std::byte stroage[sizeof(value_type)];
        // Value object constructed flag
        bool constructed = false;
    };

}
 