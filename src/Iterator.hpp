#pragma once

#include <optional>

template <typename Iterable>
class [[nodiscard]] Iterator
{
    static_assert(
        std::is_same_v<typename Iterable::value_type, typename Iterable::value_type>,
        "Iterable must be a container and specify a value_type");

  protected:
    using value_type = typename Iterable::value_type;

    explicit Iterator(const Iterable& data) noexcept;

    [[nodiscard]] bool eof() const noexcept;

    std::optional<value_type> next() noexcept;

    std::optional<value_type> peek() const noexcept;

    std::optional<value_type> peek_ahead(const std::size_t offset) const noexcept;

    std::optional<value_type> peek_behind(const std::size_t offset) const noexcept;

    std::optional<value_type> previous() const noexcept;

    void advance(const std::size_t offset) noexcept;

    [[nodiscard]] std::size_t cursor() const noexcept;


  private:
    Iterable    m_data;
    std::size_t m_cursor = 0;
};

template <typename Iterable>
Iterator<Iterable>::Iterator(const Iterable& data) noexcept : m_data{data}
{
}


template <typename Iterable>
bool Iterator<Iterable>::eof() const noexcept
{
    return m_cursor >= m_data.size();
}

template <typename Iterable>
std::optional<typename Iterator<Iterable>::value_type> Iterator<Iterable>::next() noexcept
{
    if (eof()) { return {}; }
    return m_data[m_cursor++];
}

template <typename Iterable>
std::optional<typename Iterator<Iterable>::value_type> Iterator<Iterable>::peek() const noexcept
{
    if (eof()) { return {}; }
    return m_data[m_cursor];
}

template <typename Iterable>
std::optional<typename Iterator<Iterable>::value_type>
Iterator<Iterable>::peek_ahead(const std::size_t offset) const noexcept
{
    if (m_cursor + offset >= m_data.size()) { return {}; }
    return m_data[m_cursor + offset];
}

template <typename Iterable>
void Iterator<Iterable>::advance(const std::size_t offset) noexcept
{
    m_cursor += offset;
}

template <typename Iterable>
std::size_t Iterator<Iterable>::cursor() const noexcept
{
    return m_cursor;
}

template <typename Iterable>
std::optional<typename Iterator<Iterable>::value_type>
Iterator<Iterable>::peek_behind(const std::size_t offset) const noexcept
{
    if (m_cursor - offset >= m_data.size()) { return {}; }
    return m_data[m_cursor - offset];
}

template <typename Iterable>
std::optional<typename Iterator<Iterable>::value_type> Iterator<Iterable>::previous() const noexcept
{
    return peek_behind(1);
}
