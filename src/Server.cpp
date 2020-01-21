//
// Created by martin on 03.01.20.
//

#include <algorithm>
#include <zconf.h>
#include <iostream>
#include "Server.h"
#include "CorruptedRequestException.h"
#include "constants.h"


bool Server::createPlayer(const std::string& name) {
    auto player = find_if(begin(this->playerPool), end(this->playerPool),
            [name] (const std::shared_ptr<Player>& p) { return *p == name; });

    if (player != end(this->playerPool)) {
        // reconnect player
        if ((*player)->isDead() || (*player)->getInactivityMs() > ACTIVE_TIMEOUT_MS){
            (*player)->connect(this->fd);
            return true;
        }
        // if player is logged in, just nod him success
        return (*player)->getFileDescriptor() == this->fd;
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
    while (true) {
        char buffer[chunk_size + 1]{};
        int result = read(fileDescriptor, buffer, chunk_size);
        msg += buffer;
        if (result == 0) {
            if (this->getActivePlayer()) {
                this->getActivePlayer()->disconnect();
            }
            this->close(this->fd);
            return;
        } else if (result != chunk_size || msg[msg.length() - 1] == MSG_SEP) {
            break; // did not fill the buffer
        }
    }

    // handle user request
    std::string response;
    try {
        std::cout << "[" << this->fd << "] Received: " << msg;
        response = this->proto->handle(msg);
    } catch (CorruptedRequestException& e){
        std::cout << "[" << this->fd << "] Corrupted request, closing client socket...\n";
        if (this->getActivePlayer()){
            this->getActivePlayer()->logout();
        }
        this->close(this->fd);
    }
    char response_buffer[response.length()];

    // send response
    std::cout << "[" << this->fd << "] Sending: " << response;
    response.copy(response_buffer, response.length());
    write(fileDescriptor, response_buffer, response.length());

    // disconnect if login failed
    if (!this->getActivePlayer()){
        this->close(this->fd);
    }
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
    if (!room || room->getDeltaT() < 0) {
        return false;
    }

    // try to join a return result
    return room->join(player);
}

unsigned Server::createRoom(unsigned plimit) {
    auto player = this->getPlayerByFD(this->fd);
    if (player && this->roomLimit > this->roomPool.size()) {
        std::shared_ptr<Room> room = player->getRoom().lock();
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

std::shared_ptr<Room> Server::getActiveRoom() {
    auto player = this->getPlayerByFD(this->fd);
    return player ? player->getRoom().lock() : nullptr;
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

void Server::cleanUpRooms() {
    for (auto roomIt = this->roomPool.begin(); roomIt != this->roomPool.end(); ) {
        auto room = *roomIt;
        if (!room->isActive()) {
            room->foreachPlayer([&room](const std::shared_ptr<Player>& player){
                if (player){
                    room->kick(player);
                }
            });
            std::cout << "[GC] room_" << room->getId() << std::endl;
            this->roomPool.erase(roomIt++);
        } else {
            ++roomIt;
        }
    }
}

void Server::closeActive() {
    this->close(this->fd);
}

void Server::setCloseFunction(std::function<void(const int)> closeFunc) {
    this->close = std::move(closeFunc);

}

Server::Server(int roomLimit) {
    this->proto = new Protocol(this);
    this->roomLimit = roomLimit;
    this->roomPool = std::set<std::shared_ptr<Room>>();
    this->playerPool = std::set<std::shared_ptr<Player>>();
    this->fd = -1;
}
