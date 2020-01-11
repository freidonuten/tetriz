//
// Created by martin on 03.01.20.
//

#ifndef UNTITLED_ROOM_H
#define UNTITLED_ROOM_H

#include <string>
#include <vector>
#include <functional>
#include <map>
#include <tuple>

#include "Player.h"


class Player;
class Room : public std::enable_shared_from_this<Room> {
public:
    enum class State;

private:
    static unsigned lastId;

    std::vector<std::shared_ptr<Player>> players;
    std::map<
        std::shared_ptr<Player>,
        std::vector<std::tuple<std::string, long>>
    > moves;
    unsigned roomId;
    unsigned playerLimit;
    std::chrono::high_resolution_clock::time_point startTime;

public:
    Room();
    Room(unsigned plimit, std::shared_ptr<Player> owner);

    void kick(std::shared_ptr<Player> player);
    bool join(std::shared_ptr<Player> player);
    void foreachPlayer(const std::function<void(const std::shared_ptr<Player>&)> &consumer);

    unsigned getId() const;
    unsigned getPlayerCount() const;
    unsigned getPlayerLimit() const;
    long getDeltaT() const;
    std::shared_ptr<Player> getOwner() const;
    std::string& getMove(const std::shared_ptr<Player>& player, int i);
    void addMove(const std::shared_ptr<Player>& player, std::string move);
    int getMoveCount(const std::shared_ptr<Player>& player);
    bool isActive();

    bool operator<(const Room& r) const;

    long getMoveTimestamp(const std::shared_ptr<Player> &player, int i);

};


#endif //UNTITLED_ROOM_H
