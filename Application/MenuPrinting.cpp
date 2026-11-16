#include "Menu.hpp"
#include <format>
#include <iostream>

// ANSI escape codes for coloured terminal output
static constexpr const auto* COLOUR_GREEN = "\x1b[32m";
static constexpr const auto* COLOUR_YELLOW = "\x1b[33m";
static constexpr const auto* COLOUR_RED = "\x1b[31m";
static constexpr const auto* COLOUR_RESET = "\x1b[0m";

void Hotel::Menu::printColoured(const char* colour, const char* prefix, const char* message) noexcept
{
    std::cout << colour << prefix << message << COLOUR_RESET << "\n";
}

void Hotel::Menu::printError(const char* message) noexcept
{
    printColoured(COLOUR_RED, "Error: ", message);
}

void Hotel::Menu::printWarning(const char* message) noexcept
{
    printColoured(COLOUR_YELLOW, "", message);
}

void Hotel::Menu::printSuccess(const char* message) noexcept
{
    printColoured(COLOUR_GREEN, "", message);
}

bool Hotel::Menu::printResult(const Result result, const char* successMessage) noexcept
{
    if (result == Result::Ok)
    {
        printSuccess(successMessage);
        return true;
    }

    // The change itself went through, so report it, but make sure that the user knows that it is not on disk
    if (result == Result::SaveFailed)
    {
        printSuccess(successMessage);
        printWarning(std::format("Warning: {}", result.toString()).c_str());
        return true;
    }

    printError(result.toString());
    return false;
}
