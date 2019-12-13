
#include "intcode.hpp"

namespace {

using position = std::pair<int, int>;

enum class colour { black = 0, white = 1};

enum class direction { north, south, east, west };

direction turn_right(direction d) {
    switch (d) {
    case direction::north:
        return direction::east;
    case direction::east:
        return direction::south;
    case direction::south:
        return direction::west;
    case direction::west:
        return direction::north;
    }
}

direction turn_left(direction d) {
    switch (d) {
    case direction::north:
        return direction::west;
    case direction::east:
        return direction::north;
    case direction::south:
        return direction::east;
    case direction::west:
        return direction::south;
    }
}

auto update_position = [](position old_pos, direction dir) -> position
{
    switch (dir) {
    case direction::north:
        return {old_pos.first, old_pos.second + 1};
    case direction::east:
        return {old_pos.first + 1, old_pos.second};
    case direction::south:
        return {old_pos.first, old_pos.second - 1};
    case direction::west:
        return {old_pos.first - 1, old_pos.second};
    }
};

auto paint_hull = [](const auto& prog, colour start_colour = colour::black)
{
    std::map<position, colour> panels;
    position current_pos{0, 0};
    direction current_dir = direction::north;
    panels[current_pos] = start_colour;

    auto vm = intcode{prog};

    auto get_colour = [&] {
        auto col = colour::black;
        auto it = panels.find(current_pos);
        if (it != panels.end()) {
            col = it->second;
        }
        return static_cast<int64_t>(col);
    };

    while (!vm.done()) {
        {
            auto col = vm.next_output(get_colour);
            panels[current_pos] = static_cast<colour>(col);
        }

        {
            auto dir = vm.next_output(get_colour);
            if (dir == 1) {
                current_dir = turn_right(current_dir);
            } else if (dir == 0 ) {
                current_dir = turn_left(current_dir);
            } else {
                throw std::runtime_error(fmt::format("Expected direction, got {}\n", dir));
            }
            current_pos = update_position(current_pos, current_dir);
        }
    }

    return panels;
};

auto print_map = [](auto const& map) {
    auto [min_x, max_x] = nano::minmax(map | nano::views::keys | nano::views::transform(&position::first));
    auto [min_y, max_y] = nano::minmax(map | nano::views::keys | nano::views::transform(&position::second));


    for (auto j = max_y; j >= min_y; j--) {
        for (auto i = min_x; i < max_x + 1; i++) {
            auto it = map.find({i, j});
            if (it != map.end() && it->second == colour::white) {
                fmt::print("#");
            } else {
                fmt::print(" ");
            }
        }
        fmt::print("\n");
    }
};

}

int main(int argc, char** argv)
{
    const auto prog = load_program(argv[1]);

    {
        const auto map = paint_hull(prog);
        fmt::print("Number of panels painted (part one): {}\n", map.size());
    }

    {
        const auto map = paint_hull(prog, colour::white);
        fmt::print("Part two:\n");
        print_map(map);
    }
}