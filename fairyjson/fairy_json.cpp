//
// Created by yubin on 2021/5/14.
//

#include "fairy_json.h"
#include <cassert>


#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)


namespace fairy {

    struct ParseContext {
        const char* json;
    };

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
    static JsonParseStatus parseNull(ParseContext* c, FairyValue* v) {
        EXPECT(c, 'n');
        if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
            return JsonParseStatus::PARSE_INVALID_VALUE;
        c->json += 3;
        v->type = JsonFieldType::J_NULL;
        return JsonParseStatus::PARSE_OK;
    }

    /**
     * 解析 json 值
     * @param c
     * @param v
     * @return
     */
    static JsonParseStatus parseValue(ParseContext* c, FairyValue* v) {
        switch (*c->json)
        {
            case 'n':   return parseNull(c, v);
            case '\0':  return JsonParseStatus::PARSE_EXPECT_VALUE;  // 字符串结尾
            default:    return JsonParseStatus::PARSE_INVALID_VALUE;
        }
    }

    JsonParseStatus json_parse(FairyValue* v, const char* json) {
        ParseContext c = { json };
        assert(v != nullptr);
        v->type = JsonFieldType::J_NULL;
        parseWhitespace(&c);
        return parseValue(&c, v);
    }

    JsonFieldType FairyValue::getType() const
    {
        return this->type;
    }
}
