#pragma once
#include <filesystem>
#include <ostream>
#include <string>
#include <string_view>
#include "HotelManager.hpp"

namespace Hotel
{
    class Report
    {
    public:
        explicit Report(const HotelManager& manager) noexcept;
        virtual ~Report() noexcept = default;

        virtual void generate(std::ostream& stream) const noexcept = 0;

        // A short, file name friendly title, e.g. "occupancy-2026-10-01-2026-10-31"
        [[nodiscard]] virtual std::string getTitle() const noexcept = 0;

        Result saveToFile(const std::filesystem::path& directory, std::filesystem::path& path) const noexcept;
    protected:
        const HotelManager& manager;
        static void printHeading(std::ostream& stream, const char* heading) noexcept;
    };

    // Reports that have a start and end date
    class PeriodReport : public Report
    {
    public:
        PeriodReport(const HotelManager& manager, const Date& from, const Date& to) noexcept;
    protected:
        Date from;
        Date to;

        // Converts the user-facing inclusive end date into an exclusive upper bound [from, to + 1)
        [[nodiscard]] Date getEndExclusive() const noexcept;
        [[nodiscard]] std::string formatPeriodTitle(const char* prefix) const noexcept;
    };

    class OccupancyReport final : public PeriodReport
    {
    public:
        using PeriodReport::PeriodReport;

        void generate(std::ostream& stream) const noexcept override;
        [[nodiscard]] std::string getTitle() const noexcept override;
    };

    class FinancialReport final : public PeriodReport
    {
    public:
        using PeriodReport::PeriodReport;

        void generate(std::ostream& stream) const noexcept override;
        [[nodiscard]] std::string getTitle() const noexcept override;
    };

    class GuestReport final : public Report
    {
    public:
        // A guestID of 0 creates a summary of all guests
        explicit GuestReport(const HotelManager& manager, uint64_t guestID = 0) noexcept;

        void generate(std::ostream& stream) const noexcept override;
        [[nodiscard]] std::string getTitle() const noexcept override;
    private:
        struct GuestStats
        {
            size_t stays = 0;
            size_t upcoming = 0;
            uint64_t nights = 0;
            Cents spent = 0;
        };

        uint64_t guestID = 0;

        [[nodiscard]] GuestStats computeGuestStats(uint64_t id) const noexcept;
        void generateSummary(std::ostream& stream) const noexcept;
        void generateDetails(std::ostream& stream, const Guest& guest) const noexcept;
    };

    std::ostream& operator<<(std::ostream& stream, const Report& report) noexcept;
}
