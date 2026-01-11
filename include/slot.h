/**
 * @file slot.h
 * @author HUANG He (hu4nghe@outlook.com)
 * @brief Base element of lockfree_queue
 * @version 1.0.1
 * @date 2026-01-11
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <atomic>
#include <new>

namespace lockfree
{
    constexpr auto destructive_interference = std::hardware_destructive_interference_size;

    template <class value_type> struct alignas(destructive_interference) slot
    {
        std::atomic<size_t> seq;

        private:

        alignas(value_type) std::byte storage[sizeof(value_type)];

        bool constructed;

        public:

        /**
         * @brief Default constructor.
         *
         */
        slot() noexcept
            : seq(0),
              constructed(false)
        {}

        /* Impossible to copy or move a slot. */
        slot(const slot&)            = delete;
        slot(slot&&)                 = delete;
        slot& operator=(const slot&) = delete;
        slot& operator=(slot&&)      = delete;

        /**
         * @brief
         *  Construct an item when it is pushed.
         *
         * @tparam Args
         *  Parameter pack type.

         * @param args
         *  The forwarded item.
         *
         */
        template <typename... Args> void construct(Args&&... args)
        {
            new (&storage) value_type(std::forward<Args>(args)...);
            constructed = true;
        }

        /**
         * @brief
         *  Destroy an item when it is poped.
         *
         */
        void destroy() noexcept
        {
            if (constructed)
            {
                std::launder(reinterpret_cast<value_type*>(&storage))->~value_type();
                constructed = false;
            }
        }

        /**
         * @brief
         *  Set the slot's sequence number.
         *
         * @param new_seq
         *  The new sequence number.
         *
         */
        void set_seq(size_t new_seq) noexcept
        {
            seq.store(new_seq, std::memory_order_release);
        }

        /**
         * @brief
         *  Get the item stored in the current slot.
         *
         * @return value_type&
         *  A Reference of the stored item.
         *
         */
        auto get() noexcept -> value_type&
        {
            return *std::launder(reinterpret_cast<value_type*>(&storage));
        }

        /**
         * @brief
         *  Read only version of get().
         *
         * @return const value_type&
         *  A const reference of the stored item.
         *
         */
        auto get() const noexcept -> const value_type&
        {
            return *std::launder(reinterpret_cast<const value_type*>(&storage));
        }
    };
} // namespace lockfree