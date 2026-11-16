#pragma once
#include <ostream>
#include <string>
#include <string_view>
#include "Reservation.hpp"
#include "Room.hpp"

namespace Hotel
{
    class Invoice
    {
    public:
        Invoice() noexcept = default;

        static Invoice calculate(uint64_t id, const Reservation& reservation, const Room& room, const char* billedTo, const Date& issueDate) noexcept;

        [[nodiscard]] static Cents estimateTotal(const Room& room, uint32_t nights, Cents nightlyRate) noexcept;
        [[nodiscard]] static Result validateAmounts(uint32_t nightCount, Cents roomCost, Cents tax) noexcept;

        [[nodiscard]] Result validate() const noexcept;

        [[nodiscard]] uint64_t getID() const noexcept;
        Result setID(uint64_t value) noexcept;

        [[nodiscard]] uint64_t getReservationID() const noexcept;
        Result setReservationID(uint64_t value) noexcept;

        [[nodiscard]] uint32_t getRoomNumber() const noexcept;
        Result setRoomNumber(uint32_t number) noexcept;

        [[nodiscard]] const std::string& getBilledTo() const noexcept;
        Result setBilledTo(const char* name) noexcept;

        [[nodiscard]] const Date& getIssueDate() const noexcept;
        Invoice& setIssueDate(const Date& date) noexcept;

        Result setAmounts(uint32_t nightCount, Cents roomCost, Cents tax) noexcept;

        [[nodiscard]] uint32_t getNights() const noexcept;
        [[nodiscard]] Cents getRoomCost() const noexcept;
        [[nodiscard]] Cents getTax() const noexcept;
        [[nodiscard]] Cents getTotal() const noexcept;
    private:
        uint64_t id = 0;
        uint64_t reservationID = 0;
        uint32_t roomNumber = 0;
        std::string billedTo;
        Date issueDate;

        uint32_t nights = 0;
        Cents roomCost = 0;
        Cents tax = 0;
    };

    // Prints the invoice as a formatted bill
    std::ostream& operator<<(std::ostream& stream, const Invoice& invoice) noexcept;
}
