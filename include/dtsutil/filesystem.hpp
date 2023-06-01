#ifndef DTSUTIL_FILESYSTEM_HPP
#define DTSUTIL_FILESYSTEM_HPP

#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <string_view>

namespace dts {

enum class FileStreamError : std::uint64_t {
    NoSuchFile = 0,
    NonRegularFile,
    UnableToOpen
};

[[nodiscard]] auto read_file(const std::string_view raw_path) noexcept
  -> std::expected<std::string, FileStreamError> {
    const auto path = std::filesystem::path(raw_path);

    if (!std::filesystem::exists(path)) {
        return std::unexpected(FileStreamError::NoSuchFile);
    }

    if (std::filesystem::status(path).type() != std::filesystem::file_type::regular) {
        return std::unexpected(FileStreamError::NonRegularFile);
    }

    const std::ifstream file_handle(path);
    if (!file_handle) { return std::unexpected(FileStreamError::UnableToOpen); }

    // clang-format off
    return std::string(static_cast<const std::stringstream&>(
                         std::stringstream() << file_handle.rdbuf()).str());
    // clang-format on
}

} // namespace dts

namespace dts::detail {

template<typename Error>
concept FilesystemError =
  std::is_same_v<std::remove_cvref_t<Error>, FileStreamError>;

template<FilesystemError Error>
struct FilesystemErrorFormatter {
    [[nodiscard]] constexpr static auto format(Error&& error) -> std::string = delete;
};

template<>
struct FilesystemErrorFormatter<FileStreamError> {
    [[nodiscard]] constexpr static auto format(const FileStreamError& error)
      -> std::string {
        switch (error) {
            case FileStreamError::NoSuchFile: {
                return "NoSuchFile";
            }
            case FileStreamError::NonRegularFile: {
                return "NonRegularFile";
            }
            case FileStreamError::UnableToOpen: {
                return "UnableToOpen";
            }
        }

        return "Unknown Error";
    }
};

} // namespace dts::detail

// Custom formatters
#ifdef FMT_FORMATTERS

template<dts::detail::FilesystemError Error>
struct fmt::formatter<Error> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const Error& error, FormatContext& ctx) {
        using Formatter =
          dts::detail::FilesystemErrorFormatter<std::decay_t<Error>>;

        fmt::format_to(
          ctx.out(), "[ERROR] in `dts::read_file`: {}", Formatter::format(error)
        );
    }
};

#else

template<dts::detail::FilesystemError Error>
auto operator<<(std::ostream& os, Error&& error) -> std::ostream& {
    using Formatter =
      dts::detail::FilesystemErrorFormatter<std::decay_t<Error>>;

    return os << "[ERROR] in `dts::read_file`: " << Formatter::format(error);
}

#endif // FMT_FORMATTERS

#endif //DTSUTIL_FILESYSTEM_HPP
