// Copyright 2022 - Bang & Olufsen a/s
// SPDX-License-Identifier: MIT

#pragma once

#include <cstring>
#include <functional>
#include <list>
#include <numeric>
#include <span>
#include <string>
#include <string_view>

namespace Yash {

using CommandArgs = const std::span<const std::string_view>;
typedef void (*CommandFunction)(CommandArgs);

struct Command {
    const std::string_view name;
    const std::string_view description;
    const CommandFunction function;
    const size_t requiredArguments;
};

struct Config {
    const size_t maxRequiredArgs; // The maximum amount of arguments provided in a callback
    const size_t commandHistorySize;
};

using CommandSpan = const std::span<const Command>;

template <Config TConfig>
class Yash {
public:
    /// @brief Constructor
    /// @param commands A reference to an array with the commands (can be constexpr if wanted)
    /// @param commandHistorySize The size of the command history (default 10)
    constexpr Yash(CommandSpan commands)
        : m_commands(commands)
        , m_commandHistoryIndex(m_commandHistory.begin())
        , m_commandHistorySize(TConfig.commandHistorySize)
        , m_allCommandsSizeAlignment(std::accumulate(commands.begin(), commands.end(), 0, [](size_t max, const auto& cmd) {
            std::string_view commandNameSub = cmd.name.substr(0, cmd.name.find_first_of(s_commandDelimiter));
            return std::max(max, commandNameSub.size());
        }))
    {
    }

    ~Yash() = default;

    /// @brief Sets the print function to be used
    /// @param print The print funcion to be used
    void setPrint(std::function<void(const char*)> printFunction) { m_printFunction = std::move(printFunction); }

    /// @brief Prints the specified text using the print function
    /// @param text The text to be printed
    void print(const char* text) const
    {
        if (m_printFunction)
            m_printFunction(text);
    }

    /// @brief Sets the name of the shell prompt
    /// @param prompt A string with the name to be used
    void setPrompt(const std::string& prompt) { m_prompt = prompt; }

