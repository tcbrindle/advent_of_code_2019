
#include "../common.hpp"

namespace {

using tree_t = std::map<std::string, std::vector<std::string>>;

tree_t parse_input(std::istream&& input)
{
    tree_t tree;
    std::string str;

    while (std::getline(input, str)) {
        auto bracket = str.find(')');
        assert(bracket != str.npos);
        auto left = str.substr(0, bracket);
        auto right = str.substr(bracket+1);
        tree[std::move(left)].push_back(std::move(right));
    }

    return tree;
};

int count_orbits_rec(const tree_t& tree, const std::string& node, int depth = 1)
{
    auto iter = tree.find(node);
    if (iter == tree.end()) {
        return 0;
    }

    auto const& children = iter->second;

    return aoc::accumulate(children
        | nano::views::transform([&tree, depth] (auto& child) {
            return count_orbits_rec(tree, child, depth + 1); }),
            children.size() * depth);
}

int count_orbits(const tree_t& tree) {
    return count_orbits_rec(tree, "COM");
};

std::optional<int> distance_rec(const tree_t& tree, const std::string& from, const std::string& to)
{
    auto iter = tree.find(from);
    if (iter == tree.end()) {
        return {};
    }

    auto& children = iter->second;

    if (nano::find(children, to) != nano::end(children)) {
        return 0;
    }

    for (auto const& str : iter->second) {
        if (auto d = distance_rec(tree, str, to)) {
            return {1 + *d};
        }
    }

    return {};
}

std::optional<int> dist_from_me_to_santa(const tree_t& tree, const std::string& from)
{
    auto dist_to_me = distance_rec(tree, from, "YOU");
    if (!dist_to_me) {
        return {};
    }
    auto dist_to_santa = distance_rec(tree, from, "SAN");
    if (!dist_to_santa) {
        return {};
    }
    return {*dist_to_me + *dist_to_santa};
}

}


int main(int argc, char** argv)
{
    std::string test_data =
R"(COM)B
B)C
C)D
D)E
E)F
B)G
G)H
D)I
E)J
J)K
K)L)";

   std::string test_data2 =
R"(COM)B
B)C
C)D
D)E
E)F
B)G
G)H
D)I
E)J
J)K
K)L
K)YOU
I)SAN)";

    auto const tree = parse_input(std::ifstream(argv[1]));
    fmt::print("Total number of orbits (part one): {}\n", count_orbits(tree));

    const auto min_dist = nano::min(
        tree | nano::views::keys
             | nano::views::transform([&tree] (auto& key) { return dist_from_me_to_santa(tree, key); })
             | nano::views::filter([] (auto i) { return i.has_value(); })
             | nano::views::transform([](auto i) { return *i; })
    );
    fmt::print("Minimum orbits changes between me and Santa (part two): {}\n", min_dist);
}