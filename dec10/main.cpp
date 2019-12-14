
#include "../common.hpp"

namespace {

using position = std::pair<int, int>;

auto read_input = [](std::istream&& is) {
    std::vector<position> vec;
    std::string line;
    int y = 0;
    while (is >> line) {
        for (size_t x = 0; x < line.length(); x++) {
            if (line[x] == '#') {
                vec.push_back({x, y});
            }
        }
        y++;
    }
    return vec;
};

auto calculate_angle = [] (const auto& from, const auto& to) {
    return std::atan2(to.second - from.second, to.first - from.first);
};

auto find_best_position = [](const auto& positions) {

    auto num_visible = [&positions] (const auto& pos) {
        auto angles = positions
            | nano::views::filter([&pos](const auto& pos2) { return pos2 != pos; })
            | nano::views::transform([&pos](const auto& pos2) { return calculate_angle(pos, pos2); })
            | aoc::to_vector();

        nano::sort(angles);
        return nano::unique(angles) - nano::begin(angles); // NanoRange needs updating...
    };

    auto best = nano::max(positions, {}, num_visible);
    return std::pair(best, num_visible(best));
};

auto destroy_asteroids = [](auto asteroids, const auto& laser, const int n)
{
    // Some useful lambdas
    auto angle_from_laser = [&laser](const auto& pos) { return calculate_angle(laser, pos); };
    auto dist_from_laser = [&laser] (const auto& pos) {
        return std::hypot(pos.first - laser.first, pos.second - laser.second);
    };

    // remove the asteroid with the laser from the input
    asteroids.erase(nano::remove(asteroids, laser), asteroids.end());

    // sort the input by angle from the laser, then distance from it
    nano::sort(asteroids, [&] (const auto& lhs, const auto& rhs) {
        auto angle1 = angle_from_laser(lhs);
        auto angle2 = angle_from_laser(rhs);
        if (angle1 == angle2) {
            return dist_from_laser(lhs) < dist_from_laser(rhs);
        }
        return angle1 < angle2;
    });

    // find the asteroid closest to the upwards direction (-pi/2 radians)
    auto iter = nano::lower_bound(asteroids, -M_PI/2.0, {}, angle_from_laser);

    // Get the angle to it
    auto beam_angle = angle_from_laser(*iter);

    for (int i = 0; i < n - 1; i++) {
        // Kill the asteroid
        asteroids.erase(iter);

        // Find the next one on the hit list
        iter = nano::upper_bound(asteroids, beam_angle, {}, angle_from_laser);

        // Didn't find one? Let's loop round again
        if (iter == asteroids.end()) {
            beam_angle -= M_PI * 2.0;
            iter = nano::upper_bound(asteroids, beam_angle, {}, angle_from_laser);
            assert(iter != asteroids.end());
        }

        beam_angle = angle_from_laser(*iter);
    }

    return *iter;
};

}

int main(int argc, char** argv)
{
    const std::string test_data1 =
R"(.#..#
.....
#####
....#
...##)";

    const std::string test_data2 =
R"(......#.#.
#..#.#....
..#######.
.#.#.###..
.#..#.....
..#....#.#
#..#....#.
.##.#..###
##...#..#.
.#....####)";

    const std::string test_data3 =
R"(.#..##.###...#######
##.############..##.
.#.######.########.#
.###.#######.####.#.
#####.##.#.##.###.##
..#####..#.#########
####################
#.####....###.#.#.##
##.#################
#####.##.###..####..
..######..##.#######
####.##.####...##..#
.#####..#.######.###
##...#.##########...
#.##########.#######
.####.#.###.###.#.##
....##.##.###..#####
.#.#.###########.###
#.#.#.#####.####.###
###.##.####.##.#..##)";


    //const auto input = read_input(std::istringstream(test_data3));

    const auto input = read_input(std::ifstream(argv[1]));

    const auto [best, num] = find_best_position(input);

    fmt::print("Best location is ({}, {}), with a count of {} (part one)\n", best.first, best.second, num);

    auto last = destroy_asteroids(input, best, 200);
    fmt::print("200th asteroid to be destroyed: (part two): ({}, {})\n", last.first, last.second);
}