    /// @brief Sets a received character on the shell
    /// @param character The character to be set
    void setCharacter(char character)
    {
        switch (character) {
        case '\n':
        case '\r':
            print("\r\n");
            if (!m_inputCommand.empty()) {
                runCommand();

                // Only add to history if so is allowed
                if (TConfig.commandHistorySize > 0) {
                    if (m_commandHistory.size() >= m_commandHistorySize)
                        m_commandHistory.erase(m_commandHistory.begin());

                    m_commandHistory.push_back(m_inputCommand);
                    m_inputCommand.clear();
                    m_commandHistoryIndex = m_commandHistory.end();
                }
            } else
                print(m_prompt.c_str());
            m_position = m_inputCommand.length();
            break;
        case EndOfText:
            m_inputCommand.clear();
            printInputCommand();
            m_position = m_inputCommand.length();
            break;
        case Del:
        case Backspace:
            if (!m_inputCommand.empty()) {
                if (m_position == m_inputCommand.length()) {
                    print(s_clearCharacter);
                    m_inputCommand.erase(m_inputCommand.length() - 1);
                    m_position = m_inputCommand.length();
                } else if (m_position) {
                    m_inputCommand.erase(--m_position, 1);
                    print(s_moveCursorBackward);

                    for (size_t i = m_position; i < m_inputCommand.length(); i++)
                        print(std::string(1, m_inputCommand.at(i)).c_str());

                    print(std::string(1, ' ').c_str());
                    print(s_clearCharacter); // clear unused char at the end

                    for (size_t i = m_position; i < m_inputCommand.length(); i++)
                        print(s_moveCursorBackward);
                }
            }
            break;
        case Tab:
            printBasedOnInput(AutoCompletionType::Inline);
            printInputCommand();
            m_position = m_inputCommand.length();
            break;
        case Esc:
            m_ctrlState = CtrlState::Esc;
            return;
        case LeftBracket:
            if (m_ctrlState == CtrlState::Esc)
                m_ctrlState = CtrlState::LeftBracket;
            return;
        default:
            if (m_ctrlState == CtrlState::LeftBracket) {
                m_ctrlCharacter += character;
                for (size_t index = 0; index < s_ctrlCharacters.size(); ++index) {
                    if (m_ctrlCharacter.compare(0, m_ctrlCharacter.length(), s_ctrlCharacters[index], 0, m_ctrlCharacter.length()) == 0) {
                        if (m_ctrlCharacter.length() == s_ctrlCharacters[index].length()) {
                            switch (index) {
                            case CharacterUp:
                                if (m_commandHistoryIndex != m_commandHistory.begin()) {
                                    m_inputCommand = *--m_commandHistoryIndex;
                                    printInputCommand();
                                    m_position = m_inputCommand.length();
                                }
                                break;
                            case CharacterDown:
                                if (m_commandHistoryIndex != m_commandHistory.end()) {
                                    ++m_commandHistoryIndex;
                                    if (m_commandHistoryIndex != m_commandHistory.end()) {
                                        m_inputCommand = *m_commandHistoryIndex;
                                    } else {
                                        m_inputCommand.clear();
                                    }
                                    printInputCommand();
                                    m_position = m_inputCommand.length();
                                }
                                break;
                            case CharacterRight:
                                if (m_position != m_inputCommand.length()) {
                                    print(s_moveCursorForward);
                                    m_position++;
                                }
                                break;
                            case CharacterLeft:
                                if (m_position) {
                                    print(s_moveCursorBackward);
                                    m_position--;
                                }
                                break;
                            case CharacterHome:
                                while (m_position) {
                                    print(s_moveCursorBackward);
                                    m_position--;
                                }
                                break;
                            case CharacterDelete:
                                if (m_position != m_inputCommand.length()) {
                                    m_inputCommand.erase(m_position, 1);

                                    print(std::string(1, ' ').c_str());
                                    print(s_clearCharacter); // clear deleted char

                                    for (size_t i = m_position; i < m_inputCommand.length(); i++)
                                        print(std::string(1, m_inputCommand.at(i)).c_str());

                                    print(std::string(1, ' ').c_str());
                                    print(s_clearCharacter); // clear unused char at the end

                                    for (size_t i = m_position; i < m_inputCommand.length(); i++)
                                        print(s_moveCursorBackward);
                                }
                                break;
                            case CharacterEnd:
                                while (m_position != m_inputCommand.length()) {
                                    print(s_moveCursorForward);
                                    m_position++;
                                }
                                break;
                            case CharacterCtrlRight:
                                while (m_position != m_inputCommand.length() && m_inputCommand.at(m_position) == ' ') { // skip spaces until we find the first char
                                    print(s_moveCursorForward);
                                    m_position++;
                                }
                                while (m_position != m_inputCommand.length() && m_inputCommand.at(m_position) != ' ') { // skip chars until we find the first space
                                    print(s_moveCursorForward);
                                    m_position++;
                                }
                                break;
                            case CharacterCtrlLeft:
                                if (m_position && m_position == m_inputCommand.length()) { // step inside the m_command range
                                    print(s_moveCursorBackward);
                                    m_position--;
                                }
                                if (m_position && m_position != m_inputCommand.length() && m_inputCommand.at(m_position) != ' ') { // skip the first char
                                    print(s_moveCursorBackward);
                                    m_position--;
                                }
                                while (m_position && m_position != m_inputCommand.length() && m_inputCommand.at(m_position) == ' ') { // skip spaces until we find the first char
                                    print(s_moveCursorBackward);
                                    m_position--;
                                }
                                while (m_position && m_position != m_inputCommand.length() && m_inputCommand.at(m_position) != ' ') { // skip chars until we find the first space
                                    print(s_moveCursorBackward);
                                    m_position--;
                                }
                                if (m_position && m_position != m_inputCommand.length() && m_inputCommand.at(m_position) == ' ') { // step forward if we hit a space in order to highlight a char
                                    print(s_moveCursorForward);
                                    m_position++;
                                }
                                break;
                            default:
                                break;
                            }
                        } else
                            return; // check the next ctrl character
                    }
                }
                m_ctrlCharacter.clear();
            } else {
                if (m_position == m_inputCommand.length()) {
                    print(std::string(1, character).c_str());
                    m_inputCommand += character;
                    m_position = m_inputCommand.length();
                } else {
                    print(std::string(1, character).c_str());
                    m_inputCommand.insert(m_position++, 1, character);

                    for (size_t i = m_position; i < m_inputCommand.length(); i++)
                        print(std::string(1, m_inputCommand.at(i)).c_str());
                    for (size_t i = m_position; i < m_inputCommand.length(); i++)
                        print(s_moveCursorBackward);
                }
            }
            break;
        }

        m_ctrlState = CtrlState::None;
    }

private:
    enum Character {
        EndOfText = 3,
        Backspace = 8,
        Tab = 9,
        Esc = 27,
        Up = 65,
        Down = 66,
        Right = 67,
        Left = 68,
        LeftBracket = 91,
        Del = 127,
    };

    enum CtrlCharacter {
        CharacterUp = 0,
        CharacterDown,
        CharacterRight,
        CharacterLeft,
        CharacterHome,
        CharacterDelete,
        CharacterEnd,
        CharacterCtrlRight,
        CharacterCtrlLeft
    };

    enum class CtrlState {
        None,
        Esc,
        LeftBracket
    };

    void runCommand()
    {
        for (const auto& command : m_commands) {
            auto argItr = m_commandArgs.begin();

            if (!m_inputCommand.compare(0, command.name.size(), command.name)) {
                auto args = m_inputCommand.substr(command.name.size());
                char* token = std::strtok(args.data(), s_commandDelimiter);
                while (token) {
                    *argItr++ = token;
                    token = std::strtok(nullptr, s_commandDelimiter);
                    if (argItr == m_commandArgs.end())
                        break;
                }

                size_t argsSize = std::distance(m_commandArgs.begin(), argItr);
                if (argsSize >= command.requiredArguments) {
                    command.function(std::span { m_commandArgs.begin(), argItr });
                    print(m_prompt.c_str());
                    return;
                }
            }
        }

        printBasedOnInput(AutoCompletionType::NewLine);
        print(m_prompt.c_str());
    }

