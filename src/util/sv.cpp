#include "sv.hpp"
namespace sv
{
	std::optional<std::vector<int32_t>> split_sv_to_int(std::string_view sv, const char delimiter) noexcept
	{
		if (sv.empty())
		{
			return std::nullopt;
		}

		const auto index = sv.find_first_of(delimiter);

		if (index != std::string::npos)
		{
			const auto first_part = sv.substr(0, index);
			const auto second_part = sv.substr(index + 1, sv.size());

			const auto t1 = std::atoi(std::string{ first_part }.c_str());
			const auto t2 = std::atoi(std::string{ second_part }.c_str());

			return std::vector{ t1, t2 };
		}

		return std::vector{ std::atoi(std::string{sv}.c_str()) };
	}
};
