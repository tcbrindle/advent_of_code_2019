
#include "intcode.hpp"

#include <thread>
#include <curses.h>

namespace {

enum class tile_type {
    empty,
    wall,
    block,
    paddle,
    ball
};

auto to_char = [](tile_type t) -> chtype {
    switch (t) {
    case tile_type::wall: return '#';
    case tile_type::block: return '*';
    case tile_type::paddle: return '=';
    case tile_type::ball : return 'o';
    default: return ' ';
    }
};

// Part one
auto count_blocks = [](auto const& prog) {
    auto vm = intcode(prog);
    int num_blocks = 0;

    while (!vm.done()) {
        const auto in_fn = [] { return 0; };
        vm.next_output(in_fn); // x coord
        vm.next_output(in_fn); // y coord
        const auto b = vm.next_output(in_fn);

        if (b == 2) {
            ++num_blocks;
        }
    }

    return num_blocks;
};

enum class play_mode {
    manual,
    cpu
};

struct curses_display {

    explicit curses_display()
    {
        // Initialise curses... I have no idea what most of this does
        initscr();
        cbreak();
        noecho();
        nonl();
        keypad(stdscr, TRUE);
        curs_set(0);
    }

    ~curses_display()
    {
        endwin();
    }

    void print_score(int score)
    {
        mvprintw(0, 0, fmt::format("Score: {}", score).c_str());
        refresh();
    }

    void print_tile(int x, int y, tile_type t)
    {
        mvaddch(y + 1, x, to_char(t));
        refresh();
    }
};

auto run_game = [](auto prog, play_mode mode) {
    prog[0] = 2;
    auto vm = intcode{prog};

    curses_display display{};

    int64_t high_score = 0;
    int64_t ball_x = 0;
    int64_t paddle_x = 0;

    const auto keyboard_input = [] {
        auto c = getch();
        if (c == KEY_LEFT) {
            return -1;
        } else if (c == KEY_RIGHT) {
            return 1;
        } else {
            return 0;
        }
    };

    const auto cpu_input = [&ball_x, &paddle_x] {
        // Slow things down a bit so we can watch
        std::this_thread::sleep_for(std::chrono::milliseconds{17});
        const auto diff = ball_x - paddle_x;
        return diff > 0 ? 1 : diff < 0 ? -1 : 0;
    };

    const auto input_fn = [&] () -> std::function<int()> {
        if (mode == play_mode::manual) {
            return keyboard_input;
        } else {
            return cpu_input;
        }
    }();

    while (!vm.done()) {
        const auto x = vm.next_output(input_fn);
        const auto y = vm.next_output(input_fn);
        const auto z = vm.next_output(input_fn);

        if (x == -1 && y == 0) {
            high_score = nano::max(high_score, z);
            display.print_score(z);
            continue;
        }

        const auto t = static_cast<tile_type>(z);

        if (t == tile_type::paddle) {
            paddle_x= x;
        }

        if (t == tile_type::ball) {
            ball_x = x;
        }

        display.print_tile(x, y, t);
    }

    return high_score;
};

}

int main(int argc, char** argv)
{
    if (argc < 2) {
        fmt::print(stderr, "No input!\n");
        return 1;
    }

    const play_mode mode = [&] {
        if (argc > 2 && argv[2] == std::string_view{"manual"}) {
            return play_mode::manual;
        }
        return play_mode::cpu;
    }();

    const auto prog = load_program(argv[1]);

    fmt::print("Number of block tiles (part one): {}\n", count_blocks(prog));

    auto score = run_game(prog, mode);
    fmt::print("Game over! score was {}\n", score);
}