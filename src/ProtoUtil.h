//
// Created by martin on 05.01.20.
//

#ifndef UNTITLED_PROTOUTIL_H
#define UNTITLED_PROTOUTIL_H

#include <memory>
#include "MessageTokenizer.h"
#include "CorruptedRequestException.h"

namespace ProtoUtil {

    unsigned uint_query(std::string& msg) {
        MessageTokenizer mtok(msg);
        unsigned i = mtok.nextUInt();
        if (!mtok.isDone() || i == -1) {
            throw CorruptedRequestException();
        }
        return i;
    }

    std::string string_query(std::string& msg) {
        MessageTokenizer mtok = MessageTokenizer(msg);
        std::string s = mtok.nextString();
        if (s.length() == 0 || !mtok.isDone()){
            throw CorruptedRequestException();
        }
        return s;
    }

    void zero_arg_query(std::string& msg) {
        if (!MessageTokenizer(msg).isDone()){
            throw CorruptedRequestException();
        }
    }

}

#endif //UNTITLED_PROTOUTIL_H
