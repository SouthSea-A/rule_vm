#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <cstddef>
#include <utility>
#include <type_traits>
#include <functional>
#include <optional>
#include <expected>
#include <limits>
#include <algorithm>
#include <vector>
#include "page_data_def.h"
#include "cache_mapping.h"

namespace rule_page
{
	template<typename T, std::size_t MAX, typename allocator_page>
	class page
	{
		static_assert(MAX != 0, "页容量无效");
		static_assert(MAX < std::numeric_limits<pg_data::page_data_type_base_16>::max(), "页大小超出最大范围");
	private:
		using allocator_page_traits = typename std::allocator_traits<allocator_page>;
		using allocator_page_set = allocator_page_traits::template rebind_alloc<std::optional<pg_data::element<T, pg_data::index_page_pos>>>;
		using allocator_page_free = allocator_page_traits::template rebind_alloc<pg_data::index_page_pos>;
		using allocator_page_cache = allocator_page_traits::template rebind_alloc<pg_data::index_page_pos>;
	private:
		pg_data::index_page_pos last;
		std::vector<std::optional<pg_data::element<T, pg_data::index_page_pos>>, allocator_page_set> set;
		std::vector<pg_data::index_page_pos, allocator_page_free> free;
		cmap::cache_mapping<pg_data::index_page_order, pg_data::index_page_pos, MAX, allocator_page_cache> cache;
	private:
		[[nodiscard]] constexpr bool test_validity_index_page_pos(pg_data::index_page_pos index, bool is_test_free) const noexcept
		{
			if (static_cast<std::size_t>(index) >= this->set.size())
				return false;

			return is_test_free == false ? true : this->set[static_cast<std::size_t>(index)].has_value() == true;
		}
		[[nodiscard]] constexpr bool test_validity_index_page_order(pg_data::index_page_order index) const noexcept
		{
			return static_cast<std::size_t>(index) < (this->set.size() - this->free.size());
		}
		constexpr pg_data::page_result_pos get_pos_by_order(pg_data::index_page_order index) const noexcept
		{
			cmap_data::cache_mapping_result_pos<pg_data::index_page_pos> pos_order_cache = this->cache.get(index);

			if (pos_order_cache.has_value() == false)
				return std::unexpected(pos_order_cache.error());

			return pos_order_cache.value();
		}
		constexpr pg_data::page_result_ref<T> get_element_ref_by_order(pg_data::index_page_order index) noexcept
		{
			pg_data::page_result_pos pos_target = get_pos_by_order(index);

			if (pos_target.has_value() == false)
				return std::unexpected(pos_target.error());

			return get_element_ref_by_pos(pos_target.value());
		}
		constexpr pg_data::page_result_ref<T> get_element_ref_by_pos(pg_data::index_page_pos index, bool is_get_free = false) noexcept
		{
			if (test_validity_index_page_pos(index, !is_get_free) == false)
				return std::unexpected(kernel_error::manager__page__get_element_ref_by_pos__index_pos_invalid);

			try { return this->set.at(static_cast<std::size_t>(index)); }
			catch (...) { return std::unexpected(kernel_error::manager__page__get_element_ref_by_pos__read_unexpected); }
		}
		constexpr pg_data::page_result_const_ref<T> get_element_const_ref_by_pos(pg_data::index_page_pos index, bool is_get_free = false) const noexcept
		{
			if (test_validity_index_page_pos(index, !is_get_free) == false)
				return std::unexpected(kernel_error::manager__page__get_element_const_ref_by_pos__index_pos_invalid);

			try { return this->set.at(static_cast<std::size_t>(index)); }
			catch (...) { return std::unexpected(kernel_error::manager__page__get_element_const_ref_by_pos__read_unexpected); }
		}
		pg_data::page_result add_free_by_order(pg_data::index_page_order index)
		{
			pg_data::page_result_pos pos_target = get_pos_by_order(index);

			if (pos_target.has_value() == false)
				return std::unexpected(pos_target.error());

			pg_data::index_page_pos index_pos = pos_target.value();
			pg_data::page_result_ref<T> element_target = get_element_ref_by_pos(index_pos);

			if (element_target.has_value() == false)
				return std::unexpected(element_target.error());

			try { this->free.emplace_back(index_pos); }
			catch (...) { return std::unexpected(kernel_error::manager__page__add_free_by_order__emplace_back_unexpected); }

			element_target.value().get().reset();

			return {};
		}
		pg_data::page_result add_free_by_pos(pg_data::index_page_pos index)
		{
			pg_data::page_result_ref<T> element_target = get_element_ref_by_pos(index);

			if (element_target.has_value() == false)
				return std::unexpected(element_target.error());

			try { this->free.emplace_back(index); }
			catch (...) { return std::unexpected(kernel_error::manager__page__add_free_by_pos__emplace_back_unexpected); }

			element_target.value().get().reset();

			return {};
		}
		void add_free_roll_back(pg_data::index_page_pos index)
		{
			try { this->free.emplace_back(index); }
			catch (...) { throw kernel_error_throw(kernel_error::manager__page__add_free_roll_back__emplace_back_unexpected, "kernel | manager | page | add_free_roll_back | 空闲位置回写失败，错误无法回滚"); }

			return;
		}
		pg_data::page_result_pos get_free() noexcept
		{
			if (this->free.empty() == true)
				return std::unexpected(kernel_error::manager__page__get_free__empty);

			pg_data::index_page_pos pos_free = this->free.back();

			this->free.pop_back();

			return pos_free;
		}
		void tidy_element(bool is_realloc)
		{
			pg_data::page_result_ref<T> element_current = get_element_ref_by_pos(this->last);

			while (true)
			{
				if (element_current.has_value() == true && element_current.value().get().has_value() == true)
					break;

				if (this->last == 0)
					return;

				element_current = get_element_ref_by_pos(--this->last);
			}

			if (this->last == 0)
				return;

			if (this->free.empty() == true)
				return;

			cmap_data::cache_mapping_result_pos<pg_data::index_page_pos> pos_begin_res = this->cache.get_front();

			if (pos_begin_res.has_value() == false)
				return;

			pg_data::index_page_pos pos_begin = pos_begin_res.value();

			std::sort(this->free.begin(), this->free.end());

			typename std::vector<pg_data::index_page_pos, allocator_page_free>::const_iterator pos_free_it = this->free.begin();
			pg_data::index_page_pos pos_current = this->last;
			pg_data::index_page_pos pos_free = *pos_free_it;
			pg_data::page_result_ref<T> element_free = get_element_ref_by_pos(pos_free, true);

			while (pos_free < pos_current)
			{
				if (element_free.has_value() == true && element_free.value().get().has_value() == false && element_current.has_value() == true && element_current.value().get().has_value() == true)
				{
					element_free.value().get() = std::move(element_current.value().get());
					element_current.value().get().reset();

					if (pos_current == pos_begin)
						pos_begin = pos_free;

					pg_data::element<T, pg_data::index_page_pos>& element_moved_ref = element_free.value().get().value();

					pg_data::page_result_ref<T> element_last = get_element_ref_by_pos(element_moved_ref.last);
					pg_data::page_result_ref<T> element_next = get_element_ref_by_pos(element_moved_ref.next);

					if (element_last.has_value() == true && element_last.value().get().has_value() == true)
						element_last.value().get().value().next = pos_free;

					if (element_next.has_value() == true && element_next.value().get().has_value() == true)
						element_next.value().get().value().last = pos_free;

					pos_free_it++;
				}

				if (pos_free_it == this->free.end())
					break;

				pos_free = *pos_free_it;
				element_free = get_element_ref_by_pos(pos_free, true);
				element_current = get_element_ref_by_pos(--pos_current);
			}

			while (true)
			{
				element_current = get_element_ref_by_pos(pos_current);

				if (element_current.has_value() == true && element_current.value().get().has_value() == true)
					break;

				if (pos_current == 0)
					break;

				pos_current--;
			}

			if (element_current.has_value() == true && element_current.value().get().has_value() == true)
			{
				this->last = pos_current;

				std::size_t size_new = static_cast<std::size_t>(pos_current + 1);

				try { this->set.resize(size_new); }
				catch (...) { throw kernel_error_throw(kernel_error::manager__page__tidy_element__resize_unexpected, "内存大小重置失败，错误不可恢复"); }

				if (is_realloc == true)
				{
					try { this->set.shrink_to_fit(); }
					catch (...) { throw kernel_error_throw(kernel_error::manager__page__tidy_element__shrink_to_fit_unexpected, "内存大小收缩失败，错误不可恢复"); }
				}

				this->free.clear();

				try { this->free.shrink_to_fit(); }
				catch (...) { throw kernel_error_throw(kernel_error::manager__page__tidy_element__shrink_to_fit_unexpected, "内存大小收缩失败，错误不可恢复"); }
			}
			else
			{
				this->last = 0;

				this->set.clear();
				this->free.clear();

				if (is_realloc == true)
				{
					try { this->set.shrink_to_fit(); }
					catch (...) { throw kernel_error_throw(kernel_error::manager__page__tidy_element__shrink_to_fit_unexpected, "内存大小收缩失败，错误不可恢复"); }
				}

				try { this->free.shrink_to_fit(); }
				catch (...) { throw kernel_error_throw(kernel_error::manager__page__tidy_element__shrink_to_fit_unexpected, "内存大小收缩失败，错误不可恢复"); }
			}

			this->cache.clear();

			pos_current = pos_begin;

			pg_data::index_page_order order_current = 0;

			while (order_current < this->set.size())
			{
				element_current = get_element_ref_by_pos(pos_current);

				if (element_current.has_value() == false)
					break;

				if (element_current.value().get().has_value() == false)
					break;

				cmap_data::cache_mapping_result cache_add_set_res = this->cache.add_set(order_current, pos_current);

				if (cache_add_set_res.has_value() == false)
					throw kernel_error_throw(cache_add_set_res.error(), "kernel | manager | page | tidy_element | 新缓存位置写入失败，错误不可恢复");

				pos_current = element_current.value().get().value().next;
				order_current++;
			}

			return;
		}
	public:
		page() :
			last(0), set(), free(), cache(MAX)
		{

		}
		explicit page(const allocator_page& allocator_val) :
			last(0),
			set(allocator_page_set(allocator_val)),
			free(allocator_page_free(allocator_val)),
			cache(allocator_page_cache(allocator_val), MAX)
		{

		}
	public:
		template<typename...params>
		pg_data::page_result_src<T> add(params&&...src_new)
		{
			cmap_data::cache_mapping_result_pos<pg_data::index_page_pos> pos_end_res = this->cache.get_back();
			pg_data::index_page_pos pos_new = 0;
			pg_data::index_page_order order_new = 0;
			pg_data::page_result_ref<T> element_end = pos_end_res.has_value() == false ? std::unexpected(pos_end_res.error()) : get_element_ref_by_pos(pos_end_res.value());
			pg_data::page_result_src<T> result_ref = std::unexpected(kernel_error::manager__page__add__placeholder);

			if (element_end.has_value() == true && pos_end_res.has_value() == true)
			{
				std::optional<pg_data::element<T, pg_data::index_page_pos>>& element_end_ref = element_end.value().get();

				if (element_end_ref.has_value() == false)
					return std::unexpected(kernel_error::manager__page__add__element_missing);

				pg_data::page_result_pos pos_free = get_free();

				if (pos_free.has_value() == false)
				{
					if (this->set.size() >= MAX)
						return std::unexpected(kernel_error::manager__page__add__size_max);

					pos_new = static_cast<pg_data::index_page_pos>(this->set.size());
					order_new = static_cast<pg_data::index_page_pos>(this->set.size());
					element_end_ref.value().next = pos_new;

					try
					{
						result_ref = this->set.emplace_back(std::in_place, pos_end_res.value(), pg_data::INDEX_PAGE_POS_END, std::forward<params>(src_new)...).value().src;
					}
					catch (...)
					{
						element_end = get_element_ref_by_pos(pos_end_res.value());

						if (element_end.has_value() == false)
							throw kernel_error_throw(element_end.error(), "kernel | manager | add | 元素尾插构造失败，重获取原最后一个元素失败，无法重写元素链指向，错误无法回滚");

						element_end.value().get().value().next = pg_data::INDEX_PAGE_POS_END;
						return std::unexpected(kernel_error::manager__page__add__emplace_back_unexpected);
					}

					cmap_data::cache_mapping_result cache_add_res = this->cache.add_set(order_new, pos_new);

					if (cache_add_res.has_value() == false)
					{
						this->set.pop_back();
						element_end = get_element_ref_by_pos(pos_end_res.value());

						if (element_end.has_value() == false)
							throw kernel_error_throw(element_end.error(), "kernel | manager | add | 缓存位置添加失败，重获取原最后一个元素失败，无法重写元素链指向，错误无法回滚");

						element_end.value().get().value().next = pg_data::INDEX_PAGE_POS_END;
						return std::unexpected(cache_add_res.error());
					}
				}
				else
				{
					pg_data::page_result_ref<T> element_free = get_element_ref_by_pos(pos_free.value(), true);

					if (element_free.has_value() == false)
					{
						add_free_roll_back(pos_free.value());
						return std::unexpected(element_free.error());
					}

					std::optional<pg_data::element<T, pg_data::index_page_pos>>& element_free_ref = element_free.value().get();

					try
					{
						element_free_ref.emplace(pos_end_res.value(), pg_data::INDEX_PAGE_POS_END, std::forward<params>(src_new)...);
					}
					catch (...)
					{
						element_free_ref.reset();
						add_free_roll_back(pos_free.value());
						return std::unexpected(kernel_error::manager__page__add__emplace_unexpected);
					}

					pos_new = pos_free.value();
					order_new = static_cast<pg_data::index_page_pos>(this->set.size() - this->free.size() - 1);

					cmap_data::cache_mapping_result cache_add_res = this->cache.add_set(order_new, pos_new);

					if (cache_add_res.has_value() == false)
					{
						element_free_ref.reset();
						add_free_roll_back(pos_free.value());
						return std::unexpected(cache_add_res.error());
					}

					element_end_ref.value().next = pos_new;
					result_ref = element_free_ref.value().src;
				}

				if (pos_new > this->last) this->last = pos_new;
			}
			else
			{
				order_new = 0;
				const bool reuse_free = this->free.empty() == false;

				if (reuse_free)
				{
					pos_new = this->free.back();
					pg_data::page_result_ref<T> element_free = get_element_ref_by_pos(pos_new, true);

					if (element_free.has_value() == false)
						return std::unexpected(element_free.error());

					std::optional<pg_data::element<T, pg_data::index_page_pos>>& element_free_ref = element_free.value().get();

					try { result_ref = element_free_ref.emplace(pg_data::INDEX_PAGE_POS_END, pg_data::INDEX_PAGE_POS_END, std::forward<params>(src_new)...).src; }
					catch (...) { element_free_ref.reset(); return std::unexpected(kernel_error::manager__page__add__emplace_unexpected); }
				}
				else
				{
					if (this->set.size() >= MAX)
						return std::unexpected(kernel_error::manager__page__add__size_max);

					try { result_ref = this->set.emplace_back(std::in_place, pg_data::INDEX_PAGE_POS_END, pg_data::INDEX_PAGE_POS_END, std::forward<params>(src_new)...).value().src; }
					catch (...) { return std::unexpected(kernel_error::manager__page__add__emplace_back_unexpected); }

					pos_new = static_cast<pg_data::index_page_pos>(this->set.size() - 1);
				}

				cmap_data::cache_mapping_result add_cache_res = this->cache.add_set(order_new, pos_new);

				if (add_cache_res.has_value() == false)
				{
					if (reuse_free)
					{
						try { this->set.at(pos_new).reset(); }
						catch (...) { throw kernel_error_throw(kernel_error::manager__page__add__element_deconstruct_unexpected, "kernel | manager | add | 位置缓存添加失败，元素析构失败，错误无法回滚"); }
					}
					else
					{
						this->set.pop_back();
					}

					return std::unexpected(add_cache_res.error());
				}

				if (reuse_free == true)
					this->free.pop_back();

				this->last = pos_new;
			}

			return result_ref;
		}
		template<typename...params>
		pg_data::page_result_src<T> insert_front(pg_data::index_page_order index, params&&...src_new)
		{
			pg_data::page_result_pos order_insert = get_pos_by_order(index);
			pg_data::page_result_src<T> result_ref = std::unexpected(kernel_error::manager__page__insert_front__placeholder);

			if (order_insert.has_value() == false)
			{
				result_ref = add(std::forward<params>(src_new)...);

				if (result_ref.has_value() == false)
					return std::unexpected(result_ref.error());

				return result_ref.value();
			}

			pg_data::page_result_ref<T> element_insert = get_element_ref_by_pos(order_insert.value());

			if (element_insert.has_value() == false)
				return std::unexpected(element_insert.error());

			pg_data::element<T, pg_data::index_page_pos>& element_insert_ref = element_insert.value().get().value();

			pg_data::page_result_pos pos_free = get_free();
			pg_data::index_page_pos pos_new = 0;

			if (pos_free.has_value() == false)
			{
				if (this->set.size() >= MAX)
					return std::unexpected(kernel_error::manager__page__insert_front__size_max);

				pos_new = static_cast<pg_data::index_page_pos>(this->set.size());

				pg_data::index_page_pos pos_last = element_insert_ref.last;
				pg_data::index_page_pos pos_new_last = pos_last;

				cmap_data::cache_mapping_result cache_insert_res = this->cache.insert_front(index, pos_new);

				if (cache_insert_res.has_value() == false)
					return std::unexpected(cache_insert_res.error());

				element_insert_ref.last = pos_new;

				try
				{
					result_ref = this->set.emplace_back(std::in_place, pos_new_last, order_insert.value(), std::forward<params>(src_new)...).value().src;
				}
				catch (...)
				{
					element_insert = get_element_ref_by_pos(order_insert.value());

					if (element_insert.has_value() == false)
						throw kernel_error_throw(element_insert.error(), "kernel | manager | insert_front | 元素尾插构造失败，重获取插入前原元素位置失败，无法重写元素链指向，错误无法回滚");

					pg_data::element<T, pg_data::index_page_pos>& element_insert_roll_back_ref = element_insert.value().get().value();

					element_insert_roll_back_ref.last = pos_new_last;

					cmap_data::cache_mapping_result cache_roll_back_res = this->cache.del(index);

					if (cache_roll_back_res.has_value() == false)
						throw kernel_error_throw(cache_roll_back_res.error(), "kernel | manager | insert_front | 元素尾插构造失败，删除已记录的缓存位置失败，错误无法回滚");

					return std::unexpected(kernel_error::manager__page__insert_front__emplace_back_unexpected);
				}

				pg_data::page_result_ref<T> element_insert_last = get_element_ref_by_pos(pos_last);

				if (element_insert_last.has_value() == true)
					element_insert_last.value().get().value().next = pos_new;
			}
			else
			{
				pg_data::page_result_ref<T> element_free = get_element_ref_by_pos(pos_free.value(), true);

				if (element_free.has_value() == false)
				{
					add_free_roll_back(pos_free.value());
					return std::unexpected(element_free.error());
				}

				pos_new = pos_free.value();

				pg_data::index_page_pos pos_last = element_insert_ref.last;
				pg_data::index_page_pos pos_new_last = pos_last;

				cmap_data::cache_mapping_result cache_insert_res = this->cache.insert_front(index, pos_new);

				if (cache_insert_res.has_value() == false)
				{
					add_free_roll_back(pos_free.value());
					return std::unexpected(cache_insert_res.error());
				}


				element_insert_ref.last = pos_new;

				try
				{
					element_free.value().get().emplace(pos_new_last, order_insert.value(), std::forward<params>(src_new)...);
				}
				catch (...)
				{
					element_free.value().get().reset();

					add_free_roll_back(pos_free.value());

					element_insert = get_element_ref_by_pos(order_insert.value());

					if (element_insert.has_value() == false)
						throw kernel_error_throw(element_insert.error(), "kernel | manager | insert_front | 元素原地构造失败，重获取插入前原元素位置失败，无法重写元素链指向，错误无法回滚");

					pg_data::element<T, pg_data::index_page_pos>& element_insert_roll_back_ref = element_insert.value().get().value();

					element_insert_roll_back_ref.last = pos_new_last;

					cmap_data::cache_mapping_result cache_roll_back_res = this->cache.del(index);

					if (cache_roll_back_res.has_value() == false)
						throw kernel_error_throw(cache_roll_back_res.error(), "kernel | manager | insert_front | 元素原地构造失败，删除已记录的缓存位置失败，错误无法回滚");

					return std::unexpected(kernel_error::manager__page__insert_front__emplace_unexpected);
				}

				pg_data::page_result_ref<T> element_insert_last = get_element_ref_by_pos(pos_last);

				if (element_insert_last.has_value() == true)
					element_insert_last.value().get().value().next = pos_new;

				result_ref = element_free.value().get().value().src;
			}

			if (pos_new > this->last)
				this->last = pos_new;

			return result_ref.value();
		}
		template<typename...params>
		pg_data::page_result_src<T> insert_back(pg_data::index_page_order index, params&&...src_new)
		{
			pg_data::page_result_pos order_insert = get_pos_by_order(index);
			pg_data::page_result_src<T> result_ref = std::unexpected(kernel_error::manager__page__insert_back__placeholder);

			if (order_insert.has_value() == false)
			{
				result_ref = add(std::forward<params>(src_new)...);

				if (result_ref.has_value() == false)
					return std::unexpected(result_ref.error());

				return result_ref.value();
			}

			pg_data::page_result_ref<T> element_insert = get_element_ref_by_pos(order_insert.value());

			if (element_insert.has_value() == false)
				return std::unexpected(element_insert.error());

			pg_data::element<T, pg_data::index_page_pos>& element_insert_ref = element_insert.value().get().value();

			pg_data::page_result_pos pos_free = get_free();
			pg_data::index_page_pos pos_new = 0;

			if (pos_free.has_value() == false)
			{
				if (this->set.size() >= MAX)
					return std::unexpected(kernel_error::manager__page__insert_back__size_max);

				pos_new = static_cast<pg_data::index_page_pos>(this->set.size());

				cmap_data::cache_mapping_result cache_insert_res = this->cache.insert_back(index, pos_new);

				if (cache_insert_res.has_value() == false)
					return std::unexpected(cache_insert_res.error());

				pg_data::index_page_pos pos_new_next = element_insert_ref.next;

				element_insert_ref.next = pos_new;

				try
				{
					result_ref = this->set.emplace_back(std::in_place, order_insert.value(), pos_new_next, std::forward<params>(src_new)...).value().src;
				}
				catch (...)
				{
					pg_data::page_result_ref<T> element_insert = get_element_ref_by_pos(order_insert.value());

					if (element_insert.has_value() == false)
						throw kernel_error_throw(element_insert.error(), "kernel | manager | insert_back | 元素尾插构造失败，重获取插入前原元素位置失败，无法重写元素链指向，错误无法回滚");

					pg_data::element<T, pg_data::index_page_pos>& element_insert_roll_back_ref = element_insert.value().get().value();

					element_insert_roll_back_ref.next = pos_new_next;

					cmap_data::cache_mapping_result cache_roll_back_res = this->cache.del(index + 1);

					if (cache_roll_back_res.has_value() == false)
						throw kernel_error_throw(cache_roll_back_res.error(), "kernel | manager | insert_back | 元素尾插构造失败，删除已记录的缓存位置失败，错误无法回滚");

					return std::unexpected(kernel_error::manager__page__insert_back__emplace_back_unexpected);
				}

				pg_data::page_result_ref<T> element_insert_next = get_element_ref_by_pos(pos_new_next);

				if (element_insert_next.has_value() == true)
					element_insert_next.value().get().value().last = pos_new;
			}
			else
			{
				pg_data::page_result_ref<T> element_free = get_element_ref_by_pos(pos_free.value(), true);

				if (element_free.has_value() == false)
				{
					add_free_roll_back(pos_free.value());
					return std::unexpected(element_free.error());
				}

				pos_new = pos_free.value();

				cmap_data::cache_mapping_result cache_insert_res = this->cache.insert_back(index, pos_new);

				if (cache_insert_res.has_value() == false)
				{
					add_free_roll_back(pos_free.value());
					return std::unexpected(cache_insert_res.error());
				}

				pg_data::index_page_pos pos_new_next = element_insert_ref.next;

				element_insert_ref.next = pos_new;

				try
				{
					element_free.value().get().emplace(order_insert.value(), pos_new_next, std::forward<params>(src_new)...);
				}
				catch (...)
				{
					add_free_roll_back(pos_free.value());

					element_insert = get_element_ref_by_pos(order_insert.value());

					if (element_insert.has_value() == false)
						throw kernel_error_throw(element_insert.error(), "kernel | manager | insert_back | 元素原地构造失败，重获取插入前原元素位置失败，错误无法回滚");

					pg_data::element<T, pg_data::index_page_pos>& element_insert_roll_back_ref = element_insert.value().get().value();

					element_insert_roll_back_ref.next = pos_new_next;

					cmap_data::cache_mapping_result cache_roll_back_res = this->cache.del(index + 1);

					if (cache_roll_back_res.has_value() == false)
						throw kernel_error_throw(cache_roll_back_res.error(), "kernel | manager | insert_back | 元素原地构造失败，删除已记录的缓存位置失败，错误无法回滚");

					return std::unexpected(kernel_error::manager__page__insert_back__emplace_unexpected);
				}

				pg_data::page_result_ref<T> element_insert_next = get_element_ref_by_pos(pos_new_next);

				if (element_insert_next.has_value() == true)
					element_insert_next.value().get().value().last = pos_new;

				result_ref = element_free.value().get().value().src;
			}

			if (pos_new > this->last)
				this->last = pos_new;

			return result_ref.value();
		}
		pg_data::page_result del(pg_data::index_page_order index)
		{
			pg_data::page_result_pos pos_del = get_pos_by_order(index);

			if (pos_del.has_value() == false)
				return std::unexpected(pos_del.error());

			pg_data::page_result_ref<T> element_del = get_element_ref_by_pos(pos_del.value());

			if (element_del.has_value() == false)
				return std::unexpected(element_del.error());

			pg_data::element<T, pg_data::index_page_pos>& element_del_ref = element_del.value().get().value();

			const bool is_element_last_exist = element_del_ref.last != pg_data::INDEX_PAGE_POS_END;
			const bool is_element_next_exist = element_del_ref.next != pg_data::INDEX_PAGE_POS_END;
			const pg_data::index_page_pos pos_last = element_del_ref.last;
			const pg_data::index_page_pos pos_next = element_del_ref.next;

			pg_data::element<T, pg_data::index_page_pos>* element_last_ptr = nullptr;
			pg_data::element<T, pg_data::index_page_pos>* element_next_ptr = nullptr;
			pg_data::index_page_pos element_last_pos_next = pg_data::INDEX_PAGE_POS_END;
			pg_data::index_page_pos element_next_pos_last = pg_data::INDEX_PAGE_POS_END;

			if (is_element_last_exist == true)
			{
				pg_data::page_result_ref<T> element_last = get_element_ref_by_pos(pos_last);

				if (element_last.has_value() == false)
					return std::unexpected(element_last.error());

				element_last_ptr = &element_last.value().get().value();
				element_last_pos_next = element_last_ptr->next;
				element_last_ptr->next = pos_next;
			}

			if (is_element_next_exist == true)
			{
				pg_data::page_result_ref<T> element_next = get_element_ref_by_pos(pos_next);

				if (element_next.has_value() == false)
				{
					if (element_last_ptr != nullptr) element_last_ptr->next = element_last_pos_next;

					return std::unexpected(element_next.error());
				}

				element_next_ptr = &element_next.value().get().value();
				element_next_pos_last = element_next_ptr->last;
				element_next_ptr->last = pos_last;
			}

			cmap_data::cache_mapping_result cache_del_res = this->cache.del(index);

			if (cache_del_res.has_value() == false)
			{
				if (element_last_ptr != nullptr) element_last_ptr->next = element_last_pos_next;
				if (element_next_ptr != nullptr) element_next_ptr->last = element_next_pos_last;

				return std::unexpected(cache_del_res.error());
			}

			pg_data::page_result add_free_res = add_free_by_pos(pos_del.value());

			if (add_free_res.has_value() == false)
			{
				if (element_last_ptr != nullptr) element_last_ptr->next = element_last_pos_next;
				if (element_next_ptr != nullptr) element_next_ptr->last = element_next_pos_last;

				cmap_data::cache_mapping_result cache_roll_back_res = this->cache.insert_front(index, pos_del.value());

				if (cache_roll_back_res.has_value() == false)
					throw kernel_error_throw(cache_roll_back_res.error(), "kernel | manager | del | 被删除位置添加空闲失败，重新插入缓存位置失败，错误无法回滚");

				return std::unexpected(add_free_res.error());
			}

			if (this->last == pos_del.value())
			{
				pg_data::index_page_pos pos_current = this->last;

				while (true)
				{
					if (pos_current == 0)
						break;

					pos_current--;

					pg_data::page_result_ref<T> element_last = get_element_ref_by_pos(pos_current);

					if (element_last.has_value() == true && element_last.value().get().has_value() == true)
						break;
				}

				this->last = pos_current;
			}

			return {};
		}
		constexpr pg_data::page_result_src<T> get(pg_data::index_page_order index) noexcept
		{
			pg_data::page_result_pos pos_get = get_pos_by_order(index);

			if (pos_get.has_value() == false)
				return std::unexpected(pos_get.error());

			pg_data::page_result_ref<T> element_target = get_element_ref_by_pos(pos_get.value());

			if (element_target.has_value() == false)
				return std::unexpected(element_target.error());

			return element_target.value().get().value().src;
		}
		constexpr pg_data::page_result_const_src<T> get_const(pg_data::index_page_order index) const noexcept
		{
			pg_data::page_result_pos pos_get = get_pos_by_order(index);

			if (pos_get.has_value() == false)
				return std::unexpected(pos_get.error());

			pg_data::page_result_const_ref<T> element_target = get_element_const_ref_by_pos(pos_get.value());

			if (element_target.has_value() == false)
				return std::unexpected(element_target.error());

			return element_target.value().get().value().src;
		}
		constexpr pg_data::page_result write(pg_data::index_page_order index, const T& src_new) noexcept
		{
			pg_data::page_result_pos pos_get = get_pos_by_order(index);

			if (pos_get.has_value() == false)
				return std::unexpected(pos_get.error());

			pg_data::page_result_ref<T> element_target = get_element_ref_by_pos(pos_get.value());

			if (element_target.has_value() == false)
				return std::unexpected(element_target.error());

			T& src_ref = element_target.value().get().value().src;

			try { src_ref = src_new; }
			catch (...) { return std::unexpected(kernel_error::manager__page__write__copy_unexpected); }

			return {};
		}
		pg_data::page_result cut(page& page_other, pg_data::index_page_order index_begin, pg_data::index_page_order index_end, pg_data::index_page_order index_save_begin)
		{
			if (&page_other == this)
				return std::unexpected(kernel_error::manager__page__cut__index_invalid);

			if (index_end < index_begin)
				return std::unexpected(kernel_error::manager__page__cut__index_invalid);

			if (test_validity_index_page_order(index_begin) == false)
				return std::unexpected(kernel_error::manager__page__cut__index_invalid);

			if (test_validity_index_page_order(index_end) == false)
				return std::unexpected(kernel_error::manager__page__cut__index_invalid);

			if (index_save_begin > page_other.get_size())
				return std::unexpected(kernel_error::manager__page__cut__index_invalid);

			std::size_t loop_len = static_cast<std::size_t>(index_end - index_begin) + 1;

			if (page_other.get_size_left() < loop_len)
				return std::unexpected(kernel_error::manager__page__cut__size_max);

			auto cut_roll_back = [&](std::size_t moved_count_v) -> bool
			{
				while (moved_count_v > 0)
				{
					pg_data::index_page_order order_other_rollback = static_cast<pg_data::index_page_order>(static_cast<std::size_t>(index_save_begin) + moved_count_v - 1);
					pg_data::page_result_src<T> element_rollback = page_other.get(order_other_rollback);

					if (element_rollback.has_value() == false)
						return false;

					pg_data::page_result_src<T> insert_rollback = this->insert_front(index_begin, std::move(element_rollback.value().get()));

					if (insert_rollback.has_value() == false)
						return false;

					pg_data::page_result del_rollback = page_other.del(order_other_rollback);

					if (del_rollback.has_value() == false)
						return false;

					moved_count_v--;
				}

				return true;
			};

			std::size_t order_cut_other_current = index_save_begin;
			std::size_t moved_count = 0;

			while (loop_len > 0)
			{
				pg_data::page_result_ref<T> element_current = get_element_ref_by_order(index_begin);

				if (element_current.has_value() == false || element_current.value().get().has_value() == false)
				{
					if (cut_roll_back(moved_count) == false)
						throw kernel_error_throw(kernel_error::manager__page__cut__element_missing, "kernel | manager | page | cut | 元素缺失后回滚失败，错误不可恢复");

					return std::unexpected(kernel_error::manager__page__cut__element_missing);
				}

				pg_data::page_result_src<T> insert_res = page_other.insert_front(static_cast<pg_data::index_page_order>(order_cut_other_current), std::move(element_current.value().get().value().src));

				if (insert_res.has_value() == false)
				{
					if (cut_roll_back(moved_count) == false)
						throw kernel_error_throw(insert_res.error(), "kernel | manager | page | cut | 插入失败后回滚失败，错误不可恢复");

					if constexpr (std::is_trivially_copyable_v<T> == false)
						throw kernel_error_throw(insert_res.error(), "kernel | manager | page | cut | 移动后插入失败，源值可能已丢失，错误不可恢复");

					return std::unexpected(insert_res.error());
				}

				order_cut_other_current++;

				pg_data::page_result del_res = del(index_begin);

				if (del_res.has_value() == false)
				{
					pg_data::page_result del_other_res = page_other.del(static_cast<pg_data::index_page_order>(order_cut_other_current - 1));

					if (del_other_res.has_value() == false)
						throw kernel_error_throw(del_res.error(), "kernel | manager | page | cut | 删除失败后撤销目标页插入失败，错误不可恢复");

					if (cut_roll_back(moved_count) == false)
						throw kernel_error_throw(del_res.error(), "kernel | manager | page | cut | 删除失败后回滚失败，错误不可恢复");

					return std::unexpected(del_res.error());
				}

				moved_count++;
				loop_len--;
			}

			return {};
		}
		constexpr std::size_t get_size() const noexcept
		{
			return this->set.size() - this->free.size();
		}
		constexpr std::size_t get_size_left() const noexcept
		{
			return MAX - get_size();
		}
		constexpr bool empty() const noexcept
		{
			return get_size() == 0;
		}
		void rebuild()
		{
			return tidy_element(true);
		}
	};
}
