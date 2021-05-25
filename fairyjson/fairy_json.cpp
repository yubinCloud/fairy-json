//
// Created by yubin on 2021/5/14.
//

#include "fairy_json.h"
#include <cassert>
#include <cmath>
#include <stack>
#include <vector>
#include <algorithm>
#include "utils.h"


#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

using namespace std;

namespace fairy {

    /**
     * 解析空白符
     * ws = *(%x20 / %x09 / %x0A / %x0D)
     * @param c 解析上下文
     */
    static void parseWhitespace(ParseContext* c) {
        const char *p = c->json;
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
            p++;
        c->json = p;
    }

    /**
     * 解析 null
     * null  = "null"
     * @param c
     * @param v
     * @return
     */
    static JsonParseStatus parseNull(ParseContext* c, FieldValue* v) {
        EXPECT(c, 'n');
        if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
            return JsonParseStatus::PARSE_INVALID_VALUE;
        c->json += 3;
        v->type = JsonFieldType::J_NULL;
        return JsonParseStatus::PARSE_OK;
    }

    /**
     * 解析 true
     * true == "true"
     * @param c
     * @param v
     * @return
     */
    static JsonParseStatus parseTrue(ParseContext* c, FieldValue* v) {
        EXPECT(c, 't');
        if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e') {
            return JsonParseStatus::PARSE_INVALID_VALUE;
        }
        c->json += 3;
        v->type = JsonFieldType::J_TRUE;
        return JsonParseStatus::PARSE_OK;
    }

    /**
     * 解析 false
     * @param c
     * @param v
     * @return
     */
    static JsonParseStatus parseFalse(ParseContext* c, FieldValue* v) {
        EXPECT(c, 'f');
        if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e') {
            return JsonParseStatus::PARSE_INVALID_VALUE;
        }
        c->json += 4;
        v->type = JsonFieldType::J_FALSE;
        return JsonParseStatus::PARSE_OK;
    }

    static JsonParseStatus parseNumber(ParseContext* c, FieldValue* v) {
        // 校验数字
        const char* p = c->json;
        if (*p == '-') p++;
        if (*p == '0') p++;
        else {
            if (!isDigitFrom1To9(*p))
                return JsonParseStatus::PARSE_INVALID_VALUE;
            for (p++; isDigit(*p); p++);
        }
        if (*p == '.') {
            p++;
            if (!isDigit(*p))
                return JsonParseStatus::PARSE_INVALID_VALUE;
            for (p++; isDigit(*p); p++);
        }
        if (*p == 'e' || *p == 'E') {
            p++;
            if (*p == '+' || *p == '-') p++;
            if (!isDigit(*p))
                return JsonParseStatus::PARSE_INVALID_VALUE;
            for (p++; isDigit(*p); p++);
        }
        errno = 0;
        v->data.n = std::strtod(c->json, nullptr);
        if (errno == ERANGE && (v->data.n == HUGE_VAL || v->data.n == -HUGE_VAL))
            return JsonParseStatus::PARSE_NUMBER_OVERFLOW;
        v->type = JsonFieldType::J_NUMBER;
        c->json = p;
        return JsonParseStatus::PARSE_OK;
    }

    static JsonParseStatus parseStringRaw(ParseContext* c, char** pStr, size_t* pLen) {
        EXPECT(c, '\"');
        size_t head = c->charStack.size();
        const char* p = c->json;
        size_t len;
        unsigned u = 0, u2 = 0;  // 存储码点
        while (true) {
            auto ch = *p++;
            switch (ch) {
                case '\"':
                    len = c->charStack.size() - head;
                    *pStr = fetchStrFromCharStack(c->charStack, len);
                    *pLen = len;
                    c->json = p;
                    return JsonParseStatus::PARSE_OK;
                case '\\':
                    switch (*p++) {
                        case '\"': c->charStack.push('\"'); break;
                        case '\\': c->charStack.push('\\'); break;
                        case '/':  c->charStack.push('/');  break;
                        case 'b':  c->charStack.push('\b'); break;
                        case 'f':  c->charStack.push('\f'); break;
                        case 'n':  c->charStack.push('\n'); break;
                        case 'r':  c->charStack.push('\r'); break;
                        case 't':  c->charStack.push('\t'); break;
                        case 'u':  // 对 Unicode 的处理
                            if (!(p = parseHex4(p, &u)))
                                strParseError(c, head, JsonParseStatus::PARSE_INVALID_UNICODE_HEX);
                            // surrogate handling
                            if (u >= 0xD800 && u <= 0xDBFF) {
                                if (*p++ != '\\')
                                    strParseError(c, head, JsonParseStatus::PARSE_INVALID_UNICODE_SURROGATE);
                                if (*p++ != 'u')
                                    strParseError(c, head, JsonParseStatus::PARSE_INVALID_UNICODE_SURROGATE);
                                if (!(p = parseHex4(p, &u2)))
                                    strParseError(c, head, JsonParseStatus::PARSE_INVALID_UNICODE_HEX);
                                if (u2 < 0xDC00 || u2 > 0xDFFF)
                                    strParseError(c, head, JsonParseStatus::PARSE_INVALID_UNICODE_SURROGATE);
                                u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                            }
                            encodeUtf8(c, u);
                            break;
                        default:
                            return strParseError(c, head, JsonParseStatus::PARSE_INVALID_STRING_ESCAPE);
                    }
                    break;
                case '\0':
                    return strParseError(c, head, JsonParseStatus::PARSE_MISS_QUOTATION_MARK);
                default:
                    //  %x22 是双引号，%x5C 是反斜线，都已经处理。所以不合法的字符是 %x00 至 %x1F。我们简单地在 default 里处理：
                    if ((unsigned char)ch < 0x20) {
                        return strParseError(c, head, JsonParseStatus::PARSE_INVALID_STRING_CHAR);
                    }
                    c->charStack.push(ch);
            }
        }
    }

    static JsonParseStatus parseString(ParseContext* c, FieldValue* v) {
        char* s = nullptr;
        size_t len = 0;
        const auto parseRet = parseStringRaw(c, &s, &len);
        if (parseRet == JsonParseStatus::PARSE_OK) {
            v->setJStr(s, len);
            v->setType(JsonFieldType::J_STRING);
        }
        return parseRet;
    }

    static JsonParseStatus parseValue(ParseContext* c, FieldValue* v);

    static JsonParseStatus parseArray(ParseContext* c, FieldValue* v) {
        EXPECT(c, '[');
        parseWhitespace(c);
        if (*c->json == ']') {
            ++c->json;
            v->setType(JsonFieldType::J_ARRAY);
            v->setArray(new vector<FieldValue>());
            return JsonParseStatus::PARSE_OK;
        }
        size_t arraySize = 0;
        JsonParseStatus parseRet;
        while (true) {
            FieldValue e;
            parseRet = parseValue(c, &e);
            if (parseRet != JsonParseStatus::PARSE_OK)
                break;
            c->fieldStack.push(e);
            ++arraySize;
            parseWhitespace(c);
            if (*c->json == ',') {
                ++c->json;
                parseWhitespace(c);
            } else if (*c->json == ']') {
                ++c->json;
                v->setType(JsonFieldType::J_ARRAY);
                v->data.array = new vector<FieldValue>();
                for (size_t i = 0; i < arraySize; ++i) {
                    v->data.array->push_back(c->fieldStack.top());
                    c->fieldStack.pop();
                }
                reverse(v->data.array->begin(), v->data.array->end());
                return JsonParseStatus::PARSE_OK;
            } else {
                parseRet = JsonParseStatus::PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
                break;
            }
        }
        // Pop and free values on the stack
        for (size_t i = 0; i < arraySize; ++i) {
            auto tempValue = c->fieldStack.top();
            c->fieldStack.pop();
            tempValue.freeSpace();
        }
        return parseRet;
    }

    static JsonParseStatus parseObject(ParseContext* c, FieldValue* v) {
        EXPECT(c, '{');
        JsonParseStatus retStatus;
        parseWhitespace(c);
        v->data.obj = new multimap<string, FieldValue>();
        if (*c->json == '{') {
            ++c->json;
            v->setType(JsonFieldType::J_OBJECT);
            return JsonParseStatus::PARSE_OK;
        }
        while (true) {
            char* keyStr = nullptr;
            size_t keyStrLen = 0;
            FieldValue objValue;  // 对象的一个属性
            // parse key
            if (*c->json != '"') {
                retStatus = JsonParseStatus::PARSE_MISS_KEY;
                break;
            }
            retStatus = parseStringRaw(c, &keyStr, &keyStrLen);
            if (retStatus != JsonParseStatus::PARSE_OK)
                break;
            const string objKey = string(keyStr, keyStrLen);
            // parse ws colon ws
            parseWhitespace(c);
            if (*c->json != ':') {
                retStatus = JsonParseStatus::PARSE_MISS_COLON;
                break;
            }
            c->json++;
            parseWhitespace(c);
            // parse value
            retStatus = parseValue(c, &objValue);
            if (retStatus != JsonParseStatus::PARSE_OK) {
                break;
            }
            // 完成一个键值对的解析
            v->data.obj->insert({objKey, objValue});
            // parse ws [comma | right-curly-brace] ws
            parseWhitespace(c);
            if (*c->json == ',') {
                c->json++;
                parseWhitespace(c);
            }
            else if (*c->json == '}') {
                c->json++;
                v->setType(JsonFieldType::J_OBJECT);
                return JsonParseStatus::PARSE_OK;
            }
            else {
                retStatus = JsonParseStatus::PARSE_MISS_COMMA_OR_CURLY_BRACKET;
                break;
            }
        }
        // 清理已申请的空间
        v->freeSpace();
        v->setType(JsonFieldType::J_NULL);
        return retStatus;
    }


        /**
         * 解析 json 值
         * @param c
         * @param v
         * @return
         */
    static JsonParseStatus parseValue(ParseContext* c, FieldValue* v) {
        switch (*c->json)
        {
            case 'n':   return parseNull(c, v);
            case 't':   return parseTrue(c, v);
            case 'f':   return parseFalse(c, v);
            case '\"':  return parseString(c, v);
            case '[':   return parseArray(c, v);
            case '\0':  return JsonParseStatus::PARSE_EXPECT_VALUE;  // 字符串结尾
            default:    return parseNumber(c, v);
        }
    }

    JsonParseStatus json_parse(FieldValue* v, const char* json) {
        ParseContext c;
        c.json = json;
        if (v == nullptr) {
            return JsonParseStatus::PARSE_INVALID_VALUE;
        }
        v->type = JsonFieldType::J_NULL;
        parseWhitespace(&c);
        auto retStatus = parseValue(&c, v);
        if (retStatus == JsonParseStatus::PARSE_OK) {
            parseWhitespace(&c);
            if (*c.json != '\0') {
                retStatus = JsonParseStatus::PARSE_ROOT_NOT_SINGULAR;
            }
        }
        return retStatus;
    }

    void FieldValue::freeSpace()
    {
        switch (type)
        {
            case JsonFieldType::J_STRING:
                delete getJStr()->s;
                setJStr(nullptr, 0);
                break;
            case JsonFieldType::J_ARRAY:
                for (auto& e: *this->data.array) {
                    e.freeSpace();
                }
                delete this->data.array;
                break;
            case JsonFieldType::J_OBJECT:
                for (auto& item: *this->data.obj) {
                    item.second.freeSpace();
                }
                delete this->data.obj;
                break;
            default: break;
        }
        setType(JsonFieldType::J_NULL);
    }

    FieldValue::FieldValue() :
        type(JsonFieldType::J_NULL)
    {}

    FieldValue::FieldValue(JsonFieldType t) :
        type(t)
    {}


}
