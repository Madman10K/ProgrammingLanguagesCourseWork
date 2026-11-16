#include "Guest.hpp"
#include <algorithm>
#include <cctype>

Hotel::Result Hotel::Guest::validate() const noexcept
{
    if (id == 0)
        return Result::InvalidId;
    if (!isValidName(firstName) || !isValidName(lastName))
        return Result::InvalidName;
    if (!isValidIDDocument(idDocument))
        return Result::InvalidIdDocument;
    if (!isValidPhone(phone))
        return Result::InvalidPhone;
    if (!isValidEmail(email))
        return Result::InvalidEmail;
    return Result::Ok;
}

uint64_t Hotel::Guest::getID() const noexcept
{
    return id;
}

Hotel::Result Hotel::Guest::setID(const uint64_t value) noexcept
{
    if (value == 0)
        return Result::InvalidId;
    id = value;
    return Result::Ok;
}

const std::string& Hotel::Guest::getFirstName() const noexcept
{
    return firstName;
}

Hotel::Result Hotel::Guest::setFirstName(const char* name) noexcept
{
    const std::string_view value = trim(name);
    if (!isValidName(value))
        return Result::InvalidName;

    firstName = value;
    return Result::Ok;
}

const std::string& Hotel::Guest::getLastName() const noexcept
{
    return lastName;
}

Hotel::Result Hotel::Guest::setLastName(const char* name) noexcept
{
    const std::string_view value = trim(name);
    if (!isValidName(value))
        return Result::InvalidName;

    lastName = value;
    return Result::Ok;
}

std::string Hotel::Guest::getFullName() const noexcept
{
    return firstName + " " + lastName;
}

const std::string& Hotel::Guest::getIDDocument() const noexcept
{
    return idDocument;
}

Hotel::Result Hotel::Guest::setIDDocument(const char* document) noexcept
{
    std::string value = toLower(std::string(trim(document)));
    if (!isValidIDDocument(value))
        return Result::InvalidIdDocument;

    idDocument = std::move(value);
    return Result::Ok;
}

const std::string& Hotel::Guest::getPhone() const noexcept
{
    return phone;
}

Hotel::Result Hotel::Guest::setPhone(const char* value) noexcept
{
    const std::string_view trimmed = trim(value);
    if (!isValidPhone(trimmed))
        return Result::InvalidPhone;

    phone = trimmed;
    return Result::Ok;
}

const std::string& Hotel::Guest::getEmail() const noexcept
{
    return email;
}

Hotel::Result Hotel::Guest::setEmail(const char* value) noexcept
{
    std::string normalized = toLower(std::string(trim(value)));
    if (!isValidEmail(normalized))
        return Result::InvalidEmail;

    email = std::move(normalized);

    return Result::Ok;
}

bool Hotel::Guest::isValidName(const std::string_view name) noexcept
{
    if (name.empty() || !isValidText(name, MAX_NAME_LENGTH))
        return false;

    // Names are free-form (they may contain spaces, hyphens, apostrophes and non-ASCII letters), but they must not contain digits
    return std::ranges::none_of(
        name,
        [](const unsigned char c) noexcept -> bool
        {
            return std::isdigit(c) != 0;
        }
    );
}

bool Hotel::Guest::isValidIDDocument(std::string_view document) noexcept
{
    if (document.size() < 4 || document.size() > 20)
        return false;

    return std::ranges::all_of(
        document,
        [](const unsigned char c) noexcept -> bool
        {
            return std::isalnum(c) != 0;
        }
    );
}

bool Hotel::Guest::isValidPhone(std::string_view phone) noexcept
{
    if (phone.starts_with('+'))
        phone.remove_prefix(1);

    size_t digits = 0;
    for (const unsigned char c : phone)
    {
        if (std::isdigit(c) != 0)
            ++digits;
        else if (c != ' ' && c != '-')
            return false;
    }

    return digits >= 7 && digits <= 15;
}

// Not the most perfect function for this because the email standard is way bigger,
// but I genuinely do not have the time to implement it properly
bool Hotel::Guest::isValidEmail(const std::string_view email) noexcept
{
    if (!isValidText(email) || email.find(' ') != std::string_view::npos)
        return false;

    const size_t at = email.find('@');
    if (at == std::string_view::npos || at == 0 || email.find('@', at + 1) != std::string_view::npos)
        return false;

    const std::string_view domain = email.substr(at + 1);
    const size_t dot = domain.rfind('.');

    return  dot != std::string_view::npos &&
            dot != 0 && dot != domain.size() - 1 &&
            domain.find("..") == std::string_view::npos;
}

std::ostream& Hotel::operator<<(std::ostream& stream, const Guest& guest) noexcept
{
    return stream << "#" << guest.getID() << " " << guest.getFullName() << " (ID document: " << guest.getIDDocument()
                  << ", phone: " << guest.getPhone() << ", e-mail: " << guest.getEmail() << ")";
}
