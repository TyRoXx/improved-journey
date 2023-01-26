#include <catch2/catch_test_macros.hpp>
#include <ij/direction.h>

TEST_CASE("Directions and vectors round trip", "[direction]")
{
    CHECK(ij::Direction::Up == ij::DirectionFromVector(ij::DirectionToVector(ij::Direction::Up)));
    CHECK(ij::Direction::Left == ij::DirectionFromVector(ij::DirectionToVector(ij::Direction::Left)));
    CHECK(ij::Direction::Down == ij::DirectionFromVector(ij::DirectionToVector(ij::Direction::Down)));
    CHECK(ij::Direction::Right == ij::DirectionFromVector(ij::DirectionToVector(ij::Direction::Right)));
}
