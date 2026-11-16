#include "Menu.hpp"
#include <format>
#include <iostream>

void Hotel::Menu::reportsMenu() noexcept
{
    while (!bEOF)
    {
        const size_t choice = readChoice("Reports", { "Room occupancy report", "Financial report", "Guest report" });
        if (choice == 0)
            return;

        if (choice == 3)
        {
            const auto id = readID("Guest ID, 0 for a summary of all guests", 0);
            if (!id)
                continue;
            if (*id != 0 && manager.findGuest(*id) == nullptr)
            {
                printError(Result(Result::GuestNotFound).toString());
                continue;
            }
            showReport(GuestReport(manager, *id));
            continue;
        }

        const auto from = readDate("Period start");
        if (!from)
            continue;

        const auto to = readEndDate("Period end, inclusive", *from, true, false);
        if (!to)
            continue;

        if (choice == 1)
            showReport(OccupancyReport(manager, *from, *to));
        else
            showReport(FinancialReport(manager, *from, *to));
    }
}

void Hotel::Menu::showReport(const Report& report) noexcept
{
    std::cout << "\n" << report << "\n";
    if (!confirm("Save the report to a file?"))
        return;

    std::filesystem::path path;
    const Result result = report.saveToFile("Reports", path);
    printResult(
        result,
        std::format(
            "The report was saved to {}",
            path.string()
        ).c_str()
    );
}
