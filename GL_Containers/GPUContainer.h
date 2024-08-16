#pragma once

#include <concepts>

template <typename T>
concept GPUContainer = requires(T t, std::size_t n, typename T::value_type v)
{
    { t.Size() } -> std::same_as<std::size_t>;
    { t.SizeBytes() } -> std::same_as<std::size_t>;
    { t.Capacity() } -> std::same_as<std::size_t>;
    { t.CapacityBytes() } -> std::same_as<std::size_t>;
    { t.clear() } -> std::same_as<void>;
    { t.reserve(n) } -> std::same_as<void>;
    { t.push_back(v) } -> std::same_as<void>;
    { t.removeUnordered(n)} -> std::same_as<void>;
};

