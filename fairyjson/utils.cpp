//
// Created by yubin on 2021/5/14.
//
#include "utils.h"
#include <cstdlib>

using namespace std;
using namespace fairy;

char* fetchStrFromCharStack(stack<char>& cStack, size_t len) {
    char* buf = new char[len + 1];
    buf[len] = '\0';
    while (len != 0) {
        --len;
        buf[len] = cStack.top();
        cStack.pop();
    }
    return buf;
}


void popN(std::stack<char>& s, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        s.pop();
    }
}

const char* parseHex4(const char* p, unsigned* u) {
    *u = 0;
    for (int i = 0; i < 4; ++i) {
        char ch = *p++;
        *u <<= 4;
        if      (ch >= '0' && ch <= '9')  *u |= ch - '0';
        else if (ch >= 'A' && ch <= 'F')  *u |= ch - ('A' - 10);
        else if (ch >= 'a' && ch <= 'f')  *u |= ch - ('a' - 10);
        else return nullptr;
    }
    return p;
}


static void putCharToStack(fairy::ParseContext* c, unsigned const value) {
    assert(value <= 255);
    c->charStack.push(static_cast<char>(value));
}


void encodeUtf8(fairy::ParseContext* c, unsigned u) {
    if (u <= 0x7F)
        putCharToStack(c, u & 0xFF);
    else if (u <= 0x7FF) {
        putCharToStack(c, 0xC0 | ((u >> 6) & 0xFF));
        putCharToStack(c, 0x80 | ( u       & 0x3F));
    }
    else if (u <= 0xFFFF) {
        putCharToStack(c, 0xE0 | ((u >> 12) & 0xFF));
        putCharToStack(c, 0x80 | ((u >>  6) & 0x3F));
        putCharToStack(c, 0x80 | ( u        & 0x3F));
    }
    else {
        assert(u <= 0x10FFFF);
        putCharToStack(c, 0xF0 | ((u >> 18) & 0xFF));
        putCharToStack(c, 0x80 | ((u >> 12) & 0x3F));
        putCharToStack(c, 0x80 | ((u >>  6) & 0x3F));
        putCharToStack(c, 0x80 | ( u        & 0x3F));
    }
}