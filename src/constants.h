#pragma once


// Server
constexpr inline auto PORT_NO = 4444;
constexpr inline auto EPOLL_EVENT_MAX = 5;
constexpr inline auto EPOLL_TIMEOUT = 100;
constexpr inline auto PLAYER_LIMIT = 2048;
constexpr inline auto ROOM_LIMIT = 256;

// Protocol
constexpr inline auto MSG_DELIM = ' ';
constexpr inline auto MSG_SEP = '\n';
constexpr inline auto MSG_SUCCESS = "S";
constexpr inline auto MSG_FAIL = "F";
constexpr inline auto RESPONSE_FAIL = "F\n";
constexpr inline auto RESPONSE_SUCCESS = "S\n";
constexpr inline auto DELTA_T_STARTED = 0x00000000;
constexpr inline auto DELTA_T_INFINITY = 0x7fffffff;
constexpr inline auto START_TIME_UNSET = 0x7fffffff;
constexpr inline auto ACTIVE_TIMEOUT_MS = 1500;
constexpr inline auto GAME_START_DELAY = 6000;
