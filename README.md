# fairy-json

### 介绍
使用 C++ 实现的 JSON 解析器，全部测试通过。

功能包含将一个符合标准的 JSON 字符串反序列成一个对象和将一个带有信息的对象序列化为 JSON 字符串

此处所指的对象是本库所自定义的 `FieldValue` 类对象。

### 实现方式
借助于本库自定义的 `FieldValue` 类来实现 JSON 数据的内存对象和字符串类型的转换。

将 JSON Value 分为以下类型：
+ null
+ false
+ true
+ number
+ string
+ array
+ object

其中采用 **C++ STL** 的 `std::vector` 和 `std::multimap` 来实现存储 `array` 和 `object` 类型的数据。

`string` 类型支持 UTF-8 编码。

### 示例

```c++
#include <string>
#include <iostream>
#include <fairy_json.h>

using namespace std;
using namespace fiary;

int main() {
    auto oldJsonStr = string(" { "
        "\"n\" : null , "
        "\"f\" : false , "
        "\"t\" : true , "
        "\"i\" : 123 , "
        "\"s\" : \"abc\", "
        "\"a\" : [ 1, 2, 3 ],"
        "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
        " } ");
    FieldValue v;
    auto retStatus = json_parse(&v, oldJsonStr.c_str());
    if (retStatus == JsonParseStatus::PARSE_OK) {
        cout << "parse successful!" << endl;
    }
    auto afterJsonStr = jsonStringify(&v);
    cout << "Serialized string: " << endl;
    cout << afterJsonStr << std::endl;
    
    return 0;
}

```
