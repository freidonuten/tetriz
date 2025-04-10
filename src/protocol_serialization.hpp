#pragma once

#include <boost/pfr/tuple_size.hpp>
#include <string_view>
#include <type_traits>
#include <variant>
#include <format>

#include "CorruptedRequestException.h"
#include "MessageTokenizer.h"


namespace protocol
{
    namespace detail
    {
        template <typename ...Ts>
        constexpr
        void deserialize(std::string_view message, std::variant<Ts...>& variant)
        {
            if (message.empty())
            {
                throw SerializerError("message is empty");
            }

            auto match = [&]<typename T>() constexpr {
                if (T::symbol == message.front())
                {
                    variant = T{};
                    return true;
                }

                return false;
            };

            if (!(match.template operator()<Ts>() || ...))
            {
                throw SerializerError(
                    std::format("Unknown message header {}", message.front()));
            }

            std::visit([message]<typename T>(T& message_struct){
                constexpr auto field_count = boost::pfr::tuple_size_v<T>;
                static_assert(field_count < 3, "Missing deserialization handler!");

                auto tokenizer = MessageTokenizer(message);
                if constexpr (field_count == 2)
                {
                    auto& [a, b] = message_struct;
                    tokenizer >> a >> b;
                }
                else if constexpr (field_count == 1)
                {
                    auto& [a] = message_struct;
                    tokenizer >> a;
                }

                if (!tokenizer.is_done())
                {
                    throw SerializerError(format("Corrupted message: {}", message));
                }
            }, variant);
        }
    }

    template <typename T>
    concept Message = requires {
        requires std::is_integral_v<decltype(T::symbol)>;
        requires std::is_enum_v<decltype(T::state)>;
    };

    template<typename T>
    struct is_variant_of_messages
        : std::false_type {};

    template<typename... Ts>
    struct is_variant_of_messages<std::variant<Ts...>>
        : std::bool_constant<(... && Message<Ts>)> {};

    template<typename T>
    concept MessageVariant = is_variant_of_messages<T>::value;

    template <MessageVariant T>
    constexpr
    auto deserialize(std::string_view message) -> T
    {
        auto variant = T{};
        detail::deserialize(message, variant);
        return variant;
    }
}
