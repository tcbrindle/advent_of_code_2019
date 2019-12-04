
#include "../common.hpp"

namespace {

struct position {
    int x;
    int y;

    friend constexpr bool operator==(position lhs, position rhs) {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }
    friend constexpr bool operator!=(position lhs, position rhs) { return !(lhs == rhs); }

    friend constexpr bool operator<(position lhs, position rhs) {
        return std::tie(lhs.x, lhs.y) < std::tie(rhs.x, rhs.y);
    }

    friend constexpr bool operator>(position lhs, position rhs) { return rhs < lhs; }
    friend constexpr bool operator<=(position lhs, position rhs) { return !(lhs > rhs); }
    friend constexpr bool operator>=(position lhs, position rhs) { return !(lhs < rhs); }
};

auto dist = [](position p)
{
    return abs(p.x) + abs(p.y);
};

auto parse_wire = [](std::string_view input)
{
    std::istringstream iss{std::string(input)};
    std::vector<position> wire;
    position last_position{};

    char dir;
    int dist;

    while (iss >> dir >> dist) {
        switch (dir) {
        case 'U':
            for (int i = 0; i < dist; i++) {
                last_position.y += 1;
                wire.push_back(last_position);
            }
            break;
        case 'D':
            for (int i = 0; i < dist; i++) {
                last_position.y -= 1;
                wire.push_back(last_position);
            }
            break;
        case 'R':
            for (int i = 0; i < dist; i++) {
                last_position.x += 1;
                wire.push_back(last_position);
            }
            break;
        case 'L':
            for (int i = 0; i < dist; i++) {
                last_position.x -= 1;
                wire.push_back(last_position);
            }
            break;
        default:
            throw std::runtime_error(fmt::format("Unknown direction {} when parsing", dir));
        }

        char c;
        iss >> c;
    }

    return wire;
};

auto parse_input = [](std::string_view input)
{
    auto newline_pos = input.find('\n');
    auto wire1 = parse_wire(input.substr(0, newline_pos));
    auto wire2 = parse_wire(input.substr(newline_pos + 1));
    return std::pair(std::move(wire1), std::move(wire2));
};

auto get_intersections = [](auto const& wires) {
    auto [wire1, wire2] = wires;
    nano::sort(wire1);
    nano::sort(wire2);
    std::vector<position> out;
    nano::set_intersection(wire1, wire2, nano::back_inserter(out));
    return out;
};

auto get_steps_to_point = [](auto const& wire, auto const& pos) {
    return nano::distance(wire.begin(), nano::find(wire, pos)) + 1; // Need to count the final step too
};

}

int main(int argc, char** argv)
{
#if TESTING
    constexpr std::string_view input =
    R"(R75,D30,R83,U83,L12,D49,R71,U7,L72
U62,R66,U55,R34,D71,R55,D58,R83)";
#else
    if (argc < 2) {
        fmt::print(stderr, "No input");
        return -1;
    }

    const std::string input = [&] {
        std::ifstream is(argv[1]);
        return std::string(nano::istreambuf_iterator<char>(is),
                           nano::istreambuf_iterator<char>{});
    }();
#endif

    const auto wires = parse_input(input);
    const auto intersections = get_intersections(wires);
    if (intersections.empty()) {
        fmt::print(stderr, "Oh no, no intersections!\n");
        return 1;
    }

    fmt::print("Minimum distance to an intersection (part one): {}\n",
               nano::min(intersections | nano::views::transform(dist)));

    const auto min_steps = nano::min(intersections |
        nano::views::transform([&wires] (const auto& pos) {
            return get_steps_to_point(wires.first, pos) + get_steps_to_point(wires.second, pos);
        }));

    fmt::print("Minimum totals steps to an intersection (part two): {}\n", min_steps);
}