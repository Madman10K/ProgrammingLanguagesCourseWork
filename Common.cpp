#include "Common.hpp"
#include <array>
#include <string_view>

static constexpr std::array<const char*, Hotel::RoomType::Count> ROOM_TYPE_KEYS =
{
    "standard",
    "deluxe",
    "suite"
};

static constexpr std::array<const char*, Hotel::ReservationStatus::Count> STATUS_KEYS =
{
    "booked",
    "checked-in",
    "checked-out",
    "cancelled"
};

template<typename Enum, size_t N>
[[nodiscard]] static constexpr const char* enumToString(const Enum value, const std::array<const char*, N>& names, const char* fallback = "Unknown") noexcept
{
    const auto index = static_cast<size_t>(value);
    return index < N ? names[index] : fallback;
}

template<typename Wrapper, size_t N>
[[nodiscard]] static constexpr std::optional<Wrapper> enumFromKey(const std::string_view key, const std::array<const char*, N>& keys) noexcept
{
    for (size_t i = 0; i < N; ++i)
        if (key == keys[i])
            return Wrapper(i);
    return std::nullopt;
}

const char* Hotel::Result::toString() const noexcept
{
    static constexpr std::array<const char*, Count> RESULT_MESSAGES =
    {
        "Success",
        "Invalid date, expected a real calendar date in the format YYYY-MM-DD between 2000 and 2100",
        "Invalid period, the end date must be after the start date",
        "The date is in the past",
        "A single stay cannot be longer than 365 nights",
        "Invalid amount, expected a positive number with at most 2 decimal places and no larger than 1000000.00",
        "Invalid text, it must not contain control characters and must be at most 128 characters long",
        "Invalid room number, expected a number between 1 and 9999",
        "Invalid room type",
        "Invalid capacity for this room type",
        "A room with this number already exists",
        "Room not found",
        "The room is out of service",
        "The room is already booked for (part of) this period",
        "The room has reservations and cannot be removed, mark it as out of service instead",
        "The room is still occupied, the previous guest has to check out first",
        "Invalid name, it must not be empty, contain digits or be longer than 64 characters",
        "Invalid ID document number, it must be 4-20 letters or digits",
        "Invalid phone number, expected 7-15 digits with an optional leading + (spaces and dashes are allowed)",
        "Invalid e-mail address, expected the format name@domain.tld",
        "A guest with this ID document is already registered",
        "Guest not found",
        "The guest has reservations and cannot be removed",
        "Invalid ID",
        "Invalid number of guests, at least 1 is required",
        "The number of guests exceeds the capacity of the room",
        "A reservation with this ID already exists",
        "Reservation not found",
        "Invalid reservation status",
        "This operation is not allowed for the current status of the reservation",
        "Check-in is only possible between the arrival date and the day before departure",
        "An invoice for this ID or reservation already exists",
        "Invoice not found",
        "The invoice amounts do not add up",
        "Could not open or write a data file",
        "Malformed data",
        "The change was applied, but the data could not be saved to disk"
    };

    return enumToString(value, RESULT_MESSAGES, "Unknown error");
}

const char* Hotel::RoomType::toString() const noexcept
{
    static constexpr std::array<const char*, Count> ROOM_TYPE_DISPLAY_NAMES =
    {
        "Standard",
        "Deluxe",
        "Suite"
    };

    return enumToString(value, ROOM_TYPE_DISPLAY_NAMES);
}

const char* Hotel::RoomType::toKey() const noexcept
{
    return enumToString(value, ROOM_TYPE_KEYS, "");
}

std::optional<Hotel::RoomType> Hotel::RoomType::fromKey(const char* key) noexcept
{
    return enumFromKey<RoomType>(key, ROOM_TYPE_KEYS);
}

uint32_t Hotel::RoomType::getMaxCapacity() const noexcept
{
    static constexpr std::array<uint32_t, Count> ROOM_TYPE_MAX_CAPACITIES ={ 2, 3, 6 };
    const auto index = static_cast<size_t>(value);
    return index < ROOM_TYPE_MAX_CAPACITIES.size() ? ROOM_TYPE_MAX_CAPACITIES[index] : 0;
}

const char* Hotel::ReservationStatus::toString() const noexcept
{
    static constexpr std::array<const char*, Count> STATUS_DISPLAY_NAMES =
    {
        "Booked",
        "Checked in",
        "Checked out",
        "Cancelled"
    };

    return enumToString(value, STATUS_DISPLAY_NAMES);
}

const char* Hotel::ReservationStatus::toKey() const noexcept
{
    return enumToString(value, STATUS_KEYS, "");
}

std::optional<Hotel::ReservationStatus> Hotel::ReservationStatus::fromKey(const char* key) noexcept
{
    return enumFromKey<ReservationStatus>(key, STATUS_KEYS);
}
