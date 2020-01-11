//
// Created by martin on 03.01.20.
//

#include "Player.h"

#include <utility>
#include <zconf.h>
#include <chrono>
#include "constants.h"

Player::Player() {
   this->refreshTimestamp();
}

bool Player::operator<(const Player& player) const {
    return this->name < player.name;
}

bool Player::operator==(const Player& player) const {
    return this->name == player.name;
}

bool Player::operator==(const std::string& playerName) const {
    return this->name == playerName;
}

Player::Player(std::string name, int fileDescriptor) {
    this->refreshTimestamp();
    this->name = std::move(name);
    this->state = State::LOBBY;
    this->sockfd = fileDescriptor;
}

int Player::getFileDescriptor() const {
    return this->sockfd;
}

std::string Player::getName() const {
    return this->name;
}

std::shared_ptr<Room> Player::getRoom() const {
    return this->room;
}

void Player::setRoom(std::shared_ptr<Room> r) {
    this->room = std::move(r);
}

void Player::logout() {
    this->disconnect();
    if (this->room){
        this->room->kick(shared_from_this());
    }
}

void Player::disconnect() {
    close(this->sockfd);
    this->sockfd = -1;
    this->state = Player::State::DISCONNECTED;
}

void Player::connect(int fd) {
    this->sockfd = fd;
    this->state = Player::State::LOBBY;
    this->refreshTimestamp();
}

void Player::refreshTimestamp() {
    this->timestamp = std::chrono::high_resolution_clock::now();
}

long Player::getInactivityMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - this->timestamp).count();
}

bool Player::isDead() {
    return this->state == Player::State::DISCONNECTED
        || this->getInactivityMs() > ACTIVE_TIMEOUT_MS;
}

