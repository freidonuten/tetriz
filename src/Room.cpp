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
    this->roomId = ++Room::lastId;
//    this->startTime = START_TIME_UNSET;
    this->playerLimit = plimit;
//    this->players.resize(plimit); // fixme does not compile -_-
    this->players.emplace_back(std::move(owner));
}

void Room::kick(std::shared_ptr<Player> player) {
    player->setRoom(nullptr);

    auto playerPos = std::find_if(
            this->players.begin(),
            this->players.end(),
            [&player](const std::shared_ptr<Player>& p){ return p == player; }
    );

    if (playerPos != this->players.end()) {
        this->players.erase(playerPos);
    }
}

bool Room::join(std::shared_ptr<Player> player) {
    if (getPlayerCount() < getPlayerLimit() && !player->getRoom()) {
        player->setRoom(shared_from_this());
        this->players.push_back(std::move(player));
        if (this->players.size() == this->playerLimit) {
            this->startTime = std::chrono::high_resolution_clock::now()
                    + std::chrono::duration<int, std::milli>(GAME_START_DELAY);
//            this->startTime = millis_after(GAME_START_DELAY);
            for(const std::shared_ptr<Player>& p : this->players){
                this->moves[p] = std::vector<std::tuple<std::string, long>>();
            }
        }
        return true;
    }
    return false;
}

void Room::foreachPlayer(const std::function<void(const std::shared_ptr<Player>&)> &consumer) {
    for(const std::shared_ptr<Player>& p : this->players){
       consumer(p);
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

long Room::getDeltaT() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
            this->startTime - std::chrono::high_resolution_clock::now()).count();
}

void Room::addMove(const std::shared_ptr<Player>& player, std::string move) {
    this->moves.at(player).push_back(std::make_tuple(std::move(move), -getDeltaT()));
//    std::cout << std::get<0>(this->moves.at(player).at(0)) << std::endl;
}

std::string& Room::getMove(const std::shared_ptr<Player>& player, int i) {
    return std::get<0>(this->moves.at(player).at(i));
}

long Room::getMoveTimestamp(const std::shared_ptr<Player>& player, int i) {
    return std::get<1>(this->moves.at(player).at(i));
}

int Room::getMoveCount(const std::shared_ptr<Player> &player) {
    return this->moves.at(player).size();
}

bool Room::isActive() {
    return getPlayerCount() > 0 && std::find_if(this->players.begin(), this->players.end(),
        [](const std::shared_ptr<Player>& p){
            return !p->isDead();
        }
    ) != this->players.end();
}
