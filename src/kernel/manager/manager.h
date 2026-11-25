#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <cstddef>
#include <memory>
#include <utility>
#include <type_traits>
#include <functional>
#include <optional>
#include <expected>
#include <iterator>
#include <limits>
#include <initializer_list>
#include <vector>
#include <algorithm>
#include "manager_data_def.h"
#include "page.h"
#include "cache_prefix_mapping.h"

namespace rule_manager
{
	template<typename T, std::size_t MAX = mgr_data::BLOCK_SIZE_DEFAULT, typename allocator_manager = std::allocator<T>>
	class manager;

	template<bool is_const, typename T, std::size_t MAX, typename allocator_manager = std::allocator<T>>
	class manager_iterator_x
	{
		friend manager<T, MAX, allocator_manager>;
		template<bool, typename, std::size_t, typename> friend class manager_iterator_x;
	private:
		using manager_ownership = std::conditional_t<is_const, const manager<T, MAX, allocator_manager>, manager<T, MAX, allocator_manager>>;
		using pointer_type = std::conditional_t<is_const, const T*, T*>;
		using reference_type = std::conditional_t<is_const, const T&, T&>;
	public:
		using iterator_category = std::random_access_iterator_tag;
		using iterator_concept = std::random_access_iterator_tag;
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using pointer = pointer_type;
		using reference = reference_type;
	private:
		manager_ownership* manager_ref;
		std::size_t order_block;
		mgr_data::index_manager_block block_index;
		std::size_t block_order;
		std::size_t order;
	public:
		template<typename self_this>
		decltype(auto) get_manager_ref(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.manager_ref);
		}
		template<typename self_this>
		decltype(auto) get_order_block(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.order_block);
		}
		template<typename self_this>
		decltype(auto) get_block_index(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.block_index);
		}
		template<typename self_this>
		decltype(auto) get_block_order(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.block_order);
		}
		template<typename self_this>
		decltype(auto) get_order(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.order);
		}
	public:
		void set_manager_ref(manager_ownership* manager_ref_val)
		{
			this->manager_ref = manager_ref_val;
			return;
		}
		void set_order_block(std::size_t order_block_val)
		{
			this->order_block = order_block_val;
			return;
		}
		void set_block_index(mgr_data::index_manager_block block_index_val)
		{
			this->block_index = block_index_val;
			return;
		}
		void set_block_order(std::size_t block_order_val)
		{
			this->block_order = block_order_val;
			return;
		}
		void set_order(std::size_t order_val)
		{
			this->order = order_val;
			return;
		}
	private:
		void test_ownership() const
		{
			if (this->manager_ref == nullptr)
				throw kernel_error_throw(kernel_error::manager__iterator__test_validity_iterator__manager_invalid, "kernel | manager | iterator | test_validity_iterator | 迭代器附属容器无效");

			return;
		}
		bool test_validity_iterator(bool is_only_test_ownership = false) const
		{
			test_ownership();

			return is_only_test_ownership == true ? true : this->block_index != mgr_data::INDEX_MANAGER_BLOCK_END;
		}
		constexpr void set_end() noexcept
		{
			this->order_block = mgr_data::ORDER_MANAGER_BLOCK_END;
			this->block_index = mgr_data::INDEX_MANAGER_BLOCK_END;
			this->block_order = mgr_data::ORDER_MANAGER_BLOCK_ORDER_END;
			this->order = mgr_data::ORDER_MANAGER_END;
			return;
		}
	public:
		constexpr manager_iterator_x() noexcept :
			manager_ref(nullptr),
			order_block(mgr_data::ORDER_MANAGER_BLOCK_END),
			block_index(mgr_data::INDEX_MANAGER_BLOCK_END),
			block_order(mgr_data::ORDER_MANAGER_BLOCK_ORDER_END),
			order(mgr_data::ORDER_MANAGER_END)
		{

		}
		constexpr manager_iterator_x(const manager_iterator_x<false, T, MAX, allocator_manager>& other) noexcept requires (is_const) :
			manager_ref(other.manager_ref), order_block(other.order_block), block_index(other.block_index), block_order(other.block_order), order(other.order)
		{

		}
		constexpr manager_iterator_x(const manager_iterator_x& other) noexcept :
			manager_ref(other.manager_ref), order_block(other.order_block), block_index(other.block_index), block_order(other.block_order), order(other.order)
		{

		}
		constexpr manager_iterator_x& operator=(const manager_iterator_x& other) noexcept
		{
			if (this == &other)
				return *this;

			this->manager_ref = other.manager_ref;
			this->order_block = other.order_block;
			this->block_index = other.block_index;
			this->block_order = other.block_order;
			this->order = other.order;

			return *this;
		}
		constexpr manager_iterator_x(manager_iterator_x&& other) noexcept :
			manager_ref(std::move(other.manager_ref)),
			order_block(std::move(other.order_block)),
			block_index(std::move(other.block_index)),
			block_order(std::move(other.block_order)),
			order(std::move(other.order))
		{
			other.set_end();
			other.manager_ref = nullptr;
		}
		constexpr manager_iterator_x& operator=(manager_iterator_x&& other) noexcept
		{
			if (this == &other)
				return *this;

			this->manager_ref = std::exchange(other.manager_ref, nullptr);
			this->order_block = std::move(other.order_block);
			this->block_index = std::move(other.block_index);
			this->block_order = std::move(other.block_order);
			this->order = std::move(other.order);

			other.set_end();

			return *this;
		}
		constexpr manager_iterator_x(manager_ownership& manager_ref_val) noexcept :
			manager_ref(&manager_ref_val), order_block(mgr_data::ORDER_MANAGER_BLOCK_END), block_index(mgr_data::INDEX_MANAGER_BLOCK_END), block_order(mgr_data::ORDER_MANAGER_BLOCK_ORDER_END), order(mgr_data::ORDER_MANAGER_END)
		{

		}
		constexpr manager_iterator_x(manager_ownership& manager_ref_val, std::size_t order_block_val, mgr_data::index_manager_block block_index_val, std::size_t block_order_val, std::size_t order_val) noexcept :
			manager_ref(&manager_ref_val), order_block(order_block_val), block_index(block_index_val), block_order(block_order_val), order(order_val)
		{

		}
	public:
		bool is_valid() const noexcept
		{
			if (this->manager_ref == nullptr)
				return false;

			return this->block_index != mgr_data::INDEX_MANAGER_BLOCK_END;
		}
		void invalidate() noexcept
		{
			this->manager_ref = nullptr;
			this->order_block = mgr_data::ORDER_MANAGER_BLOCK_ORDER_END;
			this->block_index = mgr_data::INDEX_MANAGER_BLOCK_END;
			this->block_order = mgr_data::ORDER_MANAGER_BLOCK_END;
			this->order = mgr_data::ORDER_MANAGER_END;
			return;
		}
	public:
		manager_iterator_x& operator++()
		{
			if (test_validity_iterator() == false)
				return *this;

			manager_ownership& manager_ref_real = *this->manager_ref;
			mgr_data::manager_result_const_ref<T, MAX, allocator_manager> block_target = manager_ref_real.get_block_ref_const(this->block_index, false);

			if (block_target.has_value() == false)
				throw kernel_error_throw(block_target.error(), "kernel | manager | iterator | operator++ | 当前块引用获取失败");

			if (this->block_order + 1 < manager_ref_real.block_get_size(block_target.value().get().value().src))
			{
				this->block_order++;
				this->order++;
				return *this;
			}

			this->block_order = 0;

			while (true)
			{
				mgr_data::index_manager_block block_index_next = block_target.value().get().value().next;

				block_target = manager_ref_real.get_block_ref_const(block_index_next, false);

				if (block_target.has_value() == false)
				{
					set_end();
					break;
				}

				if (manager_ref_real.block_get_size(block_target.value().get().value().src) > 0)
				{
					this->order_block++;
					this->block_index = block_index_next;
					this->order++;
					break;
				}
			}

			return *this;
		}
		manager_iterator_x operator++(int)
		{
			manager_iterator_x<is_const, T, MAX, allocator_manager> it_new = *this;
			++(*this);
			return it_new;
		}
		manager_iterator_x& operator--()
		{
			test_ownership();

			manager_ownership& manager_ref_real = *this->manager_ref;

			if (test_validity_iterator() == false)
			{
				cmap_data::cache_mapping_result_pos<mgr_data::index_manager_block> block_index_end = manager_ref_real.cache.get_back();

				if (block_index_end.has_value() == false)
				{
					set_end();
					return *this;
				}

				mgr_data::manager_result_const_ref<T, MAX, allocator_manager> block_end = manager_ref_real.get_block_ref_const(block_index_end.value(), false);

				if (block_end.has_value() == false)
					throw kernel_error_throw(block_end.error(), "kernel | manager | iterator | operator-- | 尾部块引用获取失败");

				std::size_t block_size_last = manager_ref_real.block_get_size(block_end.value().get().value().src);

				if (block_size_last > 0)
				{
					this->order_block = manager_ref_real.block_size - 1;
					this->block_index = block_index_end.value();
					this->block_order = block_size_last - 1;
					this->order = manager_ref_real.element_size - 1;
				}
				else
				{
					set_end();
				}

				return *this;
			}

			mgr_data::manager_result_const_ref<T, MAX, allocator_manager> block_target = manager_ref_real.get_block_ref_const(this->block_index, false);

			if (block_target.has_value() == false)
				throw kernel_error_throw(block_target.error(), "kernel | manager | iterator | operator-- | 当前块引用获取失败");

			if (this->block_order > 0)
			{
				this->block_order--;
				this->order--;
				return *this;
			}

			while (true)
			{
				mgr_data::index_manager_block block_index_last = block_target.value().get().value().last;

				block_target = manager_ref_real.get_block_ref_const(block_index_last, false);

				if (block_target.has_value() == false) break;

				std::size_t block_size_last = manager_ref_real.block_get_size(block_target.value().get().value().src);

				if (block_size_last > 0)
				{
					this->order_block--;
					this->block_index = block_index_last;
					this->block_order = block_size_last - 1;
					this->order--;
					break;
				}
			}

			return *this;
		}
		manager_iterator_x operator--(int)
		{
			manager_iterator_x<is_const, T, MAX, allocator_manager> it_new = *this;
			--(*this);
			return it_new;
		}
		manager_iterator_x& operator+=(difference_type add_val)
		{
			if (add_val < 0)
				return (*this) -= (-add_val);

			if (add_val == 0)
				return *this;

			if (test_validity_iterator() == false)
			{
				set_end();
				return *this;
			}

			std::size_t add_val_real = static_cast<std::size_t>(add_val);
			manager_ownership& manager_ref_real = *this->manager_ref;
			std::size_t index_get = this->order + add_val_real;

			if (index_get >= manager_ref_real.element_size)
			{
				set_end();
				return *this;
			}

			std::size_t order_block_get = mgr_data::ORDER_MANAGER_BLOCK_END;
			typename manager_ownership::type_cache_locate::locate_result locate_res = manager_ref_real.cache_locate.locate_by_index(index_get, order_block_get);

			if (locate_res.has_value() == false)
				throw kernel_error_throw(locate_res.error(), "kernel | manager | iterator | operator+= | 定位缓存查找失败，定位缓存与元素总量不一致，错误不可恢复");

			std::pair<mgr_data::index_manager_block, std::size_t> res = locate_res.value();

			this->order_block = order_block_get;
			this->block_index = res.first;
			this->block_order = index_get - res.second;
			this->order = index_get;

			return *this;
		}
		manager_iterator_x& operator-=(difference_type sub_val)
		{
			if (sub_val < 0)
				return (*this) += (-sub_val);

			if (sub_val == 0)
				return *this;

			std::size_t sub_val_real = 0;

			if (test_validity_iterator() == false)
			{
				--*this;
				sub_val_real = static_cast<std::size_t>(sub_val - 1);

				if (sub_val_real == 0)
					return *this;
			}
			else
			{
				sub_val_real = static_cast<std::size_t>(sub_val);
			}

			manager_ownership& manager_ref_real = *this->manager_ref;

			if ((static_cast<std::int64_t>(this->order) - static_cast<std::int64_t>(sub_val_real)) < 0)
			{
				typename manager_ownership::type_cache_locate::locate_result locate_res = manager_ref_real.cache_locate.locate_by_index(0);

				if (locate_res.has_value() == false)
					throw kernel_error_throw(locate_res.error(), "kernel | manager | iterator | operator-= | 定位缓存查找失败，定位缓存与元素总量不一致，错误不可恢复");

				std::pair<mgr_data::index_manager_block, std::size_t> res = locate_res.value();

				this->order_block = 0;
				this->block_index = res.first;
				this->block_order = 0;
				this->order = 0;

				return *this;
			}

			std::size_t index_get = this->order - sub_val_real;
			std::size_t order_block_get = mgr_data::ORDER_MANAGER_BLOCK_END;
			typename manager_ownership::type_cache_locate::locate_result locate_res = manager_ref_real.cache_locate.locate_by_index(index_get, order_block_get);

			if (locate_res.has_value() == false)
				throw kernel_error_throw(locate_res.error(), "kernel | manager | iterator | operator-= | 定位缓存查找失败，定位缓存与元素总量不一致，错误不可恢复");

			std::pair<mgr_data::index_manager_block, std::size_t> res = locate_res.value();

			this->order_block = order_block_get;
			this->block_index = res.first;
			this->block_order = index_get - res.second;
			this->order = index_get;

			return *this;
		}
		manager_iterator_x operator+(difference_type add_val) const
		{
			manager_iterator_x<is_const, T, MAX, allocator_manager> it_new(*this->manager_ref, this->order_block, this->block_index, this->block_order, this->order);

			it_new += add_val;

			return it_new;
		}
		difference_type operator-(const manager_iterator_x& it_val) const
		{
			test_ownership();
			it_val.test_ownership();

			if (it_val.manager_ref != this->manager_ref)
				throw kernel_error_throw(kernel_error::manager__iterator__operator_minus__manager_mismatch, "kernel | manager | iterator | operator- | 非同一附属迭代器，无法相减");

			manager_ownership& manager_ref_real = *this->manager_ref;

			bool is_end_this = test_validity_iterator();
			bool is_end_other = it_val.test_validity_iterator();

			if (is_end_this == false && is_end_other == false)
				return static_cast<difference_type>(this->order) - static_cast<difference_type>(it_val.order);

			if (is_end_this == false)
				return static_cast<difference_type>(manager_ref_real.element_size - it_val.order);

			if (is_end_other == false)
				return -static_cast<difference_type>(manager_ref_real.element_size - this->order);

			return static_cast<difference_type>(this->order) - static_cast<difference_type>(it_val.order);
		}
		manager_iterator_x operator-(difference_type sub_val) const
		{
			manager_iterator_x<is_const, T, MAX, allocator_manager> it_new(*this->manager_ref, this->order_block, this->block_index, this->block_order, this->order);

			it_new -= sub_val;

			return it_new;
		}
		reference operator[](difference_type index) const
		{
			return *((*this) + index);
		}
		reference operator*() const
		{
			if (test_validity_iterator() == false)
				throw kernel_error_throw(kernel_error::manager__iterator__operator_star__iterator_invalid, "kernel | manager | iterator | operator* | 迭代器无效");

			manager_ownership& manager_ref_real = *this->manager_ref;

			if constexpr (is_const == true)
			{
				mgr_data::manager_result_const_ref<T, MAX, allocator_manager> block_target = manager_ref_real.get_block_ref_const(this->block_index, false);

				if (block_target.has_value() == false)
					throw kernel_error_throw(block_target.error(), "kernel | manager | iterator | operator* | 当前块引用获取失败");

				mgr_data::manager_result_src_const_ref<T> src_target = manager_ref_real.block_get_const(block_target.value().get().value().src, this->block_order);

				if (src_target.has_value() == false)
					throw kernel_error_throw(src_target.error(), "kernel | manager | iterator | operator* | 块中元素引用获取失败");

				return src_target.value().get();
			}
			else
			{
				mgr_data::manager_result_ref<T, MAX, allocator_manager> block_target = manager_ref_real.get_block_ref(this->block_index, false);

				if (block_target.has_value() == false)
					throw kernel_error_throw(block_target.error(), "kernel | manager | iterator | operator* | 当前块引用获取失败");

				mgr_data::manager_result_src_ref<T> src_target = manager_ref_real.block_get(block_target.value().get().value().src, this->block_order);

				if (src_target.has_value() == false)
					throw kernel_error_throw(src_target.error(), "kernel | manager | iterator | operator* | 块中元素引用获取失败");

				return src_target.value().get();
			}

		}
		pointer operator->() const
		{
			if (test_validity_iterator() == false)
				throw kernel_error_throw(kernel_error::manager__iterator__operator_arrow__iterator_invalid, "kernel | manager | iterator | operator-> | 迭代器无效");

			manager_ownership& manager_ref_real = *this->manager_ref;

			if constexpr (is_const == true)
			{
				mgr_data::manager_result_const_ref<T, MAX, allocator_manager> block_target = manager_ref_real.get_block_ref_const(this->block_index, false);

				if (block_target.has_value() == false)
					throw kernel_error_throw(block_target.error(), "kernel | manager | iterator | operator-> | 当前块引用获取失败");

				mgr_data::manager_result_src_const_ref<T> src_target = manager_ref_real.block_get_const(block_target.value().get().value().src, this->block_order);

				if (src_target.has_value() == false)
					throw kernel_error_throw(src_target.error(), "kernel | manager | iterator | operator-> | 块中元素引用获取失败");

				return &src_target.value().get();
			}
			else
			{
				mgr_data::manager_result_ref<T, MAX, allocator_manager> block_target = manager_ref_real.get_block_ref(this->block_index, false);

				if (block_target.has_value() == false)
					throw kernel_error_throw(block_target.error(), "kernel | manager | iterator | operator-> | 当前块引用获取失败");

				mgr_data::manager_result_src_ref<T> src_target = manager_ref_real.block_get(block_target.value().get().value().src, this->block_order);

				if (src_target.has_value() == false)
					throw kernel_error_throw(src_target.error(), "kernel | manager | iterator | operator-> | 块中元素引用获取失败");

				return &src_target.value().get();
			}
		}
		bool operator>=(const manager_iterator_x& it_val) const
		{
			test_ownership();
			it_val.test_ownership();

			if (this->manager_ref != it_val.manager_ref)
				throw kernel_error_throw(kernel_error::manager__iterator__operator_compare__manager_mismatch, "kernel | manager | iterator | operator>= | 非同一附属迭代器，无法比较");

			return this->order >= it_val.order;
		}
		bool operator>(const manager_iterator_x& it_val) const
		{
			test_ownership();
			it_val.test_ownership();

			if (this->manager_ref != it_val.manager_ref)
				throw kernel_error_throw(kernel_error::manager__iterator__operator_compare__manager_mismatch, "kernel | manager | iterator | operator> | 非同一附属迭代器，无法比较");

			return this->order > it_val.order;
		}
		bool operator<=(const manager_iterator_x& it_val) const
		{
			test_ownership();
			it_val.test_ownership();

			if (this->manager_ref != it_val.manager_ref)
				throw kernel_error_throw(kernel_error::manager__iterator__operator_compare__manager_mismatch, "kernel | manager | iterator | operator<= | 非同一附属迭代器，无法比较");

			return this->order <= it_val.order;
		}
		bool operator<(const manager_iterator_x& it_val) const
		{
			test_ownership();
			it_val.test_ownership();

			if (this->manager_ref != it_val.manager_ref)
				throw kernel_error_throw(kernel_error::manager__iterator__operator_compare__manager_mismatch, "kernel | manager | iterator | operator< | 非同一附属迭代器，无法比较");

			return this->order < it_val.order;
		}
		bool operator==(const manager_iterator_x& it_val) const noexcept
		{
			if (this->manager_ref != it_val.manager_ref)
				return false;

			if (this->order_block != it_val.order_block)
				return false;

			if (this->block_index != it_val.block_index)
				return false;

			if (this->block_order != it_val.block_order)
				return false;

			if (this->order != it_val.order)
				return false;

			return true;
		}
		bool operator!=(const manager_iterator_x& it_val) const
		{
			return !(*this == it_val);
		}
		template<bool other_const>
		bool operator==(const manager_iterator_x<other_const, T, MAX, allocator_manager>& it_val) const noexcept
		{
			if (this->manager_ref != it_val.manager_ref)
				return false;

			if (this->order_block != it_val.order_block)
				return false;

			if (this->block_index != it_val.block_index)
				return false;

			if (this->block_order != it_val.block_order)
				return false;

			if (this->order != it_val.order)
				return false;

			return true;
		}
		template<bool other_const>
		bool operator!=(const manager_iterator_x<other_const, T, MAX, allocator_manager>& it_val) const
		{
			return !(*this == it_val);
		}
	};

	template<bool is_const, typename T, std::size_t MAX, typename allocator_manager = std::allocator<T>>
	manager_iterator_x<is_const, T, MAX, allocator_manager> operator+(typename manager_iterator_x<is_const, T, MAX, allocator_manager>::difference_type add_val, const manager_iterator_x<is_const, T, MAX, allocator_manager>& it_val)
	{
		return it_val + add_val;
	}

	template<typename T, std::size_t MAX, typename allocator_manager>
	class manager
	{
		static_assert(std::is_object_v<T>, "T 类型必须为对象类型");
		static_assert(MAX % 2 == 0, "MAX 必须为偶数");
		static_assert(std::is_same_v<typename std::allocator_traits<allocator_manager>::value_type, T>, "分配器类型必须为元素类型");
	public:
		using allocator_type = allocator_manager;
		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using pointer = value_type*;
		using reference = value_type&;
		using reference_const = const value_type&;
		using iterator = manager_iterator_x<false, T, MAX, allocator_manager>;
		using const_iterator = manager_iterator_x<true, T, MAX, allocator_manager>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using result_iterator = std::expected<iterator, kernel_error>;
		using result_const_iterator = std::expected<const_iterator, kernel_error>;
		using result_reverse_iterator = std::expected<reverse_iterator, kernel_error>;
		using result_const_reverse_iterator = std::expected<const_reverse_iterator, kernel_error>;
	public:
		friend iterator;
		friend const_iterator;
	private:
		using allocator_manager_traits = typename std::allocator_traits<allocator_manager>;
		using allocator_manager_set = allocator_manager_traits::template rebind_alloc<std::optional<pg_data::element<mgr_data::manager_block<T, MAX, allocator_manager>, mgr_data::index_manager_block>>>;
		using allocator_manager_free = allocator_manager_traits::template rebind_alloc<mgr_data::index_manager_block>;
		using allocator_manager_cache = allocator_manager_traits::template rebind_alloc<mgr_data::index_manager_block>;
		using allocator_manager_cache_locate = allocator_manager_traits::template rebind_alloc<mgr_data::index_manager_block>;
		using type_cache_locate = cpmap::cache_prefix_mapping<mgr_data::index_manager_block, std::size_t, allocator_manager_cache_locate>;
	private:
		allocator_manager allocator_;
		std::vector<std::optional<pg_data::element<mgr_data::manager_block<T, MAX, allocator_manager>, mgr_data::index_manager_block>>, allocator_manager_set> set;
		std::vector<mgr_data::index_manager_block, allocator_manager_free> free;
		cmap::cache_mapping<std::size_t, mgr_data::index_manager_block, std::numeric_limits<mgr_data::index_manager_block>::max() - 1, allocator_manager_cache> cache;
		type_cache_locate cache_locate;
		std::size_t block_size;
		std::size_t element_size;
	private:
		[[nodiscard]] constexpr bool test_validity_order(std::size_t order) const noexcept
		{
			return this->set.size() == 0 ? false : order < this->element_size;
		}
		[[nodiscard]] constexpr bool test_validity_index_block(mgr_data::index_manager_block index, bool is_test_free) const noexcept
		{
			std::size_t index_test = static_cast<std::size_t>(index);

			if (index_test >= this->set.size())
				return false;

			return is_test_free == false ? true : this->set[index_test].has_value() == true;
		}
		constexpr mgr_data::manager_result_ref<T, MAX, allocator_manager> get_block_ref(mgr_data::index_manager_block index, bool is_get_free) noexcept
		{
			if (test_validity_index_block(index, !is_get_free) == false)
				return std::unexpected(kernel_error::manager__manager__get_block_ref__index_invalid);

			return this->set[static_cast<std::size_t>(index)];
		}
		constexpr mgr_data::manager_result_ref_const<T, MAX, allocator_manager> get_block_ref_const(mgr_data::index_manager_block index, bool is_get_free) const noexcept
		{
			if (test_validity_index_block(index, !is_get_free) == false)
				return std::unexpected(kernel_error::manager__manager__get_block_ref_const__index_invalid);

			return this->set[static_cast<std::size_t>(index)];
		}
		void free_roll_back(mgr_data::index_manager_block index)
		{
			try { this->free.emplace_back(index); }
			catch (...) { throw kernel_error_throw(kernel_error::manager__manager__free_roll_back__emplace_back_unexpected, "kernel | manager | manager | free_roll_back | 空闲索引回写失败，错误不可恢复"); }

			return;
		}
		void cache_del_by_order(std::size_t order)
		{
			cmap_data::cache_mapping_result cache_roll_back_res = this->cache.del(order);

			if (cache_roll_back_res.has_value() == false)
				throw kernel_error_throw(cache_roll_back_res.error(), "kernel | manager | manager | cache_del_by_order | 块缓存回滚失败，错误不可恢复");

			return;
		}
		void locate_update(std::size_t index, std::size_t size_new)
		{
			cpmap_data::cache_prefix_mapping_result locate_res = this->cache_locate.update(index, size_new);

			if (locate_res.has_value() == false)
				throw kernel_error_throw(locate_res.error(), "kernel | manager | manager | locate_update | 定位缓存更新失败，定位缓存与块的实际大小不一致，错误不可恢复");

			return;
		}
		void move_page_all(pg::page<T, MAX / 2, allocator_manager>& page_from, pg::page<T, MAX / 2, allocator_manager>& page_to)
		{
			const std::size_t size_from = page_from.get_size();

			if (size_from == 0)
				return;

			rule_page_data_def::page_result move_res = page_from.cut(page_to, 0, static_cast<rule_page_data_def::index_page_order>(size_from - 1), static_cast<rule_page_data_def::index_page_order>(page_to.get_size()));

			if (move_res.has_value() == false)
				throw kernel_error_throw(move_res.error(), "kernel | manager | manager | move_page_all | 页元素搬迁失败，回滚不可恢复");

			return;
		}
		void remove_block_roll_back(std::size_t order, mgr_data::index_manager_block index)
		{
			mgr_data::manager_result_ref<T, MAX, allocator_manager> block_del = get_block_ref(index, false);

			if (block_del.has_value() == false)
				throw kernel_error_throw(block_del.error(), "kernel | manager | manager | remove_block_roll_back | 待回滚块不存在，错误不可恢复");

			pg_data::element<mgr_data::manager_block<T, MAX, allocator_manager>, mgr_data::index_manager_block>& block_del_ref = block_del.value().get().value();

			if (block_get_size(block_del_ref.src) != 0)
				throw kernel_error_throw(kernel_error::manager__manager__block_del__order_invalid, "kernel | manager | manager | remove_block_roll_back | 待回滚块仍含元素，回滚会丢数据，错误不可恢复");

			if (block_del_ref.last != mgr_data::INDEX_MANAGER_BLOCK_END)
			{
				mgr_data::manager_result_ref<T, MAX, allocator_manager> block_last = get_block_ref(block_del_ref.last, false);

				if (block_last.has_value() == true)
					block_last.value().get().value().next = block_del_ref.next;
			}

			if (block_del_ref.next != mgr_data::INDEX_MANAGER_BLOCK_END)
			{
				mgr_data::manager_result_ref<T, MAX, allocator_manager> block_next = get_block_ref(block_del_ref.next, false);

				if (block_next.has_value() == true)
					block_next.value().get().value().last = block_del_ref.last;
			}

			if (this->cache_locate.erase(order) == false)
				throw kernel_error_throw(kernel_error::manager__manager__block_del__order_invalid, "kernel | manager | manager | remove_block_roll_back | 定位缓存回滚失败，错误不可恢复");

			cmap_data::cache_mapping_result cache_del_res = this->cache.del(order);

			if (cache_del_res.has_value() == false)
				throw kernel_error_throw(cache_del_res.error(), "kernel | manager | manager | remove_block_roll_back | 块缓存回滚失败，错误不可恢复");

			this->set[static_cast<std::size_t>(index)].reset();

			if (static_cast<std::size_t>(index) == this->set.size() - 1)
			{
				this->set.pop_back();
			}
			else
			{
				try { this->free.emplace_back(index); }
				catch (...) { throw kernel_error_throw(kernel_error::manager__manager__add_free__emplace_back_unexpected, "kernel | manager | manager | remove_block_roll_back | 空闲索引回写失败，错误不可恢复"); }
			}

			this->block_size--;
			return;
		}
		void roll_back_split(std::size_t order_old, mgr_data::index_manager_block index_new)
		{
			cmap_data::cache_mapping_result_pos<mgr_data::index_manager_block> block_old_index = this->cache.get(order_old);

			if (block_old_index.has_value() == false)
				throw kernel_error_throw(block_old_index.error(), "kernel | manager | manager | roll_back_split | 旧块定位失败，错误不可恢复");

			mgr_data::manager_result_ref<T, MAX, allocator_manager> block_old = get_block_ref(block_old_index.value(), false);

			if (block_old.has_value() == false)
				throw kernel_error_throw(block_old.error(), "kernel | manager | manager | roll_back_split | 旧块引用获取失败，错误不可恢复");

			mgr_data::manager_result_ref<T, MAX, allocator_manager> block_new = get_block_ref(index_new, false);

			if (block_new.has_value() == false)
				throw kernel_error_throw(block_new.error(), "kernel | manager | manager | roll_back_split | 新块引用获取失败，错误不可恢复");

			pg_data::element<mgr_data::manager_block<T, MAX, allocator_manager>, mgr_data::index_manager_block>& block_old_ref = block_old.value().get().value();
			pg_data::element<mgr_data::manager_block<T, MAX, allocator_manager>, mgr_data::index_manager_block>& block_new_ref = block_new.value().get().value();

			if (block_new_ref.src.second.has_value() == true)
			{
				if (block_new_ref.src.first.has_value() == false)
					throw kernel_error_throw(kernel_error::manager__manager__block_insert_front__block_invalid_left, "kernel | manager | manager | roll_back_split | 新块缺少左页，回滚不可恢复");

				move_page_all(block_new_ref.src.second.value(), block_new_ref.src.first.value());
				block_new_ref.src.second.reset();
			}

			if (block_old_ref.src.second.has_value() == true)
			{
				if (block_old_ref.src.first.has_value() == false)
					throw kernel_error_throw(kernel_error::manager__manager__block_insert_front__block_invalid_left, "kernel | manager | manager | roll_back_split | 旧块缺少左页，回滚不可恢复");

				move_page_all(block_old_ref.src.second.value(), block_old_ref.src.first.value());
				block_old_ref.src.second.reset();
			}

			if (block_new_ref.src.first.has_value() == false)
				throw kernel_error_throw(kernel_error::manager__manager__block_insert_front__block_invalid_left, "kernel | manager | manager | roll_back_split | 新块左页丢失，回滚不可恢复");

			block_old_ref.src.second = std::move(block_new_ref.src.first);

			if (block_new_ref.src.second.has_value() == true)
				throw kernel_error_throw(kernel_error::manager__manager__block_insert_front__block_invalid_left, "kernel | manager | manager | roll_back_split | 新块右页未清空，回滚不可恢复");

			this->locate_update(order_old, block_get_size(block_old_ref.src));

			remove_block_roll_back(order_old + 1, index_new);
			return;
		}
		mgr_data::manager_result_index add_block()
		{
			mgr_data::manager_result_index index_free_res = get_free();
			cmap_data::cache_mapping_result_pos<mgr_data::index_manager_block> block_index_last = this->cache.get_back();
			mgr_data::index_manager_block index_new = mgr_data::INDEX_MANAGER_BLOCK_END;
			mgr_data::index_manager_block index_last = block_index_last.has_value() == false ? mgr_data::INDEX_MANAGER_BLOCK_END : block_index_last.value();
			bool is_free_used = false;

			if (index_free_res.has_value() == false)
			{
				index_new = static_cast<mgr_data::index_manager_block>(this->set.size());
			}
			else
			{
				index_new = index_free_res.value();
				is_free_used = true;

				mgr_data::manager_result_ref<T, MAX, allocator_manager> block_free = get_block_ref(index_new, true);

				if (block_free.has_value() == false)
				{
					this->free_roll_back(index_new);
					return std::unexpected(block_free.error());
				}
			}

			cmap_data::cache_mapping_result cache_add_res = this->cache.add_set(this->block_size, index_new);

			if (cache_add_res.has_value() == false)
			{
				if (is_free_used == true) this->free_roll_back(index_new);
				return std::unexpected(cache_add_res.error());
			}

			cpmap_data::cache_prefix_mapping_result locate_add_res = this->cache_locate.push_back(index_new, 0);

			if (locate_add_res.has_value() == false)
			{
				this->cache_del_by_order(this->block_size);

				if (is_free_used == true) this->free_roll_back(index_new);
				return std::unexpected(locate_add_res.error());
			}

			if (is_free_used == false)
			{
				try { this->set.emplace_back(std::in_place, index_last, mgr_data::INDEX_MANAGER_BLOCK_END, pg::page<T, MAX / 2, allocator_manager>(this->allocator_), std::nullopt); }
				catch (...)
				{
					this->cache_locate.erase(this->block_size);
					this->cache_del_by_order(this->block_size);
					return std::unexpected(kernel_error::manager__manager__add_block__emplace_back_unexpected);
				}
			}
			else
			{
				mgr_data::manager_result_ref<T, MAX, allocator_manager> block_free = get_block_ref(index_new, true);

				if (block_free.has_value() == false)
				{
					this->cache_locate.erase(this->block_size);
					this->cache_del_by_order(this->block_size);
					this->free_roll_back(index_new);
					return std::unexpected(block_free.error());
				}

				try { block_free.value().get().emplace(index_last, mgr_data::INDEX_MANAGER_BLOCK_END, pg::page<T, MAX / 2, allocator_manager>(this->allocator_), std::nullopt); }
				catch (...)
				{
					this->cache_locate.erase(this->block_size);
					this->cache_del_by_order(this->block_size);
					this->free_roll_back(index_new);
					return std::unexpected(kernel_error::manager__manager__add_block__emplace_unexpected);
				}
			}

			if (block_index_last.has_value() == true)
			{
				mgr_data::manager_result_ref<T, MAX, allocator_manager> block_last = get_block_ref(block_index_last.value(), false);

				if (block_last.has_value() == true) block_last.value().get().value().next = index_new;
			}

			this->block_size++;
			return index_new;
		}
		mgr_data::manager_result_index insert_front_block(std::size_t index)
		{
			cmap_data::cache_mapping_result_pos<mgr_data::index_manager_block> index_insert_res = this->cache.get(index);

			if (index_insert_res.has_value() == false)
				return add_block();

			mgr_data::index_manager_block index_insert = index_insert_res.value();

			mgr_data::manager_result_ref<T, MAX, allocator_manager> block_insert = get_block_ref(index_insert, false);

			if (block_insert.has_value() == false)
				return std::unexpected(block_insert.error());

			mgr_data::index_manager_block index_last = block_insert.value().get().value().last;
			mgr_data::index_manager_block index_next = index_insert;
			mgr_data::manager_result_index index_free_res = get_free();
			mgr_data::index_manager_block index_new = mgr_data::INDEX_MANAGER_BLOCK_END;
			bool is_free_used = false;

			if (index_free_res.has_value() == false)
			{
				index_new = static_cast<mgr_data::index_manager_block>(this->set.size());
			}
			else
			{
				index_new = index_free_res.value();
				is_free_used = true;

				mgr_data::manager_result_ref<T, MAX, allocator_manager> block_free = get_block_ref(index_new, true);

				if (block_free.has_value() == false)
				{
					this->free_roll_back(index_new);
					return std::unexpected(block_free.error());
				}
			}

			cmap_data::cache_mapping_result cache_insert_res = this->cache.insert_front(index, index_new);

			if (cache_insert_res.has_value() == false)
			{
				if (is_free_used == true) this->free_roll_back(index_new);
				return std::unexpected(cache_insert_res.error());
			}

			cpmap_data::cache_prefix_mapping_result locate_insert_res = this->cache_locate.insert(index, index_new, 0);

			if (locate_insert_res.has_value() == false)
			{
				this->cache_del_by_order(index);

				if (is_free_used == true) this->free_roll_back(index_new);
				return std::unexpected(locate_insert_res.error());
			}

			if (is_free_used == false)
			{
				try { this->set.emplace_back(std::in_place, index_last, index_next, pg::page<T, MAX / 2, allocator_manager>(this->allocator_), std::nullopt); }
				catch (...)
				{
					this->cache_locate.erase(index);
					this->cache_del_by_order(index);
					return std::unexpected(kernel_error::manager__manager__insert_front_block__emplace_back_unexpected);
				}
			}
			else
			{
				mgr_data::manager_result_ref<T, MAX, allocator_manager> block_free = get_block_ref(index_new, true);

				if (block_free.has_value() == false)
				{
					this->cache_locate.erase(index);
					this->cache_del_by_order(index);
					this->free_roll_back(index_new);
					return std::unexpected(block_free.error());
				}

				try { block_free.value().get().emplace(index_last, index_next, pg::page<T, MAX / 2, allocator_manager>(this->allocator_), std::nullopt); }
				catch (...)
				{
					this->cache_locate.erase(index);
					this->cache_del_by_order(index);
					this->free_roll_back(index_new);
					return std::unexpected(kernel_error::manager__manager__insert_front_block__emplace_unexpected);
				}
			}

			block_insert = get_block_ref(index_insert, false);

			if (block_insert.has_value() == false)
				throw kernel_error_throw(block_insert.error(), "kernel | manager | manager | insert_front_block | 新块插入后重获取原块失败，块链已改变，错误不可恢复");

			block_insert.value().get().value().last = index_new;

			mgr_data::manager_result_ref<T, MAX, allocator_manager> block_insert_last = get_block_ref(index_last, false);

			if (block_insert_last.has_value() == true)
				block_insert_last.value().get().value().next = index_new;

			this->block_size++;
			return index_new;
		}
		mgr_data::manager_result_index insert_back_block(std::size_t index)
		{
			cmap_data::cache_mapping_result_pos<mgr_data::index_manager_block> index_insert_res = this->cache.get(index);

			if (index_insert_res.has_value() == false)
				return add_block();

			mgr_data::index_manager_block index_insert = index_insert_res.value();

			mgr_data::manager_result_ref<T, MAX, allocator_manager> block_insert = get_block_ref(index_insert, false);

			if (block_insert.has_value() == false)
				return std::unexpected(block_insert.error());

			mgr_data::index_manager_block index_last = index_insert;
			mgr_data::index_manager_block index_next = block_insert.value().get().value().next;
			mgr_data::manager_result_index index_free_res = get_free();
			mgr_data::index_manager_block index_new = mgr_data::INDEX_MANAGER_BLOCK_END;
			bool is_free_used = false;

			if (index_free_res.has_value() == false)
			{
				index_new = static_cast<mgr_data::index_manager_block>(this->set.size());
			}
			else
			{
				index_new = index_free_res.value();
				is_free_used = true;

				mgr_data::manager_result_ref<T, MAX, allocator_manager> block_free = get_block_ref(index_new, true);

				if (block_free.has_value() == false)
				{
					this->free_roll_back(index_new);
					return std::unexpected(block_free.error());
				}
			}

			cmap_data::cache_mapping_result cache_insert_res = this->cache.insert_back(index, index_new);

			if (cache_insert_res.has_value() == false)
			{
				if (is_free_used == true) this->free_roll_back(index_new);
				return std::unexpected(cache_insert_res.error());
			}

			cpmap_data::cache_prefix_mapping_result locate_insert_res = this->cache_locate.insert(index + 1, index_new, 0);

			if (locate_insert_res.has_value() == false)
			{
				this->cache_del_by_order(index + 1);

				if (is_free_used == true) this->free_roll_back(index_new);
				return std::unexpected(locate_insert_res.error());
			}

			if (is_free_used == false)
			{
				try { this->set.emplace_back(std::in_place, index_last, index_next, pg::page<T, MAX / 2, allocator_manager>(this->allocator_), std::nullopt); }
				catch (...)
				{
					this->cache_locate.erase(index + 1);
					this->cache_del_by_order(index + 1);
					return std::unexpected(kernel_error::manager__manager__insert_back_block__emplace_back_unexpected);
				}
			}
			else
			{
				mgr_data::manager_result_ref<T, MAX, allocator_manager> block_free = get_block_ref(index_new, true);

				if (block_free.has_value() == false)
				{
					this->cache_locate.erase(index + 1);
					this->cache_del_by_order(index + 1);
					this->free_roll_back(index_new);
					return std::unexpected(block_free.error());
				}

				try { block_free.value().get().emplace(index_last, index_next, pg::page<T, MAX / 2, allocator_manager>(this->allocator_), std::nullopt); }
				catch (...)
				{
					this->cache_locate.erase(index + 1);
					this->cache_del_by_order(index + 1);
					this->free_roll_back(index_new);
					return std::unexpected(kernel_error::manager__manager__insert_back_block__emplace_unexpected);
				}
			}

			block_insert = get_block_ref(index_insert, false);

			if (block_insert.has_value() == false)
				throw kernel_error_throw(block_insert.error(), "kernel | manager | manager | insert_back_block | 新块插入后重获取原块失败，块链已改变，错误不可恢复");

			block_insert.value().get().value().next = index_new;

			mgr_data::manager_result_ref<T, MAX, allocator_manager> block_insert_next = get_block_ref(index_next, false);

			if (block_insert_next.has_value() == true)
				block_insert_next.value().get().value().last = index_new;

			this->block_size++;
			return index_new;
		}
		mgr_data::manager_result add_free(mgr_data::index_manager_block index)
		{
			mgr_data::manager_result_ref<T, MAX, allocator_manager> block_del_target = get_block_ref(index, false);

			if (block_del_target.has_value() == false)
				return std::unexpected(block_del_target.error());

			try { this->free.emplace_back(index); }
			catch (...) { return std::unexpected(kernel_error::manager__manager__add_free__emplace_back_unexpected); }

			pg_data::element<mgr_data::manager_block<T, MAX, allocator_manager>, mgr_data::index_manager_block>& block_del_ref = block_del_target.value().get().value();
			mgr_data::manager_result_ref<T, MAX, allocator_manager> block_del_last = get_block_ref(block_del_ref.last, false);

			if (block_del_last.has_value() == true)
				block_del_last.value().get().value().next = block_del_ref.next;

			mgr_data::manager_result_ref<T, MAX, allocator_manager> block_del_next = get_block_ref(block_del_ref.next, false);

			if (block_del_next.has_value() == true)
				block_del_next.value().get().value().last = block_del_ref.last;

			block_del_target.value().get().reset();
			return {};
		}
		mgr_data::manager_result_index get_free() noexcept
		{
			if (this->free.empty() == true)
				return std::unexpected(kernel_error::manager__manager__get_free__free_empty);

			mgr_data::index_manager_block index_free = this->free.back();

			this->free.pop_back();

			return index_free;
		}
		template<typename...params>
		mgr_data::manager_result_src_ref<T> block_add(std::size_t index, mgr_data::manager_block<T, MAX, allocator_manager>& block, params&&...src_new)
		{
			if (block.second.has_value() == false)
			{
				if (block.first.value().get_size_left() > 0)
				{
					pg_data::page_result_src<T> res_add = block.first.value().add(std::forward<params>(src_new)...);

					if (res_add.has_value() == false)
						return std::unexpected(res_add.error());

					this->locate_update(index, block_get_size(block));
					return res_add.value();
				}

				try { block.second.emplace(pg::page<T, MAX / 2, allocator_manager>(this->allocator_)); }
				catch (...) { return std::unexpected(kernel_error::manager__manager__block_add__emplace_right_unexpected); }

				pg_data::page_result_src<T> res_add = block.second.value().add(std::forward<params>(src_new)...);

				if (res_add.has_value() == false)
					return std::unexpected(res_add.error());

				this->locate_update(index, block_get_size(block));
				return res_add.value();
			}

			pg::page<T, MAX / 2, allocator_manager>& page_right = block.second.value();
			pg::page<T, MAX / 2, allocator_manager>& page_left = block.first.value();

			if (page_right.get_size_left() == 0)
			{
				pg_data::page_result res_cut = page_right.cut(page_left, 0, 0, static_cast<pg_data::index_page_order>(page_left.get_size()));

				if (res_cut.has_value() == false)
					return std::unexpected(res_cut.error());
			}

			pg_data::page_result_src<T> res_add = page_right.add(std::forward<params>(src_new)...);

			if (res_add.has_value() == false)
				return std::unexpected(res_add.error());

			this->locate_update(index, block_get_size(block));
			return res_add.value();
		}
		template<typename...params>
		mgr_data::manager_result_src_ref<T> block_insert_front(std::size_t index, mgr_data::manager_block<T, MAX, allocator_manager>& block, std::size_t order, params&&...src_new)
		{
			if (block.first.has_value() == false)
				return std::unexpected(kernel_error::manager__manager__block_insert_front__block_invalid_left);

			pg::page<T, MAX / 2, allocator_manager>& page_left = block.first.value();

			if (order < page_left.get_size())
			{
				pg_data::index_page_order page_order_insert = static_cast<pg_data::index_page_order>(order);

				if (page_left.get_size_left() > 0)
				{
					pg_data::page_result_src<T> res_insert = page_left.insert_front(page_order_insert, std::forward<params>(src_new)...);

					if (res_insert.has_value() == false)
						return std::unexpected(res_insert.error());

					this->locate_update(index, block_get_size(block));
					return res_insert.value();
				}

				if (block.second.has_value() == false)
				{
					try { block.second.emplace(pg::page<T, MAX / 2, allocator_manager>(this->allocator_)); }
					catch (...) { return std::unexpected(kernel_error::manager__manager__block_insert_front__emplace_right_unexpected); }
				}

				pg::page<T, MAX / 2, allocator_manager>& page_right = block.second.value();
				pg_data::index_page_order order_cut = static_cast<pg_data::index_page_order>(page_left.get_size() - 1);
				pg_data::page_result res_cut = page_left.cut(page_right, order_cut, order_cut, 0);

				if (res_cut.has_value() == false)
					return std::unexpected(res_cut.error());

				pg_data::page_result_src<T> res_insert = page_left.insert_front(page_order_insert, std::forward<params>(src_new)...);

				if (res_insert.has_value() == false)
					return std::unexpected(res_insert.error());

				this->locate_update(index, block_get_size(block));
				return res_insert.value();
			}

			if (block.second.has_value() == false)
				return std::unexpected(kernel_error::manager__manager__block_insert_front__order_invalid);

			pg::page<T, MAX / 2, allocator_manager>& page_right = block.second.value();
			std::size_t page_order = order - page_left.get_size();

			if (page_order > page_right.get_size())
				return std::unexpected(kernel_error::manager__manager__block_insert_front__order_invalid);

			if (page_order == 0 && page_left.get_size_left() > 0)
			{
				pg_data::page_result_src<T> res_insert = page_left.insert_front(static_cast<pg_data::index_page_order>(page_left.get_size()), std::forward<params>(src_new)...);

				if (res_insert.has_value() == false)
					return std::unexpected(res_insert.error());

				this->locate_update(index, block_get_size(block));
				return res_insert.value();
			}

			if (page_right.get_size_left() == 0)
			{
				pg_data::page_result res_cut = page_right.cut(page_left, 0, 0, static_cast<pg_data::index_page_order>(page_left.get_size()));

				if (res_cut.has_value() == false)
					return std::unexpected(res_cut.error());

				page_order -= 1;
			}

			pg_data::page_result_src<T> res_insert = page_right.insert_front(static_cast<pg_data::index_page_order>(page_order), std::forward<params>(src_new)...);

			if (res_insert.has_value() == false)
				return std::unexpected(res_insert.error());

			this->locate_update(index, block_get_size(block));
			return res_insert.value();
		}
		template<typename...params>
		mgr_data::manager_result_src_ref<T> block_insert_back(std::size_t index, mgr_data::manager_block<T, MAX, allocator_manager>& block, std::size_t order, params&&...src_new)
		{
			if (block.first.has_value() == false)
				return std::unexpected(kernel_error::manager__manager__block_insert_back__block_invalid_left);

			pg::page<T, MAX / 2, allocator_manager>& page_left = block.first.value();

			if (order < page_left.get_size())
			{
				pg_data::index_page_order page_order_insert = static_cast<pg_data::index_page_order>(order);

				if (page_left.get_size_left() > 0)
				{
					pg_data::page_result_src<T> res_insert = page_left.insert_back(page_order_insert, std::forward<params>(src_new)...);

					if (res_insert.has_value() == false)
						return std::unexpected(res_insert.error());

					this->locate_update(index, block_get_size(block));
					return res_insert.value();
				}

				if (block.second.has_value() == false)
				{
					try { block.second.emplace(pg::page<T, MAX / 2, allocator_manager>(this->allocator_)); }
					catch (...) { return std::unexpected(kernel_error::manager__manager__block_insert_back__emplace_right_unexpected); }
				}

				pg::page<T, MAX / 2, allocator_manager>& page_right = block.second.value();

				if (page_order_insert == static_cast<pg_data::index_page_order>(page_left.get_size() - 1))
				{
					pg_data::page_result_src<T> res_insert = page_right.insert_front(0, std::forward<params>(src_new)...);

					if (res_insert.has_value() == false)
						return std::unexpected(res_insert.error());

					this->locate_update(index, block_get_size(block));
					return res_insert.value();
				}

				pg_data::index_page_order order_cut = static_cast<pg_data::index_page_order>(page_left.get_size() - 1);
				pg_data::page_result res_cut = page_left.cut(page_right, order_cut, order_cut, 0);

				if (res_cut.has_value() == false)
					return std::unexpected(res_cut.error());

				pg_data::page_result_src<T> res_insert = page_left.insert_back(page_order_insert, std::forward<params>(src_new)...);

				if (res_insert.has_value() == false)
					return std::unexpected(res_insert.error());

				this->locate_update(index, block_get_size(block));
				return res_insert.value();
			}

			if (block.second.has_value() == false)
				return std::unexpected(kernel_error::manager__manager__block_insert_back__order_invalid);

			pg::page<T, MAX / 2, allocator_manager>& page_right = block.second.value();
			pg_data::index_page_order page_order_insert = static_cast<pg_data::index_page_order>(order - page_left.get_size());

			if (page_order_insert < page_right.get_size())
			{
				pg_data::page_result_src<T> res_insert = page_right.insert_back(page_order_insert, std::forward<params>(src_new)...);

				if (res_insert.has_value() == false)
					return std::unexpected(res_insert.error());

				this->locate_update(index, block_get_size(block));
				return res_insert.value();
			}

			return std::unexpected(kernel_error::manager__manager__block_insert_back__order_invalid);
		}
		constexpr mgr_data::manager_result_src_const_ref<T> block_get_const(const mgr_data::manager_block<T, MAX, allocator_manager>& block, std::size_t order) const noexcept
		{
			if (block.first.has_value() == false)
				return std::unexpected(kernel_error::manager__manager__block_get_const__block_invalid_left);

			const pg::page<T, MAX / 2, allocator_manager>& page_left = block.first.value();

			if (order < page_left.get_size())
			{
				pg_data::page_result_const_src<T> src_target = page_left.get_const(static_cast<pg_data::index_page_order>(order));

				if (src_target.has_value() == false)
					return std::unexpected(src_target.error());

				return src_target.value().get();
			}

			if (block.second.has_value() == false)
				return std::unexpected(kernel_error::manager__manager__block_get_const__order_invalid);

			const pg::page<T, MAX / 2, allocator_manager>& page_right = block.second.value();
			pg_data::index_page_order page_order_get = static_cast<pg_data::index_page_order>(order - page_left.get_size());

			if (page_order_get >= page_right.get_size())
				return std::unexpected(kernel_error::manager__manager__block_get_const__order_invalid);

			pg_data::page_result_const_src<T> src_target = page_right.get_const(page_order_get);

			if (src_target.has_value() == false)
				return std::unexpected(src_target.error());

			return src_target.value().get();
		}
		constexpr mgr_data::manager_result_src_ref<T> block_get(mgr_data::manager_block<T, MAX, allocator_manager>& block, std::size_t order) noexcept
		{
			if (block.first.has_value() == false)
				return std::unexpected(kernel_error::manager__manager__block_get__block_invalid_left);

			pg::page<T, MAX / 2, allocator_manager>& page_left = block.first.value();

			if (order < page_left.get_size())
			{
				pg_data::page_result_src<T> src_target = page_left.get(static_cast<pg_data::index_page_order>(order));

				if (src_target.has_value() == false)
					return std::unexpected(src_target.error());

				return src_target.value().get();
			}

			if (block.second.has_value() == false)
				return std::unexpected(kernel_error::manager__manager__block_get__order_invalid);

			pg::page<T, MAX / 2, allocator_manager>& page_right = block.second.value();
			pg_data::index_page_order page_order_get = static_cast<pg_data::index_page_order>(order - page_left.get_size());

			if (page_order_get >= page_right.get_size())
				return std::unexpected(kernel_error::manager__manager__block_get__order_invalid);

			pg_data::page_result_src<T> src_target = page_right.get(page_order_get);

			if (src_target.has_value() == false)
				return std::unexpected(src_target.error());

			return src_target.value().get();
		}
		constexpr std::size_t block_get_size(const mgr_data::manager_block<T, MAX, allocator_manager>& block) const noexcept
		{
			std::size_t size_total = 0;

			if (block.first.has_value() == true) size_total += block.first.value().get_size();
			if (block.second.has_value() == true) size_total += block.second.value().get_size();

			return size_total;
		}
		constexpr mgr_data::manager_result block_write(mgr_data::manager_block<T, MAX, allocator_manager>& block, std::size_t order, const T& src_new) noexcept
		{
			if (block.first.has_value() == false)
				return std::unexpected(kernel_error::manager__manager__block_write__block_invalid_left);

			pg::page<T, MAX / 2, allocator_manager>& page_left = block.first.value();

			if (order < page_left.get_size())
			{
				pg_data::page_result res_write = page_left.write(static_cast<pg_data::index_page_order>(order), src_new);

				if (res_write.has_value() == false)
					return std::unexpected(res_write.error());

				return {};
			}

			if (block.second.has_value() == false)
				return std::unexpected(kernel_error::manager__manager__block_write__order_invalid);

			pg::page<T, MAX / 2, allocator_manager>& page_right = block.second.value();
			pg_data::index_page_order page_order_write = static_cast<pg_data::index_page_order>(order - page_left.get_size());

			if (page_order_write >= page_right.get_size())
				return std::unexpected(kernel_error::manager__manager__block_write__order_invalid);

			pg_data::page_result res_write = page_right.write(page_order_write, src_new);

			if (res_write.has_value() == false)
				return std::unexpected(res_write.error());

			return {};
		}
		mgr_data::manager_result block_del(std::size_t index, mgr_data::manager_block<T, MAX, allocator_manager>& block, std::size_t order)
		{
			if (block.first.has_value() == false)
				return std::unexpected(kernel_error::manager__manager__block_del__block_invalid_left);

			pg::page<T, MAX / 2, allocator_manager>& page_left = block.first.value();

			if (order < page_left.get_size())
			{
				pg_data::page_result res_del = page_left.del(static_cast<pg_data::index_page_order>(order));

				if (res_del.has_value() == false)
					return std::unexpected(res_del.error());

				this->locate_update(index, block_get_size(block));
				return {};
			}

			if (block.second.has_value() == false)
				return std::unexpected(kernel_error::manager__manager__block_del__order_invalid);

			pg::page<T, MAX / 2, allocator_manager>& page_right = block.second.value();
			pg_data::index_page_order page_order_del = static_cast<pg_data::index_page_order>(order - page_left.get_size());

			if (page_order_del >= page_right.get_size())
				return std::unexpected(kernel_error::manager__manager__block_del__order_invalid);

			pg_data::page_result res_del = page_right.del(page_order_del);

			if (res_del.has_value() == false)
				return std::unexpected(res_del.error());

			this->locate_update(index, block_get_size(block));
			return {};
		}
		constexpr mgr_data::manager_result_index get_index_block_by_order(std::size_t order, std::size_t& order_left_save) const
		{
			if (test_validity_order(order) == false)
				return std::unexpected(kernel_error::manager__manager__get_index_block_by_order__order_invalid);

			typename type_cache_locate::locate_result locate_res = this->cache_locate.locate_by_index(order);

			if (locate_res.has_value() == false)
				return std::unexpected(locate_res.error());

			order_left_save = order - locate_res.value().second;

			return locate_res.value().first;
		}
	private:
		void update_cursor_after_insert(iterator& it_cursor, bool is_cursor_valid, std::size_t cursor_order, std::size_t cursor_block_index, std::size_t cursor_block_order, bool is_insert_at_end, std::size_t insert_order, std::size_t insert_block_index, std::size_t insert_block_order, std::size_t block_size_before)
		{
			if (is_cursor_valid == false || is_insert_at_end == true || insert_order > cursor_order)
				return;

			it_cursor.set_order(it_cursor.get_order() + 1);

			if (this->block_size == block_size_before + std::size_t(1))
			{
				if (cursor_block_index == insert_block_index)
				{
					const std::size_t half = MAX / 2;
					const std::size_t capacity_old_after = (insert_block_order < half) ? (half + std::size_t(1)) : half;
					const std::size_t offset_new = cursor_block_order + std::size_t(1);

					if (offset_new < capacity_old_after)
					{
						it_cursor.set_block_order(offset_new);
					}
					else
					{
						mgr_data::manager_result_ref<T, MAX, allocator_manager> block_old = get_block_ref(insert_block_index, false);

						if (block_old.has_value() == true)
						{
							it_cursor.set_block_index(block_old.value().get().value().next);
							it_cursor.set_order_block(it_cursor.get_order_block() + 1);
							it_cursor.set_block_order(offset_new - capacity_old_after);
						}
						else
						{
							it_cursor = this->begin() + static_cast<typename iterator::difference_type>(cursor_order + std::size_t(1));
						}
					}
				}
				else
				{
					it_cursor.set_order_block(it_cursor.get_order_block() + 1);
				}
			}
			else if (cursor_block_index == insert_block_index)
			{
				it_cursor.set_block_order(it_cursor.get_block_order() + 1);
			}

			return;
		}
	public:
		manager() :
			manager(allocator_manager())
		{

		}
		manager& operator=(const std::initializer_list<T>& params_list)
		{
			clear();

			for (const T& element_val : params_list)
			{
				mgr_data::manager_result_src_ref<T> res_add = this->emplace_back(element_val);

				if (res_add.has_value() == false)
					throw kernel_error_throw(res_add.error(), "kernel | manager | manager | operator= | 初始化列表元素添加失败，赋值无法返回错误码");
			}

			return *this;
		}
		explicit manager(const allocator_manager& allocator_val) :
			allocator_(allocator_val),
			set(allocator_manager_set(allocator_val)),
			free(allocator_manager_free(allocator_val)),
			cache(allocator_manager_cache(allocator_val)),
			cache_locate(allocator_manager_cache_locate(allocator_val)),
			block_size(0),
			element_size(0)
		{

		}
		explicit manager(const std::initializer_list<T>& params_list, const allocator_manager& allocator_val = allocator_manager()) :
			manager(allocator_val)
		{
			for (const T& element_val : params_list)
			{
				mgr_data::manager_result_src_ref<T> res_add = this->emplace_back(element_val);

				if (res_add.has_value() == false)
					throw kernel_error_throw(res_add.error(), "kernel | manager | manager | manager | 初始化列表元素添加失败，构造函数无法返回错误码");
			}
		}
		template<std::forward_iterator iterator_type>
		explicit manager(const iterator_type& it_begin, const iterator_type& it_end, const allocator_manager& allocator_val = allocator_manager()) :
			manager(allocator_val)
		{
			for (iterator_type it_val = it_begin; it_val != it_end; it_val++)
			{
				mgr_data::manager_result_src_ref<T> res_add = this->emplace_back(*it_val);

				if (res_add.has_value() == false)
					throw kernel_error_throw(res_add.error(), "kernel | manager | manager | manager | 迭代器区间元素添加失败，构造函数无法返回错误码");
			}
		}
		manager(const manager& other_val) :
			allocator_(allocator_manager_traits::select_on_container_copy_construction(other_val.allocator_)),
			set(allocator_manager_set(allocator_)),
			free(allocator_manager_free(allocator_)),
			cache(allocator_manager_cache(allocator_)),
			cache_locate(allocator_manager_cache_locate(allocator_)),
			block_size(0),
			element_size(0)
		{
			try
			{
				this->set = other_val.set;
				this->free = other_val.free;
				this->cache = other_val.cache;
				this->cache_locate = other_val.cache_locate;
			}
			catch (const kernel_error_throw&)
			{
				throw;
			}
			catch (...)
			{
				throw kernel_error_throw(kernel_error::manager__manager__copy_construct__copy_unexpected, "kernel | manager | manager | manager | 拷贝构造失败，错误不可恢复");
			}

			this->block_size = other_val.block_size;
			this->element_size = other_val.element_size;
		}
		manager& operator=(const manager& other_val)
		{
			if (this == &other_val)
				return *this;

			if constexpr (allocator_manager_traits::propagate_on_container_copy_assignment::value == true)
				this->allocator_ = other_val.allocator_;

			try
			{
				this->set = other_val.set;
				this->free = other_val.free;
				this->cache = other_val.cache;
				this->cache_locate = other_val.cache_locate;
			}
			catch (const kernel_error_throw&)
			{
				this->clear();
				throw;
			}
			catch (...)
			{
				this->clear();
				throw kernel_error_throw(kernel_error::manager__manager__copy_assign__copy_unexpected, "kernel | manager | manager | operator= | 拷贝赋值失败，容器已清空，错误不可恢复");
			}

			this->block_size = other_val.block_size;
			this->element_size = other_val.element_size;

			return *this;
		}
		manager(manager&& other_val) noexcept :
			allocator_(std::move(other_val.allocator_)),
			set(std::move(other_val.set)),
			free(std::move(other_val.free)),
			cache(std::move(other_val.cache)),
			cache_locate(std::move(other_val.cache_locate)),
			block_size(std::move(other_val.block_size)),
			element_size(std::move(other_val.element_size))
		{
			other_val.block_size = 0;
			other_val.element_size = 0;
		}
		manager& operator=(manager&& other_val)
		{
			if (this == &other_val)
				return *this;

			if constexpr (allocator_manager_traits::propagate_on_container_move_assignment::value == true)
				this->allocator_ = std::move(other_val.allocator_);

			this->set = std::move(other_val.set);
			this->free = std::move(other_val.free);
			this->cache = std::move(other_val.cache);
			this->cache_locate = std::move(other_val.cache_locate);

			this->block_size = std::exchange(other_val.block_size, 0);
			this->element_size = std::exchange(other_val.element_size, 0);

			return *this;
		}
	public:
		mgr_data::manager_result erase(const const_iterator& it_erase)
		{
			mgr_data::manager_result_ref<T, MAX, allocator_manager> block_del_target = get_block_ref(it_erase.get_block_index(), false);

			if (block_del_target.has_value() == false)
				return std::unexpected(block_del_target.error());

			pg_data::element<mgr_data::manager_block<T, MAX, allocator_manager>, mgr_data::index_manager_block>& block_del_ref = block_del_target.value().get().value();
			mgr_data::manager_result del_res = block_del(it_erase.get_order_block(), block_del_ref.src, it_erase.get_block_order());

			if (del_res.has_value() == false)
				return std::unexpected(del_res.error());

			if (block_del_ref.src.second.has_value() == true && block_del_ref.src.second.value().get_size() == 0)
				block_del_ref.src.second.reset();

			if (block_del_ref.src.first.has_value() == true && block_del_ref.src.first.value().get_size() == 0)
				block_del_ref.src.first = std::exchange(block_del_ref.src.second, std::nullopt);

			if (block_del_ref.src.first.has_value() == false)
			{
				mgr_data::manager_result free_res = add_free(it_erase.get_block_index());

				if (free_res.has_value() == false)
					throw kernel_error_throw(free_res.error(), "kernel | manager | manager | erase | 元素已删除，块摘除失败，错误不可恢复");

				if (this->cache_locate.erase(it_erase.get_order_block()) == false)
					throw kernel_error_throw(kernel_error::manager__manager__block_del__order_invalid, "kernel | manager | manager | erase | 元素已删除，块定位缓存摘除失败，错误不可恢复");

				this->cache_del_by_order(it_erase.get_order_block());
				this->block_size--;
			}

			this->element_size--;
			return {};
		}
		mgr_data::manager_result erase(const iterator& it_erase)
		{
			return erase(const_iterator(it_erase));
		}
		mgr_data::manager_result erase(const const_reverse_iterator& it_erase)
		{
			const_iterator it_erase_base = it_erase.base();

			if (it_erase_base == this->cbegin())
				return {};

			--it_erase_base;
			return erase(it_erase_base);
		}
		mgr_data::manager_result erase(const reverse_iterator& it_erase)
		{
			iterator it_erase_base = it_erase.base();

			if (it_erase_base == this->begin())
				return {};

			--it_erase_base;
			return erase(const_iterator(it_erase_base));
		}
		mgr_data::manager_result erase_ex(iterator& it_cursor, const const_iterator& it_erase)
		{
			const bool is_cursor_valid = this->is_valid(it_cursor);
			const std::size_t cursor_order = it_cursor.get_order();
			const std::size_t cursor_block_index = it_cursor.get_block_index();
			const bool is_target_valid = this->is_valid(it_erase);
			const std::size_t target_order = it_erase.get_order();
			const std::size_t target_block_index = it_erase.get_block_index();
			const std::size_t target_block_order = it_erase.get_block_order();
			std::size_t target_block_size = 0;
			mgr_data::index_manager_block target_block_next = mgr_data::INDEX_MANAGER_BLOCK_END;

			if (is_target_valid == true)
			{
				mgr_data::manager_result_ref<T, MAX, allocator_manager> block_target = get_block_ref(target_block_index, false);

				if (block_target.has_value() == true)
				{
					target_block_size = block_get_size(block_target.value().get().value().src);
					target_block_next = block_target.value().get().value().next;
				}
			}

			mgr_data::manager_result res_erase = erase(it_erase);

			if (res_erase.has_value() == false)
				return res_erase;

			if (is_cursor_valid == false || is_target_valid == false || target_block_size == 0)
				return res_erase;

			if (target_order < cursor_order)
			{
				it_cursor.set_order(it_cursor.get_order() - 1);

				if (target_block_index == cursor_block_index)
					it_cursor.set_block_order(it_cursor.get_block_order() - 1);
				else if (target_block_size == std::size_t(1))
					it_cursor.set_order_block(it_cursor.get_order_block() - 1);

				return res_erase;
			}

			if (target_order == cursor_order)
			{
				if (target_block_order + std::size_t(1) < target_block_size)
					return res_erase;

				if (target_block_size > std::size_t(1))
				{
					if (target_block_next != mgr_data::INDEX_MANAGER_BLOCK_END)
					{
						it_cursor.set_block_index(target_block_next);
						it_cursor.set_order_block(it_cursor.get_order_block() + 1);
						it_cursor.set_block_order(0);
						return res_erase;
					}

					it_cursor.set_end();
					return res_erase;
				}

				if (target_block_next != mgr_data::INDEX_MANAGER_BLOCK_END)
				{
					it_cursor.set_block_index(target_block_next);
					it_cursor.set_block_order(0);
					return res_erase;
				}

				it_cursor.set_end();
				return res_erase;
			}

			return res_erase;
		}
		mgr_data::manager_result erase_ex(iterator& it_cursor, const iterator& it_erase)
		{
			return erase_ex(it_cursor, const_iterator(it_erase));
		}
		mgr_data::manager_result_src_const_ref<T> at(std::size_t order) const
		{
			std::size_t order_left = 0;
			mgr_data::manager_result_index index_get = get_index_block_by_order(order, order_left);

			if (index_get.has_value() == false)
				return std::unexpected(index_get.error());

			mgr_data::manager_result_ref_const<T, MAX, allocator_manager> block_get_target = get_block_ref_const(index_get.value(), false);

			if (block_get_target.has_value() == false)
				return std::unexpected(block_get_target.error());

			const pg_data::element<mgr_data::manager_block<T, MAX, allocator_manager>, mgr_data::index_manager_block>& block_get_ref = block_get_target.value().get().value();

			return block_get_const(block_get_ref.src, order_left);
		}
		mgr_data::manager_result_src_ref<T> at(std::size_t order)
		{
			std::size_t order_left = 0;
			mgr_data::manager_result_index index_get = get_index_block_by_order(order, order_left);

			if (index_get.has_value() == false)
				return std::unexpected(index_get.error());

			mgr_data::manager_result_ref<T, MAX, allocator_manager> block_get_target = get_block_ref(index_get.value(), false);

			if (block_get_target.has_value() == false)
				return std::unexpected(block_get_target.error());

			pg_data::element<mgr_data::manager_block<T, MAX, allocator_manager>, mgr_data::index_manager_block>& block_get_ref = block_get_target.value().get().value();

			return block_get(block_get_ref.src, order_left);
		}
		constexpr iterator begin() noexcept
		{
			cmap_data::cache_mapping_result_pos<mgr_data::index_manager_block> block_index_begin = this->cache.get_front();

			if (block_index_begin.has_value() == false) return iterator(*this);

			return iterator(*this, 0, block_index_begin.value(), 0, 0);
		}
		constexpr iterator end() noexcept
		{
			return iterator(*this);
		}
		constexpr const_iterator begin() const noexcept
		{
			cmap_data::cache_mapping_result_pos<mgr_data::index_manager_block> block_index_begin = this->cache.get_front();

			if (block_index_begin.has_value() == false) return const_iterator(*this);

			return const_iterator(*this, 0, block_index_begin.value(), 0, 0);
		}
		constexpr const_iterator end() const noexcept
		{
			return const_iterator(*this);
		}
		constexpr const_iterator cbegin() const noexcept
		{
			cmap_data::cache_mapping_result_pos<mgr_data::index_manager_block> block_index_begin = this->cache.get_front();

			if (block_index_begin.has_value() == false) return const_iterator(*this);

			return const_iterator(*this, 0, block_index_begin.value(), 0, 0);
		}
		constexpr const_iterator cend() const noexcept
		{
			return const_iterator(*this);
		}
		constexpr reverse_iterator rbegin() noexcept
		{
			return reverse_iterator(end());
		}
		constexpr reverse_iterator rend() noexcept
		{
			return reverse_iterator(begin());
		}
		constexpr const_reverse_iterator rbegin() const noexcept
		{
			return const_reverse_iterator(end());
		}
		constexpr const_reverse_iterator rend() const noexcept
		{
			return const_reverse_iterator(begin());
		}
		constexpr const_reverse_iterator crbegin() const noexcept
		{
			return const_reverse_iterator(end());
		}
		constexpr const_reverse_iterator crend() const noexcept
		{
			return const_reverse_iterator(begin());
		}
	public:
		[[nodiscard]] bool is_valid(const iterator& it) const noexcept
		{
			return it.get_manager_ref() == this && it.is_valid() == true;
		}
		[[nodiscard]] bool is_valid(const const_iterator& it) const noexcept
		{
			return it.get_manager_ref() == this && it.is_valid() == true;
		}
		[[nodiscard]] bool is_valid(const reverse_iterator& it) const noexcept
		{
			const_iterator it_base = it.base();

			if (it_base.get_manager_ref() != this)
				return false;

			return !(it_base == this->begin());
		}
		[[nodiscard]] bool is_valid(const const_reverse_iterator& it) const noexcept
		{
			const_iterator it_base = it.base();

			if (it_base.get_manager_ref() != this)
				return false;

			return !(it_base == this->begin());
		}
		constexpr std::size_t size() const noexcept
		{
			return this->element_size;
		}
		mgr_data::manager_result_src_ref<T> push_back(const T& src_new)
		{
			return emplace_back(src_new);
		}
		mgr_data::manager_result pop_back()
		{
			if (this->element_size == 0)
				return std::unexpected(kernel_error::manager__manager__pop_back__empty);

			return erase(cend() - 1);
		}
		template<typename...params>
		mgr_data::manager_result_src_ref<T> emplace(const const_iterator& it_emplace, params&&...src_new)
		{
			mgr_data::manager_result_ref<T, MAX, allocator_manager> block_insert = get_block_ref(it_emplace.get_block_index(), false);

			if (block_insert.has_value() == false)
				return emplace_back(std::forward<params>(src_new)...);

			pg_data::element<mgr_data::manager_block<T, MAX, allocator_manager>, mgr_data::index_manager_block>& block_insert_ref = block_insert.value().get().value();

			if (block_get_size(block_insert_ref.src) < MAX)
			{
				mgr_data::manager_result_src_ref<T> res_insert = block_insert_front(it_emplace.get_order_block(), block_insert_ref.src, it_emplace.get_block_order(), std::forward<params>(src_new)...);

				if (res_insert.has_value() == false)
					return std::unexpected(res_insert.error());

				this->element_size++;
				return res_insert.value();
			}

			mgr_data::manager_result_index index_new = insert_back_block(it_emplace.get_order_block());

			if (index_new.has_value() == false)
				return std::unexpected(index_new.error());

			mgr_data::manager_result_ref<T, MAX, allocator_manager> block_insert_new = get_block_ref(index_new.value(), false);

			if (block_insert_new.has_value() == false)
				throw kernel_error_throw(block_insert_new.error(), "kernel | manager | manager | emplace | 新块引用获取失败，块已分割，错误不可恢复");

			pg_data::element<mgr_data::manager_block<T, MAX, allocator_manager>, mgr_data::index_manager_block>& block_insert_new_ref = block_insert_new.value().get().value();

			block_insert = get_block_ref(it_emplace.get_block_index(), false);

			if (block_insert.has_value() == false)
				throw kernel_error_throw(block_insert.error(), "kernel | manager | manager | emplace | 旧块引用重获取失败，块已分割，错误不可恢复");

			pg_data::element<mgr_data::manager_block<T, MAX, allocator_manager>, mgr_data::index_manager_block>& block_insert_ref_old = block_insert.value().get().value();

			block_insert_new_ref.src.first = std::exchange(block_insert_ref_old.src.second, std::nullopt);

			this->locate_update(it_emplace.get_order_block(), block_get_size(block_insert_ref_old.src));
			this->locate_update(it_emplace.get_order_block() + 1, block_get_size(block_insert_new_ref.src));

			const bool is_insert_new_block = it_emplace.get_block_order() >= MAX / 2;
			mgr_data::manager_result_src_ref<T> res_insert = is_insert_new_block == true
				? block_insert_front(it_emplace.get_order_block() + 1, block_insert_new_ref.src, it_emplace.get_block_order() - (MAX / 2), std::forward<params>(src_new)...)
				: block_insert_front(it_emplace.get_order_block(), block_insert_ref_old.src, it_emplace.get_block_order(), std::forward<params>(src_new)...);

			if (res_insert.has_value() == false)
			{
				roll_back_split(it_emplace.get_order_block(), index_new.value());

				return std::unexpected(res_insert.error());
			}

			this->element_size++;
			return res_insert.value();
		}
		template<typename...params>
		mgr_data::manager_result_src_ref<T> emplace_back(params&&...src_new)
		{
			cmap_data::cache_mapping_result_pos<mgr_data::index_manager_block> block_last = this->cache.get_back();

			if (block_last.has_value() == true)
			{
				mgr_data::manager_result_ref<T, MAX, allocator_manager> block_back = get_block_ref(block_last.value(), false);

				if (block_back.has_value() == false)
					return std::unexpected(block_back.error());

				if (block_get_size(block_back.value().get().value().src) < MAX)
				{
					mgr_data::manager_result_src_ref<T> res_add = block_add(this->block_size - 1, block_back.value().get().value().src, std::forward<params>(src_new)...);

					if (res_add.has_value() == false)
						return std::unexpected(res_add.error());

					this->element_size++;
					return res_add.value();
				}
			}

			mgr_data::manager_result_index index_new = add_block();

			if (index_new.has_value() == false)
				return std::unexpected(index_new.error());

			mgr_data::manager_result_ref<T, MAX, allocator_manager> block_new = get_block_ref(index_new.value(), false);

			if (block_new.has_value() == false)
			{
				remove_block_roll_back(this->block_size - 1, index_new.value());

				return std::unexpected(block_new.error());
			}

			mgr_data::manager_result_src_ref<T> res_add = block_add(this->block_size - 1, block_new.value().get().value().src, std::forward<params>(src_new)...);

			if (res_add.has_value() == false)
			{
				remove_block_roll_back(this->block_size - 1, index_new.value());

				return std::unexpected(res_add.error());
			}

			this->element_size++;
			return res_add.value();
		}
		template<typename...params>
		result_iterator insert(const const_iterator& it_insert, params&&...src_new)
		{
			iterator it_res(*this, it_insert.get_order_block(), it_insert.get_block_index(), it_insert.get_block_order(), it_insert.get_order());
			mgr_data::manager_result_ref<T, MAX, allocator_manager> block_insert = get_block_ref(it_insert.get_block_index(), false);

			if (block_insert.has_value() == false)
			{
				mgr_data::manager_result_src_ref<T> res_add = emplace_back(std::forward<params>(src_new)...);

				if (res_add.has_value() == false)
					return std::unexpected(res_add.error());

				cmap_data::cache_mapping_result_pos<mgr_data::index_manager_block> block_index_new = this->cache.get_back();

				if (block_index_new.has_value() == false)
					return std::unexpected(block_index_new.error());

				mgr_data::manager_result_ref<T, MAX, allocator_manager> block_new = get_block_ref(block_index_new.value(), false);

				if (block_new.has_value() == false)
					return std::unexpected(block_new.error());

				return iterator(*this, this->block_size - 1, block_index_new.value(), block_get_size(block_new.value().get().value().src) - 1, this->element_size - 1);
			}

			pg_data::element<mgr_data::manager_block<T, MAX, allocator_manager>, mgr_data::index_manager_block>& block_insert_ref = block_insert.value().get().value();

			if (block_get_size(block_insert_ref.src) < MAX)
			{
				mgr_data::manager_result_src_ref<T> res_insert = block_insert_front(it_insert.get_order_block(), block_insert_ref.src, it_insert.get_block_order(), std::forward<params>(src_new)...);

				if (res_insert.has_value() == false)
					return std::unexpected(res_insert.error());

				this->element_size++;
				return it_res;
			}

			mgr_data::manager_result_index index_new = insert_back_block(it_insert.get_order_block());

			if (index_new.has_value() == false)
				return std::unexpected(index_new.error());

			mgr_data::manager_result_ref<T, MAX, allocator_manager> block_insert_new = get_block_ref(index_new.value(), false);

			if (block_insert_new.has_value() == false)
				throw kernel_error_throw(block_insert_new.error(), "kernel | manager | manager | insert | 新块引用获取失败，块已分割，错误不可恢复");

			pg_data::element<mgr_data::manager_block<T, MAX, allocator_manager>, mgr_data::index_manager_block>& block_insert_new_ref = block_insert_new.value().get().value();

			block_insert = get_block_ref(it_insert.get_block_index(), false);

			if (block_insert.has_value() == false)
				throw kernel_error_throw(block_insert.error(), "kernel | manager | manager | insert | 旧块引用重获取失败，块已分割，错误不可恢复");

			pg_data::element<mgr_data::manager_block<T, MAX, allocator_manager>, mgr_data::index_manager_block>& block_insert_ref_old = block_insert.value().get().value();

			block_insert_new_ref.src.first = std::exchange(block_insert_ref_old.src.second, std::nullopt);

			this->locate_update(it_insert.get_order_block(), block_get_size(block_insert_ref_old.src));
			this->locate_update(it_insert.get_order_block() + 1, block_get_size(block_insert_new_ref.src));

			const bool is_insert_new_block = it_insert.get_block_order() >= MAX / 2;
			mgr_data::manager_result_src_ref<T> res_insert = is_insert_new_block == true
				? block_insert_front(it_insert.get_order_block() + 1, block_insert_new_ref.src, it_insert.get_block_order() - (MAX / 2), std::forward<params>(src_new)...)
				: block_insert_front(it_insert.get_order_block(), block_insert_ref_old.src, it_insert.get_block_order(), std::forward<params>(src_new)...);

			if (res_insert.has_value() == false)
			{
				roll_back_split(it_insert.get_order_block(), index_new.value());

				return std::unexpected(res_insert.error());
			}

			if (is_insert_new_block == true)
			{
				it_res.set_order_block(it_res.get_order_block() + 1);
				it_res.set_block_index(index_new.value());
				it_res.set_block_order(it_insert.get_block_order() - (MAX / 2));
			}

			this->element_size++;
			return it_res;
		}
		template<typename...params>
		result_iterator insert(const iterator& it_insert, params&&...src_new)
		{
			return insert(const_iterator(it_insert), std::forward<params>(src_new)...);
		}
		template<typename...params>
		result_reverse_iterator insert(const reverse_iterator& it_insert, params&&...src_new)
		{
			iterator pos = it_insert.base();

			if (pos != this->begin())
				--pos;

			result_iterator it_new = insert(const_iterator(pos), std::forward<params>(src_new)...);

			if (it_new.has_value() == false)
				return std::unexpected(it_new.error());

			return reverse_iterator(it_new.value() + 1);
		}
		template<typename...params>
		result_reverse_iterator insert(const const_reverse_iterator& it_insert, params&&...src_new)
		{
			const_iterator pos = it_insert.base();

			if (pos != this->cbegin())
				--pos;

			result_iterator it_new = insert(pos, std::forward<params>(src_new)...);

			if (it_new.has_value() == false)
				return std::unexpected(it_new.error());

			return reverse_iterator(it_new.value() + 1);
		}
	public:
		template<typename...params>
		result_iterator insert_ex(iterator& it_cursor, const const_iterator& it_insert, params&&...src_new)
		{
			const bool is_cursor_valid = this->is_valid(it_cursor);
			const std::size_t cursor_order = it_cursor.get_order();
			const std::size_t cursor_block_index = it_cursor.get_block_index();
			const std::size_t cursor_block_order = it_cursor.get_block_order();
			const bool is_insert_at_end = it_insert.get_order() >= this->element_size;
			const std::size_t insert_order = it_insert.get_order();
			const std::size_t insert_block_index = it_insert.get_block_index();
			const std::size_t insert_block_order = it_insert.get_block_order();
			const std::size_t block_size_before = this->block_size;

			result_iterator res_insert = insert(it_insert, std::forward<params>(src_new)...);

			if (res_insert.has_value() == false)
				return res_insert;

			update_cursor_after_insert(it_cursor, is_cursor_valid, cursor_order, cursor_block_index, cursor_block_order, is_insert_at_end, insert_order, insert_block_index, insert_block_order, block_size_before);
			return res_insert;
		}
		template<typename...params>
		result_iterator insert_ex(iterator& it_cursor, const iterator& it_insert, params&&...src_new)
		{
			return insert_ex(it_cursor, const_iterator(it_insert), std::forward<params>(src_new)...);
		}
		template<typename...params>
		mgr_data::manager_result_src_ref<T> emplace_ex(iterator& it_cursor, const const_iterator& it_emplace, params&&...src_new)
		{
			const bool is_cursor_valid = this->is_valid(it_cursor);
			const std::size_t cursor_order = it_cursor.get_order();
			const std::size_t cursor_block_index = it_cursor.get_block_index();
			const std::size_t cursor_block_order = it_cursor.get_block_order();
			const bool is_insert_at_end = it_emplace.get_order() >= this->element_size;
			const std::size_t insert_order = it_emplace.get_order();
			const std::size_t insert_block_index = it_emplace.get_block_index();
			const std::size_t insert_block_order = it_emplace.get_block_order();
			const std::size_t block_size_before = this->block_size;

			mgr_data::manager_result_src_ref<T> res_emplace = emplace(it_emplace, std::forward<params>(src_new)...);

			if (res_emplace.has_value() == false)
				return res_emplace;

			update_cursor_after_insert(it_cursor, is_cursor_valid, cursor_order, cursor_block_index, cursor_block_order, is_insert_at_end, insert_order, insert_block_index, insert_block_order, block_size_before);
			return res_emplace;
		}
		template<typename...params>
		mgr_data::manager_result_src_ref<T> emplace_ex(iterator& it_cursor, const iterator& it_emplace, params&&...src_new)
		{
			return emplace_ex(it_cursor, const_iterator(it_emplace), std::forward<params>(src_new)...);
		}
		template<typename...params>
		mgr_data::manager_result_src_ref<T> emplace_back_ex(iterator& it_cursor, params&&...src_new)
		{
			(void)it_cursor;
			return emplace_back(std::forward<params>(src_new)...);
		}
		mgr_data::manager_result_src_ref<T> back()
		{
			if (this->element_size == 0)
				return std::unexpected(kernel_error::manager__manager__back__empty);

			return at(this->element_size - 1);
		}
		mgr_data::manager_result_src_const_ref<T> back() const
		{
			if (this->element_size == 0)
				return std::unexpected(kernel_error::manager__manager__back__empty);

			return at(this->element_size - 1);
		}
		mgr_data::manager_result_src_ref<T> front()
		{
			if (this->element_size == 0)
				return std::unexpected(kernel_error::manager__manager__front__empty);

			return at(0);
		}
		mgr_data::manager_result_src_const_ref<T> front() const
		{
			if (this->element_size == 0)
				return std::unexpected(kernel_error::manager__manager__front__empty);

			return at(0);
		}
		void swap(manager& right) noexcept
		{
			if (this == &right)
				return;

			std::swap(this->allocator_, right.allocator_);
			this->set.swap(right.set);
			this->free.swap(right.free);
			this->cache.swap(right.cache);
			this->cache_locate.swap(right.cache_locate);
			std::swap(this->block_size, right.block_size);
			std::swap(this->element_size, right.element_size);

			return;
		}
		mgr_data::manager_result shrink_to_fit()
		{
			cmap_data::cache_mapping_result_pos<mgr_data::index_manager_block> block_index_current = this->cache.get_front();
			std::size_t index_current = 0;

			while (block_index_current.has_value() == true)
			{
				mgr_data::manager_result_ref<T, MAX, allocator_manager> block_target = get_block_ref(block_index_current.value(), false);

				if (block_target.has_value() == false)
					return std::unexpected(block_target.error());

				mgr_data::manager_block<T, MAX, allocator_manager>& block_ref = block_target.value().get().value().src;

				if (block_ref.first.has_value() == true) block_ref.first.value().rebuild();
				if (block_ref.second.has_value() == true) block_ref.second.value().rebuild();

				block_index_current = this->cache.get(++index_current);
			}

			return {};
		}
		constexpr bool empty() const noexcept
		{
			return this->element_size == 0;
		}
		constexpr void clear() noexcept
		{
			this->set.clear();
			this->free.clear();
			this->cache.clear();
			this->cache_locate.clear();
			this->block_size = 0;
			this->element_size = 0;
			return;
		}
	public:
		mgr_data::manager_result_src_const_ref<T> operator[](std::size_t order) const
		{
			return at(order);
		}
		mgr_data::manager_result_src_ref<T> operator[](std::size_t order)
		{
			return at(order);
		}
	};

	template<typename T, typename allocator_container_plus = std::allocator<T>>
	using container_plus = manager<T, mgr_data::BLOCK_SIZE_DEFAULT, allocator_container_plus>;
}
