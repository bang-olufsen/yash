// Copyright 2021 - Bang & Olufsen a/s
#include "turtle/catch.hpp"
#include <catch.hpp>

#define private public
#include "Yash.h"

using namespace std::string_literals;

namespace {

MOCK_FUNCTION(print, 1, void(const char*));
MOCK_FUNCTION(i2c, 1, void(const std::vector<std::string>& args));
MOCK_FUNCTION(second, 1, void(const std::vector<std::string>& args));

void SetupHistoryPreconditions(Yash::Yash& yash)
{
    std::string secondCommand = "proximity";
    std::string secondDescription = "proximity functions";
    yash.addCommand(secondCommand, secondDescription, &second);

    MOCK_EXPECT(print);

    MOCK_EXPECT(i2c).once();
    for (char& character : "i2c\n"s)
        yash.setCharacter(character);

    MOCK_EXPECT(second).once();
    for (char& character : "proximity\n"s)
        yash.setCharacter(character);
}
} // namespace


TEST_CASE("Yash test")
{
    Yash::Yash yash;
    std::string prompt = "$ ";
    std::string command = "i2c";
    std::string description = "i2c read/write functions";
    std::map<std::string, std::string> descriptions {{command, description}};

    yash.setPrint(print);
    yash.setPrompt(prompt);
    yash.addCommand(command, description, &i2c);

    SECTION("Test setPrompt function")
    {
        CHECK(yash.m_prompt == prompt);
    }

    SECTION("Test addCommand function")
    {
        CHECK_FALSE(yash.m_functions.empty());
        CHECK_FALSE(yash.m_descriptions.empty());
    }

    SECTION("Test setCharacter function with line feed input")
    {
        MOCK_EXPECT(print).once().with("\r\n");
        MOCK_EXPECT(print).once().with(prompt.c_str());

        yash.setCharacter('\n');
    }

    SECTION("Test setCharacter function with 'i2' input")
    {
        std::string testCommand = "i2\n";
        std::string help = command + "  " + description + "\r\n";

        MOCK_EXPECT(print).with("");
        MOCK_EXPECT(print).with("i");
        MOCK_EXPECT(print).with("2");
        MOCK_EXPECT(print).with("\r\n");
        MOCK_EXPECT(print).with(prompt.c_str());
        // Expect the help menu to be printed as the command is wrong
        MOCK_EXPECT(print).with(help.c_str());

        for (char& character : testCommand)
            yash.setCharacter(character);
    }

    SECTION("Test setCharacter function with 'i' + TAB input")
    {
        MOCK_EXPECT(print).once().with("i");
        yash.setCharacter('i');

        MOCK_EXPECT(print).once().with("2");
        MOCK_EXPECT(print).once().with("c");
        MOCK_EXPECT(print).once().with(" ");
        yash.setCharacter(Yash::Yash::Tab);
    }

    SECTION("Test setCharacter function with 'i2c' input")
    {
        std::string testCommand = "i2c\n";

        MOCK_EXPECT(i2c).once().with(std::vector<std::string> { "i2c" });
        MOCK_EXPECT(print);

        for (char& character : testCommand)
            yash.setCharacter(character);
    }

    SECTION("Test setCharacter function with 'i2c read' input")
    {
        std::string testCommand { GENERATE(as<std::string>(),
            "i2c read\n",
            "i2c read \n",
            "i2c read  \n") };

        MOCK_EXPECT(i2c).once().with(std::vector<std::string> { "i2c", "read" });
        MOCK_EXPECT(print);

        for (char& character : testCommand)
            yash.setCharacter(character);
    }

    SECTION("Test setCharacter function with 'i2c' and end of text (clear) character input")
    {
        std::string testCommand = "i2c";

        MOCK_EXPECT(print);

        for (char& character : testCommand)
            yash.setCharacter(character);

        CHECK_FALSE(yash.m_command.empty());

        yash.setCharacter(Yash::Yash::EndOfText);
        CHECK(yash.m_command.empty());
    }

    SECTION("Test setCharacter function with 'i2c1' and backspace character input")
    {
        std::string testCommand = "i2c1\b\n";

        // Expect the i2c function to be called as the character 1 will be deleted
        MOCK_EXPECT(i2c);
        MOCK_EXPECT(print);

        for (char& character : testCommand)
            yash.setCharacter(character);
    }

    SECTION("Test setCharacter with up character input")
    {
        SetupHistoryPreconditions(yash);

        MOCK_EXPECT(second).once();
        yash.setCharacter(Yash::Yash::Esc);
        yash.setCharacter(Yash::Yash::LeftBracket);
        yash.setCharacter(Yash::Yash::Up);
        yash.setCharacter('\n');
    }

    SECTION("Test setCharacter with up-up character input")
    {
        SetupHistoryPreconditions(yash);

        MOCK_EXPECT(i2c).once();
        yash.setCharacter(Yash::Yash::Esc);
        yash.setCharacter(Yash::Yash::LeftBracket);
        yash.setCharacter(Yash::Yash::Up);

        yash.setCharacter(Yash::Yash::Esc);
        yash.setCharacter(Yash::Yash::LeftBracket);
        yash.setCharacter(Yash::Yash::Up);
        yash.setCharacter('\n');
    }

    SECTION("Test setCharacter with up-up-down character input")
    {
        SetupHistoryPreconditions(yash);

        MOCK_EXPECT(second).once();
        yash.setCharacter(Yash::Yash::Esc);
        yash.setCharacter(Yash::Yash::LeftBracket);
        yash.setCharacter(Yash::Yash::Up);

        yash.setCharacter(Yash::Yash::Esc);
        yash.setCharacter(Yash::Yash::LeftBracket);
        yash.setCharacter(Yash::Yash::Up);

        yash.setCharacter(Yash::Yash::Esc);
        yash.setCharacter(Yash::Yash::LeftBracket);
        yash.setCharacter(Yash::Yash::Down);
        yash.setCharacter('\n');
    }

    SECTION("Test setCharacter with up-down character input")
    {
        SetupHistoryPreconditions(yash);

        yash.setCharacter(Yash::Yash::Esc);
        yash.setCharacter(Yash::Yash::LeftBracket);
        yash.setCharacter(Yash::Yash::Up);

        yash.setCharacter(Yash::Yash::Esc);
        yash.setCharacter(Yash::Yash::LeftBracket);
        yash.setCharacter(Yash::Yash::Down);
        yash.setCharacter('\n');
    }

    SECTION("Test setCharacter A, B does not do Up, Down")
    {
        CHECK(Yash::Yash::Character::Up == 'A');
        CHECK(Yash::Yash::Character::Down == 'B');

        SetupHistoryPreconditions(yash);

        MOCK_EXPECT(i2c).never();
        yash.setCharacter('A');
        yash.setCharacter('A');
        yash.setCharacter('\n');

        MOCK_EXPECT(second).never();
        yash.setCharacter('B');
        yash.setCharacter('\n');
    }

    SECTION("Test setCharacter - History overflow")
    {
        MOCK_EXPECT(print);

        MOCK_EXPECT(i2c).once();
        for (char& character : "i2c\n"s)
            yash.setCharacter(character);

        // Fill up the history queue so the i2c command overflows
        for (auto i { 0 }; i < YASH_HISTORY_SIZE; ++i) {
            for (char& character : "foo\n"s)
                yash.setCharacter(character);
        }

        for (auto i { 0 }; i <= YASH_HISTORY_SIZE; ++i)
            yash.setCharacter(Yash::Yash::Up);
        yash.setCharacter('\n');
    }

    SECTION("Test setCharacter function with ESC/LeftBracket input")
    {
        yash.setCharacter(Yash::Yash::Esc);
        CHECK(yash.m_ctrlState == Yash::Yash::CtrlState::Esc);
        yash.setCharacter(Yash::Yash::LeftBracket);
        CHECK(yash.m_ctrlState == Yash::Yash::CtrlState::LeftBracket);
    }

    SECTION("Test removeCommand function")
    {
        // Try to remove a non-existing command
        yash.removeCommand("i2");
        CHECK_FALSE(yash.m_functions.empty());
        CHECK_FALSE(yash.m_descriptions.empty());

        yash.removeCommand(command);
        CHECK(yash.m_functions.empty());
        CHECK(yash.m_descriptions.empty());
    }

    SECTION("Test printCommands")
    {
        MOCK_EXPECT(print).once().with(command + "  " + description + "\r\n");
        yash.printCommands(descriptions);
    }

    SECTION("Test printCommands alignment")
    {
        std::string secondCommand = "info";
        std::string secondDescription = "System info";
        descriptions.emplace(secondCommand, secondDescription);

        mock::sequence seq;
        MOCK_EXPECT(print).once().with(command + "   " + description + "\r\n").in(seq);
        MOCK_EXPECT(print).once().with(secondCommand + "  " + secondDescription + "\r\n").in(seq);
        yash.printCommands(descriptions);
    }

    mock::verify();
    mock::reset();
}
