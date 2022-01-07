// Copyright 2022 - Bang & Olufsen a/s
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <functional>
#include <map>
#include <numeric>
#include <string>
#include <vector>
#include <string_view>

namespace Yash {

typedef void (*CommandFunction)(const std::vector<std::string>&);

class Command {
public:
    std::string_view m_name;
    std::string_view m_description;
    CommandFunction m_function;
    size_t m_requiredArguments;
};

template <size_t TCommandArraySize, size_t TBufferSize = 128>
class Yash {
public:
    Yash(size_t historySize = 10)
        : m_historySize(historySize)
        , m_printFunction(::printf)
        , m_commandsIndex(m_commands.begin())
    {
    }

    ~Yash() = default;

    /// @brief Sets the print function to be used
    /// @param print The print funcion to be used
    void setPrint(std::function<void(const char*)> printFunction) { m_printFunction = std::move(printFunction); }

    /// @brief Prints the specified text using the print function
    /// @param text The text to be printed
    void print(const char* text) const { m_printFunction(text); }

    /// @brief Prints the specified text using the print function. Printf style formating can be used
    void printf(const char* fmt, ...)
    {
        std::va_list arg;
        va_start(arg, fmt);
        vsnprintf(m_buffer.data(), m_buffer.size(), fmt, arg);
        va_end(arg);
        m_printFunction(m_buffer.data());
    }

    /// @brief Sets the name of the shell prompt
    /// @param prompt A string with the name to be used
    void setPrompt(const std::string& prompt) { m_prompt = prompt; }

    /// @brief Sets the commands callback
    /// @param callback A commands callback with a reference to the command array
    void setCommandsCallback(std::function<const std::array<Command, TCommandArraySize>&()> callback) { m_commandsCallback = callback; }

