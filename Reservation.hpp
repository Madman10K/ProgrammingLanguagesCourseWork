#pragma once
#include <ostream>
#include "Date.hpp"

namespace Hotel
{
    class Reservation
    {
    public:
        Reservation() noexcept = default;

        [[nodiscard]] Result validate() const noexcept;
        [[nodiscard]] static Result validateDates(const Date& checkIn, const Date& checkOut) noexcept;

        [[nodiscard]] uint64_t getID() const noexcept;
        // CAUTION: IDs are assigned by the HotelManager, this is only meant to be used when loading data
        Result setID(uint64_t value) noexcept;

        [[nodiscard]] uint32_t getRoomNumber() const noexcept;
        Result setRoomNumber(uint32_t number) noexcept;

        [[nodiscard]] uint64_t getGuestID() const noexcept;
        Result setGuestID(uint64_t value) noexcept;

        [[nodiscard]] const Date& getCheckIn() const noexcept;
        [[nodiscard]] const Date& getCheckOut() const noexcept;
        // Sets both dates at once, because each one is only valid relative to the other
        Result setDates(const Date& checkIn, const Date& checkOut) noexcept;

        // Number of nights in the stay
        [[nodiscard]] uint32_t getNights() const noexcept;

        [[nodiscard]] uint32_t getGuestCount() const noexcept;
        Result setGuestCount(uint32_t count) noexcept;

        [[nodiscard]] Cents getNightlyRate() const noexcept;
        Result setNightlyRate(Cents rate) noexcept;

        [[nodiscard]] ReservationStatus getStatus() const noexcept;
        // CAUTION: Bypasses the status transition rules, only meant to be used when loading data
        Result setStatus(ReservationStatus value) noexcept;

        [[nodiscard]] const Date& getCreatedOn() const noexcept;
        Reservation& setCreatedOn(const Date& date) noexcept;

        Result checkIn() noexcept;
        Result checkOut() noexcept;
        Result cancel() noexcept;

        [[nodiscard]] bool overlaps(const Date& from, const Date& to) const noexcept;
        [[nodiscard]] uint32_t nightsWithin(const Date& from, const Date& to) const noexcept;

        // Whether the reservation still occupies its room, i.e. it is not cancelled
        [[nodiscard]] bool isBlockingRoom() const noexcept;
        // Booked or checked in
        [[nodiscard]] bool isActive() const noexcept;
        // Booked and scheduled to arrive on `date`
        [[nodiscard]] bool isArrivingOn(const Date& date) const noexcept;
    private:
        uint64_t id = 0;
        uint32_t roomNumber = 0;
        uint64_t guestID = 0;

        Date checkInDate;
        Date checkOutDate;

        uint32_t guestCount = 1;

        Cents nightlyRate = 0;

        ReservationStatus status = ReservationStatus::Booked;
        Date createdOn;
    };

    std::ostream& operator<<(std::ostream& stream, const Reservation& reservation) noexcept;
}
