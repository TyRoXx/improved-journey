#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <ij/Direction.h>

TEST_CASE("Directions and vectors round trip", "[direction]")
{
    const ij::Direction direction =
        GENERATE(ij::Direction::Up, ij::Direction::Left, ij::Direction::Down, ij::Direction::Right);
    CHECK(direction == ij::DirectionFromVector(ij::DirectionToVector(direction)));
}
