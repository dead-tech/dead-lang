#define FMT_HEADER_ONLY

#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/format.h>

#define FMT_FORMATTERS

#include <dtsutil/filesystem.hpp>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "Supervisor.hpp"

namespace
{
void print_usage() { fmt::println("usage: ./dead_lang <file.dl>"); }
} // namespace

int main(int argc, char** argv)
{
    const auto argv_span = std::span(argv, static_cast<std::size_t>(argc));
    if (argv_span.size() < 2) {
        print_usage();
        return 1;
    }

    const auto file_content = dts::read_file(argv_span[1]);
    if (!file_content.has_value()) {
        fmt::print(
            stderr,
            fmt::emphasis::bold | fmt::fg(fmt::color::red),
            "{}",
            file_content.error());
    }

    const auto supervisor = Supervisor::create(file_content.value());

    const auto tokens = Lexer::lex(file_content.value(), supervisor);

    if (supervisor->has_errors()) {
        supervisor->dump_errors();
        return 1;
    }

#if 0
    for (const auto& token: tokens) {
        fmt::println (stderr, "{}", token);
    }
#endif

    const auto ast = Parser::parse(tokens, supervisor);

    if (supervisor->has_errors()) {
        supervisor->dump_errors();
        return 1;
    }

    fmt::println("{}", ast->evaluate());
    return 0;
}
