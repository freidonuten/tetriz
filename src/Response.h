//
// Created by martin on 03.01.20.
//

#ifndef UNTITLED_RESPONSE_H
#define UNTITLED_RESPONSE_H

#include <vector>
#include <string>
#include <memory>


class Response {

private:
    bool success = true;
    std::vector<unsigned> ints;
    std::vector<std::string> strings;

public:
    Response& addInt(unsigned i);
    Response& addString(const std::string& s);
    Response& setFail();
    Response& setSuccess();
    std::string toString();
};


#endif //UNTITLED_RESPONSE_H
