// Copyright 2021 - Bang & Olufsen a/s
#pragma once

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <functional>
#include <map>
#include <numeric>
#include <string>
#include <vector>

#ifndef YASH_HISTORY_SIZE
#define YASH_HISTORY_SIZE 10
#endif

namespace Yash {

using YashPrint = std::function<void(const char*)>;
using YashFunction = std::function<void(const std::vector<std::string>&)>;

class Yash {
public:
    Yash()
        : m_commands{}
        , m_commandsIndex(m_commands.begin())
        , m_printFunction(::printf)
    {
    }

    ~Yash() = default;

    /// @brief Sets the print function to be used
    /// @param print The YashPrint funcion to be used
    void setPrint(YashPrint print) { m_printFunction = print; }

    /// @brief Prints the specified text using the print function
    /// @param text The text to be printed
    void print(const char* text) { m_printFunction(text); }

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

    /// @brief Adds a command to the shell
    /// @param command A string with the name of the command
    /// @param description A string with the command description
    /// @param function A YashFunction to be called when the command is executed
    void addCommand(const std::string& command, const std::string& description, YashFunction function)
    {
        addCommand(command, "", description, function);
    }

    /// @brief Adds a command with a sub command to the shell
    /// @param command A string with the name of the command
    /// @param subCommand A string with the name of the sub command
    /// @param description A string with the command description
    /// @param function A YashFunction to be called when the command is executed
    void addCommand(const std::string& command, const std::string& subCommand, const std::string& description, YashFunction function)
    {
        auto fullCommand = subCommand.empty() ? command : command + s_commandDelimiter + subCommand;
        m_functions.emplace(fullCommand, function);
        m_descriptions.emplace(fullCommand, description);
    }

    /// @brief Removes a command from the shell
    /// @param command A string with the name of the command
    void removeCommand(const std::string& command)
    {
        removeCommand(command, "");
    }

    /// @brief Removes a command from the shell
    /// @param command A string with the name of the command
    /// @param subCommand A string with the name of the sub command
    void removeCommand(const std::string& command, const std::string& subCommand)
    {
        auto fullCommand = subCommand.empty() ? command : command + s_commandDelimiter + subCommand;
        m_functions.erase(fullCommand);
        m_descriptions.erase(fullCommand);
    }

    /// @brief Sets a received character on the shell
    /// @param character The character to be set
    void setCharacter(char character)
    {
        switch (character) {
        case '\n':
        case '\r':
            print("\r\n");
            if (m_command.length()) {
                runCommand();
                m_commands.push_back(m_command);

                if (m_commands.size() > YASH_HISTORY_SIZE)
                    m_commands.erase(m_commands.begin());

                m_command.clear();
                m_commandsIndex = m_commands.end();
            } else
                print(m_prompt.c_str());
            break;
        case EndOfText:
            m_command.clear();
            printCommand();
            break;
        case Del:
        case Backspace:
            if (m_command.length()) {
                print(s_clearCharacter);
                m_command.erase(m_command.length() - 1);
            }
            break;
        case Tab:
            printDescriptions(true);
            printCommand();
            break;
        case Esc:
            m_ctrlState = CtrlState::Esc;
            return;
        case LeftBracket:
            if (m_ctrlState == CtrlState::Esc)
                m_ctrlState = CtrlState::LeftBracket;
            return;
        default:
            if (character == Up && m_ctrlState == CtrlState::LeftBracket) {
                if (m_commandsIndex != m_commands.begin()) {
                    m_command = *--m_commandsIndex;
                    printCommand();
                }
            } else if (character == Down && m_ctrlState == CtrlState::LeftBracket) {
                if (m_commandsIndex != m_commands.end()) {
                    ++m_commandsIndex;
                    if (m_commandsIndex != m_commands.end())
                        m_command = *m_commandsIndex;
                    else
                        m_command.clear();
                    printCommand();
                }
            } else {
                print(std::string(1, character).c_str());
                m_command += character;
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
        LeftBracket = 91,
        Del = 127,
    };

    enum class CtrlState {
        None,
        Esc,
        LeftBracket
    };

    void runCommand()
    {
        m_args.clear();
        for (const auto& [command, function] : m_functions) {
            std::ignore = function;
            if (!m_command.compare(0, command.length(), command)) {
                auto args = m_command.substr(command.length());
                char *token = std::strtok(args.data(), s_commandDelimiter);
                while (token) {
                    m_args.push_back(token);
                    token = std::strtok(nullptr, s_commandDelimiter);
                }
                function(m_args);
                print(m_prompt.c_str());
                return;
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
            auto line { command + alignment + description + "\r\n" };
            print(line.c_str());
        }
    }

    void printDescriptions(bool autoComplete = false)
    {
        std::map<std::string, std::string> descriptions;
        for (const auto& [command, description] : m_descriptions) {
            if (!m_command.empty() && !std::memcmp(command.data(), m_command.data(), std::min(m_command.length(), command.length())))
                descriptions.emplace(command, description);
        }

        if (descriptions.size() == 1 && autoComplete) {
            auto completeCommand = descriptions.begin()->first + s_commandDelimiter;
            if (m_command != completeCommand) {
                m_command = completeCommand;
                return;
            }
        } else {
            if (descriptions.empty()) {
                for (const auto& [command, description] : m_descriptions) {
                    auto position = command.find(s_commandDelimiter);
                    if (position != std::string::npos)
                        descriptions.emplace(command.substr(0, position), "Commands");
                    else
                        descriptions.emplace(command, description);
                }
            } else {
                std::string firstCommand;
                for (const auto& [command, description] : descriptions) {
                    std::ignore = description;
                    if (firstCommand.empty())
                        firstCommand = command.substr(0, command.find_first_of(s_commandDelimiter));
                    if (firstCommand != command.substr(0, command.find_first_of(s_commandDelimiter))) {
                        firstCommand.clear();
                        break;
                    }
                }
                if (!firstCommand.empty())
                    m_command = firstCommand + s_commandDelimiter;
            }
        }

        print("\r\n");
        printCommands(descriptions);
    }

    static constexpr const char* s_clearLine = "\033[2K\033[100D";
    static constexpr const char* s_clearScreen = "\033[2J\x1B[H";
    static constexpr const char* s_clearCharacter = "\033[1D \033[1D";
    static constexpr const char* s_commandDelimiter = " ";
    std::map<std::string, YashFunction> m_functions;
    std::map<std::string, std::string> m_descriptions;
    std::vector<std::string> m_commands;
    std::vector<std::string>::const_iterator m_commandsIndex;
    std::vector<std::string> m_args;
    std::string m_command;
    std::string m_prompt;
    YashPrint m_printFunction;
    std::array<char, 128> m_buffer;
    CtrlState m_ctrlState { CtrlState::None };
};

} // namespace Yash
