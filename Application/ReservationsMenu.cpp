#include "Menu.hpp"
#include <algorithm>
#include <format>
#include <iostream>

void Hotel::Menu::reservationsMenu() noexcept
{
    runMenu(
        "Reservations",
        {
            {
                "List reservations",
                [this]() -> void
                {
                    listReservations();
                }
            },
            {
                "Make a reservation",
                [this]() -> void
                {
                    makeReservation();
                }
            },
            {
                "Cancel a reservation",
                [this]() -> void
                {
                    cancelReservation();
                }
            },
            {
                "Check in",
                [this]() -> void
                {
                    checkIn();
                }
            },
            {
                "Check out and issue invoice",
                [this]() -> void
                {
                    checkOut();
                }
            }
        }
    );
}

void Hotel::Menu::listReservations() const noexcept
{
    printList(
        manager.getReservations(),
        "There are no reservations yet",
        [this](const Reservation& reservation) noexcept -> void
        {
            printReservation(reservation);
        }
    );
}

void Hotel::Menu::makeReservation() noexcept
{
    const Date today = manager.getToday();

    const auto checkInDate = read<Date>(
        "Arrival date (YYYY-MM-DD)",
        [&](const char* str, Date& out) -> std::string
        {
            CHECK(Date::parse(str, out));
            return out < today
                ? Result(Result::DateInThePast).toString()
                : "";
        }
    );
    if (!checkInDate)
        return;

    const auto checkOutDate = readEndDate("Departure date", *checkInDate, false, true);
    if (!checkOutDate)
        return;

    const auto guestCount = readInteger("Number of guests", 1, 6);
    if (!guestCount)
        return;

    const auto rooms = manager.findAvailableRooms(*checkInDate, *checkOutDate, static_cast<uint32_t>(*guestCount));
    if (rooms.empty())
    {
        printWarning(std::format("No room is free for {} guests in this period", *guestCount).c_str());
        return;
    }

    // Show the price of the whole stay for every room
    const auto nights = static_cast<uint32_t>(checkInDate->daysUntil(*checkOutDate));
    std::cout << "Free rooms for " << nights << " nights:\n";
    for (const auto* room : rooms)
    {
        const Cents total = Invoice::estimateTotal(*room, nights, room->getPricePerNight());
        std::cout << "  " << *room << "\n      stay total: " << formatMoney(total) << " including VAT\n";
    }

    const auto number = readRoomNumber(
        "Room number",
        [&](const uint32_t num) -> std::string
        {
            const bool bFound = std::ranges::any_of(
                rooms,
                [&](const Room* r) noexcept -> bool
                {
                    return r->getNumber() == num;
                }
            );
            return bFound ? "" : std::format("Room {} is not in the list of free rooms", num);
        }
    );
    if (!number)
        return;

    uint64_t guestID = 0;
    while (guestID == 0)
    {
        const auto input = readText(
            std::format(
                R"(Guest ID, "s" to search guests or "n" to register a new guest{})",
                CANCEL_HINT
            ).c_str(),
            false
        );
        if (!input)
            return;

        if (*input == "s" || *input == "S")
            searchGuests();
        else if (*input == "n" || *input == "N")
            guestID = addGuest();
        else if (const auto parsed = parseNumber<uint64_t>(*input); !parsed || *parsed == 0 || manager.findGuest(*parsed) == nullptr)
        {
            guestID = 0;
            printError(Result(Result::GuestNotFound).toString());
        }
        else
            guestID = *parsed;
        if (bEOF)
            return;
    }

    uint64_t id = 0;
    const Result result = manager.makeReservation(
        *number,
        guestID,
        *checkInDate,
        *checkOutDate,
        static_cast<uint32_t>(*guestCount),
        id
    );
    if (printResult(result, std::format("Reservation #{} was created", id).c_str()))
        printReservation(*manager.findReservation(id));
}

void Hotel::Menu::cancelReservation() noexcept
{
    if (listReservationsWithStatus(ReservationStatus::Booked) == 0)
        return;

    const auto id = readID("Reservation ID");
    if (!id)
        return;

    if (confirm(std::format("Cancel reservation #{}?", *id).c_str()))
        printResult(manager.cancelReservation(*id), "The reservation was cancelled");
}

void Hotel::Menu::checkIn() noexcept
{
    if (listReservationsWithStatus(ReservationStatus::Booked) == 0)
        return;

    if (const auto id = readID("Reservation ID"))
        printResult(manager.checkIn(*id), "The guest was checked in");
}

void Hotel::Menu::checkOut() noexcept
{
    if (listReservationsWithStatus(ReservationStatus::CheckedIn) == 0)
        return;

    const auto id = readID("Reservation ID");
    if (!id)
        return;

    uint64_t invoiceID = 0;
    if (printResult(manager.checkOut(*id, invoiceID), "The guest was checked out"))
        std::cout << *manager.findInvoice(invoiceID) << "\n";
}

size_t Hotel::Menu::listReservationsWithStatus(const ReservationStatus status) const noexcept
{
    size_t count = 0;
    for (const auto& reservation : manager.getReservations())
    {
        if (reservation.getStatus() == status)
        {
            printReservation(reservation);
            ++count;
        }
    }

    if (count == 0)
        printWarning(std::format("There are no reservations with the status \"{}\"", status.toString()).c_str());
    return count;
}

void Hotel::Menu::printReservation(const Reservation& reservation) const noexcept
{
    const Guest* guest = manager.findGuest(reservation.getGuestID());
    std::cout << "  " << reservation << (guest != nullptr ? " - " + guest->getFullName() : "") << "\n";
}