    /// @brief Sets a received character on the shell
    /// @param character The character to be set
    void setCharacter(char character)
    {
        switch (character) {
        case '\n':
        case '\r':
            print("\r\n");
            if (!m_command.empty()) {
                runCommand();
                m_commands.push_back(m_command);

                if (m_commands.size() > m_historySize)
                    m_commands.erase(m_commands.begin());

                m_command.clear();
                m_commandsIndex = m_commands.end();
            } else
                print(m_prompt.c_str());
            m_position = m_command.length();
            break;
        case EndOfText:
            m_command.clear();
            printCommand();
            m_position = m_command.length();
            break;
        case Del:
        case Backspace:
            if (!m_command.empty()) {
                if (m_position == m_command.length()) {
                    print(s_clearCharacter);
                    m_command.erase(m_command.length() - 1);
                    m_position = m_command.length();
                } else if (m_position) {
                    m_command.erase(--m_position, 1);
                    print(s_moveCursorBackward);

                    for (size_t i = m_position; i < m_command.length(); i++) {
                        print(std::string(1, m_command.at(i)).c_str());
                    }

                    print(std::string(1, ' ').c_str());
                    print(s_clearCharacter); // clear unused char at the end

                    for (size_t i = m_position; i < m_command.length(); i++) {
                        print(s_moveCursorBackward);
                    }
                }
            }
            break;
        case Tab:
            printDescriptions(true);
            printCommand();
            m_position = m_command.length();
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
                m_ctrlSeq += character;
                for (const auto &it : m_supportedCtrlSeq) {
                    if (m_ctrlSeq.compare(0, m_ctrlSeq.length(), it.first, 0, m_ctrlSeq.length()) == 0) {
                        if (m_ctrlSeq.length() == it.first.length()) {
                            switch (it.second) {
                            case CtrlSeq::Up:
                                if (m_commandsIndex != m_commands.begin()) {
                                    m_command = *--m_commandsIndex;
                                    printCommand();
                                    m_position = m_command.length();
                                }
                                break;
                            case CtrlSeq::Down:
                                if (m_commandsIndex != m_commands.end()) {
                                    ++m_commandsIndex;
                                    if (m_commandsIndex != m_commands.end()) {
                                        m_command = *m_commandsIndex;
                                    } else {
                                        m_command.clear();
                                    }
                                    printCommand();
                                    m_position = m_command.length();
                                }
                                break;
                            case CtrlSeq::Right:
                                if (m_position != m_command.length()) {
                                    print(s_moveCursorForward);
                                    m_position++;
                                }
                                break;
                            case CtrlSeq::Left:
                                if (m_position) {
                                    print(s_moveCursorBackward);
                                    m_position--;
                                }
                                break;
                            case CtrlSeq::Home:
                                while (m_position) {
                                    print(s_moveCursorBackward);
                                    m_position--;
                                }
                                break;
                            case CtrlSeq::Delete:
                                if (m_position != m_command.length()) {
                                    m_command.erase(m_position, 1);

                                    print(std::string(1, ' ').c_str());
                                    print(s_clearCharacter); // clear deleted char

                                    for (size_t i = m_position; i < m_command.length(); i++) {
                                        print(std::string(1, m_command.at(i)).c_str());
                                    }

                                    print(std::string(1, ' ').c_str());
                                    print(s_clearCharacter); // clear unused char at the end

                                    for (size_t i = m_position; i < m_command.length(); i++) {
                                        print(s_moveCursorBackward);
                                    }
                                }
                                break;
                            case CtrlSeq::End:
                                while (m_position != m_command.length()) {
                                    print(s_moveCursorForward);
                                    m_position++;
                                }
                                break;
                            case CtrlSeq::CtrlRight:
                                while (m_position != m_command.length() && m_command.at(m_position) == ' ') { // skip spaces until we find the first char
                                    print(s_moveCursorForward);
                                    m_position++;
                                }
                                while (m_position != m_command.length() && m_command.at(m_position) != ' ') { // skip chars until we find the first space
                                    print(s_moveCursorForward);
                                    m_position++;
                                }
                                break;
                            case CtrlSeq::CtrlLeft:
                                if (m_position && m_position == m_command.length()) { // step inside the m_command range
                                    print(s_moveCursorBackward);
                                    m_position--;
                                }
                                if (m_position && m_position != m_command.length() && m_command.at(m_position) != ' ') { // skip the first char
                                    print(s_moveCursorBackward);
                                    m_position--;
                                }
                                while (m_position && m_position != m_command.length() && m_command.at(m_position) == ' ') { // skip spaces until we find the first char
                                    print(s_moveCursorBackward);
                                    m_position--;
                                }
                                while (m_position && m_position != m_command.length() && m_command.at(m_position) != ' ') { // skip chars until we find the first space
                                    print(s_moveCursorBackward);
                                    m_position--;
                                }
                                if (m_position && m_position != m_command.length() && m_command.at(m_position) == ' ') { // step forward if we hit a space in order to highlight a char
                                    print(s_moveCursorForward);
                                    m_position++;
                                }
                                break;
                            }
                        } else {
                            return; // check the next ctrl character
                        }
                    }
                }
                m_ctrlSeq.clear();
            } else {
                if (m_position == m_command.length()) {
                    print(std::string(1, character).c_str());
                    m_command += character;
                    m_position = m_command.length();
                } else {
                    print(std::string(1, character).c_str());
                    m_command.insert(m_position++, 1, character);

                    for (size_t i = m_position; i < m_command.length(); i++) {
                        print(std::string(1, m_command.at(i)).c_str());
                    }
                    for (size_t i = m_position; i < m_command.length(); i++) {
                        print(s_moveCursorBackward);
                    }
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

    enum class CtrlState {
        None,
        Esc,
        LeftBracket
    };

    enum class CtrlSeq {
        Up,
        Down,
        Right,
        Left,
        Home,
        Delete,
        End,
        CtrlRight,
        CtrlLeft
    };

    void runCommand()
    {
        if (!m_commandsCallback)
            return;

        std::vector<std::string> arguments;
        for (const auto& command : m_commandsCallback()) {
            if (!m_command.compare(0, command.m_name.size(), command.m_name)) {
                auto args = m_command.substr(command.m_name.size());
                char *token = std::strtok(args.data(), s_commandDelimiter);
                while (token) {
                    arguments.emplace_back(token);
                    token = std::strtok(nullptr, s_commandDelimiter);
                }

                if (arguments.size() >= command.m_requiredArguments) {
                    command.m_function(arguments);
                    print(m_prompt.c_str());
                    return;
                }
            }
        }

        printDescriptions();
        print(m_prompt.c_str());
    }

    void printCommand()
    {
        print(s_clearLine);
        print(m_prompt.c_str());
        print(m_command.c_str());
    }

    void printCommands(const std::map<std::string, std::string> &descriptions)
    {
        size_t maxCommandSize { std::accumulate(begin(descriptions), end(descriptions), 0u, [](size_t max, const auto& desc) {
            return std::max(max, desc.first.size());
        }) };

        for (const auto& [command, description] : descriptions) {
            std::string alignment((maxCommandSize + 2) - command.size(), ' ');
            std::string line;
            line.reserve(command.size() + alignment.size() + description.size() + 2);
            line.append(command).append(alignment).append(description).append("\r\n");
            print(line.c_str());
        }
    }

    void printDescriptions(bool autoComplete = false)
    {
        if (!m_commandsCallback)
            return;

        std::map<std::string, std::string> descriptions;
        for (const auto& command : m_commandsCallback()) {
            if (!m_command.empty() && !std::memcmp(command.m_name.data(), m_command.data(), std::min(m_command.size(), command.m_name.size())))
                descriptions.emplace(command.m_name, command.m_description);
        }

        if ((descriptions.size() == 1) && autoComplete) {
            auto completeCommand = descriptions.begin()->first + s_commandDelimiter;
            if (completeCommand.size() > m_command.size()) {
                m_command = completeCommand;
                return;
            }
        } else {
            if (descriptions.empty()) {
                for (const auto& command : m_commandsCallback()) {
                    auto position = command.m_name.find_first_of(s_commandDelimiter);
                    if (position != std::string::npos) {
                        auto firstCommandView = command.m_name.substr(0, position);
                        std::string firstCommand = {firstCommandView.begin(), firstCommandView.end()};
                        firstCommand[0] = toupper(firstCommand[0]);
                        descriptions.emplace(command.m_name.substr(0, position), firstCommand + " commands");
                    }
                    else
                        descriptions.emplace(command.m_name, command.m_description);
                }
            } else {
                std::string firstCommand;
                for (const auto& [command, function] : descriptions) {
                    std::ignore = function;
                    if (firstCommand.empty())
                        firstCommand = command.substr(0, command.find_first_of(s_commandDelimiter));
                    if (firstCommand != command.substr(0, command.find_first_of(s_commandDelimiter))) {
                        firstCommand.clear();
                        break;
                    }
                }

                if (!firstCommand.empty() && (firstCommand.size() > m_command.size()))
                    m_command = firstCommand + s_commandDelimiter;
            }
        }

        if (autoComplete)
            print("\r\n");
        printCommands(descriptions);
    }

    static constexpr const char* s_clearLine = "\033[2K\033[100D";
    static constexpr const char* s_clearScreen = "\033[2J\x1B[H";
    static constexpr const char* s_clearCharacter = "\033[1D \033[1D";
    static constexpr const char* s_moveCursorForward = "\033[1C";
    static constexpr const char* s_moveCursorBackward = "\033[1D";
    static constexpr const char* s_commandDelimiter = " ";

    size_t m_position {0};
    size_t m_historySize {0};
    std::function<const std::array<Command, TCommandArraySize>&()> m_commandsCallback;
    std::function<void(const char*)> m_printFunction;
    std::vector<std::string> m_commands;
    std::vector<std::string>::const_iterator m_commandsIndex;
    std::string m_command;
    std::string m_prompt;
    std::array<char, TBufferSize> m_buffer{};
    CtrlState m_ctrlState { CtrlState::None };
    std::string m_ctrlSeq;
    const std::map<std::string, CtrlSeq> m_supportedCtrlSeq = {
        {"A", CtrlSeq::Up},
        {"B", CtrlSeq::Down},
        {"C", CtrlSeq::Right},
        {"D", CtrlSeq::Left},
        {"1~", CtrlSeq::Home},
        {"3~", CtrlSeq::Delete},
        {"4~", CtrlSeq::End},
        {"1;5C", CtrlSeq::CtrlRight},
        {"1;5D", CtrlSeq::CtrlLeft}
    };
};

} // namespace Yash
