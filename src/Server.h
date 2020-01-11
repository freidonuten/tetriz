//
// Created by martin on 03.01.20.
//

#ifndef UNTITLED_SERVER_H
#define UNTITLED_SERVER_H

#include <map>
#include <set>
#include <memory>
#include <functional>

#include "Player.h"
#include "Room.h"
#include "Protocol.h"

class Protocol;
class Server {

private:
    Protocol* proto;
    std::set<std::shared_ptr<Player>> playerPool;
    std::set<std::shared_ptr<Room>> roomPool;
    std::shared_ptr<Player> active;
    std::function<void(const int)> close;
    int roomLimit;
    int fd;


public:
    Server(int roomLimit);

    unsigned createRoom(unsigned plimit);
    bool createPlayer(const std::string& name);
    bool joinRoom(unsigned id);
    std::shared_ptr<Room> getRoomById(unsigned roomId);
    void notify(int fileDescriptor);
    void cleanUpRooms();
    void setCloseFunction(std::function<void(const int)> close);

    std::shared_ptr<Player> getPlayerByFD(int fileDescriptor);
    std::shared_ptr<Player> getPlayerByName(const std::string& name);
    std::shared_ptr<Room> getActiveRoom();
    std::shared_ptr<Player> getActivePlayer();

    void foreachRoom(const std::function<void(const std::shared_ptr<Room> &)> &consumer);
    void foreachPlayer(const std::function<void(const std::shared_ptr<Player> &)> &consumer);
    void closeActive();
};


#endif //UNTITLED_SERVER_H
