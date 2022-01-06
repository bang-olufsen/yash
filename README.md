# Yash - Yet Another Shell

[![build](https://github.com/bang-olufsen/yash/actions/workflows/build.yml/badge.svg)](https://github.com/bang-olufsen/yash/actions/workflows/build.yml) [![coveralls](https://coveralls.io/repos/github/bang-olufsen/yash/badge.svg?branch=main)](https://coveralls.io/github/bang-olufsen/yash?branch=main) [![codeql](https://github.com/bang-olufsen/yash/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/bang-olufsen/yash/actions/workflows/codeql-analysis.yml) [![lgtm](https://img.shields.io/lgtm/grade/cpp/g/bang-olufsen/yash.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/bang-olufsen/yash/context:cpp) [![license](https://img.shields.io/badge/license-MIT_License-blue.svg?style=flat)](LICENSE)

Yash is a C++17 header-only minimal shell for embedded devices with support for command completion.

![](https://raw.githubusercontent.com/bang-olufsen/yash/main/src/example/example.gif)

 It was created as a serial port shell but can be used for other interfaces as well by using `setPrint()`. The prompt can be customized with `setPrompt()` and commands are added using `addCommand()`. The history size can be adjusted by defining `YASH_HISTORY_SIZE` (default 10). An example can be seen below (taken from `src/example/example.cpp` and what is demoed in the image above).

```cpp
#define YASH_FUNCTION_ARRAY_SIZE 4
#include <Yash.h>

void i2cRead(const std::vector<std::string>& args) {
    printf("i2cRead(%s, %s, %s)\n", args.at(0).c_str(), args.at(1).c_str(), args.at(2).c_str());
}

void i2cWrite(const std::vector<std::string>& args) {
    printf("i2cWrite(%s, %s, %s)\n", args.at(0).c_str(), args.at(1).c_str(), args.at(2).c_str());
}

void info(const std::vector<std::string>&) {
    printf("info()\n");
}

void version(const std::vector<std::string>&) {
    printf("version()\n");
}

int main() {
    static constexpr Yash::FunctionArray functionArray {
        { { "i2c read", "I2C read <addr> <reg> <bytes>", [](const auto& args) { i2cRead(args); }, 3 },
        { "i2c write", "I2C write <addr> <reg> <value>", [](const auto& args) { i2cWrite(args); }, 3},
        { "info", "System info", [](const auto& args) { info(args); }, 0 },
        { "version", "Build version", [](const auto& args) { version(args); }, 0 } }
    };

    Yash::Yash yash;
    yash.setPrint([&](const char* str) { printf("%s", str); });
    yash.setPrompt("$ ");
    yash.setFunctionArrayCallback([]() -> const Yash::FunctionArray& { return functionArray; });

    while (true)
        yash.setCharacter(getch());

    return 0;
}
```
