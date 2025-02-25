#include "protocol_dispatch.hpp"
#include <algorithm>


namespace protocol
{
ProtocolDispatcher::ProtocolDispatcher(Model& model, Context& context)
    : model_(&model)
    , context_(&context)
{}

auto ProtocolDispatcher::operator()(const login_t& request) const -> Response
{
    if (auto player = context_->current_player(); player)
    {
        player->timestamp = Clock::now();
        return Response::success;
    }

    const auto name_exists = std::ranges::any_of(
        model_->players() | std::views::values,
        [&](const Player& player) { return player.name == request.user_name; });

    if (name_exists)
        return Response::fail;

    if (const auto oid = model_->player_create(request.user_name); oid)
    {
        oid_mapper.emplace(context_->file_descriptor(), *oid);
        return Response::success;
    }

    return Response::fail;
}

auto ProtocolDispatcher::operator()(const ping_t&) const -> Response
{
    return Response::success;
}

auto ProtocolDispatcher::operator()(const player_list_t& request) const -> Response
{
    const auto room = model_->room(request.room_id);

    if (!room)
        return Response::fail;

    auto response = Response{};
    for (const auto player_oid : room->players | std::views::keys)
        response.add_string(model_->player(player_oid)->name);

    return response;
}

auto ProtocolDispatcher::operator()(const room_list_t& request) const -> Response
{
    auto resp = Response{};

    for (const auto& [id, room] : model_->rooms())
        if (room.is_waiting())
            resp.add(id);

    return resp;
}

auto ProtocolDispatcher::operator()(const room_read_t& request) const -> Response
{
    const auto room = model_->room(request.room_id);
    const auto owner_name = room.and_then([this](const Room& room) -> std::optional<std::string> {
        if (room.players.size() > 0)
            return model_->player(room.owner)->name;
        else
            return std::nullopt;
    });

    return room
        ? Response(room->players.size(), room->player_limit, owner_name.value_or("<unknown>"))
        : Response::fail;
}

auto ProtocolDispatcher::operator()(const room_time_t&) const -> Response
{
    const auto room = context_->current_room();
    if (!room)
        return Response::fail;

    return -room->delta_t();
}


auto ProtocolDispatcher::operator()(const room_join_t& request) const -> Response
{
    const auto player_oid = context_->current_player_oid();
    const auto room_oid = request.room_id;

    return player_oid && model_->player_join_room(*player_oid, room_oid)
        ? Response::success
        : Response::fail;
}

auto ProtocolDispatcher::operator()(const room_leave_t&) const -> Response
{
    const auto player_oid = context_->current_player_oid();

    return player_oid && model_->player_leave_room(*player_oid)
        ? Response::success
        : Response::fail;
}

auto ProtocolDispatcher::operator()(const room_seed_t&) const -> Response
{
    const auto room = context_->current_room();

    return room
        ? Response(room->seed)
        : Response::fail;
}

auto ProtocolDispatcher::operator()(const room_new_t& request) const -> Response
{
    const auto room_oid = context_->current_player_oid()
        .and_then([&](auto oid) { return  model_->room_create(oid, request.player_limit); });

    return room_oid
        ? Response(*room_oid)
        : Response::fail;
}

auto ProtocolDispatcher::operator()([[maybe_unused]] const room_active_t& request) const -> Response
{
    const auto room_oid = context_->current_room_oid();

    return room_oid
        ? Response(*room_oid)
        : Response::fail;
}

auto ProtocolDispatcher::operator()(const room_delta_t&) const -> Response
{
    const auto room = context_->current_room();

    return room
        ? Response(std::max<int64_t>(0L, room->delta_t()))
        : Response::fail;
}
    
auto ProtocolDispatcher::operator()(const move_play_t& request) const -> Response
{
    const auto move_type = to_move_type(request.move);
    if (!move_type)
        return Response::fail;

    auto room = context_->current_room();
    if (!room || room->delta_t() > 0)
        return Response::fail;

    room->players
        .at(*context_->current_player_oid())
        .emplace_back(*move_type, Clock::now());

    return Response::success;
}

auto ProtocolDispatcher::operator()(const move_get_t& request) const -> Response
{
    const auto room = context_->current_room();
    if (!room)
        return Response::fail;

    const auto iterator = std::ranges::find_if(room->players, [&](const auto& p) {
        const auto oid = p.first;
        const auto player = model_->player(oid);
        return player && player->name == request.player;
    });

    if (iterator == room->players.end())
        return Response::fail;

    auto response = Response{};
    for (const auto [move, timestamp] : iterator->second | std::views::drop(request.index))
        response.add(
            std::string(from_move_type(move)),
            std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()).count()
        );

    return response;
}

auto ProtocolDispatcher::operator()(const move_last_timestamp_t& request) const -> Response
{
    const auto room = context_->current_room();
    const auto player = context_->current_player_oid();

    if (!room)
        return Response::fail;

    return room->players.at(*player).back().timestamp;
}
}
