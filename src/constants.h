//
// Created by martin on 05.01.20.
//

#ifndef UNTITLED_CONSTANTS_H
#define UNTITLED_CONSTANTS_H

/**
 * Server configuration
 */
#define PORT_NO 4444
#define EPOLL_EVENT_MAX 5
#define EPOLL_TIMEOUT 100

/**
* Protocol related stuff
*/
#define MSG_DELIM ' '
#define MSG_SEP '\n'
#define MSG_SUCCESS 'S'
#define MSG_FAIL 'F'
#define RESPONSE_FAIL (std::string(1, MSG_FAIL) + MSG_SEP)
#define RESPONSE_SUCCESS (std::string(1, MSG_SUCCESS) + MSG_SEP)
#define DELTA_T_STARTED  0x00000000
#define DELTA_T_INFINITY 0x7fffffff
#define START_TIME_UNSET 0x7fffffff
#define ACTIVE_TIMEOUT_MS 15000
#define GAME_START_DELAY 6000

/**
 * Helper macros
 */
#define millis_now() (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
#define millis_since(t) (millis_now() - (t))
#define millis_to(t) ((t) - millis_now())
#define millis_after(t) (millis_now() + (t))

#endif //UNTITLED_CONSTANTS_H
