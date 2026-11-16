#include "Utility.hpp"
#include <algorithm>
#include <cctype>
#include <format>
#include <fstream>

std::string Hotel::formatMoney(const Cents amount) noexcept
{
    const bool bNegative = amount < 0;
    const uint64_t magnitude = bNegative
        ? (0ULL - static_cast<uint64_t>(amount))
        : static_cast<uint64_t>(amount);

    return std::format(
        "{}{}.{:02}",
        bNegative ? "-" : "",
        magnitude / 100,
        magnitude % 100
    );
}

Hotel::Result Hotel::parseMoney(const char* str, Cents& out) noexcept
{
    const std::string_view value = trim(str);
    const size_t dot = value.find('.');

    const std::string_view whole = value.substr(0, dot);
    const std::string_view fraction = dot == std::string_view::npos ? std::string_view{} : value.substr(dot + 1);

    if (whole.empty() || whole.size() > 9 || fraction.size() > 2)
        return Result::InvalidAmount;

    if (dot != std::string_view::npos && fraction.empty())
        return Result::InvalidAmount;

    const auto wholePart = parseNumber<uint64_t>(whole);
    const auto fractionPart = fraction.empty() ? std::optional<uint64_t>{ 0 } : parseNumber<uint64_t>(fraction);
    if (!wholePart || !fractionPart)
        return Result::InvalidAmount;

    const auto cents = static_cast<Cents>(fraction.size() == 1 ? *fractionPart * 10 : *fractionPart);
    const Cents result = static_cast<Cents>(*wholePart) * 100 + cents;
    if (result > MAX_AMOUNT)
        return Result::InvalidAmount;

    out = result;
    return Result::Ok;
}

Hotel::Cents Hotel::calculateTax(const Cents netAmount) noexcept
{
    return (netAmount * static_cast<Cents>(TAX_RATE_BASIS_POINTS) + 5000) / 10000;
}

static inline bool isSpace(const unsigned char c) noexcept
{
    return std::isspace(c) != 0;
}

std::string_view Hotel::trim(const std::string_view str) noexcept
{
    const auto begin = std::ranges::find_if_not(str, isSpace);
    const auto end = std::ranges::find_if_not(str.rbegin(), str.rend(), isSpace).base();
    if (begin >= end)
        return {};
    return str.substr(static_cast<size_t>(begin - str.begin()), static_cast<size_t>(end - begin));
}

std::string Hotel::toLower(std::string str) noexcept
{
    std::ranges::transform(
        str,
        str.begin(),
        [](const unsigned char c) noexcept -> char
        {
            return static_cast<char>(std::tolower(c));
        }
    );
    return str;
}

void Hotel::printRule(std::ostream& stream, const char ch, const size_t width) noexcept
{
    stream << std::string(width, ch) << "\n";
}

bool Hotel::isValidText(const std::string_view str, const size_t maxLength) noexcept
{
    if (str.size() > maxLength)
        return false;

    return std::ranges::none_of(
        str,
        [](const unsigned char c) noexcept -> bool
        {
            return c < 0x20 || c == 0x7F;
        }
    );
}

Hotel::Result Hotel::ensureDirectory(const std::filesystem::path& directory) noexcept
{
    if (directory.empty())
        return Result::FileError;

    std::error_code ec;
    std::filesystem::create_directories(directory, ec);
    return ec ? Result::FileError : Result::Ok;
}

Hotel::Result Hotel::writeTextFile(const std::filesystem::path& file, const char* content) noexcept
{
    std::ofstream out(file, std::ios::binary | std::ios::trunc);
    if (!out)
        return Result::FileError;

    out << content;
    out.flush();

    return out ? Result::Ok : Result::FileError;
}
