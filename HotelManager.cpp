#include "HotelManager.hpp"
#include "Storage.hpp"
#include <algorithm>
#include <utility>

static constexpr auto roomNumberOf = [](const std::unique_ptr<Hotel::Room>& room) noexcept
{
    return room->getNumber();
};

template<typename Range, typename Key, typename Proj>
[[nodiscard]] static auto findByKey(Range& range, const Key& key, Proj proj) noexcept
{
    const auto it = std::ranges::find(range, key, proj);
    return it != std::ranges::end(range) ? &(*it) : nullptr;
}

const std::filesystem::path& Hotel::HotelManager::getDataDirectory() const noexcept
{
    return dataDirectory;
}

Hotel::HotelManager& Hotel::HotelManager::setDataDirectory(const std::filesystem::path& directory) noexcept
{
    dataDirectory = directory;
    return *this;
}

Hotel::Result Hotel::HotelManager::load(std::vector<std::string>& warnings) noexcept
{
    return Storage::load(dataDirectory, *this, warnings);
}

Hotel::Result Hotel::HotelManager::save() const noexcept
{
    return Storage::save(dataDirectory, *this);
}

void Hotel::HotelManager::clear() noexcept
{
    rooms.clear();
    guests.clear();
    reservations.clear();
    invoices.clear();
    nextGuestID = 1;
    nextReservationID = 1;
    nextInvoiceID = 1;
}

Hotel::Date Hotel::HotelManager::getToday() const noexcept
{
    return fixedToday.value_or(Date::today());
}

Hotel::HotelManager& Hotel::HotelManager::setToday(const Date& date) noexcept
{
    fixedToday = date;
    return *this;
}

Hotel::HotelManager& Hotel::HotelManager::useSystemDate() noexcept
{
    fixedToday.reset();
    return *this;
}

Hotel::Result Hotel::HotelManager::addRoom(std::unique_ptr<Room> room) noexcept
{
    CHECK(importRoom(std::move(room)));
    return save();
}

Hotel::Result Hotel::HotelManager::updateRoom(const uint32_t number, const Cents pricePerNight,
                                                const char* description, const bool bOutOfService) noexcept
{
    const auto it = std::ranges::find(rooms, number, roomNumberOf);
    if (it == rooms.end())
        return Result::RoomNotFound;

    // Validate on a copy first, so a failure halfway through does not leave the room partially modified
    auto copy = (*it)->clone();

    CHECK(copy->setPricePerNight(pricePerNight));
    CHECK(copy->setDescription(description));
    copy->setOutOfService(bOutOfService);

    *it = std::move(copy);
    return save();
}

Hotel::Result Hotel::HotelManager::removeRoom(const uint32_t number) noexcept
{
    const auto it = std::ranges::find(rooms, number, roomNumberOf);
    if (it == rooms.end())
        return Result::RoomNotFound;
    // Past reservations and invoices still reference the room, removing it would break the history of the hotel
    if (std::ranges::any_of(reservations, [&](const Reservation& r) noexcept { return r.getRoomNumber() == number; }))
        return Result::RoomHasReservations;

    rooms.erase(it);
    return save();
}

const Hotel::Room* Hotel::HotelManager::findRoom(const uint32_t number) const noexcept
{
    const auto* ptr = findByKey(rooms, number, roomNumberOf);
    return ptr != nullptr ? ptr->get() : nullptr;
}

const std::vector<std::unique_ptr<Hotel::Room>>& Hotel::HotelManager::getRooms() const noexcept
{
    return rooms;
}

Hotel::Result Hotel::HotelManager::checkAvailability(const uint32_t number, const Date& from, const Date& to,
                                                       const uint64_t ignoredReservation) const noexcept
{
    const Room* room = findRoom(number);
    if (room == nullptr)
        return Result::RoomNotFound;
    if (room->isOutOfService())
        return Result::RoomOutOfService;
    if (from >= to)
        return Result::InvalidDateRange;

    return hasConflictingBooking(number, from, to, ignoredReservation) ? Result::RoomNotAvailable : Result::Ok;
}

