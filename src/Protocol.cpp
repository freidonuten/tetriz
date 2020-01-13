//
// Created by martin on 03.01.20.
//

#include <iostream>
#include "Protocol.h"
#include "MessageTokenizer.h"
#include "Response.h"
#include "constants.h"
#include "CorruptedRequestException.h"
#include "ProtoUtil.h"


enum Message : unsigned char {
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

Protocol::Protocol(Server* server) {
    this->server = server;
}

std::string Protocol::handle(std::string& msg) {
    if (msg.length() == 0) {
        throw CorruptedRequestException();
    }

    if (this->server->getActivePlayer()){ // logged in actions
        this->server->getActivePlayer()->refreshTimestamp();
        switch(msg.at(0)) {
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
    switch(msg.at(0)) {
        case Message::LOGIN:        return handle_login(msg);
        case Message::PING:         return RESPONSE_SUCCESS;
    }

    throw CorruptedRequestException();
}

std::string Protocol::handle_login(std::string& msg) {
    return this->server->createPlayer(ProtoUtil::string_query(msg))
        ? RESPONSE_SUCCESS
        : RESPONSE_FAIL;
}

std::string Protocol::handle_room_list(std::string& msg) {
    ProtoUtil::zero_arg_query(msg);

    // prepare response
    Response response;
    this->server->foreachRoom([&response](const std::shared_ptr<Room>& room){
        if (room->getDeltaT() >= 0) {
            response.addInt(room->getId());
        }
    });

    // return response as string
    return response.toString();
}

std::string Protocol::handle_room_create(std::string& msg) {
    unsigned playerLimit = ProtoUtil::uint_query(msg);
    unsigned roomId = this->server->createRoom(playerLimit);

    if (roomId == 0){
        return RESPONSE_FAIL;
    }

    return Response()
        .addInt(roomId)
        .toString();
}

std::string Protocol::handle_room_join(std::string& msg) {
    return this->server->joinRoom(ProtoUtil::uint_query(msg))
        ? RESPONSE_SUCCESS
        : RESPONSE_FAIL;
}

std::string Protocol::handle_room_leave(std::string& msg) {
    ProtoUtil::zero_arg_query(msg);

    auto room = this->server->getActiveRoom();
    if (!room) {
        return RESPONSE_FAIL;
    }

    room->kick(this->server->getActivePlayer());

    return RESPONSE_SUCCESS;
}

std::string Protocol::handle_player_list(std::string &msg) {
    std::shared_ptr<Room> room = this->server->getRoomById(ProtoUtil::uint_query(msg));
    if (!room){
        return RESPONSE_FAIL;
    }

    Response response;
    room->foreachPlayer([&response](const std::shared_ptr<Player>& p){
        response.addString(p->getName());
    });

    return response.toString();
}

std::string Protocol::handle_room_active(std::string& msg) {
    ProtoUtil::zero_arg_query(msg);

    auto room = this->server->getActiveRoom();
    if (!room){
        return RESPONSE_FAIL;
    }

    return Response().addInt(room->getId()).toString();
}

std::string Protocol::handle_room_time(std::string &msg) {
    ProtoUtil::zero_arg_query(msg);

    auto room = this->server->getActiveRoom();
    if (!room) {
        return RESPONSE_FAIL;
    }

    return Response().addInt(-room->getDeltaT()).toString();
}

std::string Protocol::handle_room(std::string &msg) {
    std::shared_ptr<Room> room = this->server->getRoomById(ProtoUtil::uint_query(msg));
    if (!room) {
        return RESPONSE_FAIL;
    }

    return Response()
        .addInt(room->getPlayerCount())
        .addInt(room->getPlayerLimit())
        .addString(room->getOwner()->getName())
        .toString();
}

std::string Protocol::handle_delta(std::string& msg) {
    ProtoUtil::zero_arg_query(msg);

    std::shared_ptr<Room> room = this->server->getActiveRoom();
    if (!room){
        return RESPONSE_FAIL;
    }

    long deltaT = std::max<long>(0L, room->getDeltaT());

    return Response()
        .addInt(deltaT)
        .toString();
}

std::string Protocol::handle_move(std::string &msg) {
    // invalid move?
    std::string move = ProtoUtil::string_query(msg);
    if (!Protocol::validMoves.count(move)){
        return RESPONSE_FAIL;
    }

    // is player supposed to even move?
    std::shared_ptr<Room> room = this->server->getActiveRoom();
    if (!room || room->getDeltaT() > 0){
        return RESPONSE_FAIL;
    }

    // store the move
    room->addMove(this->server->getActivePlayer(), move);

    return RESPONSE_SUCCESS;
}

std::string Protocol::handle_move_get(std::string &msg) {
    MessageTokenizer mtok(msg);
    std::string playerName = mtok.nextString();
    int moveId = mtok.nextUInt();

    if (!mtok.isDone() || moveId == -1){
        return RESPONSE_FAIL;
    }

    Response response;

    auto room = this->server->getActiveRoom();
    if (!room) {
        return RESPONSE_FAIL;
    }

    auto player = this->server->getPlayerByName(playerName);
    int max = room->getMoveCount(player);

    for (int i = moveId; i < max; ++i) {
        response.addString(room->getMove(player, i));
        response.addInt(room->getMoveTimestamp(player, i));
    }

    return response.toString();
}

std::string Protocol::handle_move_last_timestamp(std::string &msg){
    ProtoUtil::zero_arg_query(msg);

    auto room = this->server->getActiveRoom();
    if (!room) {
        return RESPONSE_FAIL;
    }

    auto player = this->server->getActivePlayer();
    return Response()
        .addInt(room->getMoveTimestamp(player, room->getMoveCount(player) - 1))
        .toString();
}

std::string Protocol::handle_seed(std::string &msg) {
    ProtoUtil::zero_arg_query(msg);
    auto room = this->server->getActiveRoom();
    if (!room) {
        return RESPONSE_FAIL;
    }

    return Response().addInt(room->getSeed()).toString();
}

