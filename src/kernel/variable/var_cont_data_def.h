#pragma once
#include <cstdint>
#include <utility>
#include <type_traits>
#include <limits>
#include <span>
#include <vector>
#include <expected>
#include <functional>
#include <algorithm>
#include "../error/error_code.h"

namespace rule_var_cont_data_def
{
	using var_data_type_base_8 = std::uint8_t;
	using var_data_type_base_16 = std::uint16_t;
	using var_data_type_base_32 = std::uint32_t;
	using var_data_type_base_64 = std::uint64_t;

	using var_index = var_data_type_base_64;
	using var_type = var_data_type_base_64;
	using var_pos = var_data_type_base_64;
	using var_ref = var_data_type_base_64;
	using var_mgr_index = var_data_type_base_64;

	using var_read_width = var_data_type_base_64;

	constexpr var_index INVALID_VAR_INDEX = std::numeric_limits<var_index>::max();
	constexpr var_pos INVALID_VAR_POS = std::numeric_limits<var_pos>::max();
	constexpr var_mgr_index INVALID_VAR_MGR_INDEX = std::numeric_limits<var_mgr_index>::max();

	using var_cont_res = std::expected<void, kernel_error>;
	template<typename T>
	using var_cont_res_span = std::expected<std::span<T>, kernel_error>;
	template<typename T, typename var_cont_alloc>
	using var_cont_res_set = std::expected<std::reference_wrapper<std::vector<T, var_cont_alloc>>, kernel_error>;
	template<typename T, typename var_cont_alloc>
	using var_cont_res_set_const = std::expected<std::reference_wrapper<const std::vector<T, var_cont_alloc>>, kernel_error>;
}