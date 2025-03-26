#pragma once


template <typename T>
struct fn_scast_t
{
    [[nodiscard]]
    static constexpr auto operator()(auto value)
    {
        return static_cast<T>(value);
    }
};

template <typename T>
constexpr auto fn_scast = fn_scast_t<T>{};

template <typename T>
struct fn_ptr_t
{
    [[nodiscard]]
    static constexpr auto operator()(auto& value)
    {
        return &value;
    }
};

template <typename T>
constexpr auto fn_ptr = fn_ptr_t<T>{};

template <typename T>
struct fn_deref_t
{
    [[nodiscard]]
    static constexpr auto operator()(auto* value)
    {
        return &value;
    }
};

template <typename T>
constexpr auto fn_deref = fn_deref_t<T>{};
