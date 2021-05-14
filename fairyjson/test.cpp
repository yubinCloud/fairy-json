#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "fairy_json.h"

using namespace fairy;

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual, aLength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == aLength && memcmp(expect, actual, aLength) == 0, expect, actual, "%s")

static void test_parse_null() {
    FieldValue v = {
            .type = JsonFieldType::J_FALSE
    };
    EXPECT_EQ_INT(JsonParseStatus::PARSE_OK, json_parse(&v, "null"));
    EXPECT_EQ_INT(JsonFieldType::J_NULL, v.getType());
}

static void test_parse_expect_value() {
    FieldValue v = {
            .type = JsonFieldType::J_FALSE
    };
    EXPECT_EQ_INT(JsonParseStatus::PARSE_EXPECT_VALUE, json_parse(&v, " "));
    EXPECT_EQ_INT(JsonFieldType::J_NULL, v.getType());

    FieldValue v2 = {
            .type = JsonFieldType::J_FALSE
    };
    EXPECT_EQ_INT(JsonParseStatus::PARSE_EXPECT_VALUE, json_parse(&v2, " "));
    EXPECT_EQ_INT(JsonFieldType::J_NULL, v2.getType());
}

static void test_parse_invalid_value() {
    FieldValue v = {
            .type = JsonFieldType::J_FALSE
    };
    EXPECT_EQ_INT(JsonParseStatus::PARSE_INVALID_VALUE, json_parse(&v, "nul"));
    EXPECT_EQ_INT(JsonFieldType::J_NULL, v.getType());

    FieldValue v2 = {
            .type = JsonFieldType::J_FALSE
    };
    EXPECT_EQ_INT(JsonParseStatus::PARSE_INVALID_VALUE, json_parse(&v2, "?"));
    EXPECT_EQ_INT(JsonFieldType::J_NULL, v2.getType());
}

static void test_parse_root_not_singular() {
    FieldValue v = {
            .type = JsonFieldType::J_FALSE
    };
    EXPECT_EQ_INT(JsonParseStatus::PARSE_ROOT_NOT_SINGULAR, json_parse(&v, "null x"));
    EXPECT_EQ_INT(JsonFieldType::J_NULL, v.getType());
}

#define TEST_NUMBER(expect, json)\
    do {\
        FieldValue v = {         \
            .type = JsonFieldType::J_FALSE\
        };\
        EXPECT_EQ_INT(JsonParseStatus::PARSE_OK, json_parse(&v, json));\
        EXPECT_EQ_INT(JsonFieldType::J_NUMBER, v.getType());\
        EXPECT_EQ_DOUBLE(expect, v.getNumber());\
    } while(0)

static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

#define TEST_STRING(expect, json)\
    do {\
        FieldValue v = {         \
            .type = JsonFieldType::J_NULL\
        };\
        EXPECT_EQ_INT(JsonParseStatus::PARSE_OK, json_parse(&v, json));\
        EXPECT_EQ_INT(JsonFieldType::J_STRING, v.getType());\
        EXPECT_EQ_STRING(expect, v.getJStr()->s, v.getJStr()->len);\
        v.freeSpace();\
    } while(0)

static void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
#if 1
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
#endif
}

#define TEST_ERROR(error, json)\
    do {\
        FieldValue v = {         \
            .type = JsonFieldType::J_NULL\
        };\
        EXPECT_EQ_INT(error, json_parse(&v, json));\
        EXPECT_EQ_INT(JsonFieldType::J_NULL, v.getType());\
        v.freeSpace();\
    } while(0)

    
static void test_parse_invalid_string_escape() {
#if 1
    TEST_ERROR(JsonParseStatus::PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(JsonParseStatus::PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(JsonParseStatus::PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(JsonParseStatus::PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
#endif
}

static void test_parse_invalid_string_char() {
    TEST_ERROR(JsonParseStatus::PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(JsonParseStatus::PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_parse() {
    test_parse_null();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number();
    test_parse_string();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();
}


int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}
