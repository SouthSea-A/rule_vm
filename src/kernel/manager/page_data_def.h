#pragma once
#include <cstdint>
#include <cstddef>
#include <memory>
#include <utility>
#include <type_traits>
#include <functional>
#include <optional>
#include <expected>
#include "../error/error_code.h"

namespace rule_page
{
	template<typename T, std::size_t MAX, typename allocator_page = std::allocator<T>>
	class page;
}

namespace rule_page_data_def
{
	template<typename T, typename U>
	struct element
	{
		static_assert(std::numeric_limits<U>::is_integer, "位置索引类型无效");
		U last;
		U next;
		T src;
		template<typename...params>
		constexpr element(U last_val, U next_val, params&&...src_val) noexcept(std::is_nothrow_constructible_v<T, params...>) :
			src(std::forward<params>(src_val)...), last(last_val), next(next_val)
		{

		}
	};

	using page_data_type_base_16 = std::uint16_t;
	using page_data_type_base_32 = std::uint32_t;
	using page_data_type_base_64 = std::uint64_t;

	using index_page = page_data_type_base_64;
	using index_order_page = page_data_type_base_64;
	using index_page_pos = page_data_type_base_16;
	using index_page_order = page_data_type_base_16;

	constexpr index_page INDEX_PAGE_END = std::numeric_limits<index_page>::max();
	constexpr index_page_pos INDEX_PAGE_POS_END = std::numeric_limits<index_page_pos>::max();
	constexpr index_page_order INDEX_PAGE_ORDER_END = std::numeric_limits<index_page_order>::max();
	constexpr index_order_page INDEX_ORDER_PAGE_END = std::numeric_limits<index_order_page>::max();

	using page_result = std::expected<void, kernel_error>;
	using page_result_pos = std::expected<index_page_pos, kernel_error>;
	using page_result_index = std::expected<index_page, kernel_error>;
	template<typename T>
	using page_result_ref = std::expected<std::reference_wrapper<std::optional<element<T, index_page_pos>>>, kernel_error>;
	template<typename T>
	using page_result_const_ref = std::expected<std::reference_wrapper<const std::optional<element<T, index_page_pos>>>, kernel_error>;
	template<typename T>
	using page_result_src = std::expected<std::reference_wrapper<T>, kernel_error>;
	template<typename T>
	using page_result_const_src = std::expected<std::reference_wrapper<const T>, kernel_error>;
}
