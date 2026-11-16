#include "Storage.hpp"
#include "HotelManager.hpp"
#include <array>
#include <format>
#include <fstream>
#include <functional>
#include <optional>

static constexpr const auto* ROOMS_FILE        = "rooms.txt";
static constexpr const auto* GUESTS_FILE       = "guests.txt";
static constexpr const auto* RESERVATIONS_FILE = "reservations.txt";
static constexpr const auto* INVOICES_FILE     = "invoices.txt";

static constexpr size_t ROOM_FIELDS        = 6;
static constexpr size_t GUEST_FIELDS       = 6;
static constexpr size_t RESERVATION_FIELDS = 9;
static constexpr size_t INVOICE_FIELDS     = 9;

// Escapes `\` and `|` so a value can be stored as a single `|`-separated field
[[nodiscard]] static std::string escapeField(const std::string& str) noexcept
{
    std::string result;
    result.reserve(str.size());
    for (const char c : str)
    {
        switch (c)
        {
        case '\\':
            result += "\\\\";
            break;
        case '|':
            result += "\\|";
            break;
        default:
            result += c;
            break;
        }
    }
    return result;
}

template<size_t N>
[[nodiscard]] static std::string joinFields(const std::array<std::string, N>& fields) noexcept
{
    std::string line;
    for (size_t i = 0; i < N; ++i)
    {
        if (i != 0)
            line += '|';
        line += escapeField(fields[i]);
    }
    line += '\n';
    return line;
}

// Splits a line on unescaped `|` characters and unescapes every field. Returns std::nullopt if the line ends in a
// dangling escape character
[[nodiscard]] static std::optional<std::vector<std::string>> splitFields(const std::string& line) noexcept
{
    std::vector<std::string> fields;
    std::string current;
    for (size_t i = 0; i < line.size(); ++i)
    {
        const char c = line[i];
        if (c == '\\')
        {
            if (++i >= line.size())
                return std::nullopt;
            current += line[i];
        }
        else if (c == '|')
        {
            fields.push_back(std::move(current));
            current.clear();
        }
        else
            current += c;
    }
    fields.push_back(std::move(current));
    return fields;
}

static Hotel::Result readRecords(const std::filesystem::path& directory, const char* fileName,
                                 const size_t fieldCount, std::vector<std::string>& warnings,
                                 const std::function<std::string(const std::vector<std::string>&)>& callback) noexcept
{
    const std::filesystem::path path = directory / fileName;
    std::error_code ec;
    if (!std::filesystem::exists(path, ec))
        return Hotel::Result::Ok;

    std::ifstream file(path);
    if (!file)
        return Hotel::Result::FileError;

    std::string line;
    size_t lineNumber = 0;
    while (std::getline(file, line))
    {
        ++lineNumber;
        // Tolerate files that were edited on Windows
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (Hotel::trim(line).empty())
            continue;

        std::string error;
        const auto fields = splitFields(line);
        if (!fields)
            error = "unterminated escape sequence";
        else if (fields->size() != fieldCount)
            error = std::format("expected {} fields, found {}", fieldCount, fields->size());
        else
            error = callback(*fields);

        if (!error.empty())
            warnings.push_back(std::format("{}:{}: {}, record skipped", fileName, lineNumber, error));
    }
    if (file.bad())
        return Hotel::Result::FileError;

    return Hotel::Result::Ok;
}

[[nodiscard]] static std::string check(const Hotel::Result result) noexcept
{
    return result == Hotel::Result::Ok ? std::string{} : std::string{ result.toString() };
}

[[nodiscard]] static std::optional<bool> parseFlag(const std::string& str) noexcept
{
    if (str == "0")
        return false;
    if (str == "1")
        return true;
    return std::nullopt;
}

