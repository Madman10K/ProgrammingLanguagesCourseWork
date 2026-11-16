#pragma once
#include <format>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <HotelManager.hpp>
#include <Report.hpp>

inline constexpr const auto* CANCEL_HINT = " (leave empty to cancel): ";
inline constexpr const auto* KEEP_HINT = " (leave empty to keep): ";

// Prints an array with a lambda
template<typename Range, typename Printer>
static inline void printList(const Range& items, const char* emptyMessage, Printer printer) noexcept
{
    if (items.empty())
    {
        std::cout << emptyMessage << "\n";
        return;
    }

    for (const auto& item : items)
        printer(item);
}

namespace Hotel
{
    // The interactive terminal front end. Every prompt validates its input and asks again until it gets a valid value.
    // An empty line cancels the current operation (or keeps the current value when editing), and the end of the input
    // stream (Ctrl+D) leaves the application cleanly
    class Menu
    {
    public:
        explicit Menu(HotelManager& manager) noexcept;

        // Runs the main menu until the user exits or the input ends
        void run() noexcept;
    private:
        using MenuItem = std::pair<const char*, std::function<void()>>;

        HotelManager& manager;
        bool bEOF = false;

        void runMenu(const char* title, const std::vector<MenuItem>& items, const char* backLabel = "Back") noexcept;

        void roomsMenu() noexcept;
        void guestsMenu() noexcept;
        void reservationsMenu() noexcept;
        void reportsMenu() noexcept;
        void invoicesMenu() noexcept;

        void listRooms() const noexcept;
        void addRoom() noexcept;
        void editRoom() noexcept;
        void removeRoom() noexcept;
        void checkRoomAvailability() noexcept;

        void listGuests() const noexcept;
        void searchGuests() noexcept;
        // Returns the ID of the new guest, or 0 if the registration was cancelled
        uint64_t addGuest() noexcept;
        void editGuest() noexcept;
        void removeGuest() noexcept;

        void listReservations() const noexcept;
        void makeReservation() noexcept;
        void cancelReservation() noexcept;
        void checkIn() noexcept;
        void checkOut() noexcept;

        void showReport(const Report& report) noexcept;

        void listInvoices() const noexcept;
        void viewInvoice() noexcept;

        // Lists the reservations with the given status and returns their number
        [[nodiscard]] size_t listReservationsWithStatus(ReservationStatus status) const noexcept;
        void printReservation(const Reservation& reservation) const noexcept;

        // Reads a raw line. Returns std::nullopt at the end of the input
        std::optional<std::string> readLine(const char* prompt) noexcept;

        // Asks until the parser accepts the input. Returns std::nullopt for an empty line or at the end of the input
        template<typename T>
        std::optional<T> read(std::string_view prompt, const std::function<std::string(const char*, T&)>& parser) noexcept;

        std::optional<int64_t> readInteger(const char* prompt, int64_t min, int64_t max) noexcept;

        std::optional<uint64_t> readID(const char* prompt, uint64_t min = 1) noexcept;
        std::optional<uint32_t> readRoomNumber(const char* prompt, const std::function<std::string(uint32_t)>& extraCheck = {}) noexcept;

        std::optional<Cents> readMoney(const char* prompt) noexcept;

        std::optional<Date> readDate(const char* prompt) noexcept;
        std::optional<Date> readEndDate(const char* prompt, const Date& start, bool bInclusive, bool bLimitNights) noexcept;

        std::optional<bool> readYesNo(const char* prompt) noexcept;
        bool confirm(const char* question) noexcept;
        // Unlike the functions above, an empty line returns an empty string when bAllowEmpty is set
        std::optional<std::string> readText(const char* prompt, bool bAllowEmpty) noexcept;

        bool readGuestField(Guest& guest, const char* prompt, const std::function<Result(Guest&, const char*)>& setter) noexcept;

        template<typename T, typename Key>
        const T* readExisting(const char* prompt, Key min, Key max, const std::function<const T*(const HotelManager&, std::type_identity_t<Key>)>& finder, Result notFoundError) noexcept;

        // Shows a numbered list of options with "0. Back" and returns the selected index, or 0 at the end of the input
        size_t readChoice(const char* title, const std::vector<std::string>& options, const char* backLabel = "Back") noexcept;

        static void printColoured(const char* colour, const char* prefix, const char* message) noexcept;
        static void printError(const char* message) noexcept;
        static void printWarning(const char* message) noexcept;
        static void printSuccess(const char* message) noexcept;
        // Prints `successMessage` on success, or the description of the error otherwise. Returns whether it succeeded
        static bool printResult(Result result, const char* successMessage) noexcept;
    };

    // Read a message, sanitise and run a lambda on it
    template<typename T>
    std::optional<T> Menu::read(const std::string_view prompt, const std::function<std::string(const char*, T&)>& parser) noexcept
    {
        // Prompts that already describe what an empty line does are printed as they are
        const std::string fullPrompt = prompt.ends_with(": ") ? std::string{ prompt } : std::format("{}{}", prompt, CANCEL_HINT);
        while (true)
        {
            const auto line = readLine(fullPrompt.c_str());
            if (!line)
                return std::nullopt;
            // Copy the trimmed text, so that the parser gets a C string without the trailing whitespace
            const std::string value{ trim(*line) };
            if (value.empty())
                return std::nullopt;

            T result{};
            const std::string error = parser(value.c_str(), result);
            if (error.empty())
                return result;
            printError(error.c_str());
        }
    }

    template<typename T, typename Key>
    const T* Menu::readExisting(const char* prompt, const Key min, const Key max, const std::function<const T*(const HotelManager&, std::type_identity_t<Key>)>& finder, const Result notFoundError) noexcept
    {
        const auto id = readInteger(prompt, static_cast<int64_t>(min), static_cast<int64_t>(max));
        if (!id)
            return nullptr;

        const T* item = finder(manager, static_cast<Key>(*id));
        if (item == nullptr)
            printError(notFoundError.toString());
        return item;
    }
}
