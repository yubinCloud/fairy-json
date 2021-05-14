//
// Created by yubin on 2021/5/14.
//

#pragma once

#include <stack>

/**
 * 判断一个数字是否属于 1 ~ 9
 * @param ch 所要判断的数字字符
 * @return 是则返回 true，否则返回 false
 */
inline bool isDigitFrom1To9(char ch) {
    return ch >= '1' && ch <= '9';
}

/**
 * 判断一个字符是否是数字
 * @param ch 所要判断的字符
 * @return 是则返回 true，否则返回 false
 */
inline bool isDigit(char ch) {
    return ch >= '0' && ch <= '9';
}

char* fetchStrFromCharStack(std::stack<char>& cStack, size_t len);

/**
 * 对一个栈进行弹出 N 次的操作
 * @param s STL中的栈
 * @param n 弹出的次数
 */
void popN(std::stack<char>& s, size_t n);
