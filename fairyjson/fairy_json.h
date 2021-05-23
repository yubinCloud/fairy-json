//
// Created by yubin on 2021/5/14.
//

#pragma once

#include <string>
#include <cassert>
#include <stack>
#include <vector>
#include <memory>
#include "JString.h"

namespace fairy {
    /**
    * JSON 数据字段的类型
    */
    enum class JsonFieldType {
        J_NULL,
        J_FALSE,
        J_TRUE,
        J_NUMBER,
        J_STRING,
        J_ARRAY,
        J_OBJECT
    };


    /**
    * json 解析结果状态
    */
    enum class JsonParseStatus {
        PARSE_OK,
        PARSE_EXPECT_VALUE,
        PARSE_INVALID_VALUE,
        PARSE_ROOT_NOT_SINGULAR,
        PARSE_NUMBER_OVERFLOW,
        PARSE_MISS_QUOTATION_MARK,
        PARSE_INVALID_STRING_ESCAPE,
        PARSE_INVALID_STRING_CHAR,
        PARSE_INVALID_UNICODE_HEX,      // \u 后不是 4 位十六进位数字
        PARSE_INVALID_UNICODE_SURROGATE // 只有高代理项而欠缺低代理项，或是低代理项不在合法码点范围
    };

    /**
     * Json 中一个数据元素的类型
     */
    struct FieldValue {
        union {
            double n;       // number
            JString str;    // string
            std::vector<FieldValue>* array;   // array
        } data{};
        JsonFieldType type;

    public:
        explicit FieldValue();
        explicit FieldValue(JsonFieldType t);

        /**
         * 释放掉已申请的空间
         */
        void freeSpace();

        JsonFieldType getType() const {
            return this->type;
        }

        void setType(JsonFieldType t) {
            this->type = t;
        }

        double getNumber() const {
            assert(this->type == JsonFieldType::J_NUMBER);
            return this->data.n;
        }

        void setNumber(double n) {
            assert(this->type == JsonFieldType::J_NUMBER);
            this->data.n = n;
        }

        bool getBoolean() const {
            assert(this->type == JsonFieldType::J_TRUE || this->type == JsonFieldType::J_FALSE);
           if (this->type == JsonFieldType::J_TRUE)
                return true;
           return false;
        }

        JString* getJStr() {
            return &this->data.str;
        }

        const JString* getJStr() const {
            return &this->data.str;
        }

        void setJStr(char* s, const size_t len) {
            this->data.str.s = s;
            this->data.str.len = len;
        }

        void setJStr(const JString* pJStr) {
            this->setJStr(pJStr->s, pJStr->len);
        }

        std::vector<FieldValue>* getArray() const {
            return this->data.array;
        }

        void setArray(std::vector<FieldValue>* array) {
            this->data.array = array;
        }
    };

    /**
     * 解析上下文
     */
    struct ParseContext {
        const char* json = nullptr;
        std::stack<char> charStack;
        std::stack<FieldValue> fieldStack;
    };


    JsonParseStatus json_parse(FieldValue* v, const char* json_str);
}