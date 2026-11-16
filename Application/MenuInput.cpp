#include "Menu.hpp"
#include <format>
#include <iostream>

std::optional<std::string> Hotel::Menu::readLine(const char* prompt) noexcept
{
    if (bEOF)
        return std::nullopt;
    std::cout << prompt << std::flush;

    std::string line;
    if (!std::getline(std::cin, line))
    {
        bEOF = true;
        std::cout << "\n";
        return std::nullopt;
    }
    return line;
}

std::optional<int64_t> Hotel::Menu::readInteger(const char* prompt, const int64_t min, const int64_t max) noexcept
{
    return read<int64_t>(
        prompt,
        [&](const char* str, int64_t& out) -> std::string
        {
            const auto parsed = parseNumber<int64_t>(str);
            if (!parsed || *parsed < min || *parsed > max)
                return std::format("Expected a whole number between {} and {}", min, max);

            out = *parsed;
            return "";
        }
    );
}

std::optional<uint64_t> Hotel::Menu::readID(const char* prompt, const uint64_t min) noexcept
{
    const auto value = readInteger(prompt, static_cast<int64_t>(min), INT64_MAX);
    return value ? std::optional<uint64_t>{ static_cast<uint64_t>(*value) } : std::nullopt;
}

std::optional<uint32_t> Hotel::Menu::readRoomNumber(const char* prompt, const std::function<std::string(uint32_t)>& extraCheck) noexcept
{
    return read<uint32_t>(
        prompt,
        [&](const char* str, uint32_t& out) -> std::string
        {
            const auto parsed = parseNumber<uint32_t>(str);
            if (!parsed || !(*parsed >= MIN_ROOM_NUMBER && *parsed <= MAX_ROOM_NUMBER))
                return Result(Result::InvalidRoomNumber).toString();

            out = *parsed;
            return extraCheck ? extraCheck(out) : "";
        }
    );
}

std::optional<Hotel::Cents> Hotel::Menu::readMoney(const char* prompt) noexcept
{
    return read<Cents>(
        prompt,
        [](const char* str, Cents& out) -> std::string
        {
            if (parseMoney(str, out) != Result::Ok || out <= 0)
                return Result(Result::InvalidAmount).toString();

            return "";
        }
    );
}

std::optional<Hotel::Date> Hotel::Menu::readDate(const char* prompt) noexcept
{
    return read<Date>(
        std::format("{} (YYYY-MM-DD)", prompt),
        [](const char* str, Date& out) -> std::string
        {
            CHECK(Date::parse(str, out));
            return "";
        }
    );
}

std::optional<Hotel::Date> Hotel::Menu::readEndDate(const char* prompt, const Date& start, const bool bInclusive, const bool bLimitNights) noexcept
{
    return read<Date>(
        std::format("{} (YYYY-MM-DD)", prompt),
        [&](const char* str, Date& out) -> std::string
        {
            CHECK(Date::parse(str, out));
            if (bInclusive)
                return out < start ? "The end of the period cannot be before its start" : "";
            if (bLimitNights)
            {
                CHECK(Reservation::validateDates(start, out));
                return "";
            }
            return out <= start ? Result(Result::InvalidDateRange).toString() : "";
        }
    );
}

std::optional<bool> Hotel::Menu::readYesNo(const char* prompt) noexcept
{
    return read<bool>(
        prompt,
        [](const char* str, bool& out) -> std::string
        {
            const std::string value = toLower(str);
            if (value == "y" || value == "yes")
                out = true;
            else if (value == "n" || value == "no")
                out = false;
            else
                return "Expected y or n";
            return "";
        }
    );
}

bool Hotel::Menu::confirm(const char* question) noexcept
{
    return readYesNo(
        std::format(
            "{} (y/n){}",
            question,
            CANCEL_HINT
        ).c_str()
    )
    .value_or(false);
}

std::optional<std::string> Hotel::Menu::readText(const char* prompt, const bool bAllowEmpty) noexcept
{
    while (true)
    {
        const auto line = readLine(prompt);
        if (!line)
            return std::nullopt;

        const std::string_view value = trim(*line);
        if (value.empty() && !bAllowEmpty)
            return std::nullopt;

        if (isValidText(value))
            return std::string{ value };

        printError(Result(Result::InvalidText).toString());
    }
}

bool Hotel::Menu::readGuestField(Guest& guest, const char* prompt, const std::function<Result(Guest&, const char*)>& setter) noexcept
{
    return read<std::string>(
        prompt,
        [&](const char* str, std::string&) -> std::string
        {
            CHECK(setter(guest, str));
            return "";
        }
    ).has_value();
}

size_t Hotel::Menu::readChoice(const char* title, const std::vector<std::string>& options, const char* backLabel) noexcept
{
    std::cout << "\n--- " << title << " ---\n";
    size_t index = 0;
    for (const auto& option : options)
        std::cout << "  " << ++index << ". " << option << "\n";
    std::cout << "  0. " << backLabel << "\n";

    while (true)
    {
        const auto line = readLine("> ");
        if (!line)
            return 0;
        if (const auto choice = parseNumber<uint64_t>(trim(*line)); choice && *choice <= options.size())
            return static_cast<size_t>(*choice);
        printError(std::format("Expected a number between 0 and {}", options.size()).c_str());
    }
}