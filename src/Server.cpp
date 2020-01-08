//
// Created by martin on 03.01.20.
//

#include <algorithm>
#include <zconf.h>
#include "Server.h"
#include "CorruptedRequestException.h"

Server::Server() {
    this->proto = new Protocol(this);
}

bool Server::createPlayer(const std::string& name) {
    auto player = find_if(begin(this->playerPool), end(this->playerPool),
            [name] (const std::shared_ptr<Player>& p) { return *p == name; });

    if (player != end(this->playerPool)) {
        // reconnect player
        if ((*player)->isDead()){
            (*player)->connect(this->fd);
            return true;
        }
        // player exists and is active...
        return false;
    }

    // new player
    this->playerPool.insert(std::make_shared<Player>(name, this->fd));
    return true;
}

void Server::notify(int fileDescriptor) {
    this->active = nullptr;
    this->fd = fileDescriptor;

    // todo make it beautiful
    const int chunk_size = 16;
    std::string msg = std::string("");

    // should feed msg until buffer stops overflowing
    while (msg.length() % chunk_size == 0) {
        char buffer[chunk_size + 1]{};
        int result = read(fileDescriptor, buffer, chunk_size);
        msg += buffer;
        if (!msg.length()){ // fixme or result == 0?
            if (this->getActivePlayer()) {
                this->getActivePlayer()->disconnect();
            } else {
                close(this->fd);
            }
            return;
        }
    }

    // handle user request
    std::string response;
    try {
        response = this->proto->handle(msg);
    } catch (CorruptedRequestException& e){
        if (this->getActivePlayer()){
            this->getActivePlayer()->logout();
        } else {
            close(this->fd);
        }
    }
    char response_buffer[response.length()];

    // send response
    response.copy(response_buffer, response.length());
    write(fileDescriptor, response_buffer, response.length());
}

void Server::foreachRoom(const std::function<void(const std::shared_ptr<Room>&)>& consumer) {
    for (const std::shared_ptr<Room>& r: this->roomPool) {
        consumer(r);
    }
}

void Server::foreachPlayer(const std::function<void(const std::shared_ptr<Player>&)>& consumer) {
    for (const std::shared_ptr<Player>& p: this->playerPool) {
        consumer(p);
    }
}

bool Server::joinRoom(unsigned id) {
    // try to match player with file descriptor
    std::shared_ptr<Player> player = this->getPlayerByFD(this->fd);
    if (!player) {
        return false;
    }

    // try to find room with such id
    std::shared_ptr<Room> room = this->getRoomById(id);
    if (!room) {
        return false;
    }

    // try to join a return result
    return room->join(player);
}

unsigned Server::createRoom(unsigned plimit) {
    auto player = this->getPlayerByFD(this->fd);
    if (player) {
        std::shared_ptr<Room> room = player->getRoom();
        if (room){
            return 0; // player already has a room
        }
        room = std::make_shared<Room>(plimit, player);
        this->roomPool.insert(room);
        player->setRoom(room);

        return room->getId();
    }

    return 0;
}

std::shared_ptr<Room> Server::getRoomById(unsigned int roomId) {
    auto roomFound = std::find_if(
        begin(this->roomPool),
        end(this->roomPool),
        [&roomId](const std::shared_ptr<Room>& r){ return r->getId() == roomId; }
    );

    return roomFound == end(this->roomPool) ? nullptr : *roomFound;
}



std::shared_ptr<Player> Server::getPlayerByFD(int fileDescriptor) {
    auto playerFound = find_if(begin(this->playerPool), end(this->playerPool),
          [fileDescriptor] (const std::shared_ptr<Player>& p) {
                return p->getFileDescriptor() == fileDescriptor;
    });

    return playerFound == end(this->playerPool) ? nullptr : *playerFound;
}

std::shared_ptr<Room> Server::getRoomActive() {
    auto player = this->getPlayerByFD(this->fd);
    return player ? player->getRoom() : nullptr;
}

std::shared_ptr<Player> Server::getActivePlayer() {
    return !this->active
        ? (this->active = this->getPlayerByFD(this->fd))
        : this->active;
}

std::shared_ptr<Player> Server::getPlayerByName(const std::string &name) {
    auto player = std::find_if(this->playerPool.begin(), this->playerPool.end(),
            [&name](const std::shared_ptr<Player>& p){ return *p == name; });

    return player == this->playerPool.end() ? nullptr : *player;
}
