// Copyright 2022 - Bang & Olufsen a/s
// SPDX-License-Identifier: MIT

#include <Yash.h>
#include <con.h>

void i2c(const std::string_view command, Yash::CommandArgs args)
{
    if (command == "read")
        printf("i2cRead(%s, %s, %s)\n", args[0].data(), args[1].data(), args[2].data());
    else if (command == "write")
        printf("i2cWrite(%s, %s, %s)\n", args[0].data(), args[1].data(), args[2].data());
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
        { "i2c write", "I2C write <addr> <reg> <value>", [](Yash::CommandArgs args) { i2c("write", args); }, 3 },
        { "info", "System info", [](const auto args) { info(args); }, 0 }, // OR auto if preffered
    });

    Yash::Yash<config> yash(commands);
    yash.setPrint([&](const char* str) { printf("%s", str); });
    yash.setPrompt("$ ");

    while (true)
        yash.setCharacter(getch());

    return 0;
}
