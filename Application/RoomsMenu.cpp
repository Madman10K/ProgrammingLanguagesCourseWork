#include "Menu.hpp"
#include <format>
#include <iostream>

void Hotel::Menu::roomsMenu() noexcept
{
    runMenu(
        "Rooms",
        {
            {
                "List rooms",
                [this]() -> void
                {
                    listRooms();
                }
            },
            {
                "Add a room",
                [this]() -> void
                {
                    addRoom();
                }
            },
            { "Edit a room",
                [this]() -> void
                {
                    editRoom();
                }
            },
            { "Remove a room",
                [this]() -> void
                {
                    removeRoom();
                }
            },
            {
                "Check availability",
                [this]() -> void
                {
                    checkRoomAvailability();
                }
            }
        }
    );
}

void Hotel::Menu::listRooms() const noexcept
{
    printList(
        manager.getRooms(),
        "There are no rooms yet",
        [](const auto& room) noexcept -> void
        {
            std::cout << "  " << *room << "\n";
        }
    );
}

void Hotel::Menu::addRoom() noexcept
{
    std::vector<std::string> options;
    options.reserve(RoomType::Count);
    for (size_t i = 0; i < RoomType::Count; ++i)
    {
        const RoomType rt(i);
        options.push_back(std::format("{} (up to {} guests)", rt.toString(), rt.getMaxCapacity()));
    }

    const size_t type = readChoice("Room type", options, "Cancel");
    if (type == 0)
        return;

    const auto roomType = RoomType(type - 1);

    const auto number = readRoomNumber(
        "Room number",
        [this](const uint32_t num) -> std::string
        {
            return manager.findRoom(num) != nullptr ? Result(Result::RoomAlreadyExists).toString() : "";
        }
    );
    if (!number)
        return;

    const auto price = readMoney("Price per night");
    if (!price)
        return;

    const auto capacity = readInteger("Capacity", 1, roomType.getMaxCapacity());
    if (!capacity)
        return;

    const auto description = readText("Description (optional): ", true);
    if (!description)
        return;

    std::unique_ptr<Room> room;
    Result result = Room::create(
        roomType,
        *number,
        *price,
        static_cast<uint32_t>(*capacity),
        description->c_str(),
        false,
        room
    );
    if (result == Result::Ok)
        result = manager.addRoom(std::move(room));

    printResult(result, std::format("Room {} was added", *number).c_str());
}

void Hotel::Menu::editRoom() noexcept
{
    listRooms();
    const Room* room = readExisting<Room>("Room number", MIN_ROOM_NUMBER, MAX_ROOM_NUMBER, &HotelManager::findRoom, Result::RoomNotFound);
    if (room == nullptr)
        return;
    std::cout << "  " << *room << "\n";

    const auto price = readMoney(
        std::format(
            "New price per night, currently {}{}",
            formatMoney(
                room->getPricePerNight()
            ),
            KEEP_HINT
        ).c_str()
    );
    if (bEOF)
        return;

    const auto description = readText(
        std::format(
            "New description, \"-\" to clear{}",
            KEEP_HINT
        ).c_str(),
        true
    );
    if (bEOF)
        return;

    const auto bOutOfService = readYesNo(
        std::format(
            "Out of service (y/n), currently {}{}",
            room->isOutOfService()
                ? "yes"
                : "no",
            KEEP_HINT
        ).c_str()
    );
    if (bEOF)
        return;

    std::string newDescription = room->getDescription();
    if (description && *description == "-")
        newDescription.clear();
    else if (description && !description->empty())
        newDescription = *description;

    printResult(
        manager.updateRoom(
            room->getNumber(),
            price.value_or(room->getPricePerNight()),
            newDescription.c_str(),
            bOutOfService.value_or(room->isOutOfService())
        ),
        "The room was updated"
    );
}

void Hotel::Menu::removeRoom() noexcept
{
    listRooms();

    const auto number = readRoomNumber("Room number");
    if (!number)
        return;

    if (confirm(std::format("Remove room {}?", *number).c_str()))
        printResult(manager.removeRoom(*number), "The room was removed");
}

void Hotel::Menu::checkRoomAvailability() noexcept
{
    const auto from = readDate("Arrival date");
    if (!from)
        return;

    const auto to = readEndDate("Departure date", *from, false, false);
    if (!to)
        return;

    const auto rooms = manager.findAvailableRooms(*from, *to, 1);
    std::cout << rooms.size() << " of " << manager.getRooms().size() << " rooms are free from " << *from << " to " << *to << ":\n";
    for (const auto* room : rooms)
        std::cout << "  " << *room << "\n";
}
