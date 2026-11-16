#include "Date.hpp"
#include <format>

Hotel::Date::Date(const std::chrono::sys_days days) noexcept : value(days)
{
}

Hotel::Result Hotel::Date::parse(const char* str, Date& out) noexcept
{
    const std::string_view trimmed = trim(str);

    // 10 from "YYYY-MM-DD"
    if (trimmed.size() != 10 || trimmed[4] != '-' || trimmed[7] != '-')
        return Result::InvalidDate;

    const auto year = parseNumber<uint64_t>(trimmed.substr(0, 4));
    const auto month = parseNumber<uint64_t>(trimmed.substr(5, 2));
    const auto day = parseNumber<uint64_t>(trimmed.substr(8, 2));
    if (!year || !month || !day)
        return Result::InvalidDate;

    return fromComponents(
        static_cast<int>(*year),
        static_cast<unsigned>(*month),
        static_cast<unsigned>(*day),
        out
    );
}

Hotel::Result Hotel::Date::fromComponents(const int year, const unsigned month, const unsigned day, Date& out) noexcept
{
    if (year < 2000 || year > 2100)
        return Result::InvalidDate;

    const std::chrono::year_month_day ymd =
    {
        std::chrono::year(year),
        std::chrono::month(month),
        std::chrono::day(day)
    };
    if (!ymd.ok())
        return Result::InvalidDate;

    out.value = std::chrono::sys_days(ymd);
    return Result::Ok;
}

Hotel::Date Hotel::Date::today() noexcept
{
    const auto now = std::chrono::system_clock::now();
    try
    {
        const auto localNow = std::chrono::current_zone()->to_local(now);
        const auto localDays = std::chrono::floor<std::chrono::days>(localNow);

        return Date
        {
            std::chrono::sys_days
            {
                std::chrono::year_month_day
                {
                    std::chrono::local_days
                    {
                        localDays.time_since_epoch()
                    }
                }
            }
        };
    }
    catch (...)
    {
        return Date{ std::chrono::floor<std::chrono::days>(now) };
    }
}

int64_t Hotel::Date::daysUntil(const Date& other) const noexcept
{
    return (other.value - value).count();
}

Hotel::Date Hotel::Date::addDays(const int64_t days) const noexcept
{
    return Date{ value + std::chrono::days{ days } };
}

int Hotel::Date::getYear() const noexcept
{
    const std::chrono::year_month_day ymd{ value };
    return static_cast<int>(ymd.year());
}

unsigned Hotel::Date::getMonth() const noexcept
{
    const std::chrono::year_month_day ymd{ value };
    return static_cast<unsigned>(ymd.month());
}

unsigned Hotel::Date::getDay() const noexcept
{
    const std::chrono::year_month_day ymd{ value };
    return static_cast<unsigned>(ymd.day());
}

std::string Hotel::Date::toString() const noexcept
{
    return std::format("{:04}-{:02}-{:02}", getYear(), getMonth(), getDay());
}

std::ostream& Hotel::operator<<(std::ostream& stream, const Date& date) noexcept
{
    return stream << date.toString();
}
