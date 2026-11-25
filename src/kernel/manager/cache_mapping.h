#pragma once
#include "../kernel_namespace.h"
#include <cstddef>
#include <cstdint>
#include <utility>
#include <type_traits>
#include <vector>
#include <expected>
#include "cache_mapping_data_def.h"

namespace rule_cache_mapping
{
	template<typename T, typename U, std::size_t MAX, typename allocator_cache = std::allocator<U>>
	class cache_mapping
	{
		static_assert(MAX < static_cast<std::size_t>(std::numeric_limits<T>::max()), "缓存大小超出最大范围");
		static_assert(std::numeric_limits<T>::is_integer, "缓存逻辑映射类型无效");
		static_assert(std::numeric_limits<U>::is_integer, "缓存位置映射类型无效");
	private:
		std::vector<U, allocator_cache> set;
	public:
		cache_mapping() = default;
		cache_mapping(std::size_t size_val) :
			set()
		{
			this->set.reserve(size_val);
		}
		explicit cache_mapping(const allocator_cache& allocator_val, std::size_t size_val) :
			set(allocator_val)
		{
			this->set.reserve(size_val);
		}
		explicit cache_mapping(const allocator_cache& allocator_val) :
			set(allocator_val)
		{

		}
	public:
		cmap_data::cache_mapping_result add_set(T cache_order_new, U cache_pos_new)
		{
			std::size_t pos_target = static_cast<std::size_t>(cache_order_new);

			if (this->set.size() >= MAX)
				return std::unexpected(kernel_error::manager__cache_mapping__add_set__size_max);

			if (pos_target >= MAX)
				return std::unexpected(kernel_error::manager__cache_mapping__add_set__order_out_of_range);

			if (pos_target >= this->set.size())
			{
				try { this->set.resize(pos_target + 1, std::numeric_limits<U>::max()); }
				catch (...) { return std::unexpected(kernel_error::manager__cache_mapping__add_set__resize_unexpected); }
			}

			try { this->set.at(pos_target) = cache_pos_new; }
			catch (...) { return std::unexpected(kernel_error::manager__cache_mapping__add_set__read_unexpected); }

			return {};
		}
		cmap_data::cache_mapping_result insert_front(T cache_order_insert, U cache_pos_new)
		{
			std::size_t pos_target = static_cast<std::size_t>(cache_order_insert);

			if (this->set.size() >= MAX)
				return std::unexpected(kernel_error::manager__cache_mapping__insert_front__size_max);

			if (pos_target >= MAX)
				return std::unexpected(kernel_error::manager__cache_mapping__insert_front__order_out_of_range);

			if (pos_target >= this->set.size())
			{
				try { this->set.resize(pos_target + 1, std::numeric_limits<U>::max()); }
				catch (...) { return std::unexpected(kernel_error::manager__cache_mapping__insert_front__resize_unexpected); }

				try { this->set.at(pos_target) = cache_pos_new; }
				catch (...) { return std::unexpected(kernel_error::manager__cache_mapping__insert_front__read_unexpected); }
			}
			else
			{
				try { this->set.insert(this->set.begin() + pos_target, cache_pos_new); }
				catch (...) { return std::unexpected(kernel_error::manager__cache_mapping__insert_front__insert_unexpected); }
			}

			return {};
		}
		cmap_data::cache_mapping_result insert_back(T cache_order_insert, U cache_pos_new)
		{
			return insert_front(cache_order_insert + 1, cache_pos_new);
		}
		constexpr cmap_data::cache_mapping_result_pos<U> get(T index) const noexcept
		{
			std::size_t pos_target = static_cast<std::size_t>(index);

			if (pos_target >= this->set.size())
				return std::unexpected(kernel_error::manager__cache_mapping__get__order_out_of_range);

			U pos_target_get = 0;

			try { pos_target_get = this->set.at(pos_target); }
			catch (...) { return std::unexpected(kernel_error::manager__cache_mapping__get__read_unexpected); }

			if (pos_target_get == std::numeric_limits<U>::max())
				return std::unexpected(kernel_error::manager__cache_mapping__get__read_pos_invalid);

			return pos_target_get;
		}
		constexpr cmap_data::cache_mapping_result_pos<U> get_next(T index) const noexcept
		{
			return get(index + 1);
		}
		constexpr cmap_data::cache_mapping_result_pos<U> get_last(T index) const noexcept
		{
			if (index == 0)
				return std::unexpected(kernel_error::manager__cache_mapping__get_last__order_out_of_range);

			return get(index - 1);
		}
		constexpr cmap_data::cache_mapping_result_pos<U> get_front() const noexcept
		{
			return get(0);
		}
		constexpr cmap_data::cache_mapping_result_pos<U> get_back() const noexcept
		{
			return get(static_cast<T>(this->set.size() - 1));
		}
		cmap_data::cache_mapping_result del(T index)
		{
			std::size_t pos_target = static_cast<std::size_t>(index);

			if (pos_target >= this->set.size())
				return std::unexpected(kernel_error::manager__cache_mapping__del__order_out_of_range);

			try { this->set.erase(this->set.begin() + pos_target); }
			catch (...) { return std::unexpected(kernel_error::manager__cache_mapping__del__erase_unexpected); }

			return {};
		}
		void clear() noexcept
		{
			this->set.clear();
			return;
		}
		void swap(cache_mapping& other)
		{
			if (this == &other)
				return;

			this->set.swap(other.set);
			return;
		}
	};
}
