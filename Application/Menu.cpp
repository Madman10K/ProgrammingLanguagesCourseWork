#include "Menu.hpp"
#include <iostream>
#include <ranges>
#include <vector>

Hotel::Menu::Menu(HotelManager& hotelManager) noexcept : manager(hotelManager)
{
}

void Hotel::Menu::runMenu(const char* title, const std::vector<MenuItem>& items, const char* backLabel) noexcept
{
    std::vector<std::string> labels;
    labels.reserve(items.size());
    for (const auto& label : items)
        labels.emplace_back(label.first);

    while (!bEOF)
    {
        const size_t choice = readChoice(title, labels, backLabel);
        if (choice == 0 || choice > items.size())
            return;

        items[choice - 1].second();
    }
}

void Hotel::Menu::run() noexcept
{
    std::cout << "HotelManager - data directory: " << manager.getDataDirectory().string() << ", today is " << manager.getToday() << "\n";

    runMenu(
        "Main menu",
        {
            {
                "Rooms",
                [this]() -> void
                {
                    roomsMenu();
                }
            },
            {
                "Guests",
                [this]() -> void
                {
                    guestsMenu();
                }
            },
            {
                "Reservations",
                [this]() -> void
                {
                    reservationsMenu();
                }
            },
            {
                "Reports",
                [this]() -> void
                {
                    reportsMenu();
                }
            },
            {
                "Invoices",
                [this]() -> void
                {
                    invoicesMenu();
                }
            }
        },
        "Save and exit"
    );
}
