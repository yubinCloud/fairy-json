//
// Created by yubin on 2021/5/14.
//
#include "utils.h"
#include <cstdlib>

using namespace std;

char* fetchStrFromCharStack(stack<char>& cStack, size_t len) {
    char* buf = (char*)malloc(len + 1);
    buf[len] = '\0';
    while (len != 0) {
        --len;
        buf[len] = cStack.top();
        cStack.pop();
    }
    return buf;
}