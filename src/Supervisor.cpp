#include "Supervisor.hpp"

#include <utility>

std::shared_ptr<Supervisor> Supervisor::create(std::string file_contents, std::string project_root_file) noexcept
{
    return std::make_shared<Supervisor>(Supervisor(
        std::move(file_contents), std::filesystem::path(std::move(project_root_file))));
}

Supervisor::Supervisor(std::string&& file_contents, std::filesystem::path project_root) noexcept
    : m_file_contents{file_contents},
      m_project_root{std::move(project_root)}
{
}

void Supervisor::push_error(const DLError& error) noexcept
{
    m_errors.push_back(error);
}

void Supervisor::dump_errors() const
{
    for (const auto& error : m_errors) { print_error(error); }

    m_errors.clear();
}

std::vector<Position> Supervisor::compute_line_positions() const noexcept
{
    std::vector<Position> line_positions;

    std::size_t start = 0;
    for (std::size_t i = 0; i < m_file_contents.size(); ++i) {
        if (m_file_contents[i] == '\n') {
            line_positions.push_back(Position::create(start, ++i));
            start = i + 1;
        }
    }

    return line_positions;
}

void Supervisor::print_error(const DLError& error) const
{
    if (m_file_contents.empty()) { return; }

    fmt::print(stderr, fmt::fg(fmt::color::red), "error");
    fmt::print(stderr, fmt::emphasis::bold, ": {}\n", error.message());

    // Find in which line is present the error span
    std::size_t error_line_index  = 0;
    std::size_t error_line_number = 0;
    const auto  line_positions    = compute_line_positions();
    for (std::size_t line_index = 0; line_index < line_positions.size(); ++line_index) {
        const auto& current_line_position = line_positions[line_index];
        if (error.position().start() + 1 >= current_line_position.start() &&
            error.position().start() + 1 <= current_line_position.end()) {
            error_line_index = line_index;
        }

        if (error.position().end() + 1 >= current_line_position.start() &&
            error.position().end() + 1 <= current_line_position.end()) {
            error_line_number = line_index + 1;
        }
    }

    fmt::println(stderr, " --> {}:{}", error_line_number, error.position().start() + 1);
    fmt::println(stderr, "  |");
    fmt::print(stderr, "  {} \t", error_line_number);

    // Print error line contents
    const auto& error_line_position = line_positions[error_line_index];
    const auto  error_line_contents = m_file_contents.substr(
        error_line_position.start(),
        (error_line_position.end() - error_line_position.start()));
    fmt::print(stderr, "{}", error_line_contents);

    // Print '^^^^' below span and error message next
    const auto spaces =
        std::string(error.position().start() - error_line_position.start(), ' ');
    const auto carets =
        std::string(error.position().end() + 1 - error.position().start() + 1, '^');
    fmt::print(
        stderr,
        fmt::fg(fmt::color::red),
        "  |    {}{} {}\n",
        spaces,
        carets,
        error.message());
}
