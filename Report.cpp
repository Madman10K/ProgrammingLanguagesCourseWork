#include "Report.hpp"
#include <array>
#include <format>
#include <iomanip>
#include <sstream>

constexpr size_t REPORT_WIDTH = 78;

[[nodiscard]] static std::string percent(const uint64_t numerator, const uint64_t denominator) noexcept
{
    return std::format(
        "{:.1f}%",
        denominator == 0
            ? 0.0
            : 100.0 * static_cast<double>(numerator) / static_cast<double>(denominator)
    );
}

[[nodiscard]] static std::string fit(const std::string_view str, const size_t width) noexcept
{
    return str.size() <= width
        ? std::string{ str }
        : std::format(
            "{}...",
            str.substr(
                0,
                width - 3
            )
        );
}

static void printAmount(std::ostream& stream, const char* label, const Hotel::Cents amount) noexcept
{
    stream << "  " << std::left << std::setw(40) << label << std::right << std::setw(14) << Hotel::formatMoney(amount) << "\n";
}

Hotel::Report::Report(const HotelManager& hotelManager) noexcept : manager(hotelManager)
{
}

Hotel::Result Hotel::Report::saveToFile(const std::filesystem::path& directory, std::filesystem::path& path) const noexcept
{
    CHECK(ensureDirectory(directory));

    const std::filesystem::path file = directory / (getTitle() + ".txt");
    std::ostringstream buffer;
    generate(buffer);

    CHECK(writeTextFile(file, buffer.str().c_str()));
    path = file;
    return Result::Ok;
}

void Hotel::Report::printHeading(std::ostream& stream, const char* heading) noexcept
{
    printRule(stream, '=', REPORT_WIDTH);
    stream << " " << heading << "\n";
    printRule(stream, '=', REPORT_WIDTH);
}

Hotel::PeriodReport::PeriodReport(const HotelManager& hotelManager, const Date& from, const Date& to) noexcept
    : Report(hotelManager), from(from), to(to)
{
}

Hotel::Date Hotel::PeriodReport::getEndExclusive() const noexcept
{
    return to.addDays(1);
}

std::string Hotel::PeriodReport::formatPeriodTitle(const char* prefix) const noexcept
{
    return std::format("{}-{}-{}", prefix, from.toString(), to.toString());
}

void Hotel::OccupancyReport::generate(std::ostream& stream) const noexcept
{
    const Date end = getEndExclusive();
    const uint64_t periodNights = from < end ? static_cast<uint64_t>(from.daysUntil(end)) : 0;
    const Date today = manager.getToday();

    printHeading(
        stream,
        std::format(
            "Occupancy report: {} to {} ({} nights)",
            from.toString(),
            to.toString(),
            periodNights
        ).c_str()
    );

    stream << std::left << std::setw(7) << "Room" << std::setw(10) << "Type" << std::right << std::setw(14) << "Booked nights"
           << std::setw(12) << "Occupancy" << "   " << "Status today" << "\n";
    printRule(stream, '-', REPORT_WIDTH);

    uint64_t totalBooked = 0;
    size_t outOfService = 0;
    for (const auto& room : manager.getRooms())
    {
        uint64_t booked = 0;
        std::string status = room->isOutOfService() ? "Out of service" : "Free";
        bool bOccupied = false;
        for (const auto& reservation : manager.getReservations())
        {
            if (reservation.getRoomNumber() != room->getNumber() || !reservation.isBlockingRoom())
                continue;
            booked += reservation.nightsWithin(from, end);

            // A guest who is currently in the room takes precedence over an arrival that is expected today
            if (reservation.getStatus() == ReservationStatus::CheckedIn)
            {
                const Guest* guest = manager.findGuest(reservation.getGuestID());
                status = guest != nullptr ? "Occupied by " + guest->getFullName() : "Occupied";
                bOccupied = true;
            }
            else if (!bOccupied && reservation.isArrivingOn(today))
                status = "Arrival expected today";
        }
        totalBooked += booked;
        if (room->isOutOfService())
            ++outOfService;

        stream << std::left << std::setw(7) << room->getNumber() << std::setw(10) << room->getTypeName() << std::right
               << std::setw(14) << std::format("{}/{}", booked, periodNights)
               << std::setw(12) << percent(booked, periodNights) << "   " << fit(status, 32) << "\n";
    }

    const uint64_t available = periodNights * manager.getRooms().size();
    size_t occupied = 0;
    size_t arrivals = 0;
    size_t departures = 0;
    for (const auto& reservation : manager.getReservations())
    {
        if (reservation.getStatus() == ReservationStatus::CheckedIn)
        {
            ++occupied;
            if (reservation.getCheckOut() <= today)
                ++departures;
        }
        else if (reservation.isArrivingOn(today))
            ++arrivals;
    }

    printRule(stream, '-', REPORT_WIDTH);
    stream << "Total: " << totalBooked << " of " << available << " room-nights booked, occupancy "
           << percent(totalBooked, available) << "\n"
           << "Rooms: " << manager.getRooms().size() << " (" << outOfService << " out of service)\n"
           << "Today (" << today << "): " << occupied << " occupied, " << arrivals << " expected arrivals, "
           << departures << " expected departures\n";
}

