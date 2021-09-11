// Copyright 2021 - Bang & Olufsen a/s
#include <stdlib.h>
#include <con.h>

#include <Yash.h>

void i2cRead(const std::vector<std::string>& args) {
    printf("i2cRead command called with %lu args\n", args.size());
}

void i2cWrite(const std::vector<std::string>& args) {
    printf("i2cWrite command called with %lu args\n", args.size());
}

void info(const std::vector<std::string>& args) {
    printf("info command called with %lu args\n", args.size());
}

void gpio(const std::vector<std::string>& args) {
    printf("gpio command called with %lu args\n", args.size());
}

int main() {
    Yash::Yash yash;
    yash.setPrint([&](const char* str) { printf("%s", str); });
    yash.setPrompt("$ ");
    yash.addCommand("i2c", "read", "I2C read <bytes>", [&](const auto& args) { i2cRead(args); });
    yash.addCommand("i2c", "write", "I2C write <bytes>", [&](const auto& args) { i2cWrite(args); });
    yash.addCommand("info", "System info", [&](const auto& args) { info(args); });
    yash.addCommand("gpio", "GPIO read/write", [&](const auto& args) { gpio(args); });

    while (true)
        yash.setCharacter(getch());

    return 0;
}
