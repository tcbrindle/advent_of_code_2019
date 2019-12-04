
#include "../common.hpp"

namespace {

constexpr auto run_program = [](auto state) {
    std::size_t pc = 0;

    auto add = [&state] (int arg0, int arg1, int res) { state[res] = state[arg0] + state[arg1]; };
    auto mult = [&state] (int arg0, int arg1, int res) { state[res] = state[arg0] * state[arg1]; };

    while (true) {
        const int opcode = state[pc];
        switch (opcode) {
        case 1: {
            add(state[pc + 1], state[pc + 2], state[pc + 3]);
            pc += 4;
            break;
        }
        case 2:
            mult(state[pc + 1], state[pc + 2], state[pc + 3]);
            pc += 4;
            break;
        case 99:
            return state;
        default:
            fmt::print("Error, got opcode {}\n", state[pc]);
            return state;
        }
    }
};

// Grr, std::array op== is not constexpr in C++17
static_assert(nano::equal(run_program(std::array{1,0,0,0,99}), std::array{2,0,0,0,99}));
static_assert(nano::equal(run_program(std::array{2,3,0,3,99}), std::array{2,3,0,6,99}));
static_assert(nano::equal(run_program(std::array{2,4,4,5,99,0}), std::array{2,4,4,5,99,9801}));
static_assert(nano::equal(run_program(std::array{1,1,1,4,99,5,6,0,99}),
                          std::array{30,1,1,4,2,5,6,0,99}));
static_assert(nano::equal(run_program(std::array{1,9,10,3,2,3,11,0,99,30,40,50}),
                          std::array{3500,9,10,70,2,3,11,0,99,30,40,50}));

constexpr auto run_program_with = [](auto state, int noun, int verb)
{
    state[1] = noun;
    state[2] = verb;
    return run_program(std::move(state));
};

auto read_input = [] (const char* path) {
    std::ifstream stream(path);
    std::vector<int> vec;
    int i;
    while (stream >> i) {
        vec.push_back(i);
        char c; // eat the comma
        stream >> c;
    }
    return vec;
};

}

int main(int argc, char** argv)
{
    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return -1;
    }

    const auto in = read_input(argv[1]);

    fmt::print("Part one: {}\n", run_program_with(in, 12, 2)[0]);

    constexpr int target = 19690720; // from problem description

    for (int i : nano::views::iota(0, 100)) {
        for (int j : nano::views::iota(0, 100)) {
            if (run_program_with(in, i, j)[0] == target) {
                fmt::print("Part two: {}\n", 100 * i + j);
                return 0;
            }
        }
    }
    fmt::print("Error, could not find part2 solution\n");
}