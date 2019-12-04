
#include "../common.hpp"

namespace {

constexpr auto to_digits = [](int num) {
    // Could do this with generate(), but whatever
    return std::array<int, 6>{
        num % 1'000'000 / 100'000,
        num % 100'000 / 10'000,
        num % 10'000 / 1'000,
        num % 1'000 / 100,
        num % 100 / 10,
        num % 10
    };
};

constexpr auto is_possible_password1 = [] (int num) {
    const auto digits = to_digits(num);
    return nano::is_sorted(digits) && nano::adjacent_find(digits) != digits.end();
};

constexpr auto is_possible_password2 = [] (int num) {
    const auto digits = to_digits(num);
    return nano::is_sorted(digits) && nano::any_of(digits, [&digits] (int d) {
        return nano::count(digits, d) == 2;
    });
};

// Tests from the problem description
static_assert(is_possible_password1(111111));
static_assert(!is_possible_password1(223450));
static_assert(!is_possible_password1(123789));

static_assert(is_possible_password2(112233));
static_assert(!is_possible_password2(123444));
static_assert(is_possible_password2(111122));

}

int main()
{
    // These are my personal input values
    constexpr int start = 236491;
    constexpr int end = 713787;

    fmt::print("Got {} possible passwords (part one)\n",
               nano::count_if(nano::views::iota(start, end), is_possible_password1));

    fmt::print("Got {} possible passwords (part two)\n",
               nano::count_if(nano::views::iota(start, end), is_possible_password2));
}