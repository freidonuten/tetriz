//
// Created by martin on 03.01.20.
//

#include <chrono>
#include <iostream>
#include "Room.h"
#include "constants.h"

unsigned Room::lastId = 0;

Room::Room() {}

Room::Room(unsigned plimit, std::shared_ptr<Player> owner) {
    this->listed = true;
    this->roomId = ++Room::lastId;
    this->startTime = START_TIME_UNSET;
    this->playerLimit = plimit;
//    this->players.resize(plimit); // fixme does not compile -_-
    this->players.emplace_back(std::move(owner));
}

void Room::leave(std::shared_ptr<Player> player) {
    this->players.erase(
        std::remove_if(
            this->players.begin(),
            this->players.end(),
            [&player](const std::shared_ptr<Player>& p){ return p == player; }
        ), this->players.end()
    );

    player->setRoom(nullptr);
}

bool Room::join(std::shared_ptr<Player> player) {
    if (getPlayerCount() < getPlayerLimit() && !player->getRoom()) {
        player->setRoom(shared_from_this());
        this->players.push_back(std::move(player));
        if (this->players.size() == this->playerLimit) {
            this->startTime = millis_after(GAME_START_DELAY);
            this->listed = false;
            for(const std::shared_ptr<Player>& p : this->players){
                this->moves[p] = std::vector<std::string>();
            }
        }
        return true;
    }
    return false;
}

void Room::foreachPlayer(const std::function<void(const Player&)> &consumer) {
    for(const std::shared_ptr<Player>& p : this->players){
       consumer(*p);
    }
}

unsigned Room::getId() const {
    return this->roomId;
}

unsigned Room::getPlayerCount() const {
    return this->players.size();
}

unsigned Room::getPlayerLimit() const {
    return this->playerLimit;
}

std::shared_ptr<Player> Room::getOwner() const {
    return this->players[0];
}

bool Room::operator<(const Room& r) const {
    return this->getId() < r.getId();
}

unsigned Room::getDeltaT() const {
    return this->startTime < millis_now()
        ? 0
        : millis_to(this->startTime);
}

void Room::addMove(const std::shared_ptr<Player>& player, std::string move) {
    this->moves.at(player).push_back(std::move(move));
    std::cout << this->moves.at(player).at(0) << std::endl;
}

std::string& Room::getMove(const std::shared_ptr<Player>& player, int i) {
    return this->moves.at(player).at(i);
}

int Room::getMoveCount(const std::shared_ptr<Player> &player) {
    return this->moves.at(player).size();
}
