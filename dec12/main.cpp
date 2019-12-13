
#include "../common.hpp"

namespace {

struct component {
    int position;
    int velocity;

    friend constexpr bool operator==(component const& lhs, component const& rhs) {
        return lhs.position == rhs.position && lhs.velocity == rhs.velocity;
    }

    friend constexpr bool operator!=(component const& lhs, component const& rhs) {
        return !(lhs == rhs);
    }
};

constexpr int num_dimensions = 3;

using moon = std::array<component, num_dimensions>;

constexpr int num_moons = 4;

using moons = std::array<moon, num_moons>;

constexpr auto abs = [](int i) { return i < 0 ? -i : i; };

auto parse_input = [](std::istream&& input) {
    moons system;

    nano::generate(system, [&input] {
        constexpr const auto& pattern = "<x=%d, y=%d, z=%d>";
        std::string str;
        std::getline(input, str);
        moon m{};

        const auto ret = std::sscanf(str.c_str(), pattern, &m[0].position, &m[1].position, &m[2].position);
        assert(ret == 3);
        return m;
    });

    return system;
};

auto process_time_step_single = [](auto& system, int dim)
{
    for (int i = 0; i < num_moons; i++) {
        for (int j = i + 1; j < num_moons; j++) {
            auto pos1 = system[i][dim].position;
            auto pos2 = system[j][dim].position;

            auto const val = [&] {
                if (pos1 > pos2) { return 1; }
                if (pos2 > pos1) { return -1; }
                return 0;
            }();

            system[i][dim].velocity -= val;
            system[j][dim].velocity += val;
        }
    }

    for (int i = 0; i < num_moons; i++) {
        system[i][dim].position += system[i][dim].velocity;
    }
};

auto process_time_step = [](auto system)
{
    for (int i = 0; i < num_dimensions; i++) {
        process_time_step_single(system, i);
    }
    return system;
};

auto apply_time_steps = [](auto system, int steps) {
    for (int i = 0; i < steps; i++) {
        system = process_time_step(system);
    }
    return system;
};

auto print_system = [](const auto& system) {
    for (const auto& m: system) {
        fmt::print("pos=<x={}, y={}, z={}>, vel=<x={}, y={}, z={}>\n",
                   m[0].position, m[1].position, m[2].position, m[0].velocity, m[1].velocity, m[2].velocity);
    }
};

auto calculate_total_energy = [](const auto& system) {
    const auto moon_energy = [](const auto& m) {
        const auto pot = aoc::accumulate(m, {}, {}, [](auto& c) { return abs(c.position); });
        const auto kin = aoc::accumulate(m, {}, {}, [](auto& c) { return abs(c.velocity); });
        return pot * kin;
    };
    return aoc::accumulate(system, {}, {}, moon_energy);
};

auto calculate_repeat_period = [](const auto& initial, int dim)
{
    int64_t counter = 0;
    auto system = initial;
    while (true) {
        ++counter;
        process_time_step_single(system, dim);
        const auto proj = [dim](auto const& m) {
            return m[dim];
        };
        if (nano::equal(system, initial, {}, proj, proj)) {
            return counter;
        }
    }
};

constexpr const auto& test_data =
R"(<x=-1, y=0, z=2>
<x=2, y=-10, z=-7>
<x=4, y=-8, z=8>
<x=3, y=5, z=-1>)";

constexpr const auto& test_data2 =
R"(<x=-8, y=-10, z=0>
<x=5, y=5, z=10>
<x=2, y=-7, z=3>
<x=9, y=-8, z=-3>)";

}

int main(int argc, char** argv)
{
    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return 1;
    }

    const auto system = parse_input(std::ifstream(argv[1]));

    fmt::print("Total energy (part one): {}\n", calculate_total_energy(apply_time_steps(system, 1000)));

    {
        const auto repeat0 = calculate_repeat_period(system, 0);
        const auto repeat1 = calculate_repeat_period(system, 1);
        const auto repeat2 = calculate_repeat_period(system, 2);
        const auto combined = std::lcm(std::lcm(repeat0, repeat1), repeat2);
        fmt::print("Repeat period (part two): {}\n", combined);
    }
}