# Yash - Yet Another Shell

[![build](https://github.com/bang-olufsen/yash/actions/workflows/build.yml/badge.svg)](https://github.com/bang-olufsen/yash/actions/workflows/build.yml) [![coveralls](https://coveralls.io/repos/github/bang-olufsen/yash/badge.svg?branch=main)](https://coveralls.io/github/bang-olufsen/yash?branch=main) [![codeql](https://github.com/bang-olufsen/yash/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/bang-olufsen/yash/actions/workflows/codeql-analysis.yml) [![lgtm](https://img.shields.io/lgtm/grade/cpp/g/bang-olufsen/yash.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/bang-olufsen/yash/context:cpp) [![license](https://img.shields.io/badge/license-MIT_License-blue.svg?style=flat)](LICENSE)

Yash is a C++20 header-only minimal shell for embedded devices with support for command completion.

![](https://raw.githubusercontent.com/bang-olufsen/yash/main/example/example.gif)

 It was created as a serial port shell but can be used for other interfaces as well by using `setPrint()`. The prompt can be customized with `setPrompt()` and commands are added as a std::array which can be constexpr to save memory. The command history size can be adjusted by the constructor (default 10). An example can be seen below (taken from [example.cpp](https://github.com/bang-olufsen/yash/blob/main/example/example.cpp) and what is demoed in the image above).

```cpp
#include <Yash.h>

void i2c(const std::string_view command, Yash::CommandArgs args)
{
    if (command == "read")
        printf("i2cRead(%s, %s, %s)\n", args.at(0).data(), args.at(1).data(), args.at(2).data());
    else if (command == "write")
        printf("i2cWrite(%s, %s, %s)\n", args.at(0).data(), args.at(1).data(), args.at(2).data());
}

void info(Yash::CommandArgs /* unused */)
{
    printf("info()\n");
}

int main()
{
    static constexpr Yash::Config config { .maxRequiredArgs = 3, .commandHistorySize = 10 };
    static constexpr auto commands = std::to_array<Yash::Command>({
        { "i2c read", "I2C read <addr> <reg> <bytes>", [](Yash::CommandArgs args) { i2c("read", args); }, 3 },
        { "i2c write", "I2C write <addr> <reg> <value>", [](Yash::Commands args) { i2c("write", args); }, 3 },
        { "info", "System info", [](const auto args) { info(args); }, 0 }, // OR auto if preffered
    });

    Yash::Yash<config> yash(commands);
    yash.setPrint([&](const char* str) { printf("%s", str); });
    yash.setPrompt("$ ");

    while (true)
        yash.setCharacter(getch());

    return 0;
}
```
