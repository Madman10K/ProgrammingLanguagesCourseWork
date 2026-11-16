cmake_minimum_required(VERSION 3.21)

project(HotelManager VERSION 1.0.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(.)

if (MSVC)
    set(HOTEL_WARNINGS /W4 /permissive-)
else()
    set(HOTEL_WARNINGS -Wall -Wextra -Wpedantic -Wshadow)
endif()

add_executable(HotelManager
        Common.cpp Common.hpp Utility.cpp Utility.hpp Date.cpp Date.hpp Room.cpp Room.hpp Guest.cpp Guest.hpp
        Reservation.cpp Reservation.hpp Invoice.cpp Invoice.hpp Report.cpp Report.hpp Storage.cpp Storage.hpp
        HotelManager.cpp HotelManager.hpp
        Application/main.cpp Application/Menu.cpp Application/Menu.hpp
        Application/RoomsMenu.cpp Application/GuestsMenu.cpp Application/ReservationsMenu.cpp
        Application/ReportsMenu.cpp Application/InvoicesMenu.cpp
        Application/MenuInput.cpp Application/MenuPrinting.cpp
)
target_compile_options(HotelManager PRIVATE ${HOTEL_WARNINGS})
# The application uses the data folder in the build directory by default
target_compile_definitions(HotelManager PRIVATE HOTEL_DATA_DIRECTORY="${PROJECT_BINARY_DIR}/Data")
set_target_properties(HotelManager PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# Copy example data
if (NOT EXISTS "${PROJECT_BINARY_DIR}/Data")
    file(COPY "${PROJECT_SOURCE_DIR}/Data" DESTINATION "${PROJECT_BINARY_DIR}")
endif()
