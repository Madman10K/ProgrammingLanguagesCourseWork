#pragma once
#include <charconv>
#include <concepts>
#include <filesystem>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include "Common.hpp"

namespace Hotel
{
    [[nodiscard]] std::string formatMoney(Cents amount) noexcept;

    Result parseMoney(const char* str, Cents& out) noexcept;

    [[nodiscard]] Cents calculateTax(Cents netAmount) noexcept;

    [[nodiscard]] std::string_view trim(std::string_view str) noexcept;
    [[nodiscard]] std::string toLower(std::string str) noexcept;

    void printRule(std::ostream& stream, char ch = '-', size_t width = 78) noexcept;

    [[nodiscard]] bool isValidText(std::string_view str, size_t maxLength = 128) noexcept;

    template<std::integral T>
    [[nodiscard]] std::optional<T> parseNumber(const std::string_view str) noexcept
    {
        if (str.empty())
            return std::nullopt;

        T result{};
        const char* end = str.data() + str.size();
        if (const auto [ptr, ec] = std::from_chars(str.data(), end, result); ec != std::errc{} || ptr != end)
            return std::nullopt;
        return result;
    }

    Result ensureDirectory(const std::filesystem::path& directory) noexcept;

    Result writeTextFile(const std::filesystem::path& file, const char* content) noexcept;
}
