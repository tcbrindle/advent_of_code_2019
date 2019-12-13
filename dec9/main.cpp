
#include "../common.hpp"

namespace {

struct intcode {

    template <typename Program>
    constexpr explicit intcode(const Program& prog)
    {
        nano::copy(prog, memory_.begin());
    }

    template <typename In, typename Out>
    constexpr auto run(In in_fn, Out out_fn)
    {
        while (!done_) {
            process_next(in_fn, out_fn);
        }
    }

    constexpr bool done() const { return done_; }

private:
    enum class param_mode : uint8_t {
        position = 0,
        immediate = 1,
        relative = 2
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

    template <typename In, typename Out>
    constexpr void process_next(In& in_fn, Out& out_fn)
    {
        const auto [opcode, modes] = decode_instruction(memory_[iptr_]);

        auto arg = [modes = modes, this] (int64_t argnum) -> int64_t& {
            auto idx = iptr_ + argnum + 1;
            switch (modes[argnum]) {
            case param_mode::position:
                return memory_[memory_[idx]];
            case param_mode::immediate:
                return memory_[idx];
            case param_mode::relative:
                return memory_[relbase_ + memory_[idx]];
            default:
                throw std::runtime_error(fmt::format("Unknown mode kind {}\n", modes[argnum]));
            }
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
        case 9:
            relbase_ += arg(0);
            iptr_ += 2;
            break;
        case 99:
            done_ = true;
            break;
        default:
            throw std::runtime_error{fmt::format("Error, got unknown opcode {}\n", opcode)};
        }
    }

    std::array<int64_t, 4096> memory_{};
    int64_t iptr_ = 0;
    int64_t relbase_ = 0;
    bool done_ = false;
};

namespace test {

static_assert([] {
    const auto prog = std::array{109,1,204,-1,1001,100,1,100,1008,100,16,101,1006,101,0,99};

    auto in_fn = [] { return 0; };
    auto output = std::array<int, 16>{};

    auto out_fn = [&output, i = 0] (int val) mutable {
        output[i++] = val;
    };

    intcode{prog}.run(in_fn, out_fn);

    return nano::equal(output, prog);
}());

static_assert([] {
    auto prog = std::array<int64_t, 8>{1102,34915192,34915192,7,4,7,99,0};
    auto in_fn = [] { return 0; };
    int64_t output = 0;
    auto out_fn = [&output](int64_t val) { output = val; };
    intcode{prog}.run(in_fn, out_fn);
    return output >= 1'000'000'000'000'000 &&
            output <= 9'999'999'999'999'999;
}());

static_assert([] {
    auto prog = std::array<int64_t, 3>{104,1125899906842624,99};
    auto in_fn = [] { return 0; };
    int64_t output = 0;
    auto out_fn = [&output](int64_t val) { output = val; };
    intcode{prog}.run(in_fn, out_fn);
    return output == 1125899906842624;
}());

}

auto read_input = [] (const char* path) {
    std::ifstream stream(path);

    return nano::istream_view<char>(stream)
        | nano::views::split(nano::views::single(','))
        | nano::views::transform([](auto rng) {
            return std::stoll(aoc::to_string(rng));
        })
        | aoc::to_vector();
};

}

int main(int argc, char** argv)
{
    const auto prog = read_input(argv[1]);

    {
        auto in = [] { return 1; };
        std::vector<int64_t> outputs;
        auto out = [&outputs] (auto i) { outputs.push_back(i); };

        intcode{prog}.run(in, out);
        assert(outputs.size() == 1);
        fmt::print("BOOST keycode (part one): {}\n", outputs[0]);
    }

    {
        auto in = [] { return 2; };
        std::vector<int64_t> outputs;
        auto out = [&outputs] (auto i) { outputs.push_back(i); };

        intcode{prog}.run(in, out);
        assert(outputs.size() == 1);
        fmt::print("Distress signal coordinates (part two): {}\n", outputs[0]);
    }
}