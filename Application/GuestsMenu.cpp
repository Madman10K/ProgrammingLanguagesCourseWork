#include "Menu.hpp"
#include <format>
#include <iostream>

void Hotel::Menu::guestsMenu() noexcept
{
    runMenu(
        "Guests",
        {
            {
                "List guests",
                [this]() -> void
                {
                    listGuests();
                }
            },
            {
                "Search guests",
                [this]() -> void
                {
                    searchGuests();
                }
            },
            {
                "Register a guest",
                [this]() -> void
                {
                    addGuest();
                }
            },
            {
                "Edit a guest",
                [this]() -> void
                {
                    editGuest();
                }
            },
            {
                "Remove a guest",
                [this]() -> void
                {
                    removeGuest();
                }
            }
        }
    );
}

void Hotel::Menu::listGuests() const noexcept
{
    printList(
        manager.getGuests(),
        "There are no registered guests yet",
        [](const Guest& guest) noexcept -> void
        {
            std::cout << "  " << guest << "\n";
        }
    );
}

void Hotel::Menu::searchGuests() noexcept
{
    const auto query = readText(
        std::format(
            "Name, ID document, phone or e-mail{}",
            CANCEL_HINT
        ).c_str(),
        false
    );
    if (!query)
        return;

    const auto guests = manager.searchGuests(query->c_str());
    std::cout << guests.size() << " guests found\n";
    for (const auto* guest : guests)
        std::cout << "  " << *guest << "\n";
}

uint64_t Hotel::Menu::addGuest() noexcept
{
    Guest guest;
    if (    !readGuestField(guest, "First name", &Guest::setFirstName)
        ||  !readGuestField(guest, "Last name", &Guest::setLastName)
        ||  !readGuestField(guest, "ID document (passport or ID card number)", &Guest::setIDDocument)
        ||  !readGuestField(guest, "Phone", &Guest::setPhone)
        ||  !readGuestField(guest, "E-mail", &Guest::setEmail)
    )
        return 0;

    // Add the guest first: the message needs the new ID, and the order in which function arguments are evaluated is
    // unspecified, so formatting it in the same call could read `id` before addGuest has set it
    uint64_t id = 0;
    const Result result = manager.addGuest(guest, id);
    if (!printResult(result, std::format("{} was registered as guest #{}", guest.getFullName(), id).c_str()))
        return 0;
    return id;
}

void Hotel::Menu::editGuest() noexcept
{
    const auto* existing = readExisting<Guest>(
        "Guest ID",
        uint64_t{ 1 },
        static_cast<uint64_t>(INT64_MAX),
        &HotelManager::findGuest,
        Result::GuestNotFound
    );
    if (existing == nullptr)
        return;

    std::cout << "  " << *existing << "\n";

    Guest guest = *existing;
    const auto editField = [&](const char* label, const char* current, const std::function<Result(Guest&, const char*)>& setter) -> void
    {
        readGuestField(
            guest,
            std::format(
                "{}, currently {}{}",
                label,
                current,
                KEEP_HINT
            ).c_str(),
            setter
        );
    };

    editField("First name", guest.getFirstName().c_str(), &Guest::setFirstName);
    editField("Last name", guest.getLastName().c_str(), &Guest::setLastName);
    editField("ID document", guest.getIDDocument().c_str(), &Guest::setIDDocument);
    editField("Phone", guest.getPhone().c_str(), &Guest::setPhone);
    editField("E-mail", guest.getEmail().c_str(), &Guest::setEmail);

    if (bEOF)
        return;
    printResult(manager.updateGuest(guest), "The guest was updated");
}

void Hotel::Menu::removeGuest() noexcept
{
    const auto* guest = readExisting<Guest>(
        "Guest ID",
        uint64_t{ 1 },
        static_cast<uint64_t>(INT64_MAX),
        &HotelManager::findGuest,
        Result::GuestNotFound
    );
    if (guest == nullptr)
        return;

    if (confirm(std::format("Remove {}?", guest->getFullName()).c_str()))
        printResult(manager.removeGuest(guest->getID()), "The guest was removed");
}
