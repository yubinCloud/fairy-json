//
// Created by yubin on 2021/5/14.
//

#pragma once

#include <string>
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
    };


    struct FieldValue {
        union {
            JString str;    // string
            double n;       // number
        } data;
        JsonFieldType type;

        JsonFieldType getType() const;
        double getNumber() const;
    };


    JsonParseStatus json_parse(FieldValue* v, const char* json_str);
}