    void printInputCommand()
    {
        print(s_clearLine);
        print(m_prompt.c_str());
        print(m_inputCommand.c_str());
    }

    void printNameAndDescription(const std::string_view name, const std::string_view desc, size_t allignmentSize)
    {
        print(name.data());

        for (size_t start = 0; start < ((allignmentSize + 2) - name.size()); start++)
            print(" ");

        print(desc.data());
        print("\r\n");
    }

    enum class AutoCompletionType {
        Inline,
        NewLine,
    };

    void printBasedOnInput(AutoCompletionType completionType)
    {
        auto compareWithInputCommand = [this](const Command& command) {
            return !m_inputCommand.empty() && !std::memcmp(command.name.data(), m_inputCommand.data(), std::min(m_inputCommand.size(), command.name.size()));
        };

        const bool inlineCompletion = completionType == AutoCompletionType::Inline;
        size_t inputCommandCounter { 0 };
        size_t alignmentSize { 0 };
        std::string_view autoCompleteView;

        for (const auto& command : m_commands) {
            if (compareWithInputCommand(command)) {
                autoCompleteView = command.name;
                alignmentSize = std::max(alignmentSize, command.name.size());
                inputCommandCounter++;
            }
        }

        // Only one command with the given input - print auto completion for this one
        if (inlineCompletion && inputCommandCounter == 1) {
            auto completeCommand = std::string { autoCompleteView } + std::string { s_commandDelimiter };
            if (completeCommand.size() > m_inputCommand.size()) {
                m_inputCommand = completeCommand;
                return;
            }
        }

        // Go to next line to make it look like we're still inline
        if (inlineCompletion)
            print("\r\n");

        // No commands matching the input was found so print all instead
        if (inputCommandCounter == 0)
            return printAllCommands();

        // More than one command was found so print all descriptions matching instead
        std::string_view lastCommandView;
        size_t uniqueCounter { 0 };

        for (const auto& command : m_commands) {
            if (compareWithInputCommand(command)) {
                printNameAndDescription(command.name, command.description, alignmentSize);
                std::string_view commandNameSub = command.name.substr(0, command.name.find_first_of(s_commandDelimiter));
                if (commandNameSub != lastCommandView) {
                    lastCommandView = commandNameSub;
                    uniqueCounter++;
                }
            }
        }

        // Auto complete if a single sub name was found
        if (uniqueCounter == 1 && (lastCommandView.size() > m_inputCommand.size()))
            m_inputCommand = std::string { lastCommandView } + std::string { s_commandDelimiter };
    }


    void printAllCommands()
    {
        std::list<std::string_view> groupedCommands;
        for (const auto& command : m_commands) {
            // If command name contains a delimiter it means that we can group those
            auto position = command.name.find_first_of(s_commandDelimiter);
            if (position != std::string::npos) {
                auto subView = command.name.substr(0, position);
                if (std::find(groupedCommands.begin(), groupedCommands.end(), subView) != groupedCommands.end())
                    continue;

                groupedCommands.push_back(subView);

                // Group commands like: i2c  I2c commands
                auto firstCommandView = command.name.substr(0, position);
                std::string firstCommand = std::string { firstCommandView.begin(), firstCommandView.end() } + " commands";
                firstCommand[0] = toupper(firstCommand[0]);

                // We have to do the string conversion as the subspan_view is not terminated
                printNameAndDescription(std::string { subView }, firstCommand, m_allCommandsSizeAlignment);
            } else {
                printNameAndDescription(command.name, command.description, m_allCommandsSizeAlignment);
            }
        }
    }

    static constexpr const char* s_clearLine = "\033[2K\033[100D";
    static constexpr const char* s_clearScreen = "\033[2J\x1B[H";
    static constexpr const char* s_clearCharacter = "\033[1D \033[1D";
    static constexpr const char* s_moveCursorForward = "\033[1C";
    static constexpr const char* s_moveCursorBackward = "\033[1D";
    static constexpr const char* s_commandDelimiter = " ";
    static constexpr std::array<std::string_view, 9> s_ctrlCharacters { { { "A" }, { "B" }, { "C" }, { "D" }, { "1~" }, { "3~" }, { "4~" }, { "1;5C" }, { "1;5D" } } };

    CtrlState m_ctrlState { CtrlState::None };
    CommandSpan m_commands;
    std::array<std::string_view, TConfig.maxRequiredArgs> m_commandArgs;
    std::function<void(const char*)> m_printFunction;
    std::list<std::string> m_commandHistory;
    std::list<std::string>::const_iterator m_commandHistoryIndex;
    std::string m_inputCommand;
    std::string m_prompt { "Yash$ " };
    std::string m_ctrlCharacter;
    size_t m_position { 0 };
    size_t m_commandHistorySize { 0 };

    const size_t m_allCommandsSizeAlignment;
};

} // namespace Yash
