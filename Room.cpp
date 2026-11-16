#include "Room.hpp"
#include <iomanip>

std::unique_ptr<Hotel::Room> Hotel::Room::create(const RoomType type) noexcept
{
    switch (type)
    {
    case RoomType::Standard:
        return std::make_unique<StandardRoom>();
    case RoomType::Deluxe:
        return std::make_unique<DeluxeRoom>();
    case RoomType::Suite:
        return std::make_unique<Suite>();
    default:
        return nullptr;
    }
}

Hotel::Result Hotel::Room::create(const RoomType type, const uint32_t number, const Cents pricePerNight,
                                    const uint32_t capacity, const char* description, const bool bOutOfService,
                                    std::unique_ptr<Room>& out) noexcept
{
    auto room = create(type);
    if (room == nullptr)
        return Result::InvalidRoomType;

    CHECK(room->setNumber(number));
    CHECK(room->setPricePerNight(pricePerNight));
    CHECK(room->setCapacity(capacity));
    CHECK(room->setDescription(description));

    room->setOutOfService(bOutOfService);

    out = std::move(room);
    return Result::Ok;
}

const char* Hotel::Room::getTypeName() const noexcept
{
    return getType().toString();
}

bool Hotel::Room::isValidCapacity(const uint32_t value) const noexcept
{
    return value >= 1 && value <= getMaxCapacity();
}

Hotel::Cents Hotel::Room::calculateStayCost(const uint32_t nights, const Cents nightlyRate) const noexcept
{
    return static_cast<Cents>(nights) * nightlyRate;
}

void Hotel::Room::print(std::ostream& stream) const noexcept
{
    stream << "Room " << std::left << std::setw(5) << number
           << std::setw(9) << getTypeName()
           << std::right << std::setw(10) << formatMoney(pricePerNight) << "/night  "
           << "up to " << capacity << " guests"
           << (bOutOfService ? "  [OUT OF SERVICE]" : "");
    if (!description.empty())
        stream << "  - " << description;
}

Hotel::Result Hotel::Room::validate() const noexcept
{
    if (!(number >= MIN_ROOM_NUMBER && number <= MAX_ROOM_NUMBER))
        return Result::InvalidRoomNumber;
    if (!(pricePerNight > 0 && pricePerNight <= MAX_AMOUNT))
        return Result::InvalidAmount;
    if (!isValidCapacity(capacity))
        return Result::InvalidCapacity;
    if (!isValidText(description))
        return Result::InvalidText;
    return Result::Ok;
}

uint32_t Hotel::Room::getNumber() const noexcept
{
    return number;
}

Hotel::Result Hotel::Room::setNumber(const uint32_t value) noexcept
{
    if (!(value >= MIN_ROOM_NUMBER && value <= MAX_ROOM_NUMBER))
        return Result::InvalidRoomNumber;
    number = value;
    return Result::Ok;
}

Hotel::Cents Hotel::Room::getPricePerNight() const noexcept
{
    return pricePerNight;
}

Hotel::Result Hotel::Room::setPricePerNight(const Cents price) noexcept
{
    if (!(price > 0 && price <= MAX_AMOUNT))
        return Result::InvalidAmount;
    pricePerNight = price;
    return Result::Ok;
}

uint32_t Hotel::Room::getCapacity() const noexcept
{
    return capacity;
}

Hotel::Result Hotel::Room::setCapacity(const uint32_t value) noexcept
{
    if (!isValidCapacity(value))
        return Result::InvalidCapacity;
    capacity = value;
    return Result::Ok;
}

const std::string& Hotel::Room::getDescription() const noexcept
{
    return description;
}

Hotel::Result Hotel::Room::setDescription(const char* value) noexcept
{
    if (!isValidText(value))
        return Result::InvalidText;
    description = trim(value);
    return Result::Ok;
}

bool Hotel::Room::isOutOfService() const noexcept
{
    return bOutOfService;
}

Hotel::Room& Hotel::Room::setOutOfService(const bool bValue) noexcept
{
    bOutOfService = bValue;
    return *this;
}

std::unique_ptr<Hotel::Room> Hotel::StandardRoom::clone() const noexcept
{
    return std::make_unique<StandardRoom>(*this);
}

Hotel::RoomType Hotel::StandardRoom::getType() const noexcept
{
    return RoomType::Standard;
}

uint32_t Hotel::StandardRoom::getMaxCapacity() const noexcept
{
    return getType().getMaxCapacity();
}

std::unique_ptr<Hotel::Room> Hotel::DeluxeRoom::clone() const noexcept
{
    return std::make_unique<DeluxeRoom>(*this);
}

Hotel::RoomType Hotel::DeluxeRoom::getType() const noexcept
{
    return RoomType::Deluxe;
}

uint32_t Hotel::DeluxeRoom::getMaxCapacity() const noexcept
{
    return getType().getMaxCapacity();
}

std::unique_ptr<Hotel::Room> Hotel::Suite::clone() const noexcept
{
    return std::make_unique<Suite>(*this);
}

Hotel::RoomType Hotel::Suite::getType() const noexcept
{
    return RoomType::Suite;
}

uint32_t Hotel::Suite::getMaxCapacity() const noexcept
{
    return getType().getMaxCapacity();
}

std::ostream& Hotel::operator<<(std::ostream& stream, const Room& room) noexcept
{
    room.print(stream);
    return stream;
}
