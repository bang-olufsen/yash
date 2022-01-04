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
    std::string secondCommand = "version";
    std::string secondDescription = "Build info";
    yash.addCommand(secondCommand, secondDescription, &second);

    MOCK_EXPECT(print);

    MOCK_EXPECT(i2c).once();
    for (char& character : "i2c read 1 2 3\n"s)
        yash.setCharacter(character);

    MOCK_EXPECT(second).once();
    for (char& character : "version\n"s)
        yash.setCharacter(character);
}

constexpr const char *s_clearCharacter = "\033[1D \033[1D";
constexpr const char *s_moveCursorForward = "\033[1C";
constexpr const char *s_moveCursorBackward = "\033[1D";
} // namespace


TEST_CASE("Yash test")
{
    Yash::Yash yash;
    std::string prompt = "$ ";
    std::string command = "i2c read";
    std::string description = "I2C read <addr> <reg> <bytes>";

    yash.setPrint(print);
    yash.setPrompt(prompt);
    yash.addCommand(command, description, &i2c, 3);

    SECTION("Test setPrompt function")
    {
        CHECK(yash.m_prompt == prompt);
    }

    SECTION("Test addCommand function")
    {
        CHECK_FALSE(yash.m_functions.empty());
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
        mock::sequence seq;
        MOCK_EXPECT(print).once().in(seq).with("i");
        MOCK_EXPECT(print).once().in(seq).with(mock::any);
        MOCK_EXPECT(print).once().in(seq).with(prompt.c_str());
        MOCK_EXPECT(print).once().in(seq).with("i2c read ");
        yash.setCharacter('i');
        yash.setCharacter(Yash::Yash::Tab);
    }

    SECTION("Test setCharacter function with 'i' + TAB input and two similar commands")
    {
        std::string secondCommand = "info";
        std::string secondDescription = "System info";
        yash.addCommand(secondCommand, secondDescription, nullptr);

        mock::sequence seq;
        MOCK_EXPECT(print).once().in(seq).with("i");
        MOCK_EXPECT(print).once().in(seq).with(mock::any);
        MOCK_EXPECT(print).once().in(seq).with(command + "  " + description + "\r\n");
        MOCK_EXPECT(print).once().in(seq).with(secondCommand + "      " + secondDescription + "\r\n");
        MOCK_EXPECT(print).once().in(seq).with(mock::any);
        MOCK_EXPECT(print).once().in(seq).with(prompt.c_str());
        MOCK_EXPECT(print).once().in(seq).with("i");
        yash.setCharacter('i');
        yash.setCharacter(Yash::Yash::Tab);
    }

    SECTION("Test enter not at command end")
    {
        mock::sequence seq;
        MOCK_EXPECT(print).once().with("i").in(seq);
        MOCK_EXPECT(print).once().with("2").in(seq);
        MOCK_EXPECT(print).once().with("c").in(seq);
        yash.setCharacter('i'); // pos 0
        yash.setCharacter('2'); // pos 1
        yash.setCharacter('c'); // pos 2

        MOCK_EXPECT(print).exactly(3).with(s_moveCursorBackward).in(seq);
        yash.setCharacter(Yash::Yash::Esc);
        yash.setCharacter(Yash::Yash::LeftBracket);
        yash.setCharacter('1');
        yash.setCharacter(';');
        yash.setCharacter('5');
        yash.setCharacter('D');


        MOCK_EXPECT(print).once().with('C').in(seq);
        MOCK_EXPECT(print).once().with("i").in(seq);
        MOCK_EXPECT(print).once().with("2").in(seq);
        MOCK_EXPECT(print).once().with("c").in(seq);
        MOCK_EXPECT(print).exactly(3).with(s_moveCursorBackward).in(seq);
        yash.setCharacter(Yash::Yash::Right);


    }

    SECTION("Test setCharacter function with 'i2c read 1 2 3' input")
    {
        std::string testCommand = "i2c read 1 2 3\n";

        MOCK_EXPECT(i2c).once().with(std::vector<std::string> { "1", "2", "3" });
        MOCK_EXPECT(print);

        for (char& character : testCommand)
            yash.setCharacter(character);
    }

    SECTION("Test setCharacter function with 'i2c read 1 2 3' input")
    {
        std::string testCommand { GENERATE(as<std::string>(),
            "i2c read 1 2 3\n",
            "i2c read 1 2 3 \n",
            "i2c read 1 2 3  \n") };

        MOCK_EXPECT(i2c).once().with(std::vector<std::string> { "1", "2", "3" });
        MOCK_EXPECT(print);

        for (char& character : testCommand)
            yash.setCharacter(character);
    }

    SECTION("Test setCharacter function with 'i2c read 1 2 3' and end of text (clear) character input")
    {
        std::string testCommand = "i2c read 1 2 3";

        MOCK_EXPECT(print);

        for (char& character : testCommand)
            yash.setCharacter(character);

        CHECK_FALSE(yash.m_command.empty());

        yash.setCharacter(Yash::Yash::EndOfText);
        CHECK(yash.m_command.empty());
    }

    SECTION("Test setCharacter function with 'i2c read 1 2 3' and backspace character input")
    {
        std::string testCommand = "i2c read 1 2 3\b\n";

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
        for (char& character : "i2c read 1 2 3\n"s)
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

        yash.removeCommand(command);
        CHECK(yash.m_functions.empty());
    }

    SECTION("Test setCharacter function with 'i21c' and backspace character input")
    {
        // Expect the i2c function to be called as the character 1 will be inserted
        MOCK_EXPECT(i2c);
        MOCK_EXPECT(print);

        yash.setCharacter('i');
        yash.setCharacter('2');
        yash.setCharacter('1');
        yash.setCharacter('c');

        yash.setCharacter(Yash::Yash::Esc);
        yash.setCharacter(Yash::Yash::LeftBracket);
        yash.setCharacter(Yash::Yash::Left);

        yash.setCharacter(Yash::Yash::Backspace);

        yash.setCharacter(Yash::Yash::Esc);
        yash.setCharacter(Yash::Yash::LeftBracket);
        yash.setCharacter(Yash::Yash::Right);

        yash.setCharacter('\n');
    }

    SECTION("Test setCharacter function with 'i2c' with left and right character input")
    {
        {
            mock::sequence seq;
            MOCK_EXPECT(print).once().with("i").in(seq);
            MOCK_EXPECT(print).once().with("2").in(seq);
            MOCK_EXPECT(print).once().with("c").in(seq);
            yash.setCharacter('i'); // pos 0
            yash.setCharacter('2'); // pos 1
            yash.setCharacter('c'); // pos 2
            // cursor at pos 3
            mock::verify();
            mock::reset();
        }

        {
            // move cursor back 3 steps to char 'i' @pos 0
            mock::sequence seq;
            MOCK_EXPECT(print).exactly(3).with(s_moveCursorBackward).in(seq);
            yash.setCharacter(Yash::Yash::Esc);
            yash.setCharacter(Yash::Yash::LeftBracket);
            yash.setCharacter(Yash::Yash::Left);
            yash.setCharacter(Yash::Yash::Esc);
            yash.setCharacter(Yash::Yash::LeftBracket);
            yash.setCharacter(Yash::Yash::Left);
            yash.setCharacter(Yash::Yash::Esc);
            yash.setCharacter(Yash::Yash::LeftBracket);
            yash.setCharacter(Yash::Yash::Left);
            mock::verify();
            mock::reset();
        }

        {
            // move cursor forward 3 steps to end @pos 3
            mock::sequence seq;
            MOCK_EXPECT(print).exactly(3).with(s_moveCursorForward).in(seq);
            yash.setCharacter(Yash::Yash::Esc);
            yash.setCharacter(Yash::Yash::LeftBracket);
            yash.setCharacter(Yash::Yash::Right);
            yash.setCharacter(Yash::Yash::Esc);
            yash.setCharacter(Yash::Yash::LeftBracket);
            yash.setCharacter(Yash::Yash::Right);
            yash.setCharacter(Yash::Yash::Esc);
            yash.setCharacter(Yash::Yash::LeftBracket);
            yash.setCharacter(Yash::Yash::Right);
            mock::verify();
            mock::reset();
        }
    }

    SECTION("Test setCharacter function with 'i2c' with delete character input")
    {
        {
            mock::sequence seq;
            MOCK_EXPECT(print).once().with("i").in(seq);
            MOCK_EXPECT(print).once().with("2").in(seq);
            MOCK_EXPECT(print).once().with("c").in(seq);
            yash.setCharacter('i'); // pos 0
            yash.setCharacter('2'); // pos 1
            yash.setCharacter('c'); // pos 2
            // cursor at pos 3
            mock::verify();
            mock::reset();
        }

        {
            // move cursor back 2 steps to char '2' @pos 1
            mock::sequence seq;
            MOCK_EXPECT(print).exactly(2).with(s_moveCursorBackward).in(seq);
            yash.setCharacter(Yash::Yash::Esc);
            yash.setCharacter(Yash::Yash::LeftBracket);
            yash.setCharacter(Yash::Yash::Left);
            yash.setCharacter(Yash::Yash::Esc);
            yash.setCharacter(Yash::Yash::LeftBracket);
            yash.setCharacter(Yash::Yash::Left);
            mock::verify();
            mock::reset();
        }

        {
            // Generate delete press to remove char '2'
            mock::sequence seq;
            MOCK_EXPECT(print).once().with(" ").in(seq);
            MOCK_EXPECT(print).once().with(s_clearCharacter).in(seq);
            MOCK_EXPECT(print).once().with("c").in(seq);
            MOCK_EXPECT(print).once().with(" ").in(seq);
            MOCK_EXPECT(print).once().with(s_clearCharacter).in(seq);
            MOCK_EXPECT(print).exactly(1).with(s_moveCursorBackward).in(seq);
            yash.setCharacter(Yash::Yash::Esc);
            yash.setCharacter(Yash::Yash::LeftBracket);
            yash.setCharacter('3');
            yash.setCharacter('~');
            mock::verify();
            mock::reset();
        }

        CHECK(yash.m_command == "ic");
    }

    SECTION("Test setCharacter function with 'i2c' with home and end character input to change cursor position")
    {
        {
            mock::sequence seq;
            MOCK_EXPECT(print).once().with("i").in(seq);
            MOCK_EXPECT(print).once().with("2").in(seq);
            MOCK_EXPECT(print).once().with("c").in(seq);
            MOCK_EXPECT(print).once().with(" ").in(seq);
            MOCK_EXPECT(print).once().with("i").in(seq);
            MOCK_EXPECT(print).once().with("2").in(seq);
            MOCK_EXPECT(print).once().with("c").in(seq);
            yash.setCharacter('i'); // pos 0
            yash.setCharacter('2'); // pos 1
            yash.setCharacter('c'); // pos 2
            yash.setCharacter(' '); // pos 3
            yash.setCharacter('i'); // pos 4
            yash.setCharacter('2'); // pos 5
            yash.setCharacter('c'); // pos 6
            // cursor at pos 7
            mock::verify();
            mock::reset();
        }

        {
            // Generate home key press to set cursor position to the beginning @pos 0
            mock::sequence seq;
            MOCK_EXPECT(print).exactly(7).with(s_moveCursorBackward).in(seq);
            yash.setCharacter(Yash::Yash::Esc);
            yash.setCharacter(Yash::Yash::LeftBracket);
            yash.setCharacter('1');
            yash.setCharacter('~');
            mock::verify();
            mock::reset();
        }

        {
            // Generate end key press to set cursor position to the end again @pos 7
            mock::sequence seq;
            MOCK_EXPECT(print).exactly(7).with(s_moveCursorForward).in(seq);
            yash.setCharacter(Yash::Yash::Esc);
            yash.setCharacter(Yash::Yash::LeftBracket);
            yash.setCharacter('4');
            yash.setCharacter('~');
            mock::verify();
            mock::reset();
        }
    }

    SECTION("Test setCharacter function with 'i2c' with ctrl+left and ctrl+right character input to change cursor position")
    {
        {
            mock::sequence seq;
            MOCK_EXPECT(print).once().with("i").in(seq);
            MOCK_EXPECT(print).once().with("2").in(seq);
            MOCK_EXPECT(print).once().with("c").in(seq);
            MOCK_EXPECT(print).once().with(" ").in(seq);
            MOCK_EXPECT(print).once().with("i").in(seq);
            MOCK_EXPECT(print).once().with("2").in(seq);
            MOCK_EXPECT(print).once().with("c").in(seq);
            yash.setCharacter('i'); // pos 0
            yash.setCharacter('2'); // pos 1
            yash.setCharacter('c'); // pos 2
            yash.setCharacter(' '); // pos 3
            yash.setCharacter('i'); // pos 4
            yash.setCharacter('2'); // pos 5
            yash.setCharacter('c'); // pos 6
            // cursor at pos 7
            mock::verify();
            mock::reset();
        }

        {
            // Generate ctrl+left key press to set cursor position to the 'i' @pos 4
            mock::sequence seq;
            MOCK_EXPECT(print).exactly(4).with(s_moveCursorBackward).in(seq);
            MOCK_EXPECT(print).once().with(s_moveCursorForward).in(seq);
            yash.setCharacter(Yash::Yash::Esc);
            yash.setCharacter(Yash::Yash::LeftBracket);
            yash.setCharacter('1');
            yash.setCharacter(';');
            yash.setCharacter('5');
            yash.setCharacter('D');
            mock::verify();
            mock::reset();
        }

        {
            // Generate ctrl+left key press to set cursor position to the 'i' @pos 0
            mock::sequence seq;
            MOCK_EXPECT(print).exactly(4).with(s_moveCursorBackward).in(seq);
            yash.setCharacter(Yash::Yash::Esc);
            yash.setCharacter(Yash::Yash::LeftBracket);
            yash.setCharacter('1');
            yash.setCharacter(';');
            yash.setCharacter('5');
            yash.setCharacter('D');
            mock::verify();
            mock::reset();
        }

        {
            // Generate ctrl+right key press to set cursor position to the space @pos 3
            mock::sequence seq;
            MOCK_EXPECT(print).exactly(3).with(s_moveCursorForward).in(seq);
            yash.setCharacter(Yash::Yash::Esc);
            yash.setCharacter(Yash::Yash::LeftBracket);
            yash.setCharacter('1');
            yash.setCharacter(';');
            yash.setCharacter('5');
            yash.setCharacter('C');
            mock::verify();
            mock::reset();
        }

        {
            // Generate ctrl+right key press to set cursor position to the end again @pos 7
            mock::sequence seq;
            MOCK_EXPECT(print).exactly(4).with(s_moveCursorForward).in(seq);
            yash.setCharacter(Yash::Yash::Esc);
            yash.setCharacter(Yash::Yash::LeftBracket);
            yash.setCharacter('1');
            yash.setCharacter(';');
            yash.setCharacter('5');
            yash.setCharacter('C');
            mock::verify();
            mock::reset();
        }
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

    SECTION("Test setCharacter C, D does not do Right, Left")
    {
        CHECK(Yash::Yash::Character::Right == 'C');
        CHECK(Yash::Yash::Character::Left == 'D');

        SetupHistoryPreconditions(yash);

        MOCK_EXPECT(i2c).never();
        yash.setCharacter('C');
        yash.setCharacter('C');
        yash.setCharacter('\n');

        MOCK_EXPECT(second).never();
        yash.setCharacter('D');
        yash.setCharacter('\n');
    }

    SECTION("Test setCharacter C, D does not do Right, Left")
    {
        CHECK(Yash::Yash::Character::Right == 'C');
        CHECK(Yash::Yash::Character::Left == 'D');

        SetupHistoryPreconditions(yash);

        MOCK_EXPECT(i2c).never();
        yash.setCharacter('C');
        yash.setCharacter('C');
        yash.setCharacter('\n');

        MOCK_EXPECT(second).never();
        yash.setCharacter('D');
        yash.setCharacter('\n');
    }

    mock::verify();
    mock::reset();
}