std::string Hotel::OccupancyReport::getTitle() const noexcept
{
    return formatPeriodTitle("occupancy");
}

struct Totals
{
    size_t invoices = 0;
    uint64_t nights = 0;
    Hotel::Cents roomCost = 0;
    Hotel::Cents tax = 0;
    Hotel::Cents total = 0;

    void add(const Hotel::Invoice& invoice) noexcept
    {
        ++invoices;
        nights += invoice.getNights();
        roomCost += invoice.getRoomCost();
        tax += invoice.getTax();
        total += invoice.getTotal();
    }
};

void Hotel::FinancialReport::generate(std::ostream& stream) const noexcept
{
    Totals totals;
    std::array<Totals, static_cast<size_t>(RoomType::Count)> byType{};

    printHeading(
        stream,
        std::format(
            "Financial report: {} to {}",
            from.toString(),
            to.toString()
        ).c_str()
    );

    stream << std::left << std::setw(8) << "Invoice" << std::setw(12) << "Date" << std::setw(6) << "Room"
           << std::setw(26) << "Guest" << std::right << std::setw(7) << "Nights" << std::setw(14) << "Total" << "\n";
    printRule(stream, '-', REPORT_WIDTH);

    for (const auto& invoice : manager.getInvoices())
    {
        if (invoice.getIssueDate() < from || invoice.getIssueDate() > to)
            continue;
        totals.add(invoice);
        if (const Room* room = manager.findRoom(invoice.getRoomNumber()); room != nullptr)
            byType[static_cast<size_t>(room->getType())].add(invoice);

        stream << std::left << std::setw(8) << std::format("#{}", invoice.getID())
               << std::setw(12) << invoice.getIssueDate().toString()
               << std::setw(6) << invoice.getRoomNumber()
               << std::setw(26) << fit(invoice.getBilledTo(), 25) << std::right
               << std::setw(7) << invoice.getNights()
               << std::setw(14) << formatMoney(invoice.getTotal()) << "\n";
    }
    if (totals.invoices == 0)
        stream << "  No invoices were issued in this period\n";

    const Cents netRoomRevenue = totals.roomCost;
    printRule(stream, '-', REPORT_WIDTH);
    stream << "Totals (" << totals.invoices << " invoices, " << totals.nights << " nights):\n";

    printAmount(stream, "Net revenue (excluding VAT)", netRoomRevenue);
    printAmount(stream, "VAT collected", totals.tax);
    printAmount(stream, "Total invoiced", totals.total);
    printAmount(stream, "Average daily rate (net room/night)", totals.nights == 0 ? 0 : netRoomRevenue / static_cast<Cents>(totals.nights));

    stream << "\nBy room type:\n"
           << "  " << std::left << std::setw(10) << "Type" << std::right << std::setw(10) << "Invoices" << std::setw(8) << "Nights"
           << std::setw(18) << "Net revenue" << std::setw(10) << "Share" << "\n";

    for (size_t i = 0; i < byType.size(); ++i)
    {
        const Totals& t = byType[i];
        const Cents net = t.roomCost;
        stream << "  " << std::left << std::setw(10) << static_cast<RoomType>(i).toString() << std::right
               << std::setw(10) << t.invoices << std::setw(8) << t.nights << std::setw(18) << formatMoney(net)
               << std::setw(10) << percent(static_cast<uint64_t>(net), static_cast<uint64_t>(netRoomRevenue)) << "\n";
    }

    // Stays in the period that are still running or have not started yet, valued at their booked rate
    const Date end = getEndExclusive();

    size_t pending = 0;
    uint64_t pendingNights = 0;

    Cents expected = 0;
    for (const auto& reservation : manager.getReservations())
    {
        if (!reservation.isActive() || !reservation.overlaps(from, end))
            continue;

        const uint32_t nights = reservation.nightsWithin(from, end);
        ++pending;
        pendingNights += nights;

        const Room* room = manager.findRoom(reservation.getRoomNumber());
        expected += room != nullptr
            ? room->calculateStayCost(nights, reservation.getNightlyRate())
            : static_cast<Cents>(nights) * reservation.getNightlyRate();
    }
    stream << "\nNot invoiced yet: " << pending << " active stays, "
           << pendingNights << " nights in the period\n";
    printAmount(stream, "Expected revenue (excluding VAT)", expected);
}

