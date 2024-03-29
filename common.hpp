
#ifndef ADVENT_OF_CODE_2019_COMMON_HPP
#define ADVENT_OF_CODE_2019_COMMON_HPP

#include <array>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "extern/nanorange.hpp"

#define FMT_HEADER_ONLY
#include "extern/fmt/format.h"

namespace aoc {

template <typename T>
std::vector<T> vector_from_file(const char* path)
{
    std::ifstream str(path);
    return std::vector<T>(std::istream_iterator<T>(str),
                          std::istream_iterator<T>{});
}

// A range-ified version of std::accumulate, but without any proper concepts
// beyond avoiding ambiguous iterator/range overloads
inline constexpr struct {
private:
    template <typename I, typename S, typename Init, typename Func, typename Proj>
    static constexpr Init impl(I first, S last, Init init, Func& func, Proj& proj)
    {
        while (first != last) {
            init = nano::invoke(func, std::move(init), nano::invoke(proj, *first));
            ++first;
        }
        return init;
    }

public:
    template <typename I, typename S,
              typename Proj = nano::identity,
              typename Init = nano::iter_value_t<nano::projected<I, Proj>>,
              typename Func = std::plus<>>
    constexpr std::enable_if_t<
        nano::input_iterator<I> &&
        nano::sentinel_for<S, I>,
        Init>
    operator()(I first, S last, Init init = Init{},
               Func func = Func{}, Proj proj = Proj{}) const

    {
        return impl(std::move(first), std::move(last), std::move(init), func, proj);
    }

    template <typename R,
              typename Proj = nano::identity,
              typename Init = nano::iter_value_t<nano::projected<nano::iterator_t<R>, Proj>>,
              typename Func = std::plus<>>
    constexpr std::enable_if_t<
        nano::input_range<R> &&
        !nano::input_iterator<nano::remove_cvref_t<R>>,
        Init>
    operator()(R&& rng, Init init = Init{}, Func func = Func{}, Proj proj = Proj{}) const
    {
        return impl(nano::begin(rng), nano::end(rng), std::move(init), func, proj);
    }

} accumulate{};

// Range to container
template <template <class...> class C, typename Rng>
constexpr auto to(Rng&& rng)
{
    auto view = nano::views::common(rng);
    return C(view.begin(), view.end());
}

template <typename C, typename Rng>
constexpr auto to(Rng&& rng)
{
    auto view = nano::views::common(rng);
    return C(view.begin(), view.end());
}

template <typename C>
constexpr auto to()
{
    return nano::detail::rao_proxy{[](auto&& rng) {
        auto view = nano::views::common(rng);
        return C(view.begin(), view.end());
    }};
}

template <template <class...> class C>
constexpr auto to()
{
    return nano::detail::rao_proxy{[](auto&& rng) {
        auto view = nano::views::common(rng);
        return C(view.begin(), view.end());
    }};
}

inline constexpr struct {

    constexpr auto operator()() const
    {
        return aoc::to<std::vector>();
    }

    template <typename Rng>
    constexpr auto operator()(Rng&& rng) const
    {
        return aoc::to<std::vector>(std::forward<Rng>(rng));
    }

} to_vector;

inline constexpr struct {

    constexpr auto operator()() const
    {
        return aoc::to<std::string>();
    }

    template <typename Rng>
    constexpr auto operator()(Rng&& rng) const
    {
        return aoc::to<std::string>(std::forward<Rng>(rng));
    }

} to_string;


} // namespace aoc

#endif
