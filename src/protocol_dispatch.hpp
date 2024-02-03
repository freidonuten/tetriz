#pragma once

#include "protocol_structs.hpp"
#include "Player.h"
#include "Response.h"
#include "Room.h"
#include "Server.h"
#include "util.h"
#include <variant>

namespace protocol
{
    class ProtocolDispatcher
    {
    public:
        constexpr
        ProtocolDispatcher(Server& server)
            : server(&server)
        {}

        auto operator()(const login_t& request) const -> Response
        {
            return server->create_player(request.user_name)
                ? Response::success
                : Response::fail;
        }

        auto operator()([[maybe_unused]] const ping_t& request) const -> Response
        {
            return Response::success;
        }

        auto operator()(const player_list_t& request) const -> Response
        {

            const auto room = server->get_room_by_id(request.room_id);

            if (!room)
            {
                return Response::fail;
            }

            auto response = Response{};
            auto append_to_response = [&response](const auto& player){
                response.add_string(player->get_name());
            };

            room->foreach_player(append_to_response);

            return response;
        }

        auto operator()(const room_list_t& request) const -> Response
        {
            auto resp = Response{};
            const auto record_id = [&resp](const auto& room) {
                if (room->get_delta_t() >= 0)
                {
                    resp.add_int(room->get_id());
                }
            };

            server->foreach_room(record_id);

            return resp;
        }

        auto operator()(const room_read_t& request) const -> Response
        {
            const auto room = server->get_room_by_id(request.room_id);

            return room
                ? Response()
                    .add_int(room->get_player_count())
                    .add_int(room->get_player_limit())
                    .add_string(room->get_owner()->get_name())
                : Response::fail;
        }

        auto operator()([[maybe_unused]] const room_time_t& request) const -> Response
        {
            const auto room = server->get_active_room();

            return room
                ? Response().add_int(-room->get_delta_t())
                : Response::fail;
        }

        auto operator()(const room_join_t& request) const -> Response
        {
            return server->join_room(request.room_id)
                ? Response::success
                : Response::fail;
        }

        auto operator()([[maybe_unused]] const room_leave_t& request) const -> Response
        {
            const auto room = server->get_active_room();
            if (!room)
            {
                return Response::fail;
            }

            room->kick(server->get_active_player());

            return Response::success;
        }

        auto operator()([[maybe_unused]] const room_seed_t& request) const -> Response
        {
            const auto room = server->get_active_room();

            return room
                ? Response().add_int(room->get_seed())
                : Response::fail;
        }

        auto operator()(const room_new_t& request) const -> Response
        {
            const auto room_id = server->create_room(request.player_limit);

            return room_id == 0
                ? Response::fail
                : Response().add_int(room_id);
        }

        auto operator()([[maybe_unused]] const room_active_t& request) const -> Response
        {
            const auto room = server->get_active_room();

            return room
                ? Response().add_int(room->get_id())
                : Response::fail;
        }

        auto operator()([[maybe_unused]] const room_delta_t& request) const -> Response
        {
            const auto room = server->get_active_room();

            return room
                ? Response().add_int(std::max<long>(0L, room->get_delta_t()))
                : Response::fail;
        }
            
        auto operator()(const move_play_t& request) const -> Response
        {
            if (!util::is_any<"L", "R", "D", "SWP", "ROT", "DRP">(request.move))
            {
                return Response::fail;
            }

            // is player supposed to even move?
            const auto room = server->get_active_room();
            if (!room || room->get_delta_t() > 0)
            {
                return Response::fail;
            }

            // store the move
            room->add_move(server->get_active_player(), request.move);

            return Response::success;
        }

        auto operator()(const move_get_t& request) const -> Response
        {
            const auto room = server->get_active_room();

            if (!room)
            {
                return Response::fail;
            }

            const auto player = server->get_player_by_name(request.player);
            const auto max = room->get_move_count(player);
            auto response = Response{};

            for (int i = request.index; i < max; ++i)
            {
                response.add_string(room->get_move(player, i));
                response.add_int(room->get_move_timestamp(player, i));
            }

            return response;
        }

        auto operator()([[maybe_unused]] const move_last_timestamp_t& request) const -> Response
        {
            const auto room = server->get_active_room();

            if (!room)
            {
                return Response::fail;
            }

            const auto player = server->get_active_player();
            const auto move = room->get_move_count(player) - 1;

            return Response().add_int(room->get_move_timestamp(player, move));
        }

    private:
        Server* server;
    };
}
