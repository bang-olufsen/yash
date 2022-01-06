// Copyright 2021 - Bang & Olufsen a/s
#include <stdlib.h>
#include <con.h>

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
