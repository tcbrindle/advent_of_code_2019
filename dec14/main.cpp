
#include "../common.hpp"

#include <deque>

using namespace std::string_view_literals;

namespace {

struct ingredient {
    int amount;
    std::string name;
};

struct recipe {
    std::vector<ingredient> ingredients;
    ingredient output;
};

auto parse_input = [](auto input) {

    auto make_ingredient = [](auto chars) {
        auto view = nano::views::split(chars, ' ');
        auto it = nano::begin(view);
        auto amt = std::stoi(aoc::to_string(*it++));
        auto name = aoc::to_string(*it);
        return ingredient{amt, std::move(name)};
    };

    auto make_recipe = [&](auto line) {
        auto halves = nano::views::split(line, " => "sv);

        auto it = nano::begin(halves);
        auto ingredients = *it++
            | nano::views::split(", "sv)
            | nano::views::transform(make_ingredient)
            | aoc::to_vector();

        auto output = make_ingredient(*it);

        return recipe{std::move(ingredients), std::move(output)};
    };

    auto vec = input
        | nano::views::split('\n')
        | nano::views::transform(make_recipe)
        | aoc::to_vector();

    // sort the recipes by name so we can binary search them later
    nano::sort(vec, {}, [](const auto& r) { return r.output.name; });

    return vec;
};

auto find_ore_requirement = [](auto const& recipes, int64_t fuel_amount) {

    int64_t ore_count = 0;

    std::map<std::string, int64_t> stock;
    std::vector<std::pair<std::string, int64_t>> queue{{"FUEL", fuel_amount}};

    while (!queue.empty()) {

        auto [req_name, req_amount] = queue.back();
        queue.pop_back();

        if (req_name == "ORE") {
            ore_count += req_amount;
            continue;
        }

        // Do we have this ingredient in stock? If so, use what we can
        if (auto it = stock.find(req_name); it != stock.end() && it->second != 0) {
            const auto used = nano::min(it->second, req_amount);
            it->second -= used;
            req_amount -= used;
        }

        // Are we done?
        if (req_amount == 0) {
            continue;
        }

        // We need some more: let's find the recipe that produces this ingredient
        auto iter = nano::lower_bound(recipes, req_name, {}, [] (const auto& r) {
            return r.output.name;
        });

        const auto prod_amount = iter->output.amount;

        // How many times do we need to apply the recipe?
        const auto apps = req_amount/prod_amount + (req_amount % prod_amount != 0);

        // We might have some left over; add it to our stock
        stock[req_name] += apps * prod_amount - req_amount;

        // For each of its ingredients, add them to the queue to be resolved
        for (auto const& [amount, name] : iter->ingredients) {
            queue.emplace_back(name, apps * amount);
        }
    }

    return ore_count;
};

auto lots_of_ore = [] (auto const& recipes) {

    constexpr int64_t one_trillion = 1'000'000'000'000;

    // This search range is needlessly huge...
    auto range = nano::views::iota(1LL, one_trillion);

    // upper_bound gives us the first element larger than the target value
    // we want the one just before this
    return *nano::prev(nano::upper_bound(range, one_trillion, {}, [&recipes] (int64_t fuel) {
        return find_ore_requirement(recipes, fuel);
    }));
};

}

int main(int argc, char** argv)
{
    constexpr auto test_data1 =
R"(10 ORE => 10 A
1 ORE => 1 B
7 A, 1 B => 1 C
7 A, 1 C => 1 D
7 A, 1 D => 1 E
7 A, 1 E => 1 FUEL)"sv;

    constexpr auto test_data2 =
R"(9 ORE => 2 A
8 ORE => 3 B
7 ORE => 5 C
3 A, 4 B => 1 AB
5 B, 7 C => 1 BC
4 C, 1 A => 1 CA
2 AB, 3 BC, 4 CA => 1 FUEL)"sv;


    constexpr auto test_data3 =
R"(157 ORE => 5 NZVS
165 ORE => 6 DCFZ
44 XJWVT, 5 KHKGT, 1 QDVJ, 29 NZVS, 9 GPVTF, 48 HKGWZ => 1 FUEL
12 HKGWZ, 1 GPVTF, 8 PSHF => 9 QDVJ
179 ORE => 7 PSHF
177 ORE => 5 HKGWZ
7 DCFZ, 7 PSHF => 2 XJWVT
165 ORE => 2 GPVTF
3 DCFZ, 7 NZVS, 5 HKGWZ, 10 PSHF => 8 KHKGT)"sv;

    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return 1;
    }

    //const auto recipes = parse_input(test_data3);

    const auto recipes = parse_input([&] {
        std::ifstream stream(argv[1]);
        return std::string(nano::istreambuf_iterator<char>(stream), nano::istreambuf_iterator<char>());
    }());

    fmt::print("Ore required to make 1 FUEL (part one): {}\n", find_ore_requirement(recipes, 1));

    fmt::print("FUEL made with 1 trillion ORE (part two): {}\n", lots_of_ore(recipes));
}