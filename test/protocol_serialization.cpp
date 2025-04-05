#include <gtest/gtest.h>
#include "CorruptedRequestException.h"
#include "protocol_structs.hpp"


TEST(protocol_serialization, deserialize_plain)
{
    const auto variant = protocol::deserialize("V\n");

    ASSERT_TRUE(std::holds_alternative<protocol::room_leave_t>(variant));
}

TEST(protocol_serialization, deserialize_string)
{
    using Type = protocol::login_t;
    const auto variant = protocol::deserialize("Lfranta\n");

    ASSERT_TRUE(std::holds_alternative<Type>(variant));
    ASSERT_TRUE(std::get<Type>(variant).user_name == "franta");
}

TEST(protocol_serialization, deserialize_integral)
{
    using Type = protocol::room_read_t;
    const auto variant = protocol::deserialize("R12837\n");

    ASSERT_TRUE(std::holds_alternative<Type>(variant));
    ASSERT_TRUE(std::get<Type>(variant).room_id == 12837);
}

TEST(protocol_serialization, deserialize_mixture)
{
    using Type = protocol::move_get_t;
    const auto variant = protocol::deserialize("Gfranta 43\n");

    ASSERT_TRUE(std::holds_alternative<Type>(variant));
    ASSERT_TRUE(std::get<Type>(variant).player == "franta");
    ASSERT_TRUE(std::get<Type>(variant).index == 43);
}

TEST(protocol_serialization, deserialize_empty_message)
{
    ASSERT_THROW(protocol::deserialize(""), SerializerError);
    ASSERT_THROW(protocol::deserialize("\n"), SerializerError);
}

TEST(protocol_serialization, deserialize_invalid_header)
{
    ASSERT_THROW(protocol::deserialize("_ahoj\n"), SerializerError);
}

TEST(protocol_serialization, deserialize_missing_terminator)
{
    //ASSERT_THROW(protocol::deserialize("Ltest"), SerializerError);
}

TEST(protocol_serialization, deserialize_invalid_arguments)
{
    ASSERT_THROW(protocol::deserialize("Rtest\n"), SerializerError);
}

TEST(protocol_serialization, deserialize_too_many_arguments)
{
    ASSERT_THROW(protocol::deserialize("R12 13\n"), SerializerError);
}
