#include "../common.hpp"

#include <charconv>

struct intcode {
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

public:
    template <typename Program>
    constexpr explicit intcode(const Program& prog)
    {
        nano::copy(prog, memory_.begin());
    }

    template <typename In, typename Out>
    constexpr void run(In in_fn, Out out_fn)
    {
        while (!done_) {
            process_next(in_fn, out_fn);
        }
    }

    template <typename InFn>
    constexpr int64_t next_output(InFn in_fn)
    {
        auto out_fn1 = [] (int64_t) { };

        while (!done()) {
            const auto [opcode, modes] = decode_instruction(memory_[iptr_]);
            if (opcode == 4) {
                break;
            }
            process_next(in_fn, out_fn1);
        }

        int64_t output = 0;
        auto out_fn2 = [&output] (int64_t val) { output = val; };
        process_next(in_fn, out_fn2);
        return output;

    }

    constexpr bool done() const { return done_; }

private:
    template <typename In, typename Out>
    constexpr void process_next(In& in_fn, Out& out_fn)
    {
        const auto [opcode, modes] = decode_instruction(memory_[iptr_]);

        auto arg = [modes = modes, this] (int64_t argnum) -> int64_t& {
            auto idx = iptr_ + argnum + 1;
            switch (modes[argnum]) {
            case param_mode::position:
                return memory_.at(memory_.at(idx));
            case param_mode::immediate:
                return memory_.at(idx);
            case param_mode::relative:
                return memory_.at(relbase_ + memory_.at(idx));
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

inline auto load_program(const char* path) {
    std::ifstream stream(path);

    return nano::istream_view<char>(stream)
        | nano::views::split(nano::views::single(','))
        | nano::views::transform([](auto rng) {
            return std::stoll(aoc::to_string(rng));
        })
        | aoc::to_vector();
}
