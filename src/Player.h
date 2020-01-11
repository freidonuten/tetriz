//
// Created by martin on 03.01.20.
//

#ifndef UNTITLED_PLAYER_H
#define UNTITLED_PLAYER_H

#include <string>
#include <memory>
#include <chrono>
#include "Room.h"

class Room;
class Player : public std::enable_shared_from_this<Player> {

private:
    enum class State {
        DISCONNECTED,
        LOBBY,
        PLAYING,
        SPECTATING
    };


    int sockfd;
    std::chrono::high_resolution_clock::time_point timestamp;
    std::string name;
    std::weak_ptr<Room> room;
    State state;

public:
    Player();
    Player(std::string name, int file_descriptor);

    int getFileDescriptor() const;
    std::string getName() const;
    std::weak_ptr<Room> getRoom() const;
    void logout();
    void connect(int fd);
    void disconnect();
    void setRoom(const std::shared_ptr<Room>& r);
    void refreshTimestamp();
    long getInactivityMs();
    bool isDead();

    bool operator<(const Player& player) const;
    bool operator==(const Player& player) const;
    bool operator==(const std::string& playerName) const;

};


#endif //UNTITLED_PLAYER_H