std::string Hotel::FinancialReport::getTitle() const noexcept
{
    return formatPeriodTitle("financial");
}

Hotel::GuestReport::GuestReport(const HotelManager& hotelManager, const uint64_t id) noexcept
    : Report(hotelManager), guestID(id)
{
}

void Hotel::GuestReport::generate(std::ostream& stream) const noexcept
{
    if (guestID == 0)
    {
        generateSummary(stream);
        return;
    }

    const Guest* guest = manager.findGuest(guestID);
    if (guest == nullptr)
    {
        printHeading(stream, "Guest report");
        stream << "Guest #" << guestID << " does not exist\n";
        return;
    }
    generateDetails(stream, *guest);
}

std::string Hotel::GuestReport::getTitle() const noexcept
{
    return guestID == 0
        ? std::format("guests-all-{}", manager.getToday().toString())
        : std::format("guest-{}-{}", guestID, manager.getToday().toString());
}

Hotel::GuestReport::GuestStats Hotel::GuestReport::computeGuestStats(const uint64_t id) const noexcept
{
    GuestStats stats{};
    for (const auto* reservation : manager.getGuestReservations(id))
    {
        if (reservation->getStatus() == ReservationStatus::CheckedOut)
        {
            ++stats.stays;
            if (const Invoice* invoice = manager.findInvoiceForReservation(reservation->getID()); invoice != nullptr)
            {
                stats.nights += invoice->getNights();
                stats.spent += invoice->getTotal();
            }
            else
                stats.nights += reservation->getNights();
        }
        else if (reservation->isActive())
            ++stats.upcoming;
    }
    return stats;
}

void Hotel::GuestReport::generateSummary(std::ostream& stream) const noexcept
{
    printHeading(stream, std::format("Guest report: all guests ({})", manager.getGuests().size()).c_str());
    stream << std::left << std::setw(6) << "ID" << std::setw(28) << "Name" << std::right << std::setw(7) << "Stays"
           << std::setw(8) << "Nights" << std::setw(10) << "Upcoming" << std::setw(16) << "Total spent" << "\n";
    printRule(stream, '-', REPORT_WIDTH);

    Cents grandTotal = 0;
    for (const auto& guest : manager.getGuests())
    {
        const GuestStats stats = computeGuestStats(guest.getID());
        grandTotal += stats.spent;

        stream << std::left << std::setw(6) << guest.getID() << std::setw(28) << fit(guest.getFullName(), 27) << std::right
               << std::setw(7) << stats.stays << std::setw(8) << stats.nights << std::setw(10) << stats.upcoming
               << std::setw(16) << formatMoney(stats.spent) << "\n";
    }
    printRule(stream, '-', REPORT_WIDTH);
    stream << std::left << std::setw(59) << "Total spent by all guests" << std::right << std::setw(16) << formatMoney(grandTotal) << "\n";
}

void Hotel::GuestReport::generateDetails(std::ostream& stream, const Guest& guest) const noexcept
{
    printHeading(stream, ("Guest report: " + guest.getFullName()).c_str());
    stream << "Guest ID:     " << guest.getID() << "\n"
           << "ID document:  " << guest.getIDDocument() << "\n"
           << "Phone:        " << guest.getPhone() << "\n"
           << "E-mail:       " << guest.getEmail() << "\n\n";

    const auto reservations = manager.getGuestReservations(guest.getID());
    stream << "Reservations (" << reservations.size() << "):\n";
    if (reservations.empty())
        stream << "  None\n";

    for (const auto* reservation : reservations)
    {
        stream << "  " << *reservation << "\n";

        if (const Invoice* invoice = manager.findInvoiceForReservation(reservation->getID()); invoice != nullptr)
        {
            stream << "      Invoice #" << invoice->getID() << " issued " << invoice->getIssueDate()
                   << ", total " << formatMoney(invoice->getTotal()) << "\n";
        }
    }

    const GuestStats stats = computeGuestStats(guest.getID());
    stream << "\nCompleted stays: " << stats.nights << " nights"
           << ", total spent: " << formatMoney(stats.spent) << "\n";
}

std::ostream& Hotel::operator<<(std::ostream& stream, const Report& report) noexcept
{
    report.generate(stream);
    return stream;
}
