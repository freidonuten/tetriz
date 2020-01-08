//
// Created by martin on 05.01.20.
//

#ifndef UNTITLED_CORRUPTEDREQUESTEXCEPTION_H
#define UNTITLED_CORRUPTEDREQUESTEXCEPTION_H


#include <bits/exception.h>
#include <string>

class CorruptedRequestException: public std::exception {

public:
    virtual const char* what() {
        return "Corrupted request!";
    }
};


#endif //UNTITLED_CORRUPTEDREQUESTEXCEPTION_H