bool Hotel::HotelManager::isRoomAvailable(const uint32_t number, const Date& from, const Date& to) const noexcept
{
    return checkAvailability(number, from, to) == Result::Ok;
}

std::vector<const Hotel::Room*> Hotel::HotelManager::findAvailableRooms(const Date& from, const Date& to, const uint32_t guestCount, const std::optional<RoomType> type) const noexcept
{
    std::vector<const Room*> result;
    for (const auto& room : rooms)
        if ((!type.has_value() || room->getType() == *type) && room->getCapacity() >= guestCount && isRoomAvailable(room->getNumber(), from, to))
            result.push_back(room.get());
    return result;
}

Hotel::Result Hotel::HotelManager::addGuest(Guest guest, uint64_t& id) noexcept
{
    guest.setID(nextGuestID);
    CHECK(importGuest(guest));
    id = guest.getID();
    return save();
}

Hotel::Result Hotel::HotelManager::updateGuest(const Guest& guest) noexcept
{
    Guest* existing = findGuestMutable(guest.getID());
    if (existing == nullptr)
        return Result::GuestNotFound;

    CHECK(guest.validate());
    if (isIDDocumentTaken(guest.getIDDocument().c_str(), guest.getID()))
        return Result::GuestAlreadyExists;

    *existing = guest;
    return save();
}

Hotel::Result Hotel::HotelManager::removeGuest(const uint64_t id) noexcept
{
    const auto it = std::ranges::find(guests, id, &Guest::getID);
    if (it == guests.end())
        return Result::GuestNotFound;
    if (
        std::ranges::any_of(
            reservations,
            [&](const Reservation& r) noexcept -> bool
            {
                return r.getGuestID() == id;
            }
        )
    )
        return Result::GuestHasReservations;

    guests.erase(it);
    return save();
}

const Hotel::Guest* Hotel::HotelManager::findGuest(const uint64_t id) const noexcept
{
    return findByKey(guests, id, &Guest::getID);
}

std::vector<const Hotel::Guest*> Hotel::HotelManager::searchGuests(const char* query) const noexcept
{
    const std::string needle = toLower(std::string(trim(query)));
    std::vector<const Guest*> result;
    for (const auto& guest : guests)
    {
        const std::string haystack = toLower(guest.getFullName() + "\n" + guest.getIDDocument() + "\n"
            + guest.getPhone() + "\n" + guest.getEmail());
        if (haystack.find(needle) != std::string::npos)
            result.push_back(&guest);
    }
    return result;
}

const std::vector<Hotel::Guest>& Hotel::HotelManager::getGuests() const noexcept
{
    return guests;
}

Hotel::Result Hotel::HotelManager::makeReservation(const uint32_t roomNumber, const uint64_t guestID,
                                                     const Date& checkIn, const Date& checkOut,
                                                     const uint32_t guestCount, uint64_t& id) noexcept
{
    const Room* room = findRoom(roomNumber);
    if (room == nullptr)
        return Result::RoomNotFound;
    if (findGuest(guestID) == nullptr)
        return Result::GuestNotFound;
    if (checkIn < getToday())
        return Result::DateInThePast;

    Reservation reservation;

    if (guestCount > room->getCapacity())
        return Result::CapacityExceeded;

    CHECK(reservation.setDates(checkIn, checkOut));
    CHECK(reservation.setGuestCount(guestCount));
    CHECK(checkAvailability(roomNumber, checkIn, checkOut));

    reservation.setID(nextReservationID);
    reservation.setRoomNumber(roomNumber);
    reservation.setGuestID(guestID);
    reservation.setNightlyRate(room->getPricePerNight());
    reservation.setCreatedOn(getToday());

    CHECK(importReservation(reservation));
    id = reservation.getID();
    return save();
}

