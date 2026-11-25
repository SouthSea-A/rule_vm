#pragma once
#include <expected>
#include "../error/error_code.h"

namespace rule_cache_mapping_data_def
{
	using cache_mapping_result = std::expected<void, kernel_error>;
	template<typename T>
	using cache_mapping_result_pos = std::expected<T, kernel_error>;
}
