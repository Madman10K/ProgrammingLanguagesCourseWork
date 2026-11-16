#pragma once
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include "Utility.hpp"

namespace Hotel
{
    // Abstract base class for every room in the hotel. Each room type decides its own capacity limit
    class Room
    {
    public:
        Room() noexcept = default;
        virtual ~Room() noexcept = default;

        // Creates an empty room of the given type, returns nullptr for an invalid type
        static std::unique_ptr<Room> create(RoomType type) noexcept;

        // Creates a room of the given type and validates every field. `out` is only modified on success
        static Result create(RoomType type, uint32_t number, Cents pricePerNight, uint32_t capacity,
                             const char* description, bool bOutOfService, std::unique_ptr<Room>& out) noexcept;

        // Creates a copy of the room with the same dynamic type
        [[nodiscard]] virtual std::unique_ptr<Room> clone() const noexcept = 0;

        [[nodiscard]] virtual RoomType getType() const noexcept = 0;
        [[nodiscard]] const char* getTypeName() const noexcept;
        [[nodiscard]] virtual uint32_t getMaxCapacity() const noexcept = 0;
        [[nodiscard]] bool isValidCapacity(uint32_t value) const noexcept;

        // The cost of the room for a stay before tax (nights * nightlyRate).
        // The rate is passed in because reservations keep the price they were booked at
        [[nodiscard]] Cents calculateStayCost(uint32_t nights, Cents nightlyRate) const noexcept;

        // Prints the room on a single line
        void print(std::ostream& stream) const noexcept;

        // Validates every field of the room, used before adding a room that was built up with setters
        [[nodiscard]] Result validate() const noexcept;

        [[nodiscard]] uint32_t getNumber() const noexcept;
        Result setNumber(uint32_t value) noexcept;

        [[nodiscard]] Cents getPricePerNight() const noexcept;
        Result setPricePerNight(Cents price) noexcept;

        [[nodiscard]] uint32_t getCapacity() const noexcept;
        Result setCapacity(uint32_t value) noexcept;

        [[nodiscard]] const std::string& getDescription() const noexcept;
        Result setDescription(const char* value) noexcept;

        [[nodiscard]] bool isOutOfService() const noexcept;
        Room& setOutOfService(bool bValue) noexcept;
    protected:
        Room(const Room&) noexcept = default;
        Room& operator=(const Room&) noexcept = default;
    private:
        uint32_t number = 0;
        Cents pricePerNight = 0;
        uint32_t capacity = 1;
        std::string description;
        bool bOutOfService = false;
    };

    class StandardRoom final : public Room
    {
    public:
        StandardRoom() noexcept = default;

        [[nodiscard]] std::unique_ptr<Room> clone() const noexcept override;
        [[nodiscard]] RoomType getType() const noexcept override;
        [[nodiscard]] uint32_t getMaxCapacity() const noexcept override;
    };

    class DeluxeRoom final : public Room
    {
    public:
        DeluxeRoom() noexcept = default;

        [[nodiscard]] std::unique_ptr<Room> clone() const noexcept override;
        [[nodiscard]] RoomType getType() const noexcept override;
        [[nodiscard]] uint32_t getMaxCapacity() const noexcept override;
    };

    class Suite final : public Room
    {
    public:
        Suite() noexcept = default;

        [[nodiscard]] std::unique_ptr<Room> clone() const noexcept override;
        [[nodiscard]] RoomType getType() const noexcept override;
        [[nodiscard]] uint32_t getMaxCapacity() const noexcept override;
    };

    // Outputs the room via Room::print
    std::ostream& operator<<(std::ostream& stream, const Room& room) noexcept;
}
