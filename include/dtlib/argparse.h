#ifndef DTLIB_ARGPARSE_H
#define DTLIB_ARGPARSE_H

#include <vector>
#include <string_view>
#include <unordered_map>
#include <optional>

#include "print.h"

namespace dt
{
  enum class ArgumentType
  {
    positional,
    flag,
  };

  struct Argument
  {
    using fn_ptr = std::optional<void (*)()>;

    std::string_view name;
    std::string_view value;
    ArgumentType type;
    bool required;
    fn_ptr callback;

    Argument(const std::string_view arg_name,
             const ArgumentType& arg_type,
             const bool required = false,
             fn_ptr ptr = std::nullopt
    )
      : name{arg_name}, type{arg_type}, required{required}, callback{ptr}
    {}
  };

  class ArgParser
  {
  private:
    using parsed_type = std::unordered_map<std::string_view, std::string_view>;
    using argv_type = std::vector<std::string_view>;

  public:

    ArgParser() = delete;
    ArgParser(const int argc, char** argv)
        : argv{argv_type(argv + 1, argv + argc)}
    {}

    void add_argument(
        const std::string_view arg_name,
        const ArgumentType& arg_type,
        const bool required = false,
        Argument::fn_ptr callback = std::nullopt
    )
    {
      this->to_find.emplace_back(arg_name, arg_type, required, callback);
    }

    DTLIB_NODISCARD("Can't discard parse return type")
    parsed_type parse() noexcept
    {
      parsed_type ret;
      for (const auto& arg : this->to_find)
      {
        const auto it = std::find_if(this->argv.begin(), this->argv.end(), [&arg](const std::string_view sv){ return sv.starts_with(arg.name); });

        if (it == this->argv.end() && !arg.required)
        {
          continue;
        }
        else if (it == this->argv.end() && arg.required)
        {
          dt::println("[ERROR] Could not find required argument %", arg.name);
          return {};
        }

        if (arg.type == ArgumentType::positional)
        {
          const auto equals_sign = it->find("=");

          if (equals_sign == std::string::npos)
          {
            dt::print("[ERROR] Positional argument % does not specify a value", arg.name);
            return {};
          }

          const std::string_view value = it->substr( + 1, it->size() - 1);
          ret.emplace(arg.name, value);
        }
        else
        {
          ret.emplace(arg.name, "flag argument");
        }

        if (arg.callback)
        {
          std::invoke(arg.callback.value());
        }

      }
      return ret;
    }

  private:
    std::vector<Argument> to_find;
    argv_type argv;
  };
}

#endif // DTLIB_ARGPARSE_H
