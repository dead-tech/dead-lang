#ifndef DTSUTIL_PROCESS_HPP
#define DTSUTIL_PROCESS_HPP

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <expected>
#include <optional>
#include <ranges>
#include <vector>

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#else
#include <windows.h>
#endif

namespace dts {

struct [[nodiscard]] ProcessError {
    enum class ErrorType : std::uint8_t { EmptyCommand = 0, ForkFailed };

    ErrorType                  type;
    std::optional<std::string> message;
};


namespace detail {

class [[nodiscard]] ShellCommand {
  public:
    [[nodiscard]] static auto create(const std::string& command) noexcept
      -> ShellCommand {
        return ShellCommand(command);
    }

    [[nodiscard]] auto program_name() const noexcept -> std::string {
        return m_command.front();
    }

    [[nodiscard]] auto command() const noexcept
      -> const std::vector<std::string>& {
        return { m_command };
    }

    [[nodiscard]] auto argc() const noexcept -> std::size_t {
        return m_command.size();
    }

  private:
    explicit ShellCommand(const std::string& command) {
        const std::size_t argc = std::ranges::count(command, ' ');
        m_command.reserve(argc + 1);

        for (const auto& word : std::views::split(command, ' ')) {
            m_command.emplace_back(word);
        }
    }

    std::vector<std::string> m_command;
};

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
[[nodiscard]] auto run_posix(const ShellCommand& command) noexcept
  -> std::expected<int, ProcessError> {
    const auto child_pid = fork();
    if (child_pid < 0) {
        return std::unexpected(ProcessError(
          ProcessError::ErrorType::ForkFailed, std::strerror(errno)
        ));
    }

    if (child_pid == 0) {
        // Split a string into a vector of strings  on whitespace
        const auto&        raw_command = command.command();
        std::vector<char*> argv;
        for (const auto& piece : raw_command) {
            argv.push_back(const_cast<char*>(piece.data()));
        }
        argv.push_back(nullptr);


        const auto return_code =
          execvp(command.program_name().data(), argv.data());

        if (return_code == -1) { return return_code; }
    }

    return 0;
}
#else
[[nodiscard]] auto run_windows(const ShellCommand& command) noexcept
  -> std::expected<int, ProcessError> {}
#endif

[[nodiscard]] auto run(const ShellCommand& command) noexcept
  -> std::expected<int, ProcessError> {
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
    return run_posix(command);
#else
    return run_windows(command);
#endif
}

} // namespace detail

[[nodiscard]] auto subprocess_run(const std::string& command) noexcept
  -> std::expected<int, ProcessError> {
    if (command.empty()) {
        return std::unexpected(
          ProcessError(ProcessError::ErrorType::EmptyCommand, {})
        );
    }
    return detail::run(detail::ShellCommand::create(command));
};

} // namespace dts

#endif //DTSUTIL_PROCESS_HPP
