//
// Created by martin on 03.01.20.
//

#include "MessageTokenizer.h"
#include "constants.h"



MessageTokenizer::MessageTokenizer(std::string& message) {
    this->base = message;
    this->tok_end = 1;
    this->lead = this->base.at(0);

    // trim start
    consumeDelimiters();
    this->tok_start = this->tok_end;
}

bool MessageTokenizer::isTerminated() {
    return this->base.at(this->base.length() - 1) == MSG_SEP;
}

bool MessageTokenizer::isExhausted() {
    return this->tok_start >= this->base.length() - 1;
}

char MessageTokenizer::leadingChar() {
    return this->lead;
}

int MessageTokenizer::nextUInt() {

    int result = 0;

    // build result
    while (this->tok_end < this->base.length()){
        char c = this->base.at(this->tok_end);
        if (isdigit(c)) {
            result = result * 10 + c - '0';
            ++this->tok_end;
        } else {
            break;
        }
    }

    // return result
    if (this->tok_end != this->tok_start){
        consumeDelimiters();
        this->tok_start = this->tok_end;
        return result;
    } else {
        return -1;
    }

}

std::string MessageTokenizer::nextString() {

    // iterate until delim/end of message
    while (this->tok_end < this->base.length()) {
        char c = this->base.at(this->tok_end);

        if (c == MSG_DELIM || c == MSG_SEP) {
            break;
        }

        ++this->tok_end;
    }

    // build result and set new boundaries
    std::string result = this->base.substr(this->tok_start, this->tok_end - this->tok_start);
    consumeDelimiters();
    this->tok_start = this->tok_end;

    return result;
}

void MessageTokenizer::consumeDelimiters() {
    while (this->base.at(this->tok_end) == MSG_DELIM){
        ++this->tok_end;
    }
}

bool MessageTokenizer::isDone() {
    return isTerminated() && isExhausted();
}