[[nodiscard]] static std::string loadRoom(const std::vector<std::string>& f, Hotel::HotelManager& manager) noexcept
{
    const auto number = Hotel::parseNumber<uint32_t>(f[0]);
    const auto type = Hotel::RoomType::fromKey(f[1].c_str());
    const auto price = Hotel::parseNumber<int64_t>(f[2]);
    const auto capacity = Hotel::parseNumber<uint32_t>(f[3]);
    const auto bOutOfService = parseFlag(f[4]);

    if (!number || !type || !price || !capacity || !bOutOfService)
        return Hotel::Result(Hotel::Result::ParseError);

    std::unique_ptr<Hotel::Room> room;
    CHECK(Hotel::Room::create(*type, *number, *price, *capacity, f[5].c_str(), *bOutOfService, room));
    return check(manager.importRoom(std::move(room)));
}

[[nodiscard]] static std::string loadGuest(const std::vector<std::string>& f, Hotel::HotelManager& manager) noexcept
{
    const auto id = Hotel::parseNumber<uint64_t>(f[0]);
    if (!id)
        return Hotel::Result(Hotel::Result::ParseError);

    Hotel::Guest guest;
    CHECK(guest.setID(*id));
    CHECK(guest.setFirstName(f[1].c_str()));
    CHECK(guest.setLastName(f[2].c_str()));
    CHECK(guest.setIDDocument(f[3].c_str()));
    CHECK(guest.setPhone(f[4].c_str()));
    CHECK(guest.setEmail(f[5].c_str()));

    return check(manager.importGuest(guest));
}

[[nodiscard]] static std::string loadReservation(const std::vector<std::string>& f, Hotel::HotelManager& manager) noexcept
{
    const auto id = Hotel::parseNumber<uint64_t>(f[0]);
    const auto room = Hotel::parseNumber<uint32_t>(f[1]);
    const auto guestID = Hotel::parseNumber<uint64_t>(f[2]);

    Hotel::Date checkIn;
    Hotel::Date checkOut;

    const auto guestCount = Hotel::parseNumber<uint32_t>(f[5]);
    const auto rate = Hotel::parseNumber<int64_t>(f[6]);
    const auto status = Hotel::ReservationStatus::fromKey(f[7].c_str());
    Hotel::Date createdOn;

    if (    !id
        ||  !room
        ||  !guestID
        ||  Hotel::Date::parse(f[3].c_str(), checkIn) != Hotel::Result::Ok
        ||  Hotel::Date::parse(f[4].c_str(), checkOut) != Hotel::Result::Ok
        ||  !guestCount
        ||  !rate
        ||  !status
        ||  Hotel::Date::parse(f[8].c_str(), createdOn) != Hotel::Result::Ok
    )
        return Hotel::Result(Hotel::Result::ParseError);

    Hotel::Reservation reservation;
    CHECK(reservation.setID(*id));
    CHECK(reservation.setRoomNumber(*room));
    CHECK(reservation.setGuestID(*guestID));
    CHECK(reservation.setDates(checkIn, checkOut));
    CHECK(reservation.setGuestCount(*guestCount));
    CHECK(reservation.setNightlyRate(*rate));
    CHECK(reservation.setStatus(*status));
    reservation.setCreatedOn(createdOn);

    return check(manager.importReservation(reservation));
}

[[nodiscard]] static std::string loadInvoice(const std::vector<std::string>& f, Hotel::HotelManager& manager) noexcept
{
    const auto id = Hotel::parseNumber<uint64_t>(f[0]);
    const auto reservationID = Hotel::parseNumber<uint64_t>(f[1]);
    const auto room = Hotel::parseNumber<uint32_t>(f[2]);

    Hotel::Date issueDate;

    const auto nights = Hotel::parseNumber<uint32_t>(f[5]);
    const auto roomCost = Hotel::parseNumber<int64_t>(f[6]);
    const auto tax = Hotel::parseNumber<int64_t>(f[7]);
    const auto total = Hotel::parseNumber<int64_t>(f[8]);

    if (    !id
        ||  !reservationID
        ||  !room
        ||  Hotel::Date::parse(f[4].c_str(), issueDate) != Hotel::Result::Ok
        ||  !nights
        ||  !roomCost
        ||  !tax
        ||  !total
    )
        return Hotel::Result(Hotel::Result::ParseError);

    Hotel::Invoice invoice;
    CHECK(invoice.setID(*id));
    CHECK(invoice.setReservationID(*reservationID));
    CHECK(invoice.setRoomNumber(*room));
    CHECK(invoice.setBilledTo(f[3].c_str()));
    CHECK(invoice.setAmounts(*nights, *roomCost, *tax));
    invoice.setIssueDate(issueDate);

    if (invoice.getTotal() != *total)
        return Hotel::Result(Hotel::Result::InvoiceMismatch);
    return check(manager.importInvoice(invoice));
}

