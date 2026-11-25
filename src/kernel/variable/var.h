#pragma once
#include "../kernel_namespace.h"
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
#include "../manager/manager.h"
#include "var_data_def.h"
#include "var_cont.h"

namespace rule_var
{
	template<typename T, typename var_mgr_alloc = std::allocator<T>>
	class variable_manager
	{
	private:
		using var_mgr_alloc_traits = typename std::allocator_traits<var_mgr_alloc>;
		using var_set_allocator = var_mgr_alloc_traits::template rebind_alloc<var_data::var_sgl<T, var_mgr_alloc>>;
	public:
		using allocator_type = var_mgr_alloc;
		using value_type = T;
		using size_type = std::size_t;
		using index_type = var_cont_data::var_index;
		using index_this_type = var_cont_data::var_mgr_index;
		using iterator = typename mgr_ns::container_plus<var_data::var_sgl<value_type, var_mgr_alloc>, var_set_allocator>::iterator;
		using const_iterator = typename mgr_ns::container_plus<var_data::var_sgl<value_type, var_mgr_alloc>, var_set_allocator>::const_iterator;
		using reverse_iterator = typename mgr_ns::container_plus<var_data::var_sgl<value_type, var_mgr_alloc>, var_set_allocator>::reverse_iterator;
		using const_reverse_iterator = typename mgr_ns::container_plus<var_data::var_sgl<value_type, var_mgr_alloc>, var_set_allocator>::const_reverse_iterator;
	public:
		static constexpr index_this_type index_this_invalid_value = var_cont_data::INVALID_VAR_MGR_INDEX;
	private:
		allocator_type allocator_;
		mgr_ns::container_plus<var_data::var_sgl<value_type, var_mgr_alloc>, var_set_allocator> set;
		var_cont_data::var_ref ref_total = 0;
	private:
		constexpr bool test_validity_var_index(var_cont_data::var_index index) const
		{
			if (index == var_cont_data::INVALID_VAR_INDEX)
				return false;

			const_iterator it_res = std::lower_bound
			(
				this->set.begin(),
				this->set.end(),
				index,
				[](const var_data::var_sgl<value_type, var_mgr_alloc>& left, var_cont_data::var_index right)->bool
				{
					return std::less<>{}(left.index, right);
				}
			);

			if (it_res == this->set.end())
				return false;

			return it_res->get_index() == index;
		}
		constexpr var_data::var_mgr_res_var_sgl_it<iterator> test_validity_var_index_it(var_cont_data::var_index index, bool is_save_lower)
		{
			if (index == var_cont_data::INVALID_VAR_INDEX)
				return std::unexpected(kernel_error::variable__variable_manager__test_validity_var_index_it__index_invalid);

			iterator it_end = this->set.end();
			iterator it = std::lower_bound(
				this->set.begin(),
				it_end,
				index,
				[](const var_data::var_sgl<value_type, var_mgr_alloc>& left, var_cont_data::var_index right)->bool
				{
					return std::less<>{}(left.index, right);
				}
			);

			if (it == it_end)
				return std::unexpected(kernel_error::variable__variable_manager__test_validity_var_index_it__index_invalid);

			if (it->get_index() != index && is_save_lower == false)
				return std::unexpected(kernel_error::variable__variable_manager__test_validity_var_index_it__index_invalid);

			return it;
		}
		constexpr var_data::var_mgr_res_var_sgl_it<const_iterator> test_validity_var_index_it_const(var_cont_data::var_index index, bool is_save_lower) const
		{
			if (index == var_cont_data::INVALID_VAR_INDEX)
				return std::unexpected(kernel_error::variable__variable_manager__test_validity_var_index_it_const__index_invalid);

			const_iterator it_end = this->set.end();
			const_iterator it = std::lower_bound(
				this->set.begin(),
				it_end,
				index,
				[](const var_data::var_sgl<value_type, var_mgr_alloc>& left, var_cont_data::var_index right)->bool
				{
					return std::less<>{}(left.index, right);
				}
			);

			if (it == it_end)
				return std::unexpected(kernel_error::variable__variable_manager__test_validity_var_index_it_const__index_invalid);

			if (it->get_index() != index && is_save_lower == false)
				return std::unexpected(kernel_error::variable__variable_manager__test_validity_var_index_it_const__index_invalid);

			return it;
		}
		var_data::var_mgr_res_var_sgl<value_type, var_mgr_alloc> insert_var_sgl(var_cont_data::var_index index, var_cont_data::var_type type, bool is_increase_ref = true)
		{
			if (index == var_cont_data::INVALID_VAR_INDEX)
				return std::unexpected(kernel_error::variable__variable_manager__insert_var_sgl__index_invalid);

			var_data::var_mgr_res_var_sgl_it<iterator> var_sing_target = test_validity_var_index_it(index, true);

			if (var_sing_target.has_value() == false)
			{
				var_data::var_mgr_res_var_sgl<value_type, var_mgr_alloc> emplace_res = this->set.emplace_back(index, type, 1, vcont::var_content<value_type, var_mgr_alloc>(this->allocator_));

				if (emplace_res.has_value() == false)
					return std::unexpected(emplace_res.error());

				this->ref_total++;
				return emplace_res.value();
			}

			if (var_sing_target.value()->get_index() == index)
			{
				var_data::var_sgl<value_type, var_mgr_alloc>& var_sgl_target = *var_sing_target.value();

				if (is_increase_ref == true)
				{
					var_sgl_target.get_ref()++;
					this->ref_total++;
				}

				var_sgl_target.set_type(type);

				return var_sgl_target;
			}

			var_data::var_mgr_res_var_sgl_it<iterator> insert_res = this->set.insert(var_sing_target.value(), index, type, 1, vcont::var_content<value_type, var_mgr_alloc>(this->allocator_));

			if (insert_res.has_value() == false)
				return std::unexpected(insert_res.error());

			this->ref_total++;
			return *insert_res.value();
		}
		var_data::var_mgr_res_var_sgl<value_type, var_mgr_alloc> insert_var_sgl_ex(var_cont_data::var_index index, var_cont_data::var_type type, bool is_increase_ref, iterator& it_cursor)
		{
			if (index == var_cont_data::INVALID_VAR_INDEX)
				return std::unexpected(kernel_error::variable__variable_manager__insert_var_sgl__index_invalid);

			var_data::var_mgr_res_var_sgl_it<iterator> var_sing_target = test_validity_var_index_it(index, true);

			if (var_sing_target.has_value() == false)
			{
				var_data::var_mgr_res_var_sgl<value_type, var_mgr_alloc> emplace_res = this->set.emplace_back(index, type, 1, vcont::var_content<value_type, var_mgr_alloc>(this->allocator_));

				if (emplace_res.has_value() == false)
					return std::unexpected(emplace_res.error());

				this->ref_total++;
				return emplace_res.value();
			}

			if (var_sing_target.value()->get_index() == index)
			{
				var_data::var_sgl<value_type, var_mgr_alloc>& var_sgl_target = *var_sing_target.value();

				if (is_increase_ref == true)
				{
					var_sgl_target.get_ref()++;
					this->ref_total++;
				}

				var_sgl_target.set_type(type);

				return var_sgl_target;
			}

			var_data::var_mgr_res_var_sgl_it<iterator> insert_res = this->set.insert_ex(it_cursor, var_sing_target.value(), index, type, 1, vcont::var_content<value_type, var_mgr_alloc>(this->allocator_));

			if (insert_res.has_value() == false)
				return std::unexpected(insert_res.error());

			this->ref_total++;
			return *insert_res.value();
		}
		void roll_back_insert(var_cont_data::var_index index, var_data::var_sgl<value_type, var_mgr_alloc>& var_sgl_ref)
		{
			if (var_sgl_ref.get_ref() == 1)
			{
				var_data::var_mgr_res erase_res = erase_var_sgl(index);

				if (erase_res.has_value() == false)
					throw kernel_error_throw(erase_res.error(), "kernel | variable | variable_manager | add_set | 新变量回滚删除失败，错误不可恢复");

				return;
			}

			var_sgl_ref.get_ref()--;
			this->ref_total--;
			return;
		}
		void roll_back_insert_ex(var_cont_data::var_index index, var_data::var_sgl<value_type, var_mgr_alloc>& var_sgl_ref, iterator& it_cursor)
		{
			if (var_sgl_ref.get_ref() == 1)
			{
				var_data::var_mgr_res erase_res = erase_var_sgl_ex(index, it_cursor);

				if (erase_res.has_value() == false)
					throw kernel_error_throw(erase_res.error(), "kernel | variable | variable_manager | add_set_ex | 新变量回滚删除失败，错误不可恢复");

				return;
			}

			var_sgl_ref.get_ref()--;
			this->ref_total--;
			return;
		}
		void roll_back_insert_dispatch(var_cont_data::var_index index, var_data::var_sgl<value_type, var_mgr_alloc>& var_sgl_ref, iterator* it_cursor)
		{
			if (it_cursor != nullptr)
				this->roll_back_insert_ex(index, var_sgl_ref, *it_cursor);
			else
				this->roll_back_insert(index, var_sgl_ref);

			return;
		}
		void roll_back_add_set(var_cont_data::var_index index, bool is_existed, var_cont_data::var_type type_old, var_cont_data::var_ref ref_old, std::size_t size_old, kernel_error add_error, iterator* it_cursor = nullptr)
		{
			var_data::var_mgr_res_var_sgl<value_type, var_mgr_alloc> var_sgl_now_res = this->get(index);

			if (is_existed == false)
			{
				if (var_sgl_now_res.has_value() == true)
				{
					if (it_cursor != nullptr)
					{
						var_data::var_mgr_res erase_res = erase_var_sgl_ex(index, *it_cursor);

						if (erase_res.has_value() == false)
							throw kernel_error_throw(erase_res.error(), "kernel | variable | variable_manager | add_set | 新变量回滚删除失败，错误不可恢复");
					}
					else
					{
						var_data::var_mgr_res del_res = this->del(index);

						if (del_res.has_value() == false)
							throw kernel_error_throw(del_res.error(), "kernel | variable | variable_manager | add_set | 新变量回滚删除失败，错误不可恢复");
					}
				}

				return;
			}

			if (var_sgl_now_res.has_value() == false)
				throw kernel_error_throw(add_error, "kernel | variable | variable_manager | add_set | 变量在失败后丢失，回滚不可恢复");

			var_data::var_sgl<value_type, var_mgr_alloc>& var_sgl_now_ref = var_sgl_now_res.value().get();
			var_sgl_now_ref.set_type(type_old);

			if (var_sgl_now_ref.get_ref() != ref_old)
				throw kernel_error_throw(add_error, "kernel | variable | variable_manager | add_set | 变量引用计数未恢复，回滚不可恢复");

			if (var_sgl_now_ref.get_content().size() != size_old)
				throw kernel_error_throw(add_error, "kernel | variable | variable_manager | add_set | 变量内容已扩张，无法廉价回滚");

			return;
		}
		var_data::var_mgr_res erase_var_sgl(var_cont_data::var_index index)
		{
			var_data::var_mgr_res_var_sgl_it<iterator> var_sing_target = test_validity_var_index_it(index, false);

			if (var_sing_target.has_value() == false)
				return std::unexpected(var_sing_target.error());

			if (--(var_sing_target.value()->get_ref()) == 0)
			{
				mgr_data::manager_result erase_res = this->set.erase(var_sing_target.value());

				if (erase_res.has_value() == false)
					throw kernel_error_throw(erase_res.error(), "kernel | variable | variable_manager | erase_var_sgl | 引用已归零，删除变量失败，错误不可恢复");
			}

			this->ref_total--;
			return {};
		}
	public:
		var_data::var_mgr_res erase_var_sgl_ex(var_cont_data::var_index index, iterator& it_cursor)
		{
			var_data::var_mgr_res_var_sgl_it<iterator> var_sing_target = test_validity_var_index_it(index, false);

			if (var_sing_target.has_value() == false)
				return std::unexpected(var_sing_target.error());

			if (--(var_sing_target.value()->get_ref()) == 0)
			{
				mgr_data::manager_result erase_res = this->set.erase_ex(it_cursor, var_sing_target.value());

				if (erase_res.has_value() == false)
					throw kernel_error_throw(erase_res.error(), "kernel | variable | variable_manager | erase_var_sgl_ex | 引用已归零，删除变量失败，错误不可恢复");
			}

			this->ref_total--;
			return {};
		}
		var_data::var_mgr_res del_ex(var_cont_data::var_index index, iterator& it_cursor)
		{
			return erase_var_sgl_ex(index, it_cursor);
		}
		var_data::var_mgr_res del_it_ex(iterator it, iterator& it_cursor)
		{
			if (it == this->set.end())
				return std::unexpected(kernel_error::variable__variable_manager__del_it__iterator_invalid);

			if (--it->get_ref() == 0)
			{
				mgr_data::manager_result erase_res = this->set.erase_ex(it_cursor, it);

				if (erase_res.has_value() == false)
					throw kernel_error_throw(erase_res.error(), "kernel | variable | variable_manager | del_it_ex | 引用已归零，删除变量失败，错误不可恢复");
			}

			this->ref_total--;
			return {};
		}
		var_data::var_mgr_res del_it_ex(const_iterator it, iterator& it_cursor)
		{
			if (it == this->cend())
				return std::unexpected(kernel_error::variable__variable_manager__del_it__iterator_invalid);

			return erase_var_sgl_ex(it->get_index(), it_cursor);
		}
		var_data::var_mgr_res del_ex(var_cont_data::var_index index, var_cont_data::var_pos pos_del_begin, var_cont_data::var_read_width width_del, bool is_del_full_span, iterator& it_cursor)
		{
			var_data::var_mgr_res_var_sgl_it<iterator> it_target = test_validity_var_index_it(index, false);

			if (it_target.has_value() == false)
				return std::unexpected(it_target.error());

			var_data::var_sgl<value_type, var_mgr_alloc>& var_sgl_target = *it_target.value();

			const std::size_t size_before = var_sgl_target.get_content().size();
			var_cont_data::var_cont_res del_res = var_sgl_target.get_content().del(pos_del_begin, width_del);

			if (del_res.has_value() == false)
				return std::unexpected(del_res.error());

			if (is_del_full_span == true &&
				static_cast<std::size_t>(pos_del_begin) + static_cast<std::size_t>(width_del) >= size_before)
				return del_it_ex(it_target.value(), it_cursor);

			return {};
		}
	private:
		var_data::var_mgr_res_var_sgl<value_type, var_mgr_alloc> get_var_sgl(var_cont_data::var_index index)
		{
			var_data::var_mgr_res_var_sgl_it<iterator> it_target = test_validity_var_index_it(index, false);

			if (it_target.has_value() == false)
				return std::unexpected(it_target.error());

			return *it_target.value();
		}
		var_data::var_mgr_res_var_sgl_const<value_type, var_mgr_alloc> get_var_sgl_const(var_cont_data::var_index index) const
		{
			var_data::var_mgr_res_var_sgl_it<const_iterator> it_target = test_validity_var_index_it_const(index, false);

			if (it_target.has_value() == false)
				return std::unexpected(it_target.error());

			return *it_target.value();
		}
	public:
		variable_manager() :
			allocator_(), set(), ref_total(0)
		{

		}
		explicit variable_manager(const var_mgr_alloc& alloc_val) :
			allocator_(alloc_val), set(var_set_allocator(alloc_val)), ref_total(0)
		{

		}
		variable_manager(const variable_manager& other_val) :
			allocator_(var_mgr_alloc_traits::select_on_container_copy_construction(other_val.allocator_)),
			set(var_set_allocator(allocator_)),
			ref_total(0)
		{
			try
			{
				this->set = other_val.set;
				this->ref_total = other_val.ref_total;
			}
			catch (const kernel_error_throw&)
			{
				throw;
			}
			catch (...)
			{
				throw kernel_error_throw(kernel_error::variable__variable_manager__copy_construct__copy_unexpected, "kernel | variable | variable_manager | copy_construct | 拷贝构造失败，错误不可恢复");
			}
		}
		variable_manager& operator=(const variable_manager& other_val)
		{
			if (this == &other_val)
				return *this;

			if constexpr (var_mgr_alloc_traits::propagate_on_container_copy_assignment::value == true)
				this->allocator_ = other_val.allocator_;

			try
			{
				this->set = other_val.set;
				this->ref_total = other_val.ref_total;
			}
			catch (const kernel_error_throw&)
			{
				this->clear();
				throw;
			}
			catch (...)
			{
				this->clear();
				throw kernel_error_throw(kernel_error::variable__variable_manager__copy_assign__copy_unexpected, "kernel | variable | variable_manager | operator= | 拷贝赋值失败，容器已清空，错误不可恢复");
			}

			return *this;
		}
		variable_manager(variable_manager&& other_val) noexcept :
			allocator_(std::move(other_val.allocator_)), set(std::move(other_val.set)), ref_total(std::move(other_val.ref_total))
		{

		}
		variable_manager& operator=(variable_manager&& other_val)
		{
			if (this == &other_val)
				return *this;

			if constexpr (var_mgr_alloc_traits::propagate_on_container_move_assignment::value == true)
				this->allocator_ = std::move(other_val.allocator_);

			this->set = std::move(other_val.set);
			this->ref_total = std::move(other_val.ref_total);

			return *this;
		}
	private:
		var_data::var_mgr_res add_set_write_span(var_cont_data::var_index index, var_data::var_sgl<value_type, var_mgr_alloc>& var_sgl_ref, var_cont_data::var_pos pos_write_begin, const std::span<const value_type>& span_write, bool is_increase_ref, bool is_dense, iterator* it_cursor = nullptr)
		{
			vcont::var_content<value_type, var_mgr_alloc>& var_content_target = var_sgl_ref.get_content();

			if (static_cast<std::size_t>(pos_write_begin) > std::numeric_limits<std::size_t>::max() - span_write.size())
			{
				if (is_increase_ref == true)
					{
						if (it_cursor != nullptr)
							this->roll_back_insert_ex(index, var_sgl_ref, *it_cursor);
						else
							this->roll_back_insert_dispatch(index, var_sgl_ref, it_cursor);
					}

				return std::unexpected(kernel_error::variable__variable_manager__add_set__range_out_of_range);
			}

			if (is_dense == true)
			{
				var_cont_data::var_cont_res dense_res = var_content_target.ensure_dense(static_cast<var_cont_data::var_read_width>(static_cast<std::size_t>(pos_write_begin) + span_write.size()));

				if (dense_res.has_value() == false)
				{
					if (is_increase_ref == true)
						this->roll_back_insert_dispatch(index, var_sgl_ref, it_cursor);

					return std::unexpected(dense_res.error());
				}
			}

			var_cont_data::var_cont_res_span<value_type> span_write_res = (is_dense == true) ?
				(span_write.empty() == true ? var_cont_data::var_cont_res_span<value_type>(std::span<value_type>()) : var_content_target.read(pos_write_begin, static_cast<var_cont_data::var_read_width>(span_write.size()))) :
				var_content_target.ensure(pos_write_begin, static_cast<var_cont_data::var_read_width>(span_write.size()));

			if (span_write_res.has_value() == false)
			{
				if (is_increase_ref == true)
					{
						if (it_cursor != nullptr)
							this->roll_back_insert_ex(index, var_sgl_ref, *it_cursor);
						else
							this->roll_back_insert_dispatch(index, var_sgl_ref, it_cursor);
					}

				return std::unexpected(span_write_res.error());
			}

			std::span<value_type>& span_write_target = span_write_res.value();

			try
			{
				if (std::less<>{}(span_write_target.data(), span_write.data()))
					std::copy(span_write.begin(), span_write.end(), span_write_target.begin());
				else if (std::less<>{}(span_write.data(), span_write_target.data()))
					std::copy_backward(span_write.begin(), span_write.end(), span_write_target.end());
			}
			catch (...)
			{
				if (is_increase_ref == true)
					{
						if (it_cursor != nullptr)
							this->roll_back_insert_ex(index, var_sgl_ref, *it_cursor);
						else
							this->roll_back_insert_dispatch(index, var_sgl_ref, it_cursor);
					}

				return std::unexpected(kernel_error::variable__variable_manager__add_set__copy_unexpected);
			}

			return {};
		}
		var_data::var_mgr_res add_set_write_width(var_cont_data::var_index index, var_data::var_sgl<value_type, var_mgr_alloc>& var_sgl_ref, var_cont_data::var_pos pos_write_begin, var_cont_data::var_read_width width_write_fill, bool is_increase_ref, bool is_dense, iterator* it_cursor = nullptr)
		{
			vcont::var_content<value_type, var_mgr_alloc>& var_content_target = var_sgl_ref.get_content();

			if (static_cast<std::size_t>(pos_write_begin) > std::numeric_limits<std::size_t>::max() - static_cast<std::size_t>(width_write_fill))
			{
				if (is_increase_ref == true)
					{
						if (it_cursor != nullptr)
							this->roll_back_insert_ex(index, var_sgl_ref, *it_cursor);
						else
							this->roll_back_insert_dispatch(index, var_sgl_ref, it_cursor);
					}

				return std::unexpected(kernel_error::variable__variable_manager__add_set__range_out_of_range);
			}

			if (is_dense == true)
			{
				var_cont_data::var_cont_res dense_res = var_content_target.ensure_dense(static_cast<var_cont_data::var_read_width>(static_cast<std::size_t>(pos_write_begin) + static_cast<std::size_t>(width_write_fill)));

				if (dense_res.has_value() == false)
				{
					if (is_increase_ref == true)
						this->roll_back_insert_dispatch(index, var_sgl_ref, it_cursor);

					return std::unexpected(dense_res.error());
				}

				return {};
			}

			var_cont_data::var_cont_res_span<value_type> span_res = var_content_target.ensure(pos_write_begin, width_write_fill);

			if (span_res.has_value() == false)
			{
				if (is_increase_ref == true)
					{
						if (it_cursor != nullptr)
							this->roll_back_insert_ex(index, var_sgl_ref, *it_cursor);
						else
							this->roll_back_insert_dispatch(index, var_sgl_ref, it_cursor);
					}

				return std::unexpected(span_res.error());
			}

			return {};
		}
	public:
		var_data::var_mgr_res add_set(var_cont_data::var_index index, var_cont_data::var_type type, var_cont_data::var_pos pos_write_begin, const std::span<const value_type>& span_write, bool is_increase_ref = true, bool is_dense = false)
		{
			const bool is_existed = this->get(index).has_value();
			var_cont_data::var_type type_old = 0;
			var_cont_data::var_ref ref_old = 0;
			std::size_t size_old = 0;

			if (is_existed == true)
			{
				auto var_sgl_old_res = this->get(index);
				type_old = var_sgl_old_res.value().get().get_type();
				ref_old = var_sgl_old_res.value().get().get_ref();
				size_old = var_sgl_old_res.value().get().get_content().size();
			}

			var_data::var_mgr_res_var_sgl<value_type, var_mgr_alloc> insert_res = insert_var_sgl(index, type, is_increase_ref);

			if (insert_res.has_value() == false)
				return std::unexpected(insert_res.error());

			var_data::var_mgr_res write_res = add_set_write_span(index, insert_res.value().get(), pos_write_begin, span_write, is_increase_ref, is_dense);

			if (write_res.has_value() == false)
				roll_back_add_set(index, is_existed, type_old, ref_old, size_old, write_res.error());

			return write_res;
		}
		var_data::var_mgr_res add_set_ex(var_cont_data::var_index index, var_cont_data::var_type type, var_cont_data::var_pos pos_write_begin, const std::span<const value_type>& span_write, bool is_increase_ref, iterator& it_cursor, bool is_dense = false)
		{
			const bool is_existed = this->get(index).has_value();
			var_cont_data::var_type type_old = 0;
			var_cont_data::var_ref ref_old = 0;
			std::size_t size_old = 0;

			if (is_existed == true)
			{
				auto var_sgl_old_res = this->get(index);
				type_old = var_sgl_old_res.value().get().get_type();
				ref_old = var_sgl_old_res.value().get().get_ref();
				size_old = var_sgl_old_res.value().get().get_content().size();
			}

			var_data::var_mgr_res_var_sgl<value_type, var_mgr_alloc> insert_res = insert_var_sgl_ex(index, type, is_increase_ref, it_cursor);

			if (insert_res.has_value() == false)
				return std::unexpected(insert_res.error());

			var_data::var_mgr_res write_res = add_set_write_span(index, insert_res.value().get(), pos_write_begin, span_write, is_increase_ref, is_dense, &it_cursor);

			if (write_res.has_value() == false)
				roll_back_add_set(index, is_existed, type_old, ref_old, size_old, write_res.error(), &it_cursor);

			return write_res;
		}
		var_data::var_mgr_res add_set(var_cont_data::var_index index, var_cont_data::var_type type, var_cont_data::var_pos pos_write_begin, var_cont_data::var_read_width width_write_fill, bool is_increase_ref = true, bool is_dense = false)
		{
			bool is_inserted_new_ignore = false;

			return this->add_set(index, type, pos_write_begin, width_write_fill, is_increase_ref, is_dense, is_inserted_new_ignore);
		}
		var_data::var_mgr_res add_set(var_cont_data::var_index index, var_cont_data::var_type type, var_cont_data::var_pos pos_write_begin, var_cont_data::var_read_width width_write_fill, bool is_increase_ref, bool is_dense, bool& is_inserted_new)
		{
			const bool is_existed = this->get(index).has_value();
			var_cont_data::var_type type_old = 0;
			var_cont_data::var_ref ref_old = 0;
			std::size_t size_old = 0;

			if (is_existed == true)
			{
				auto var_sgl_old_res = this->get(index);
				type_old = var_sgl_old_res.value().get().get_type();
				ref_old = var_sgl_old_res.value().get().get_ref();
				size_old = var_sgl_old_res.value().get().get_content().size();
			}

			is_inserted_new = (is_existed == false);

			var_data::var_mgr_res_var_sgl<value_type, var_mgr_alloc> insert_res = insert_var_sgl(index, type, is_increase_ref);

			if (insert_res.has_value() == false)
				return std::unexpected(insert_res.error());

			var_data::var_mgr_res write_res = add_set_write_width(index, insert_res.value().get(), pos_write_begin, width_write_fill, is_increase_ref, is_dense);

			if (write_res.has_value() == false)
				roll_back_add_set(index, is_existed, type_old, ref_old, size_old, write_res.error());

			return write_res;
		}
		var_data::var_mgr_res add_set_ex(var_cont_data::var_index index, var_cont_data::var_type type, var_cont_data::var_pos pos_write_begin, var_cont_data::var_read_width width_write_fill, bool is_increase_ref, iterator& it_cursor, bool is_dense = false)
		{
			bool is_inserted_new_ignore = false;

			return this->add_set_ex(index, type, pos_write_begin, width_write_fill, is_increase_ref, it_cursor, is_dense, is_inserted_new_ignore);
		}
		var_data::var_mgr_res add_set_ex(var_cont_data::var_index index, var_cont_data::var_type type, var_cont_data::var_pos pos_write_begin, var_cont_data::var_read_width width_write_fill, bool is_increase_ref, iterator& it_cursor, bool is_dense, bool& is_inserted_new)
		{
			const bool is_existed = this->get(index).has_value();
			var_cont_data::var_type type_old = 0;
			var_cont_data::var_ref ref_old = 0;
			std::size_t size_old = 0;

			if (is_existed == true)
			{
				auto var_sgl_old_res = this->get(index);
				type_old = var_sgl_old_res.value().get().get_type();
				ref_old = var_sgl_old_res.value().get().get_ref();
				size_old = var_sgl_old_res.value().get().get_content().size();
			}

			is_inserted_new = (is_existed == false);

			var_data::var_mgr_res_var_sgl<value_type, var_mgr_alloc> insert_res = insert_var_sgl_ex(index, type, is_increase_ref, it_cursor);

			if (insert_res.has_value() == false)
				return std::unexpected(insert_res.error());

			var_data::var_mgr_res write_res = add_set_write_width(index, insert_res.value().get(), pos_write_begin, width_write_fill, is_increase_ref, is_dense, &it_cursor);

			if (write_res.has_value() == false)
				roll_back_add_set(index, is_existed, type_old, ref_old, size_old, write_res.error(), &it_cursor);

			return write_res;
		}
	public:
		var_data::var_mgr_res_span<value_type> read(var_cont_data::var_index index, var_cont_data::var_pos get_begin, var_cont_data::var_read_width read_width)
		{
			var_data::var_mgr_res_var_sgl<value_type, var_mgr_alloc> var_sgl_res = get_var_sgl(index);

			if (var_sgl_res.has_value() == false)
				return std::unexpected(var_sgl_res.error());

			var_data::var_sgl<value_type, var_mgr_alloc>& var_sgl_target = var_sgl_res.value().get();
			var_cont_data::var_cont_res_span<value_type> span_res = var_sgl_target.get_content().read(get_begin, read_width);

			if (span_res.has_value() == false)
				return std::unexpected(span_res.error());

			return span_res.value();
		}
		var_data::var_mgr_res_var_sgl<value_type, var_mgr_alloc> get(var_cont_data::var_index index)
		{
			return get_var_sgl(index);
		}
		var_data::var_mgr_res_var_sgl_const<value_type, var_mgr_alloc> get(var_cont_data::var_index index) const
		{
			return get_var_sgl_const(index);
		}
		var_data::var_mgr_res_size<size_type> get_size(var_cont_data::var_index index) const
		{
			var_data::var_mgr_res_var_sgl_it<const_iterator> var_sgl_target = test_validity_var_index_it_const(index, false);

			if (var_sgl_target.has_value() == false)
				return std::unexpected(var_sgl_target.error());

			return static_cast<size_type>(var_sgl_target.value()->get_content().size());
		}
		var_data::var_mgr_res_size<size_type> get_ref(var_cont_data::var_index index) const
		{
			var_data::var_mgr_res_var_sgl_it<const_iterator> var_sgl_target = test_validity_var_index_it_const(index, false);

			if (var_sgl_target.has_value() == false)
				return std::unexpected(var_sgl_target.error());

			return static_cast<size_type>(var_sgl_target.value()->get_ref());
		}
		template<typename self_this>
		decltype(auto) get_ref_total(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.ref_total);
		}
		var_data::var_mgr_res_var_sgl_it<iterator> get_it(var_cont_data::var_index index)
		{
			var_data::var_mgr_res_var_sgl_it<iterator> it_target = test_validity_var_index_it(index, false);

			if (it_target.has_value() == false)
				return std::unexpected(it_target.error());

			return it_target;
		}
		var_data::var_mgr_res_var_sgl_it<const_iterator> get_it(var_cont_data::var_index index) const
		{
			var_data::var_mgr_res_var_sgl_it<const_iterator> it_target = test_validity_var_index_it_const(index, false);

			if (it_target.has_value() == false)
				return std::unexpected(it_target.error());

			return it_target;
		}
		var_data::var_mgr_res del(var_cont_data::var_index index)
		{
			return erase_var_sgl(index);
		}
		var_data::var_mgr_res del_it(iterator it)
		{
			if (it == this->set.end())
				return std::unexpected(kernel_error::variable__variable_manager__del_it__iterator_invalid);

			if (--it->get_ref() == 0)
			{
				mgr_data::manager_result erase_res = this->set.erase(it);

				if (erase_res.has_value() == false)
					throw kernel_error_throw(erase_res.error(), "kernel | variable | variable_manager | del_it | 引用已归零，删除变量失败，错误不可恢复");
			}

			this->ref_total--;
			return {};
		}
		var_data::var_mgr_res del_it(const_iterator it)
		{
			if (it == this->cend())
				return std::unexpected(kernel_error::variable__variable_manager__del_it__iterator_invalid);

			return erase_var_sgl(it->get_index());
		}
		var_data::var_mgr_res del(var_cont_data::var_index index, var_cont_data::var_pos pos_del_begin, var_cont_data::var_read_width width_del, bool is_del_full_span = false)
		{
			var_data::var_mgr_res_var_sgl_it<iterator> it_target = test_validity_var_index_it(index, false);

			if (it_target.has_value() == false)
				return std::unexpected(it_target.error());

			var_data::var_sgl<value_type, var_mgr_alloc>& var_sgl_target = *it_target.value();

			const std::size_t size_before = var_sgl_target.get_content().size();
			var_cont_data::var_cont_res del_res = var_sgl_target.get_content().del(pos_del_begin, width_del);

			if (del_res.has_value() == false)
				return std::unexpected(del_res.error());

			if (is_del_full_span == true &&
				static_cast<std::size_t>(pos_del_begin) + static_cast<std::size_t>(width_del) >= size_before)
				return del_it(it_target.value());

			return {};
		}
	public:
		void rebind(iterator& it) noexcept
		{
			it.set_manager_ref(&this->set);
			return;
		}
		void rebind(const_iterator& it) noexcept
		{
			it.set_manager_ref(&this->set);
			return;
		}
		bool restore_var(var_cont_data::var_index index, var_cont_data::var_type type, var_cont_data::var_ref ref)
		{
			if (ref == 0)
				return false;

			auto insert_res = this->set.emplace_back(index, type, ref, vcont::var_content<value_type, var_mgr_alloc>(this->allocator_));

			if (insert_res.has_value() == false)
				return false;

			this->ref_total += ref;
			return true;
		}
	public:
		void clear() noexcept
		{
			this->set.clear();
			this->ref_total = 0;
		}
		var_data::var_mgr_res shrink_to_fit()
		{
			return this->set.shrink_to_fit();
		}
		void swap(variable_manager& other_val) noexcept
		{
			if (this == &other_val)
				return;

			std::swap(this->allocator_, other_val.allocator_);
			this->set.swap(other_val.set);
			std::swap(this->ref_total, other_val.ref_total);
			return;
		}
		bool empty() const noexcept
		{
			return this->set.empty();
		}
		size_type size() const noexcept
		{
			return this->set.size();
		}
		iterator begin() noexcept
		{
			return this->set.begin();
		}
		iterator end() noexcept
		{
			return this->set.end();
		}
		reverse_iterator rbegin() noexcept
		{
			return this->set.rbegin();
		}
		reverse_iterator rend() noexcept
		{
			return this->set.rend();
		}
		const_iterator begin() const noexcept
		{
			return this->set.begin();
		}
		const_iterator end() const noexcept
		{
			return this->set.end();
		}
		const_iterator cbegin() const noexcept
		{
			return this->set.cbegin();
		}
		const_iterator cend() const noexcept
		{
			return this->set.cend();
		}
		const_reverse_iterator crbegin() const noexcept
		{
			return this->set.rbegin();
		}
		const_reverse_iterator crend() const noexcept
		{
			return this->set.rend();
		}
	public:
		[[nodiscard]] bool is_valid(const iterator& it) const noexcept
		{
			return this->set.is_valid(it);
		}
		[[nodiscard]] bool is_valid(const const_iterator& it) const noexcept
		{
			return this->set.is_valid(it);
		}
		[[nodiscard]] bool is_valid(const reverse_iterator& it) const noexcept
		{
			return this->set.is_valid(it);
		}
		[[nodiscard]] bool is_valid(const const_reverse_iterator& it) const noexcept
		{
			return this->set.is_valid(it);
		}
	};

	using var_type_base = std::byte;
	using var_set = variable_manager<var_type_base, std::allocator<var_type_base>>;


}