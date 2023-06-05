#include "Position.hpp"

Position Position::create(const std::size_t start, const std::size_t end) noexcept {
    return {start, end};
}

Position Position::create_dumb() noexcept {
    return {0, 0};
}

Position::Position(const std::size_t start, const std::size_t end) noexcept
    : m_start {start}, m_end {end} {}
