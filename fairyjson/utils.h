//
// Created by yubin on 2021/5/14.
//

#pragma once

#include <stack>
#include "fairy_json.h"

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


/**
 * 在解析字符串时出现错误所调用的函数，该函数会完成一些退出前的清理工作
 * @param c 解析上下文
 * @param head 原先 stack 的头位置
 * @param retStatus 解析的错误状态
 * @return 完成清理工作后直接返回所传入的错误状态
 */
inline fairy::JsonParseStatus strParseError(fairy::ParseContext* c, size_t head, const fairy::JsonParseStatus retStatus) {
    popN(c->charStack, c->charStack.size() - head);
    return retStatus;
}