#define FMT_HEADER_ONLY

#include <sys/wait.h>

#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/format.h>

#define FMT_FORMATTERS

#include <argparse/argparse.hpp>
#include <dtsutil/filesystem.hpp>
#include <dtsutil/process.hpp>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "Supervisor.hpp"

int main(int argc, char** argv)
{
    argparse::ArgumentParser parser("dl", "0.0.1");
    parser.add_argument("file").help("path to dl file to transpile");
    parser.add_argument("-o", "--output").help("compiled binary output path").default_value("a.out");
    parser.add_argument("-r", "--compile-and-run")
        .help("compiles and runs the specified file")
        .default_value(false)
        .implicit_value(true);
    parser.add_argument("-L", "--output-to-stdout")
        .help("prints transpiled file to stdout")
        .default_value(false)
        .implicit_value(true);
    parser.add_argument("-I", "--intermediates")
        .help("generate intermediate files")
        .default_value(false)
        .implicit_value(true);
    parser.add_argument("-T", "--tokens")
        .help("print lexed tokens to stdout")
        .default_value(false)
        .implicit_value(true);

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        fmt::print(stderr, fmt::fg(fmt::color::red) | fmt::emphasis::bold, "error: ", err.what());
        fmt::print(stderr, fmt::emphasis::bold, "{}", err.what());
        fmt::print(stderr, "{}", parser.help().str());
        return 1;
    }

    const auto input_file_path = parser.get<std::string>("file");

    const auto file_content = dts::read_file(input_file_path);
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

    const auto debug_tokens = parser.get<bool>("--tokens");
    if (debug_tokens) {
        for (const auto& token : tokens) { fmt::println(stderr, "{}", token); }
    }

    const auto ast = Parser::parse(tokens, supervisor);
    if (supervisor->has_errors()) {
        supervisor->dump_errors();
        return 1;
    }

    const auto transpiled_file_content = ast->evaluate();

    const auto output_to_stdout = parser.get<bool>("--output-to-stdout");
    if (output_to_stdout) {
        fmt::print("{}", transpiled_file_content);
        return 0;
    }

    const std::string intermediate_file = "intermediate.cpp";
    std::ofstream     intermediate_file_fd(intermediate_file);
    intermediate_file_fd << transpiled_file_content;
    intermediate_file_fd.close();

    const auto output_file_path       = parser.get<std::string>("--output");
    const auto compile_process_result = dts::subprocess_run(
        fmt::format("gcc -o {} -xc++ {}", output_file_path, intermediate_file));
    if (!compile_process_result) {
        fmt::print(
            stderr,
            fmt::emphasis::bold | fmt::fg(fmt::color::red),
            "error while invoking gcc to compile the transpiled file: {}",
            input_file_path);
        return 1;
    }

    int _status = 0;
    wait(&_status);

    const auto intermediate_files = parser.get<bool>("--intermediates");
    if (!intermediate_files) {
        const auto cleanup_process_result =
            dts::subprocess_run(fmt::format("rm {}", intermediate_file));
        if (!cleanup_process_result) {
            fmt::print(stderr, fmt::emphasis::bold | fmt::fg(fmt::color::red), "error while cleaning up intermediate files");
            return 1;
        }
    }

    wait(&_status);

    const auto compile_and_run = parser.get<bool>("--compile-and-run");
    if (compile_and_run) {
        const auto run_process_result =
            dts::subprocess_run(fmt::format("./{}", output_file_path));
        if (!run_process_result) {
            fmt::print(
                stderr,
                fmt::emphasis::bold | fmt::fg(fmt::color::red),
                "error while invoking the compiled binary: {}",
                input_file_path);
            return 1;
        }
    }

    return 0;
}
