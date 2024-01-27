#include <iostream>
#include "Protocol.h"
#include "MessageTokenizer.h"
#include "Response.h"
#include "Server.h"
#include "Player.h"
#include "constants.h"
#include "CorruptedRequestException.h"
#include "ProtoUtil.h"
#include "Room.h"


enum class Message : unsigned char
{
    LOGIN           = 'L',
    ROOM_LIST       = 'C',
    PLAYER_LIST     = 'P',
    ROOM            = 'R',
    ROOM_TIME       = 'O',
    JOIN            = 'J',
    LEAVE           = 'V',
    NEW             = 'N',
    DELTA           = 'T',
    SEED            = 'S',
    MOVE            = 'M',
    MOVE_GET        = 'G',
    MOVE_LAST_TS    = 'X',
    PING            = '!',
    ACTIVE_ROOM     = '?',
};

Protocol::Protocol(Server* server)
    : server(server)
{ }

auto Protocol::handle(std::string& msg) -> std::string
{
    if (msg.empty())
    {
        throw CorruptedRequestException();
    }

    const auto command = static_cast<Message>(msg.front());

    if (server->get_active_player())
    {
        // logged in actions
        server->get_active_player()->refresh_timestamp();
        switch(command)
        {
            case Message::ROOM_LIST:    return handle_room_list(msg);
            case Message::NEW:          return handle_room_create(msg);
            case Message::JOIN:         return handle_room_join(msg);
            case Message::LEAVE:        return handle_room_leave(msg);
            case Message::ACTIVE_ROOM:  return handle_room_active(msg);
            case Message::ROOM_TIME:    return handle_room_time(msg);
            case Message::ROOM:         return handle_room(msg);
            case Message::PLAYER_LIST:  return handle_player_list(msg);
            case Message::DELTA:        return handle_delta(msg);
            case Message::MOVE:         return handle_move(msg);
            case Message::MOVE_GET:     return handle_move_get(msg);
            case Message::MOVE_LAST_TS: return handle_move_last_timestamp(msg);
            case Message::SEED:         return handle_seed(msg);
        }
    }

    // logged off actions
    switch(command) {
        case Message::LOGIN:        return handle_login(msg);
        case Message::PING:         return std::string{RESPONSE_SUCCESS};
    }

    throw CorruptedRequestException();
}

auto Protocol::handle_login(std::string& msg) -> std::string
{
    return server->create_player(ProtoUtil::string_query(msg))
        ? RESPONSE_SUCCESS
        : RESPONSE_FAIL;
}

auto Protocol::handle_room_list(std::string& msg) -> std::string
{
    ProtoUtil::zero_arg_query(msg);

    auto response = Response{};
    const auto record_id = [&response](const std::shared_ptr<Room>& room) {
        if (room->get_delta_t() >= 0)
        {
            response.add_int(room->get_id());
        }
    };

    server->foreach_room(record_id);

    // return response as string
    return response.to_string();
}

auto Protocol::handle_room_create(std::string& msg) -> std::string
{
    const auto playerLimit = ProtoUtil::uint_query(msg);
    const auto roomId = server->create_room(playerLimit);

    if (roomId == 0)
    {
        return RESPONSE_FAIL;
    }

    return Response()
        .add_int(roomId)
        .to_string();
}

auto Protocol::handle_room_join(std::string& msg) -> std::string
{
    return server->join_room(ProtoUtil::uint_query(msg))
        ? RESPONSE_SUCCESS
        : RESPONSE_FAIL;
}

auto Protocol::handle_room_leave(std::string& msg) -> std::string
{
    ProtoUtil::zero_arg_query(msg);

    auto room = server->get_active_room();
    if (!room)
    {
        return RESPONSE_FAIL;
    }

    room->kick(server->get_active_player());

    return RESPONSE_SUCCESS;
}

auto Protocol::handle_player_list(std::string &msg) -> std::string
{
    auto room = server->get_room_by_id(ProtoUtil::uint_query(msg));

    if (!room)
    {
        return RESPONSE_FAIL;
    }

    auto response = Response{};
    auto append_to_response = [&response](const auto& player){
        response.add_string(player->get_name());
    };

    room->foreach_player(append_to_response);

    return response.to_string();
}

auto Protocol::handle_room_active(std::string& msg) -> std::string
{
    ProtoUtil::zero_arg_query(msg);

    auto room = server->get_active_room();
    if (!room)
    {
        return RESPONSE_FAIL;
    }

    return Response().add_int(room->get_id()).to_string();
}

auto Protocol::handle_room_time(std::string &msg) -> std::string
{
    ProtoUtil::zero_arg_query(msg);

    auto room = server->get_active_room();
    if (!room)
    {
        return RESPONSE_FAIL;
    }

    return Response().add_int(-room->get_delta_t()).to_string();
}

auto Protocol::handle_room(std::string &msg) -> std::string
{
    auto room = server->get_room_by_id(ProtoUtil::uint_query(msg));
    if (!room)
    {
        return RESPONSE_FAIL;
    }

    return Response()
        .add_int(room->get_player_count())
        .add_int(room->get_player_limit())
        .add_string(room->get_owner()->get_name())
        .to_string();
}

auto Protocol::handle_delta(std::string& msg) -> std::string
{
    ProtoUtil::zero_arg_query(msg);

    auto room = server->get_active_room();
    if (!room)
    {
        return RESPONSE_FAIL;
    }

    long deltaT = std::max<long>(0L, room->get_delta_t());

    return Response()
        .add_int(deltaT)
        .to_string();
}

auto Protocol::handle_move(std::string &msg) -> std::string
{
    // invalid move?
    std::string move = ProtoUtil::string_query(msg);
    if (!Protocol::valid_moves.contains(move))
    {
        return RESPONSE_FAIL;
    }

    // is player supposed to even move?
    auto room = server->get_active_room();
    if (!room || room->get_delta_t() > 0)
    {
        return RESPONSE_FAIL;
    }

    // store the move
    room->add_move(server->get_active_player(), move);

    return RESPONSE_SUCCESS;
}

auto Protocol::handle_move_get(std::string &msg) -> std::string
{
    auto mtok = MessageTokenizer(msg);
    auto playerName = mtok.next_string();
    auto moveId = mtok.next_uint();

    if (!mtok.is_done() || moveId == -1)
    {
        return RESPONSE_FAIL;
    }

    auto response = Response{};
    auto room = server->get_active_room();

    if (!room)
    {
        return RESPONSE_FAIL;
    }

    auto player = server->get_player_by_name(playerName);
    auto max = room->get_move_count(player);

    for (int i = moveId; i < max; ++i)
    {
        response.add_string(room->get_move(player, i));
        response.add_int(room->get_move_timestamp(player, i));
    }

    return response.to_string();
}

auto Protocol::handle_move_last_timestamp(std::string &msg) -> std::string
{
    ProtoUtil::zero_arg_query(msg);

    auto room = server->get_active_room();

    if (!room)
    {
        return RESPONSE_FAIL;
    }

    auto player = server->get_active_player();

    return Response()
        .add_int(room->get_move_timestamp(player, room->get_move_count(player) - 1))
        .to_string();
}

auto Protocol::handle_seed(std::string &msg) -> std::string
{
    ProtoUtil::zero_arg_query(msg);

    auto room = server->get_active_room();

    if (!room)
    {
        return RESPONSE_FAIL;
    }

    return Response().add_int(room->get_seed()).to_string();
}

