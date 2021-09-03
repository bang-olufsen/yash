// Copyright 2021 - Bang & Olufsen a/s
#include <stdlib.h>
#include <con.h>

#include <Yash.h>

void i2c(const std::vector<std::string>& args) {
    printf("i2c command called with %lu args\n", args.size());
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
    yash.addCommand("i2c", "I2C read/write", [&](const auto& args) { i2c(args); });
    yash.addCommand("info", "System info", [&](const auto& args) { info(args); });
    yash.addCommand("gpio", "GPIO read/write", [&](const auto& args) { gpio(args); });

    while (true)
        yash.setCharacter(getch());

    return 0;
}
