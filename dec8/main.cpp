
#include "../common.hpp"

constexpr int layer_width = 25;
constexpr int layer_height = 6;
constexpr int layer_size = layer_height * layer_width;

auto print_layer = [](std::string_view layer)
{
    for (auto i : nano::views::iota(0, layer_height)) {
        for (auto j : nano::views::iota(0, layer_width)) {
            const char c = layer[i * layer_width + j];
            fmt::print(c == '1' ? "#" : " ");
        }
        fmt::print("\n");
    }
};

int main(int argc, char** argv)
{
    if (argc < 2) {
        fmt::print(stderr, "No input!");
        return 1;
    }

    const std::string input = [&] {
        std::ifstream is(argv[1]);
        std::string str;
        is >> str;
        return str;
    }();

    // Range-V3's chunk view would be handy here...
    const auto layers = [&input] {
        std::vector<std::string_view> vec;
        for (auto i = 0; i < input.size(); i += layer_size) {
            vec.push_back(std::string_view{input}.substr(i, layer_size));
        }
        return vec;
    }();

    // Part one
    {
        const auto layer = nano::min(layers, {}, [](auto const& layer) {
            return nano::count(layer, '0');
        });

        fmt::print("Checksum (part one): {}\n",
                   nano::count(layer, '1') * nano::count(layer, '2'));
    }

    // Part two
    {
        std::string out(layer_size, '2');

        // Coalesce layers
        for (auto layer : layers) {
            nano::transform(out, layer, out.begin(), [](char cur, char new_) {
                return cur == '2' ? new_ : cur;
            });
        }

        fmt::print("Part two:\n");
        print_layer(out);
    }
}