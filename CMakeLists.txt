cmake_minimum_required(VERSION 3.15)
project(spm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Silence the CMP0135 warning for URL downloads in modern CMake
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)

FetchContent_Declare(
    CLI11
    URL https://github.com/CLIUtils/CLI11/archive/refs/tags/v2.4.2.tar.gz
)
FetchContent_MakeAvailable(CLI11)

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(sqlite3
    GIT_REPOSITORY https://github.com/sjinks/sqlite3-cmake
    GIT_TAG v3.49.1
)
FetchContent_MakeAvailable(sqlite3)

add_executable(spm
    src/main.cpp
    src/downloader.cpp
    src/database.cpp
)

target_include_directories(spm PRIVATE include)

target_link_libraries(spm PRIVATE 
    CLI11::CLI11 
    nlohmann_json
    SQLite::SQLite3
)