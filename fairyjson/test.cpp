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

static void test_parse_null() {
    FieldValue v = {
            .type = JsonFieldType::J_FALSE
    };
    EXPECT_EQ_INT(JsonParseStatus::PARSE_OK, json_parse(&v, "null"));
    EXPECT_EQ_INT(JsonFieldType::J_NULL, v.getType());
}

static void test_parse() {
    test_parse_null();
}

int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}
