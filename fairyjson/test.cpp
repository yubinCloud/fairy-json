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
}

static void test_parse() {
    test_parse_null();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number();
}


int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}
