/*
 * objectpool.hpp
 *
 *  Created on: Aug 8, 2025
 *      Author: ChatGPT
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <new>
#include <utility>
#include <type_traits>

template<typename T, std::size_t N>
class ObjectPool {
public:
    static_assert(std::is_nothrow_destructible<T>::value || std::is_trivially_destructible<T>::value,
                  "T should be safely destructible");

    ObjectPool() {
        for (std::size_t i = 0; i < N; ++i) {
            used[i] = false;
            objects[i] = nullptr;
        }
        count = 0;
    }

    template<typename... Args>
    T* allocate(Args&&... args) {
        for (std::size_t i = 0; i < N; ++i) {
            if (!used[i]) {
                // adresse du i-ème slot (en octets)
                void* place = static_cast<void*>(buffer + i * sizeof(T));
                T* obj = new (place) T(std::forward<Args>(args)...);
                objects[i] = obj;
                used[i] = true;
                ++count;
                return obj;
            }
        }
        return nullptr; // pool plein
    }

    void free(T* ptr) {
        for (std::size_t i = 0; i < N; ++i) {
            if (objects[i] == ptr && used[i]) {
                ptr->~T();
                objects[i] = nullptr;
                used[i] = false;
                --count;
                return;
            }
        }
    }

    void clear() {
        for (std::size_t i = 0; i < N; ++i) {
            if (used[i] && objects[i]) {
                objects[i]->~T();
                objects[i] = nullptr;
                used[i] = false;
            }
        }
        count = 0;
    }

    std::size_t activeCount() const { return count; }
    constexpr std::size_t capacity() const { return N; }

    // utile pour debug : adresse base du slot i
    const void* slotAddress(std::size_t i) const {
        if (i >= N) return nullptr;
        return static_cast<const void*>(buffer + i * sizeof(T));
    }

private:
    alignas(T) std::uint8_t buffer[N * sizeof(T)];
    T* objects[N];
    bool used[N];
    std::size_t count = 0;
};
