cmake_minimum_required(VERSION 3.15)
project(untitled)

set(CMAKE_CXX_STANDARD 20)

add_executable(untitled
        src/main.cpp
        src/Player.cpp src/Player.h
        src/Server.cpp src/Server.h
        src/Room.cpp src/Room.h
        src/Protocol.cpp src/Protocol.h
        src/MessageTokenizer.cpp src/MessageTokenizer.h
        src/Response.cpp src/Response.h
        src/constants.h
        src/CorruptedRequestException.h src/ProtoUtil.h)