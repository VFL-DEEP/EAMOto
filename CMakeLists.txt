cmake_minimum_required(VERSION 3.14)
project(EAMOto VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find dependencies
find_package(CURL REQUIRED)
find_package(SQLite3 REQUIRED)

# nlohmann_json is often header-only. If find_package fails, we could use FetchContent, but we will try find_package first.
find_package(nlohmann_json QUIET)

# Include directories
include_directories(include)
include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES
    src/main.cpp
    src/cookie.cpp
    src/auth.cpp
    src/config.cpp
    src/utils.cpp
)

add_executable(eamoto ${SOURCES})

target_link_libraries(eamoto PRIVATE
    CURL::libcurl
    SQLite::SQLite3
)

if(nlohmann_json_FOUND)
    target_link_libraries(eamoto PRIVATE nlohmann_json::nlohmann_json)
else()
    message(STATUS "nlohmann_json not found via find_package, assuming header is in include path or system path")
endif()
