#include "Reservation.hpp"
#include <algorithm>

Hotel::Result Hotel::Reservation::validate() const noexcept
{
    if (id == 0 || guestID == 0)
        return Result::InvalidId;
    if (!(roomNumber >= MIN_ROOM_NUMBER && roomNumber <= MAX_ROOM_NUMBER))
        return Result::InvalidRoomNumber;

    CHECK(validateDates(checkInDate, checkOutDate));

    if (guestCount < 1)
        return Result::InvalidGuestCount;
    if (!(nightlyRate > 0 && nightlyRate <= MAX_AMOUNT))
        return Result::InvalidAmount;
    if (!status.isValid())
        return Result::InvalidStatus;
    return Result::Ok;
}

Hotel::Result Hotel::Reservation::validateDates(const Date& checkIn, const Date& checkOut) noexcept
{
    const int64_t nights = checkIn.daysUntil(checkOut);
    if (nights < 1)
        return Result::InvalidDateRange;
    if (nights > 365)
        return Result::StayTooLong;
    return Result::Ok;
}

uint64_t Hotel::Reservation::getID() const noexcept
{
    return id;
}

Hotel::Result Hotel::Reservation::setID(const uint64_t value) noexcept
{
    if (value == 0)
        return Result::InvalidId;
    id = value;
    return Result::Ok;
}

uint32_t Hotel::Reservation::getRoomNumber() const noexcept
{
    return roomNumber;
}

Hotel::Result Hotel::Reservation::setRoomNumber(const uint32_t number) noexcept
{
    if (!(number >= MIN_ROOM_NUMBER && number <= MAX_ROOM_NUMBER))
        return Result::InvalidRoomNumber;
    roomNumber = number;
    return Result::Ok;
}

uint64_t Hotel::Reservation::getGuestID() const noexcept
{
    return guestID;
}

Hotel::Result Hotel::Reservation::setGuestID(const uint64_t value) noexcept
{
    if (value == 0)
        return Result::InvalidId;
    guestID = value;

    return Result::Ok;
}

const Hotel::Date& Hotel::Reservation::getCheckIn() const noexcept
{
    return checkInDate;
}

const Hotel::Date& Hotel::Reservation::getCheckOut() const noexcept
{
    return checkOutDate;
}

Hotel::Result Hotel::Reservation::setDates(const Date& checkIn, const Date& checkOut) noexcept
{
    CHECK(validateDates(checkIn, checkOut));

    checkInDate = checkIn;
    checkOutDate = checkOut;

    return Result::Ok;
}

uint32_t Hotel::Reservation::getNights() const noexcept
{
    const int64_t nights = checkInDate.daysUntil(checkOutDate);
    return nights > 0 ? static_cast<uint32_t>(nights) : 0;
}

uint32_t Hotel::Reservation::getGuestCount() const noexcept
{
    return guestCount;
}

Hotel::Result Hotel::Reservation::setGuestCount(const uint32_t count) noexcept
{
    if (count < 1)
        return Result::InvalidGuestCount;
    guestCount = count;

    return Result::Ok;
}

Hotel::Cents Hotel::Reservation::getNightlyRate() const noexcept
{
    return nightlyRate;
}

Hotel::Result Hotel::Reservation::setNightlyRate(const Cents rate) noexcept
{
    if (!(rate > 0 && rate <= MAX_AMOUNT))
        return Result::InvalidAmount;
    nightlyRate = rate;

    return Result::Ok;
}

Hotel::ReservationStatus Hotel::Reservation::getStatus() const noexcept
{
    return status;
}

Hotel::Result Hotel::Reservation::setStatus(const ReservationStatus value) noexcept
{
    if (!value.isValid())
        return Result::InvalidStatus;
    status = value;
    return Result::Ok;
}

const Hotel::Date& Hotel::Reservation::getCreatedOn() const noexcept
{
    return createdOn;
}

Hotel::Reservation& Hotel::Reservation::setCreatedOn(const Date& date) noexcept
{
    createdOn = date;
    return *this;
}

Hotel::Result Hotel::Reservation::checkIn() noexcept
{
    if (status != ReservationStatus::Booked)
        return Result::InvalidStatusTransition;
    status = ReservationStatus::CheckedIn;

    return Result::Ok;
}

Hotel::Result Hotel::Reservation::checkOut() noexcept
{
    if (status != ReservationStatus::CheckedIn)
        return Result::InvalidStatusTransition;
    status = ReservationStatus::CheckedOut;

    return Result::Ok;
}

Hotel::Result Hotel::Reservation::cancel() noexcept
{
    if (status != ReservationStatus::Booked)
        return Result::InvalidStatusTransition;
    status = ReservationStatus::Cancelled;

    return Result::Ok;
}

bool Hotel::Reservation::overlaps(const Date& from, const Date& to) const noexcept
{
    return checkInDate < to && from < checkOutDate;
}

uint32_t Hotel::Reservation::nightsWithin(const Date& from, const Date& to) const noexcept
{
    const Date start = std::max(checkInDate, from);
    const Date end = std::min(checkOutDate, to);
    const int64_t nights = start.daysUntil(end);

    return nights > 0 ? static_cast<uint32_t>(nights) : 0;
}

bool Hotel::Reservation::isBlockingRoom() const noexcept
{
    return status != ReservationStatus::Cancelled;
}

bool Hotel::Reservation::isActive() const noexcept
{
    return status == ReservationStatus::Booked || status == ReservationStatus::CheckedIn;
}

bool Hotel::Reservation::isArrivingOn(const Date& date) const noexcept
{
    return status == ReservationStatus::Booked && checkInDate == date;
}

std::ostream& Hotel::operator<<(std::ostream& stream, const Reservation& reservation) noexcept
{
    stream << "Reservation #" << reservation.getID() << ": room " << reservation.getRoomNumber()
           << ", guest #" << reservation.getGuestID()
           << ", " << reservation.getCheckIn() << " -> " << reservation.getCheckOut()
           << " (" << reservation.getNights() << " nights)"
           << ", " << reservation.getGuestCount() << " guests"
           << ", " << formatMoney(reservation.getNightlyRate()) << "/night"
           << ", " << reservation.getStatus().toString();
    return stream;
}
