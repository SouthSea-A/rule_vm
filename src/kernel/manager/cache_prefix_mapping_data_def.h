#pragma once
#include <expected>
#include "../error/error_code.h"

namespace rule_cache_prefix_mapping_data_def
{
	using cache_prefix_mapping_result = std::expected<void, kernel_error>;
	template<typename value_type>
	using cache_prefix_mapping_result_value = std::expected<value_type, kernel_error>;
}