Hotel::Result Hotel::HotelManager::cancelReservation(const uint64_t id) noexcept
{
    Reservation* reservation = findReservationMutable(id);
    if (reservation == nullptr)
        return Result::ReservationNotFound;

    CHECK(reservation->cancel());
    return save();
}

Hotel::Result Hotel::HotelManager::checkIn(const uint64_t id) noexcept
{
    Reservation* reservation = findReservationMutable(id);
    if (reservation == nullptr)
        return Result::ReservationNotFound;
    // Check status before the date window so wrong-status errors take precedence over OutsideStayPeriod
    if (reservation->getStatus() != ReservationStatus::Booked)
        return Result::InvalidStatusTransition;

    const Date today = getToday();
    if (today < reservation->getCheckIn() || today >= reservation->getCheckOut())
        return Result::OutsideStayPeriod;

    // The previous guest may have stayed past their departure date without checking out yet
    const bool bRoomOccupied = std::ranges::any_of(
        reservations,
        [&](const Reservation& r) noexcept -> bool
        {
            return r.getRoomNumber() == reservation->getRoomNumber() && r.getStatus() == ReservationStatus::CheckedIn;
        }
    );
    if (bRoomOccupied)
        return Result::RoomOccupied;

    CHECK(reservation->checkIn());
    return save();
}

Hotel::Result Hotel::HotelManager::checkOut(const uint64_t id, uint64_t& invoiceID) noexcept
{
    Reservation* reservation = findReservationMutable(id);
    if (reservation == nullptr)
        return Result::ReservationNotFound;
    const Room* room = findRoom(reservation->getRoomNumber());
    const Guest* guest = findGuest(reservation->getGuestID());
    if (room == nullptr)
        return Result::RoomNotFound;
    if (guest == nullptr)
        return Result::GuestNotFound;

    CHECK(reservation->checkOut());

    // Early departure: bill only the nights that were actually spent and free the room for the remaining ones. A
    // departure on the arrival day is still billed as 1 night, and a late departure is billed as booked
    const Date today = getToday();
    const Date departure = std::max(today, reservation->getCheckIn().addDays(1));
    if (departure < reservation->getCheckOut())
        reservation->setDates(reservation->getCheckIn(), departure);

    invoices.push_back(Invoice::calculate(nextInvoiceID++, *reservation, *room, guest->getFullName().c_str(), today));
    invoiceID = invoices.back().getID();
    return save();
}

const Hotel::Reservation* Hotel::HotelManager::findReservation(const uint64_t id) const noexcept
{
    return findByKey(reservations, id, &Reservation::getID);
}

const std::vector<Hotel::Reservation>& Hotel::HotelManager::getReservations() const noexcept
{
    return reservations;
}

std::vector<const Hotel::Reservation*> Hotel::HotelManager::getGuestReservations(const uint64_t guestID) const noexcept
{
    std::vector<const Reservation*> result;
    for (const auto& reservation : reservations)
        if (reservation.getGuestID() == guestID)
            result.push_back(&reservation);
    return result;
}

const Hotel::Invoice* Hotel::HotelManager::findInvoice(const uint64_t id) const noexcept
{
    return findByKey(invoices, id, &Invoice::getID);
}

const Hotel::Invoice* Hotel::HotelManager::findInvoiceForReservation(const uint64_t reservationID) const noexcept
{
    return findByKey(invoices, reservationID, &Invoice::getReservationID);
}

const std::vector<Hotel::Invoice>& Hotel::HotelManager::getInvoices() const noexcept
{
    return invoices;
}

Hotel::Result Hotel::HotelManager::importRoom(std::unique_ptr<Room> room) noexcept
{
    if (room == nullptr)
        return Result::InvalidRoomType;
    CHECK(room->validate());
    if (findRoom(room->getNumber()) != nullptr)
        return Result::RoomAlreadyExists;

    // Keep the rooms sorted by number, so that every listing and report is in a predictable order
    const auto position = std::ranges::upper_bound(rooms, room->getNumber(), {}, roomNumberOf);
    rooms.insert(position, std::move(room));
    return Result::Ok;
}

