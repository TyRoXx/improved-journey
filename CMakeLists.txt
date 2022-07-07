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
    # disable the useless "warnings"
    add_definitions(/Wall /WX /wd5045 /wd4514 /wd4711 /wd4710 /wd4820 /wd5031 /wd4625 /wd4626 /wd5027 /wd4464 /wd4668 /wd4623 /wd5243 /wd4619 /wd5026 /wd4266 /wd4371 /wd4866 /wd4582 /wd5039 /wd5204 /wd4868 /wd4061)
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
