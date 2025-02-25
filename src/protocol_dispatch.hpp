#pragma once

#include "protocol_structs.hpp"
#include "Model.hpp"
#include "Response.h"
#include "Context.hpp"


namespace protocol
{
class ProtocolDispatcher
{
public:
    ProtocolDispatcher(Model& model, Context& context);

    auto operator()(const login_t&) const -> Response;
    auto operator()(const ping_t&) const -> Response;
    auto operator()(const player_list_t&) const -> Response;
    auto operator()(const room_list_t&) const -> Response;
    auto operator()(const room_read_t&) const -> Response;
    auto operator()(const room_time_t&) const -> Response;
    auto operator()(const room_join_t&) const -> Response;
    auto operator()(const room_leave_t&) const -> Response;
    auto operator()(const room_seed_t&) const -> Response;
    auto operator()(const room_new_t&) const -> Response;
    auto operator()(const room_active_t&) const -> Response;
    auto operator()(const room_delta_t&) const -> Response;
    auto operator()(const move_play_t&) const -> Response;
    auto operator()(const move_get_t&) const -> Response;
    auto operator()(const move_last_timestamp_t&) const -> Response;

private:
    Model* model_;
    Context* context_;
};
}
