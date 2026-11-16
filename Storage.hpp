#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include "Utility.hpp"

namespace Hotel
{
    class HotelManager;

    class Storage
    {
    public:
        static Result load(const std::filesystem::path& directory, HotelManager& manager, std::vector<std::string>& warnings) noexcept;
        static Result save(const std::filesystem::path& directory, const HotelManager& manager) noexcept;
    };
}
