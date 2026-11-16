#pragma once
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include "Guest.hpp"
#include "Invoice.hpp"
#include "Reservation.hpp"
#include "Room.hpp"

namespace Hotel
{
    class HotelManager
    {
    public:
        HotelManager() noexcept = default;
        HotelManager(const HotelManager&) = delete;
        HotelManager& operator=(const HotelManager&) = delete;

        [[nodiscard]] const std::filesystem::path& getDataDirectory() const noexcept;
        HotelManager& setDataDirectory(const std::filesystem::path& directory) noexcept;

        Result load(std::vector<std::string>& warnings) noexcept;
        [[nodiscard]] Result save() const noexcept;

        void clear() noexcept;

        [[nodiscard]] Date getToday() const noexcept;
        HotelManager& setToday(const Date& date) noexcept;
        HotelManager& useSystemDate() noexcept;

        Result addRoom(std::unique_ptr<Room> room) noexcept;
        Result updateRoom(uint32_t number, Cents pricePerNight, const char* description, bool bOutOfService) noexcept;
        Result removeRoom(uint32_t number) noexcept;

        [[nodiscard]] const Room* findRoom(uint32_t number) const noexcept;
        [[nodiscard]] const std::vector<std::unique_ptr<Room>>& getRooms() const noexcept;

        [[nodiscard]] Result checkAvailability(uint32_t number, const Date& from, const Date& to, uint64_t ignoredReservation = 0) const noexcept;
        [[nodiscard]] bool isRoomAvailable(uint32_t number, const Date& from, const Date& to) const noexcept;

        [[nodiscard]] std::vector<const Room*> findAvailableRooms(const Date& from, const Date& to, uint32_t guestCount, std::optional<RoomType> type = std::nullopt) const noexcept;

        Result addGuest(Guest guest, uint64_t& id) noexcept;
        Result updateGuest(const Guest& guest) noexcept;
        Result removeGuest(uint64_t id) noexcept;

        [[nodiscard]] const Guest* findGuest(uint64_t id) const noexcept;
        [[nodiscard]] std::vector<const Guest*> searchGuests(const char* query) const noexcept;
        [[nodiscard]] const std::vector<Guest>& getGuests() const noexcept;

        Result makeReservation(uint32_t roomNumber, uint64_t guestID, const Date& checkIn, const Date& checkOut, uint32_t guestCount, uint64_t& id) noexcept;
        Result cancelReservation(uint64_t id) noexcept;

        Result checkIn(uint64_t id) noexcept;
        Result checkOut(uint64_t id, uint64_t& invoiceID) noexcept;

        [[nodiscard]] const Reservation* findReservation(uint64_t id) const noexcept;
        [[nodiscard]] const std::vector<Reservation>& getReservations() const noexcept;
        [[nodiscard]] std::vector<const Reservation*> getGuestReservations(uint64_t guestID) const noexcept;

        [[nodiscard]] const Invoice* findInvoice(uint64_t id) const noexcept;
        [[nodiscard]] const Invoice* findInvoiceForReservation(uint64_t reservationID) const noexcept;
        [[nodiscard]] const std::vector<Invoice>& getInvoices() const noexcept;

        Result importRoom(std::unique_ptr<Room> room) noexcept;
        Result importGuest(const Guest& guest) noexcept;
        Result importReservation(const Reservation& reservation) noexcept;
        Result importInvoice(const Invoice& invoice) noexcept;
    private:
        std::vector<std::unique_ptr<Room>> rooms;
        std::vector<Guest> guests;
        std::vector<Reservation> reservations;
        std::vector<Invoice> invoices;

        uint64_t nextGuestID = 1;
        uint64_t nextReservationID = 1;
        uint64_t nextInvoiceID = 1;

        std::filesystem::path dataDirectory;
        std::optional<Date> fixedToday;

        Guest* findGuestMutable(uint64_t id) noexcept;
        Reservation* findReservationMutable(uint64_t id) noexcept;

        [[nodiscard]] bool hasConflictingBooking(uint32_t roomNumber, const Date& from, const Date& to, uint64_t ignoredReservation = 0) const noexcept;

        [[nodiscard]] bool isIDDocumentTaken(const char* document, uint64_t ignoredID) const noexcept;
    };
}
