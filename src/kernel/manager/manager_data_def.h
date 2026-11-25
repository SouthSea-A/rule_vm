#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <cstddef>
#include <utility>
#include <type_traits>
#include <functional>
#include <optional>
#include <expected>
#include "page_data_def.h"

namespace rule_manager_data_def
{
	using manager_data_type_base_64 = std::uint64_t;

	template<typename T, std::size_t MAX, typename allocator_manager = std::allocator<T>>
	using manager_block = std::pair<std::optional<pg::page<T, MAX / 2, allocator_manager>>, std::optional<pg::page<T, MAX / 2, allocator_manager>>>;

	using index_manager_block = manager_data_type_base_64;

	constexpr index_manager_block INDEX_MANAGER_BLOCK_END = std::numeric_limits<index_manager_block>::max();
	constexpr std::size_t ORDER_MANAGER_BLOCK_ORDER_END = std::numeric_limits<std::size_t>::max();
	constexpr std::size_t ORDER_MANAGER_BLOCK_END = std::numeric_limits<std::size_t>::max();
	constexpr std::size_t ORDER_MANAGER_END = std::numeric_limits<std::size_t>::max();

	using manager_result = std::expected<void, kernel_error>;
	using manager_result_index = std::expected<index_manager_block, kernel_error>;
	template<typename T, std::size_t MAX, typename allocator_manager = std::allocator<T>>
	using manager_result_ref = std::expected<std::reference_wrapper<std::optional<pg_data::element<manager_block<T, MAX, allocator_manager>, index_manager_block>>>, kernel_error>;
	template<typename T, std::size_t MAX, typename allocator_manager = std::allocator<T>>
	using manager_result_ref_const = std::expected<std::reference_wrapper<const std::optional<pg_data::element<manager_block<T, MAX, allocator_manager>, index_manager_block>>>, kernel_error>;
	template<typename T>
	using manager_result_src_const_ref = std::expected<std::reference_wrapper<const T>, kernel_error>;
	template<typename T>
	using manager_result_src_ref = std::expected<std::reference_wrapper<T>, kernel_error>;
	template<typename T, std::size_t MAX, typename allocator_manager = std::allocator<T>>
	using manager_result_const_ref = std::expected<std::reference_wrapper<const std::optional<pg_data::element<manager_block<T, MAX, allocator_manager>, index_manager_block>>>, kernel_error>;

	constexpr std::size_t BLOCK_SIZE_DEFAULT = 2048;
}
