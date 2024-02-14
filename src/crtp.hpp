#pragma once

template <typename Derived>
struct crtp_base
{
protected:
    crtp_base() = default;

    [[nodiscard]] constexpr
    auto self() -> Derived&
    { return static_cast<Derived&>(*this); }

    [[nodiscard]] constexpr
    auto self() const -> const Derived&
    { return static_cast<const Derived&>(*this); }
};

