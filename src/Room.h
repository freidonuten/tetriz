//
// Created by martin on 03.01.20.
//

#ifndef UNTITLED_ROOM_H
#define UNTITLED_ROOM_H

#include <string>
#include <vector>
#include <functional>
#include <map>
#include "Player.h"


class Player;
class Room : public std::enable_shared_from_this<Room> {
public:
    enum class State;

private:
    static unsigned lastId;

    std::vector<std::shared_ptr<Player>> players;
    std::map<std::shared_ptr<Player>, std::vector<std::string>> moves;
    unsigned roomId;
    unsigned playerLimit;
    unsigned startTime;
    bool listed;

public:
    Room();
    Room(unsigned plimit, std::shared_ptr<Player> owner);

    void leave(std::shared_ptr<Player> player);
    bool join(std::shared_ptr<Player> player);
    void foreachPlayer(const std::function<void(const Player &)> &consumer);

    unsigned getId() const;
    unsigned getPlayerCount() const;
    unsigned getPlayerLimit() const;
    unsigned getDeltaT() const;
    std::shared_ptr<Player> getOwner() const;
    std::string& getMove(const std::shared_ptr<Player>& player, int i);
    void addMove(const std::shared_ptr<Player>& player, std::string move);
    int getMoveCount(const std::shared_ptr<Player>& player);

    bool operator<(const Room& r) const;
};


#endif //UNTITLED_ROOM_H
