#pragma once
#include <ostream>
#include <string>
#include <string_view>
#include "Utility.hpp"

namespace Hotel
{
    class Guest
    {
    public:
        Guest() noexcept = default;

        // Validates every field of the guest, used before adding a guest that was built up with setters
        [[nodiscard]] Result validate() const noexcept;

        [[nodiscard]] uint64_t getID() const noexcept;
        // CAUTION: IDs are assigned by the HotelManager, this is only meant to be used when loading data
        Result setID(uint64_t value) noexcept;

        [[nodiscard]] const std::string& getFirstName() const noexcept;
        Result setFirstName(const char* name) noexcept;

        [[nodiscard]] const std::string& getLastName() const noexcept;
        Result setLastName(const char* name) noexcept;

        [[nodiscard]] std::string getFullName() const noexcept;

        // Passport or national ID card number, used to tell guests with the same name apart
        [[nodiscard]] const std::string& getIDDocument() const noexcept;
        Result setIDDocument(const char* document) noexcept;

        [[nodiscard]] const std::string& getPhone() const noexcept;
        Result setPhone(const char* value) noexcept;

        [[nodiscard]] const std::string& getEmail() const noexcept;
        Result setEmail(const char* value) noexcept;

        [[nodiscard]] static bool isValidName(std::string_view name) noexcept;
        [[nodiscard]] static bool isValidIDDocument(std::string_view document) noexcept;
        [[nodiscard]] static bool isValidPhone(std::string_view phone) noexcept;
        [[nodiscard]] static bool isValidEmail(std::string_view email) noexcept;
    private:
        uint64_t id = 0;

        std::string firstName;
        std::string lastName;
        std::string idDocument;
        std::string phone;
        std::string email;
    };

    std::ostream& operator<<(std::ostream& stream, const Guest& guest) noexcept;
}
