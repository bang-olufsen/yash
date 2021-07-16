# Yash - Yet Another Shell

[![build](https://github.com/bang-olufsen/yash/actions/workflows/build.yml/badge.svg)](https://github.com/bang-olufsen/yash/actions/workflows/build.yml) [![coveralls](https://coveralls.io/repos/github/bang-olufsen/yash/badge.svg?branch=main)](https://coveralls.io/github/bang-olufsen/yash?branch=main) [![lgtm](https://img.shields.io/lgtm/grade/cpp/g/bang-olufsen/yash.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/bang-olufsen/yash/context:cpp) [![codefactor](https://www.codefactor.io/repository/github/bang-olufsen/yash/badge)](https://www.codefactor.io/repository/github/bang-olufsen/yash) [![license](https://img.shields.io/badge/license-MIT_License-blue.svg?style=flat)](LICENSE)

Yash is a C++11 header-only minimal shell for embedded devices.

It has been created as a serial port shell but can be used for other interfaces as well by using `setPrint()`. The prompt can be customized with `setPrompt()` and commands are added using `addCommand()`. An example can be seen below.

```cpp
#include <Yash.h>

void i2c(const std::vector<std::string>& args) {
    printf("i2c command called with %lu args\n", args.size());
}

void gpio(const std::vector<std::string>& args) {
    printf("gpio command called with %lu args\n", args.size());
}

int main() {
    Yash::Yash yash;
    yash.setPrint([&](const char* str) { printf("%s", str); });
    yash.setPrompt("$ ");
    yash.addCommand("i2c", "I2C read/write", [&](const auto& args) { i2c(args); });
    yash.addCommand("gpio", "GPIO read/write", [&](const auto& args) { gpio(args); });

    while (true)
        yash.setCharacter(getchar());

    return 0;
}
```
