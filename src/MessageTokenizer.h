//
// Created by martin on 03.01.20.
//

#ifndef UNTITLED_MESSAGETOKENIZER_H
#define UNTITLED_MESSAGETOKENIZER_H

#include <memory>
#include <string>

class MessageTokenizer {
private:
    std::string base;
    char lead;
    int tok_start;
    int tok_end;

    void consumeDelimiters();
    bool isTerminated();
    bool isExhausted();

public:
    MessageTokenizer(std::string& message);
    bool isDone();
    char leadingChar();
    int nextUInt();
    std::string nextString();

};


#endif //UNTITLED_MESSAGETOKENIZER_H
