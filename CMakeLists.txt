cmake_minimum_required(VERSION 3.22)
project(improved_journey LANGUAGES CXX)

if(MSVC)
    set(VCPKG_TARGET_TRIPLET x64-windows-static)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
include(vcpkg/scripts/buildsystems/vcpkg.cmake)

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS graphics REQUIRED)
find_package(Catch2 REQUIRED)
find_package(imgui REQUIRED)
find_package(ImGui-SFML REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

if(UNIX)
    add_definitions(
        -Wall
        -Wextra
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wrestrict
        -Wnull-dereference
        -Wdouble-promotion
        #-Wshadow
        -fstrict-aliasing
        -Wformat=2
        -Wuseless-cast
        -pedantic
        -Werror
        -Wconversion
        -Wunused
        -Wundef
        -Wvla
        -Wswitch-enum
        -Wuninitialized
        -Wstrict-overflow=5
        -std=c++20
    )
endif()

if(MSVC)
    add_definitions(/std:c++20)
    add_definitions(/WX)
    add_definitions(/Wall)
    # disable the useless "warnings"
    add_definitions(/wd4061)
    add_definitions(/wd4514)
    add_definitions(/wd4582)
    add_definitions(/wd4623)
    add_definitions(/wd4625)
    add_definitions(/wd4626)
    add_definitions(/wd4668)
    add_definitions(/wd4820)
    add_definitions(/wd5027)
    add_definitions(/wd5045)
    add_definitions(/wd5026)
    add_definitions(/wd5204)
    # left-to-right evaluation order
    add_definitions(/wd4868)
endif()

find_program(FO_CLANG_FORMAT NAMES clang-format PATHS "C:/Program Files/LLVM/bin")
if(FO_CLANG_FORMAT)
    file(GLOB_RECURSE formatted
        benchmarks/**.cpp benchmarks/**.h
        tests/**.cpp tests/**.h
        game/**.cpp game/**.h
    )
    add_custom_target(clang-format COMMAND "${FO_CLANG_FORMAT}" -i ${formatted} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

include_directories(.)

add_subdirectory(game)
add_subdirectory(tests)
