
#include "../common.hpp"

namespace {

template <typename Memory>
struct intcode {

    constexpr explicit intcode(const Memory& memory)
        : memory_(memory)
    {}

    template <typename In, typename Out>
    constexpr auto run(In in_fn, Out out_fn)
    {
        while (!done_) {
            process_next(in_fn, out_fn);
        }
    }

    constexpr void run_until_input(int input)
    {
        if (done_) {
            return;
        }

        bool input_read = false;

        auto in_fn = [&input_read, input] { input_read = true; return input; };
        auto out_fn = [](int) {};

        while (!input_read) {
            process_next(in_fn, out_fn);
        }
    }

    constexpr int run_until_output(int input)
    {
        if (done_) {
            return input;
        }

        auto in_fn = [input] { return input; };
        bool have_output = false;
        int output = 0;

        auto out_fn = [&output, &have_output](int val) {
            output = val;
            have_output = true;
        };

        while(!done_ && !have_output) {
            process_next(in_fn, out_fn);
        }

        return have_output ? output : input;
    }

    constexpr bool done() const { return done_; }

private:
    template <typename In, typename Out>
    constexpr void process_next(In& in_fn, Out& out_fn)
    {
        const auto [opcode, modes] = decode_instruction(memory_[iptr_]);

        auto arg = [this, modes = modes] (int argnum) -> int& {
            auto idx = iptr_ + argnum + 1;
            return modes[argnum] == param_mode::position ? memory_[memory_[idx]] : memory_[idx];
        };

        switch (opcode) {
        case 1:
            arg(2) = arg(0) + arg(1);
            iptr_ += 4;
            break;
        case 2:
            arg(2) = arg(0) * arg(1);
            iptr_ += 4;
            break;
        case 3:
            arg(0) = in_fn();
            iptr_ += 2;
            break;
        case 4:
            out_fn(arg(0));
            iptr_ += 2;
            break;
        case 5:
            iptr_ = arg(0) != 0 ? arg(1) : iptr_ + 3;
            break;
        case 6:
            iptr_ = arg(0) == 0 ? arg(1) : iptr_ + 3;
            break;
        case 7:
            arg(2) = arg(0) < arg(1) ? 1 : 0;
            iptr_ += 4;
            break;
        case 8:
            arg(2) = arg(0) == arg(1) ? 1 : 0;
            iptr_ += 4;
            break;
        case 99:
            done_ = true;
            break;
        default:
            throw std::runtime_error{fmt::format("Error, got unknown opcode {}\n", opcode)};
        }
    }

    enum class param_mode : bool {
        position,
        immediate
    };

    static constexpr auto decode_instruction(int in) {
        int opcode = in % 100;
        in /= 100;

        std::array<param_mode, 3> modes{};
        for (int i = 0; i < 3; i++) {
            modes[i] = static_cast<param_mode>(in % 10);
            in /= 10;
        }
        return std::pair(opcode, modes);
    };

    Memory memory_;
    int iptr_ = 0;
    bool done_ = false;
};

constexpr auto run_amplifiers = [](const auto& prog, auto phases)
{
    auto next_input = 0;

    for (auto i : nano::views::iota(0, 5)) {
        auto inputs = std::array{phases[i], next_input};
        auto in_fn = [inputs, n = 0] () mutable {
            return inputs.at(n++);
        };
        auto out_fn = [&next_input] (int i) {
            next_input = i;
        };

        auto vm = intcode{prog};
        vm.run(in_fn, out_fn);
    }

    return next_input;
};

constexpr auto run_all_permutations = [](const auto& prog)
{
    auto phases = std::array{0, 1, 2, 3, 4};

    int max_signal = 0;

    do {
        auto sig = run_amplifiers(prog, phases);
        max_signal = nano::max(sig, max_signal);
    } while (nano::next_permutation(phases).found);

    return max_signal;
};

constexpr auto run_amps_with_feedback = [](const auto& prog, const auto phases)
{
    auto vms = std::array{intcode{prog}, intcode{prog}, intcode{prog}, intcode{prog}, intcode{prog}};

    // Initial setup
    for (int i = 0; i < 5; i++) {
        vms[i].run_until_input(phases[i]);
    }

    // Run loop
    int next_input = 0;
    int next_vm = 0;

    while (!vms.back().done()) {
        next_input = vms[next_vm].run_until_output(next_input);
        next_vm = (next_vm + 1) % 5;
    }

    return next_input;
};

constexpr auto run_all_permutations_with_feedback = [](const auto& prog)
{
    auto phases = std::array{5, 6, 7, 8, 9};

    int max_signal = 0;

    do {
        auto sig = run_amps_with_feedback(prog, phases);
        max_signal = nano::max(sig, max_signal);
    } while (nano::next_permutation(phases).found);

    return max_signal;
};

namespace test {

constexpr auto prog1 = std::array{3,15,3,16,1002,16,10,16,1,16,15,15,4,15,99,0,0};
static_assert(run_all_permutations(prog1) == 43210);

constexpr auto prog2 = std::array{3,23,3,24,1002,24,10,24,1002,23,-1,23,
101,5,23,23,1,24,23,23,4,23,99,0,0};
static_assert(run_all_permutations(prog2) == 54321);

constexpr auto prog3 = std::array{3,31,3,32,1002,32,10,32,1001,31,-2,31,1007,31,0,33,
1002,33,7,33,1,33,31,31,1,32,31,31,4,31,99,0,0,0};
static_assert(run_all_permutations(prog3) == 65210);

constexpr auto prog4 = std::array{3,26,1001,26,-4,26,3,27,1002,27,2,27,1,27,26,
27,4,27,1001,28,-1,28,1005,28,6,99,0,0,5};
static_assert(run_amps_with_feedback(prog4, std::array{9, 8, 7, 6, 5}) == 139629729);

constexpr auto prog5 = std::array{3,52,1001,52,-5,52,3,53,1,52,56,54,1007,54,5,55,1005,55,26,1001,54,
-5,54,1105,1,12,1,53,54,53,1008,54,0,55,1001,55,1,55,2,53,55,53,4,
53,1001,56,-1,56,1005,56,6,99,0,0,0,0,10};
static_assert(run_amps_with_feedback(prog5, std::array{9, 7, 8, 5, 6}) == 18216);

}

auto read_input = [] (const char* path) {
    std::ifstream stream(path);

    return nano::istream_view<char>(stream)
        | nano::views::split(nano::views::single(','))
        | nano::views::transform([](auto rng) {
            return std::stoi(aoc::to_string(rng));
        })
        | aoc::to_vector();
};

}

int main(int argc, char** argv)
{
    const auto prog = read_input(argv[1]);

    fmt::print("Highest signal (part one): {}\n", run_all_permutations(prog));

    fmt::print("Highest signal with feedback (part two): {}\n", run_all_permutations_with_feedback(prog));
}