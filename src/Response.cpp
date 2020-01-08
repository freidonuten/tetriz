//
// Created by martin on 03.01.20.
//

#include "Response.h"
#include "constants.h"

Response& Response::addInt(unsigned i) {
    this->ints.push_back(i);
    return *this;
}

Response& Response::addString(const std::string& s) {
    this->strings.push_back(s);
    return *this;
}

Response& Response::setFail() {
    this->success = false;
    return *this;
}

Response& Response::setSuccess() {
    this->success = true;
    return *this;
}

std::string Response::toString() {
    std::string response = this->success
            ? std::string(1, MSG_SUCCESS)
            : std::string(1, MSG_FAIL);

    for (unsigned i: this->ints) {
        response.append(" " + std::to_string(i));
    }

    for (const std::string& s: this->strings) {
        response.append(" " + s);
    }

    return response + MSG_SEP;
}


