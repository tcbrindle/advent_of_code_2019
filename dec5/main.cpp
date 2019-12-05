
#include "../common.hpp"

namespace {

enum class param_mode : bool {
    position,
    immediate
};

constexpr auto decode_instruction = [](int in) {
    int opcode = in % 100;
    in /= 100;

    std::array<param_mode, 3> modes{};
    for (int i = 0; i < 3; i++) {
        modes[i] = static_cast<param_mode>(in % 10);
        in /= 10;
    }
    return std::pair(opcode, modes);
};

constexpr auto run_program = [](auto state, int in, auto out_fn) {
    auto iptr = state.data();

    while (true) {
        const auto [opcode, modes] = decode_instruction(*iptr);
        /* sigh, can't capture structured bindings in C++17... */
        auto& modes_ref = modes;

        auto arg = [&modes_ref, iptr, &state] (int argnum) -> int& {
            return modes_ref[argnum] == param_mode::position ? state[iptr[argnum + 1]] : iptr[argnum + 1];
        };

        switch (opcode) {
        case 1:
            arg(2) = arg(0) + arg(1);
            iptr += 4;
            break;
        case 2:
            arg(2) = arg(0) * arg(1);
            iptr += 4;
            break;
        case 3:
            arg(0) = in;
            iptr += 2;
            break;
        case 4:
            out_fn(arg(0));
            iptr += 2;
            break;
        case 5:
            iptr = arg(0) != 0 ? state.data() + arg(1) : iptr + 3;
            break;
        case 6:
            iptr = arg(0) == 0 ? state.data() + arg(1) : iptr + 3;
            break;
        case 7:
            arg(2) = arg(0) < arg(1) ? 1 : 0;
            iptr += 4;
            break;
        case 8:
            arg(2) = arg(0) == arg(1) ? 1 : 0;
            iptr += 4;
            break;
        case 99:
            return state;
        default:
            fmt::print(stderr, "Error, got unknown opcode {}\n", opcode);
            return state;
        }
    }
};

// Test program for part 2, from the problem description
namespace test {

constexpr auto prog = std::array{
    3,21,1008,21,8,20,1005,20,22,107,8,21,20,1006,20,31,
    1106,0,36,98,0,0,1002,21,125,20,4,20,1105,1,46,104,
    999,1105,1,46,1101,1000,1,20,4,20,1105,1,46,98,99};

constexpr auto run = [](auto& prog, int arg) {
    int out = 0;
    run_program(prog, arg, [&out] (int i) { out = i; });
    return out;
};

static_assert(run(prog, 7) == 999);
static_assert(run(prog, 8) == 1000);
static_assert(run(prog, 9) == 1001);

}

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

    auto collect_output = [] (auto& prog, int in_arg) {
        std::vector<int> out;
        run_program(prog, in_arg, [&out](int i) { out.push_back(i); });
        return out;
    };

    {
        // Part one
        const auto out = collect_output(in, 1);

        if (nano::any_of(out.begin(), nano::prev(out.end()), [] (int i) { return i != 0; })) {
            fmt::print(stderr, "Boo, got test failure (part one) :-(\n");
            return -1;
        }
        fmt::print("Status code (part one): {}\n", out.back());
    }

    {
        // Part two
        const auto out = collect_output(in, 5);

        if (out.size() != 1) {
            fmt::print(stderr, "Boo, got more than one output (part two)\n");
            return -1;
        }
        fmt::print("Status code (part two): {}\n", out[0]);
    }
}