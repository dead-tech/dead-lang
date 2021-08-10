#ifndef SV_HPP
#define SV_HPP

#include <vector>
#include <optional>
#include <string_view>
#include <string>

namespace sv
{
	std::optional<std::vector<int32_t>> split_sv_to_int(std::string_view sv, const char delimiter = ',') noexcept;
}

#endif //SV_HPP