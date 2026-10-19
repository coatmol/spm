cmake_minimum_required(VERSION 3.15)
project(spm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

add_executable(spm
    src/main.cpp
    # src/cli.cpp
    # src/downloader.cpp
)

target_include_directories(spm PRIVATE include)

target_link_libraries(spm PRIVATE 
    CLI11::CLI11 
    nlohmann_json
)