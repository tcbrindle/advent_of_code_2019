
#include "../common.hpp"

namespace {

constexpr auto calculate_fuel = [](int mass)
{
    return mass/3 - 2;
};

static_assert(calculate_fuel(12) == 2);
static_assert(calculate_fuel(14) == 2);
static_assert(calculate_fuel(1969) == 654);
static_assert(calculate_fuel(100756) == 33583);

constexpr auto calculate_total_fuel = [](int mass)
{
    int added_fuel = calculate_fuel(mass);
    int total_fuel = 0;
    while (added_fuel > 0) {
        total_fuel += added_fuel;
        added_fuel = calculate_fuel(added_fuel);
    }
    return total_fuel;
};

static_assert(calculate_total_fuel(14) == 2);
static_assert(calculate_total_fuel(1969) == 966);
static_assert(calculate_total_fuel(100756) == 50346);

}

int main(int argc, char** argv)
{
    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return -1;
    }

    const auto vec = aoc::vector_from_file<int>(argv[1]);

    fmt::print("Fuel required (pt1): {}\n",
                aoc::accumulate(vec, {}, {}, calculate_fuel));

    fmt::print("Fuel required (pt2): {}\n",
                aoc::accumulate(vec, {}, {}, calculate_total_fuel));
}