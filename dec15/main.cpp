
#include "intcode.hpp"

#include "../extern/fmt/ostream.h"

#define PPCAT_IMPL(x, y) x ## y
#define PPCAT(x, y) PPCAT_IMPL(x, y)
#define _ PPCAT(unused, __COUNTER__)

namespace {

enum class direction {
    north = 1,
    south = 2,
    west = 3,
    east = 4
};

auto directions = [] {
    return nano::views::iota(1, 5)
        | nano::views::transform([](int i) {
            return static_cast<direction>(i);
        });
};

direction reverse_dir(direction d)
{
    switch (d) {
    case direction::north: return direction::south;
    case direction::east: return direction::west;
    case direction::south: return direction::north;
    case direction::west: return direction::east;
    }
}

enum class tile_type {
    unknown = -1,
    wall = 0,
    empty = 1,
    goal = 2,
    oxygen = 3
};

using position = std::pair<int, int>;

auto adjacent = [](auto const& pos, direction dir) -> position
{
    switch (dir) {
    case direction::north:  return {pos.first, pos.second + 1};
    case direction::south: return {pos.first, pos.second - 1};
    case direction::west: return {pos.first - 1, pos.second};
    case direction::east: return {pos.first + 1, pos.second};
    }
};

auto print_map = [](const auto& map) {
    auto [min_x, max_x] = nano::minmax(map | nano::views::keys | nano::views::transform(&position::first));
    auto [min_y, max_y] = nano::minmax(map | nano::views::keys | nano::views::transform(&position::second));

     for (auto j : nano::views::reverse(nano::views::iota(min_y, max_y + 1))) {
        for (auto i : nano::views::iota(min_x, max_x + 1)) {
            char c = ' ';
            if (auto it = map.find({i, j}); it != map.end()) {
                switch (it->second) {
                case tile_type::unknown: c = ' '; break;
                case tile_type::wall: c = '#'; break;
                case tile_type::empty: c ='.'; break;
                case tile_type::goal: c = 'G'; break;
                case tile_type::oxygen: c = 'O'; break;
                }
            }
            fmt::print("{}", c);
        }
        fmt::print("\n");
    }
};

enum class mode {
    solve,
    map
};

auto map_room = [](const auto& prog, mode mode = mode::solve)
{
    auto vm = intcode{prog};

    std::map<position, tile_type> map{{{0, 0}, tile_type::empty}};
    std::vector<direction> path;
    position pos{0, 0};
    direction dir = direction::north;

    while (true) {
        auto surrounding_unknowns = nano::views::filter(directions(), [&](auto d) {
            return map.find(adjacent(pos, d)) == map.end();
        });

        if (!surrounding_unknowns.empty()) {
            dir = *surrounding_unknowns.begin();
            path.push_back(dir);
        } else {
            if (path.empty()) {
                break;
            }
            dir = reverse_dir(path.back());
            path.pop_back();
        }

        auto in_fn = [&] { return static_cast<int64_t>(dir); };

        auto tile = static_cast<tile_type>(vm.next_output(in_fn));

        if (tile == tile_type::wall) {
            path.pop_back();
            map[adjacent(pos, dir)] = tile;
            continue;
        }

        pos = adjacent(pos, dir);
        map[pos] = tile;

        if (tile == tile_type::goal && mode == mode::solve) {
            break;
        }
    }

    return std::tuple(map, path.size(), pos);
};

auto flood_oxygen = [](auto map, const position& start) {

    auto empty_count = [&map] {
        return nano::count(nano::views::values(map), tile_type::empty);
    };

    map[start] = tile_type::oxygen;
    std::vector<position> empties;
    int counter = 0;

    while (empty_count() > 0) {
        for (const auto& [pos, type] : map) {
            if (type != tile_type::oxygen) {
                continue;
            }

            for (auto d : directions()) {
                if (auto it = map.find(adjacent(pos, d)); it != map.end() && it->second == tile_type::empty) {
                    empties.push_back(it->first);
                }
            }
        }

        for (auto e : empties) {
            map[e] = tile_type::oxygen;
        }

        empties.clear();
        ++counter;
    }

    return counter;
};

}

int main(int argc, char** argv)
{
    if (argc < 2) {
        fmt::print(stderr, "No input!\n");
        return 1;
    }

    const auto prog = load_program(argv[1]);

    auto [_, n_steps, goal_pos] = map_room(prog);

    fmt::print("Took {} steps to reach the goal (part one)\n", n_steps);

    auto [map, _,  _] = map_room(prog, mode::map);

    print_map(map);

    auto mins = flood_oxygen(map, goal_pos);

    fmt::print("Took {} minutes to fill the room with oxygen\n", mins);
}