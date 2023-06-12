#pragma once

#include <cstdint>

#include <fmt/format.h>

class [[nodiscard]] Position
{
  public:
    [[nodiscard]] static Position create(const std::size_t start, const std::size_t end) noexcept;

    [[nodiscard]] static Position create_dumb() noexcept;

    [[nodiscard]] constexpr std::size_t start() const noexcept
    {
        return m_start;
    }

    [[nodiscard]] constexpr std::size_t end() const noexcept { return m_end; }

  private:
    Position(const std::size_t start, const std::size_t end) noexcept;

    std::size_t m_start = 0;
    std::size_t m_end   = 0;
};

// {fmt} formatters
template <>
struct fmt::formatter<Position>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const Position& position, FormatContext& ctx)
    {
        return fmt::format_to(
            ctx.out(), "{{ start: {}, end: {} }}", position.start(), position.end());
    }
};
