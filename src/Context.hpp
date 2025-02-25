#pragma once

#include "Model.hpp"

#include <cstdint>


class Context
{
public:
    Context(Model& model) : model_(&model) {}

    void rebind(int32_t file_descriptor)
    {
        file_descriptor_ = file_descriptor;
    }

    auto current_player_oid() const -> std::optional<ObjectId>
    {
        if (const auto oid_iter = oid_mapper.find(file_descriptor_); oid_iter != oid_mapper.end())
            return oid_iter->second;
        else
            return {};
    }

    auto current_room_oid() const -> std::optional<ObjectId>
    {
        return current_player().and_then([](Player& p){ return p.room_oid; });
    }

    auto current_player() const -> optional_ref<Player>
    {
        if (const auto oid = current_player_oid(); oid)
            return model_->player(*oid);
        else
            return {};
    }

    auto current_room() const -> optional_ref<Room>
    {
        if (const auto oid = current_room_oid(); oid)
            return model_->room(*oid);
        else
            return {};
    }

    auto file_descriptor() const -> int32_t
    {
        return file_descriptor_;
    }

private:
    Model* model_;
    int32_t file_descriptor_;
};
