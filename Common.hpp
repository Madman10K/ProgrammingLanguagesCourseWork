#pragma once
#include <cstdint>
#include <optional>
#include <string>

// Early-returns on error codes
#define CHECK(f)                                                             \
    do                                                                       \
    {                                                                        \
        if (const Hotel::Result result = (f); result != Hotel::Result::Ok)   \
            return result;                                                   \
    } while (false)

namespace Hotel
{
    // All amounts of money are stored as an integer number of cents, so that sums and rounding are exact
    using Cents = int64_t;

    // 10% VAT
    inline constexpr uint32_t TAX_RATE_BASIS_POINTS = 1000;

    // Largest nightly rate ($1,000,000.00 in cents)
    inline constexpr Cents MAX_AMOUNT = 100'000'000;

    // Room numbers are in the range [MIN_ROOM_NUMBER, MAX_ROOM_NUMBER]
    inline constexpr uint32_t MIN_ROOM_NUMBER = 1;
    inline constexpr uint32_t MAX_ROOM_NUMBER = 9999;

    // First and last names are limited separately, the full name ("first last") can be up to MAX_FULL_NAME_LENGTH long
    inline constexpr size_t MAX_NAME_LENGTH = 64;
    inline constexpr size_t MAX_FULL_NAME_LENGTH = MAX_NAME_LENGTH * 2 + 1;

    // Like an enum class but the implicit conversion operators save us so much code
    struct Result
    {
        enum ResultType
        {
            Ok = 0,

            InvalidDate,
            InvalidDateRange,
            DateInThePast,
            StayTooLong,
            InvalidAmount,
            InvalidText,

            InvalidRoomNumber,
            InvalidRoomType,
            InvalidCapacity,
            RoomAlreadyExists,
            RoomNotFound,
            RoomOutOfService,
            RoomNotAvailable,
            RoomHasReservations,
            RoomOccupied,

            InvalidName,
            InvalidIdDocument,
            InvalidPhone,
            InvalidEmail,
            GuestAlreadyExists,
            GuestNotFound,
            GuestHasReservations,

            InvalidId,
            InvalidGuestCount,
            CapacityExceeded,
            ReservationAlreadyExists,
            ReservationNotFound,
            InvalidStatus,
            InvalidStatusTransition,
            OutsideStayPeriod,

            InvoiceAlreadyExists,
            InvoiceNotFound,
            InvoiceMismatch,

            FileError,
            ParseError,
            SaveFailed,

            Count
        };

        ResultType value = Ok;

        constexpr Result() noexcept = default;
        constexpr Result(const ResultType type) noexcept : value(type) {}
        constexpr explicit Result(const size_t index) noexcept : value(static_cast<ResultType>(index)) {}

        constexpr operator ResultType() const noexcept { return value; }
        operator std::string() const noexcept { return toString(); }

        [[nodiscard]] constexpr bool isValid() const noexcept { return static_cast<size_t>(value) < Count; }

        // Human-readable description of a result code, used for every error shown to the user
        [[nodiscard]] const char* toString() const noexcept;
    };

    struct RoomType
    {
        enum RoomTypeType
        {
            Standard = 0,
            Deluxe = 1,
            Suite = 2,

            Count
        };

        RoomTypeType value = Standard;

        constexpr RoomType() noexcept = default;
        constexpr RoomType(const RoomTypeType type) noexcept : value(type) {}
        constexpr explicit RoomType(const size_t index) noexcept : value(static_cast<RoomTypeType>(index)) {}

        constexpr operator RoomTypeType() const noexcept { return value; }

        [[nodiscard]] constexpr bool isValid() const noexcept { return static_cast<size_t>(value) < Count; }
        [[nodiscard]] const char* toString() const noexcept;
        [[nodiscard]] const char* toKey() const noexcept;
        [[nodiscard]] static std::optional<RoomType> fromKey(const char* key) noexcept;
        [[nodiscard]] uint32_t getMaxCapacity() const noexcept;
    };

    struct ReservationStatus
    {
        enum ReservationStatusType
        {
            Booked = 0,
            CheckedIn = 1,
            CheckedOut = 2,
            Cancelled = 3,

            Count
        };

        ReservationStatusType value = Booked;

        constexpr ReservationStatus() noexcept = default;
        constexpr ReservationStatus(const ReservationStatusType type) noexcept : value(type) {}
        constexpr explicit ReservationStatus(const size_t index) noexcept : value(static_cast<ReservationStatusType>(index)) {}

        constexpr operator ReservationStatusType() const noexcept { return value; }

        [[nodiscard]] constexpr bool isValid() const noexcept { return static_cast<size_t>(value) < Count; }
        [[nodiscard]] const char* toString() const noexcept;
        [[nodiscard]] const char* toKey() const noexcept;
        [[nodiscard]] static std::optional<ReservationStatus> fromKey(const char* key) noexcept;
    };
}
