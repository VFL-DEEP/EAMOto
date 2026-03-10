cmake_minimum_required(VERSION 3.14)
project(EAMOto VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(nlohmann_json QUIET)

include_directories(${CMAKE_SOURCE_DIR})

set(SOURCES
    main.cpp
    cookie.cpp
    auth.cpp
    config.cpp
    utils.cpp
)

add_executable(eamoto ${SOURCES})

target_link_libraries(eamoto PRIVATE
    CURL::libcurl
    SQLite::SQLite3
)

if(nlohmann_json_FOUND)
    target_link_libraries(eamoto PRIVATE nlohmann_json::nlohmann_json)
endif()
