#include <iostream>
#include "Menu.hpp"

// Set by CMake to the Data folder in the build directory
#ifndef HOTEL_DATA_DIRECTORY
    #define HOTEL_DATA_DIRECTORY "Data"
#endif

static inline void printStorageError(const char* action, const char* preposition, const std::filesystem::path& directory, const Hotel::Result result) noexcept
{
    std::cerr << "Could not " << action << " the data " << preposition << " " << directory.string() << ": " << result.toString() << "\n";
}

int main(const int, char**)
{
    Hotel::HotelManager manager;
    // An explicit directory on the command line overrides the default one
    manager.setDataDirectory(std::filesystem::path{ HOTEL_DATA_DIRECTORY });

    std::vector<std::string> warnings;
    if (const Hotel::Result result = manager.load(warnings); result != Hotel::Result::Ok)
    {
        printStorageError("load", "from", manager.getDataDirectory(), result);
        return 1;
    }
    for (const auto& warning : warnings)
        std::cerr << "Warning: " << warning << "\n";

    Hotel::Menu menu(manager);
    menu.run();

    // Every change is already saved as it happens, this is a final safety net
    if (const Hotel::Result result = manager.save(); result != Hotel::Result::Ok)
    {
        printStorageError("save", "to", manager.getDataDirectory(), result);
        return 1;
    }
    std::cout << "Data saved to " << manager.getDataDirectory().string() << ", goodbye!\n";
    return 0;
}
