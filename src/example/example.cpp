// Copyright 2021 - Bang & Olufsen a/s
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
