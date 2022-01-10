// Copyright 2022 - Bang & Olufsen a/s
// SPDX-License-Identifier: MIT

#include <Yash.h>
#include <con.h>
#include <cstdlib>


void i2cRead(Yash::Arguments args)
{

    printf("i2cRead(%s, %s, %s)\n",
        std::next(args.begin(), 0)->c_str(),
        std::next(args.begin(), 1)->c_str(),
        std::next(args.begin(), 2)->c_str());
}

void i2cWrite(Yash::Arguments args)
{
    printf("i2cWrite(%s, %s, %s)\n",
        std::next(args.begin(), 0)->c_str(),
        std::next(args.begin(), 1)->c_str(),
        std::next(args.begin(), 2)->c_str());
}

void info(Yash::Arguments)
{
    printf("info()\n");
}

int main()
{
    static constexpr std::array<Yash::Command, 3> commands {
        { { "i2c read", "I2C read <addr> <reg> <bytes>", [](const auto& args) { i2cRead(args); }, 3 },
            { "i2c write", "I2C write <addr> <reg> <value>", [](const auto& args) { i2cWrite(args); }, 3 },
            { "info", "System info", [](const auto& args) { info(args); }, 0 } }
    };

    Yash::Yash<std::size(commands)> yash(commands);
    yash.setPrint([&](const char* str) { printf("%s", str); });
    yash.setPrompt("$ ");

    while (true)
        yash.setCharacter(getch());

    return 0;
}
