#pragma once
#include <chrono>
#include <compare>
#include <ostream>
#include <string>
#include <string_view>
#include "Utility.hpp"

namespace Hotel
{
    // A calendar date without a time of day.
    class Date
    {
    public:
        Date() noexcept = default;

        // Strictly parses a YYYY-MM-DD string
        static Result parse(const char* str, Date& out) noexcept;
        static Result fromComponents(int year, unsigned month, unsigned day, Date& out) noexcept;

        static Date today() noexcept;

        [[nodiscard]] int64_t daysUntil(const Date& other) const noexcept;
        [[nodiscard]] Date addDays(int64_t days) const noexcept;

        [[nodiscard]] int getYear() const noexcept;
        [[nodiscard]] unsigned getMonth() const noexcept;
        [[nodiscard]] unsigned getDay() const noexcept;

        // Formats the date as YYYY-MM-DD
        [[nodiscard]] std::string toString() const noexcept;

        auto operator<=>(const Date&) const noexcept = default;
        bool operator==(const Date&) const noexcept = default;
    private:
        explicit Date(std::chrono::sys_days days) noexcept;

        std::chrono::sys_days value{};
    };

    std::ostream& operator<<(std::ostream& stream, const Date& date) noexcept;
}
