#include "Invoice.hpp"

#include <cstring>
#include <format>
#include <iomanip>

Hotel::Invoice Hotel::Invoice::calculate(const uint64_t id, const Reservation& reservation, const Room& room, const char* billedTo, const Date& issueDate) noexcept
{
    Invoice invoice{};
    invoice.id = id;
    invoice.reservationID = reservation.getID();
    invoice.roomNumber = reservation.getRoomNumber();
    invoice.billedTo = billedTo;
    invoice.issueDate = issueDate;

    invoice.nights = reservation.getNights();
    invoice.roomCost = room.calculateStayCost(invoice.nights, reservation.getNightlyRate());
    invoice.tax = calculateTax(invoice.roomCost);
    return invoice;
}

Hotel::Cents Hotel::Invoice::estimateTotal(const Room& room, const uint32_t nights, const Cents nightlyRate) noexcept
{
    const Cents cost = room.calculateStayCost(nights, nightlyRate);
    return cost + calculateTax(cost);
}

Hotel::Result Hotel::Invoice::validateAmounts(const uint32_t nightCount, const Cents room, const Cents taxAmount) noexcept
{
    if (nightCount < 1)
        return Result::InvalidDateRange;
    if (nightCount > 365)
        return Result::StayTooLong;

    // A single invoice can at most cover 365 nights at the maximum rate, everything above that limit
    // is corrupt data. This also guarantees that the tax calculation below cannot overflow
    if (room <= 0 || taxAmount < 0 || room > MAX_AMOUNT * (365 + 1) * 10)
        return Result::InvalidAmount;

    if (taxAmount != calculateTax(room))
        return Result::InvoiceMismatch;

    return Result::Ok;
}

Hotel::Result Hotel::Invoice::validate() const noexcept
{
    if (id == 0 || reservationID == 0)
        return Result::InvalidId;
    if (!(roomNumber >= MIN_ROOM_NUMBER && roomNumber <= MAX_ROOM_NUMBER))
        return Result::InvalidRoomNumber;
    if (billedTo.empty() || !isValidText(billedTo, MAX_FULL_NAME_LENGTH))
        return Result::InvalidName;
    return validateAmounts(nights, roomCost, tax);
}

uint64_t Hotel::Invoice::getID() const noexcept
{
    return id;
}

Hotel::Result Hotel::Invoice::setID(const uint64_t value) noexcept
{
    if (value == 0)
        return Result::InvalidId;
    id = value;
    return Result::Ok;
}

uint64_t Hotel::Invoice::getReservationID() const noexcept
{
    return reservationID;
}

Hotel::Result Hotel::Invoice::setReservationID(const uint64_t value) noexcept
{
    if (value == 0)
        return Result::InvalidId;
    reservationID = value;
    return Result::Ok;
}

uint32_t Hotel::Invoice::getRoomNumber() const noexcept
{
    return roomNumber;
}

Hotel::Result Hotel::Invoice::setRoomNumber(const uint32_t number) noexcept
{
    if (!(number >= MIN_ROOM_NUMBER && number <= MAX_ROOM_NUMBER))
        return Result::InvalidRoomNumber;
    roomNumber = number;
    return Result::Ok;
}

const std::string& Hotel::Invoice::getBilledTo() const noexcept
{
    return billedTo;
}

Hotel::Result Hotel::Invoice::setBilledTo(const char* name) noexcept
{
    if ((name == nullptr || std::strlen(name) == 0) || !isValidText(name, MAX_FULL_NAME_LENGTH))
        return Result::InvalidName;
    billedTo = name;
    return Result::Ok;
}

const Hotel::Date& Hotel::Invoice::getIssueDate() const noexcept
{
    return issueDate;
}

Hotel::Invoice& Hotel::Invoice::setIssueDate(const Date& date) noexcept
{
    issueDate = date;
    return *this;
}

Hotel::Result Hotel::Invoice::setAmounts(const uint32_t nightCount, const Cents room, const Cents taxAmount) noexcept
{
    CHECK(validateAmounts(nightCount, room, taxAmount));

    nights = nightCount;
    roomCost = room;
    tax = taxAmount;
    return Result::Ok;
}

uint32_t Hotel::Invoice::getNights() const noexcept
{
    return nights;
}

Hotel::Cents Hotel::Invoice::getRoomCost() const noexcept
{
    return roomCost;
}

Hotel::Cents Hotel::Invoice::getTax() const noexcept
{
    return tax;
}

Hotel::Cents Hotel::Invoice::getTotal() const noexcept
{
    return roomCost + tax;
}

std::ostream& Hotel::operator<<(std::ostream& stream, const Invoice& invoice) noexcept
{
    constexpr size_t width = 46;
    const auto line = [&](const char* label, const Cents amount) -> void
    {
        const std::string value = formatMoney(amount);
        stream << std::left << std::setw(static_cast<int>(width - value.size())) << label << value << "\n";
    };

    const std::string title = std::format(" INVOICE #{} ", invoice.getID());
    const size_t padding = (width - title.size()) / 2;
    stream << std::string(padding, '=') << title << std::string(width - padding - title.size(), '=') << "\n"
           << "Issued:      " << invoice.getIssueDate() << "\n"
           << "Billed to:   " << invoice.getBilledTo() << "\n"
           << "Reservation: #" << invoice.getReservationID() << ", room " << invoice.getRoomNumber() << "\n";
    printRule(stream, '-', width);

    line(std::format("Room ({} nights)", invoice.getNights()).c_str(), invoice.getRoomCost());
    line(std::format("VAT {}%", TAX_RATE_BASIS_POINTS / 100).c_str(), invoice.getTax());

    printRule(stream, '-', width);
    line("TOTAL", invoice.getTotal());
    stream << std::string(width, '=') << std::right;
    return stream;
}
