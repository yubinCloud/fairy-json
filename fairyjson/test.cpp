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
#if defined(_MSC_VER)
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%Iu")
#else
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")
#endif


static void test_parse_null() {
    FieldValue v(JsonFieldType::J_FALSE);
    EXPECT_EQ_INT(JsonParseStatus::PARSE_OK, json_parse(&v, "null"));
    EXPECT_EQ_INT(JsonFieldType::J_NULL, v.getType());
}

static void test_parse_expect_value() {
    FieldValue v(JsonFieldType::J_FALSE);
    EXPECT_EQ_INT(JsonParseStatus::PARSE_EXPECT_VALUE, json_parse(&v, " "));
    EXPECT_EQ_INT(JsonFieldType::J_NULL, v.getType());

    FieldValue v2(JsonFieldType::J_FALSE);
    EXPECT_EQ_INT(JsonParseStatus::PARSE_EXPECT_VALUE, json_parse(&v2, " "));
    EXPECT_EQ_INT(JsonFieldType::J_NULL, v2.getType());
}

static void test_parse_invalid_value() {
    FieldValue v(JsonFieldType::J_FALSE);
    EXPECT_EQ_INT(JsonParseStatus::PARSE_INVALID_VALUE, json_parse(&v, "nul"));
    EXPECT_EQ_INT(JsonFieldType::J_NULL, v.getType());

    FieldValue v2(JsonFieldType::J_FALSE);
    EXPECT_EQ_INT(JsonParseStatus::PARSE_INVALID_VALUE, json_parse(&v2, "?"));
    EXPECT_EQ_INT(JsonFieldType::J_NULL, v2.getType());
}

static void test_parse_root_not_singular() {
    FieldValue v(JsonFieldType::J_FALSE);
    EXPECT_EQ_INT(JsonParseStatus::PARSE_ROOT_NOT_SINGULAR, json_parse(&v, "null x"));
    EXPECT_EQ_INT(JsonFieldType::J_NULL, v.getType());
}

#define TEST_NUMBER(expect, json)\
    do {\
        FieldValue v(JsonFieldType::J_FALSE);\
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
        FieldValue v(JsonFieldType::J_FALSE);\
        EXPECT_EQ_INT(JsonParseStatus::PARSE_OK, json_parse(&v, json));\
        EXPECT_EQ_INT(JsonFieldType::J_STRING, v.getType());\
        EXPECT_EQ_STRING(expect, v.getJStr()->s, v.getJStr()->len);\
        v.freeSpace();\
    } while(0)

static void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");

    TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

#define TEST_ERROR(error, json)\
    do {\
        FieldValue v;\
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

static void test_parse_array() {
    size_t i, j;
    FieldValue v;

    EXPECT_EQ_INT(JsonParseStatus::PARSE_OK, json_parse(&v, "[ ]"));
    EXPECT_EQ_INT(JsonFieldType::J_ARRAY, v.getType());
    EXPECT_EQ_SIZE_T(0, v.getArray()->size());
    v.freeSpace();

    EXPECT_EQ_INT(JsonParseStatus::PARSE_OK, json_parse(&v, "[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ_INT(JsonFieldType::J_ARRAY, v.getType());
    EXPECT_EQ_SIZE_T(5, v.getArray()->size());
    EXPECT_EQ_INT(JsonFieldType::J_NULL,   (*v.getArray())[0].getType());
    EXPECT_EQ_INT(JsonFieldType::J_FALSE,  (*v.getArray())[1].getType());
    EXPECT_EQ_INT(JsonFieldType::J_TRUE,   (*v.getArray())[2].getType());
    EXPECT_EQ_INT(JsonFieldType::J_NUMBER, (*v.getArray())[3].getType());
    EXPECT_EQ_INT(JsonFieldType::J_STRING, (*v.getArray())[4].getType());
    EXPECT_EQ_DOUBLE(123.0, (*v.getArray())[3].getNumber());
    EXPECT_EQ_STRING("abc", (*v.getArray())[4].getJStr()->s, (*v.getArray())[4].getJStr()->len);
    v.freeSpace();
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
    test_parse_array();
}


int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}