Hotel::Result Hotel::HotelManager::importGuest(const Guest& guest) noexcept
{
    CHECK(guest.validate());
    if (findGuest(guest.getID()) != nullptr || isIDDocumentTaken(guest.getIDDocument().c_str(), 0))
        return Result::GuestAlreadyExists;

    guests.push_back(guest);
    nextGuestID = std::max(nextGuestID, guest.getID() + 1);
    return Result::Ok;
}

Hotel::Result Hotel::HotelManager::importReservation(const Reservation& reservation) noexcept
{
    CHECK(reservation.validate());
    if (findReservation(reservation.getID()) != nullptr)
        return Result::ReservationAlreadyExists;

    const Room* room = findRoom(reservation.getRoomNumber());
    if (room == nullptr)
        return Result::RoomNotFound;
    if (findGuest(reservation.getGuestID()) == nullptr)
        return Result::GuestNotFound;
    if (reservation.getGuestCount() > room->getCapacity())
        return Result::CapacityExceeded;

    // Rooms that went out of service after being booked keep their existing bookings, so only check for double
    // bookings here instead of calling checkAvailability
    if (reservation.isBlockingRoom()
        && hasConflictingBooking(reservation.getRoomNumber(), reservation.getCheckIn(), reservation.getCheckOut()))
        return Result::RoomNotAvailable;

    reservations.push_back(reservation);
    nextReservationID = std::max(nextReservationID, reservation.getID() + 1);
    return Result::Ok;
}

Hotel::Result Hotel::HotelManager::importInvoice(const Invoice& invoice) noexcept
{
    CHECK(invoice.validate());
    if (findInvoice(invoice.getID()) != nullptr || findInvoiceForReservation(invoice.getReservationID()) != nullptr)
        return Result::InvoiceAlreadyExists;

    const Reservation* reservation = findReservation(invoice.getReservationID());
    if (reservation == nullptr)
        return Result::ReservationNotFound;
    if (reservation->getStatus() != ReservationStatus::CheckedOut)
        return Result::InvalidStatus;
    if (invoice.getRoomNumber() != reservation->getRoomNumber() || invoice.getNights() != reservation->getNights())
        return Result::InvoiceMismatch;

    // The room cost must match the rate that the reservation was booked at
    const Room* room = findRoom(invoice.getRoomNumber());
    if (room == nullptr)
        return Result::RoomNotFound;
    if (invoice.getRoomCost() != room->calculateStayCost(invoice.getNights(), reservation->getNightlyRate()))
        return Result::InvoiceMismatch;

    invoices.push_back(invoice);
    nextInvoiceID = std::max(nextInvoiceID, invoice.getID() + 1);
    return Result::Ok;
}

Hotel::Guest* Hotel::HotelManager::findGuestMutable(const uint64_t id) noexcept
{
    return const_cast<Guest*>(std::as_const(*this).findGuest(id));
}

Hotel::Reservation* Hotel::HotelManager::findReservationMutable(const uint64_t id) noexcept
{
    return const_cast<Reservation*>(std::as_const(*this).findReservation(id));
}

bool Hotel::HotelManager::hasConflictingBooking(const uint32_t roomNumber, const Date& from, const Date& to,
                                                const uint64_t ignoredReservation) const noexcept
{
    return std::ranges::any_of(
        reservations,
        [&](const Reservation& reservation) noexcept -> bool
        {
            return      reservation.getID() != ignoredReservation
                    &&  reservation.getRoomNumber() == roomNumber
                    &&  reservation.isBlockingRoom()
                    &&  reservation.overlaps(from, to);
        }
    );
}

bool Hotel::HotelManager::isIDDocumentTaken(const char* document, const uint64_t ignoredID) const noexcept
{
    return std::ranges::any_of(
        guests,
        [&](const Guest& guest) noexcept -> bool
        {
            return guest.getID() != ignoredID && guest.getIDDocument() == document;
        }
    );
}