Hotel::Result Hotel::Storage::load(const std::filesystem::path& directory, HotelManager& manager, std::vector<std::string>& warnings) noexcept
{
    CHECK(ensureDirectory(directory));
    manager.clear();

    struct TableLoader
    {
        const char* file;
        size_t fieldCount;
        std::function<std::string(const std::vector<std::string>&, HotelManager&)> load;
    };

    static const std::array<TableLoader, 4> TABLES =
    {
        {
            {
                .file = ROOMS_FILE,
                .fieldCount = ROOM_FIELDS,
                .load = &loadRoom
            },
            {
                .file = GUESTS_FILE,
                .fieldCount = GUEST_FIELDS,
                .load = &loadGuest
            },
            {
                .file = RESERVATIONS_FILE,
                .fieldCount = RESERVATION_FIELDS,
                .load = &loadReservation
            },
            {
                .file = INVOICES_FILE,
                .fieldCount = INVOICE_FIELDS,
                .load = &loadInvoice
            }
        }
    };

    for (const auto& table : TABLES)
    {
        CHECK(
            readRecords(
                directory,
                table.file,
                table.fieldCount,
                warnings,
                [&](const auto& fields) noexcept -> std::string
                {
                    return table.load(fields, manager);
                }
            )
        );
    }

    return Result::Ok;
}

Hotel::Result Hotel::Storage::save(const std::filesystem::path& directory, const HotelManager& manager) noexcept
{
    CHECK(ensureDirectory(directory));

    std::string rooms;
    for (const auto& room : manager.getRooms())
    {
        rooms += joinFields(
            std::array<std::string, ROOM_FIELDS>
            {
                std::to_string(room->getNumber()),
                room->getType().toKey(),

                std::to_string(room->getPricePerNight()),
                std::to_string(room->getCapacity()),

                room->isOutOfService() ? "1" : "0",
                room->getDescription()
            }
        );
    }

    std::string guests;
    for (const auto& guest : manager.getGuests())
    {
        guests += joinFields(
            std::array<std::string, GUEST_FIELDS>
            {
                std::to_string(guest.getID()),
                guest.getFirstName(),
                guest.getLastName(),

                guest.getIDDocument(),

                guest.getPhone(),
                guest.getEmail()
            }
        );
    }

    std::string reservations;
    for (const auto& reservation : manager.getReservations())
    {
        reservations += joinFields(
            std::array<std::string, RESERVATION_FIELDS>
            {
                std::to_string(reservation.getID()),
                std::to_string(reservation.getRoomNumber()),
                std::to_string(reservation.getGuestID()),

                reservation.getCheckIn().toString(),
                reservation.getCheckOut().toString(),

                std::to_string(reservation.getGuestCount()),
                std::to_string(reservation.getNightlyRate()),

                reservation.getStatus().toKey(),
                reservation.getCreatedOn().toString()
            }
        );
    }

    std::string invoices;
    for (const auto& invoice : manager.getInvoices())
    {
        invoices += joinFields(
            std::array<std::string, INVOICE_FIELDS>{
                std::to_string(invoice.getID()),
                std::to_string(invoice.getReservationID()),
                std::to_string(invoice.getRoomNumber()),

                invoice.getBilledTo(),
                invoice.getIssueDate().toString(),

                std::to_string(invoice.getNights()),
                std::to_string(invoice.getRoomCost()),
                std::to_string(invoice.getTax()),
                std::to_string(invoice.getTotal())
            }
        );
    }

    CHECK(writeTextFile(directory / ROOMS_FILE, rooms.c_str()));
    CHECK(writeTextFile(directory / GUESTS_FILE, guests.c_str()));
    CHECK(writeTextFile(directory / RESERVATIONS_FILE, reservations.c_str()));
    return writeTextFile(directory / INVOICES_FILE, invoices.c_str());
}
