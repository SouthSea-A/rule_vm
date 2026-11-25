#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <utility>
#include <type_traits>
#include <limits>
#include <functional>
#include <deque>
#include <expected>
#include "../manager/manager.h"
#include "../variable/var.h"
#include "../command/cmd.h"
#include "rc_data_def.h"

namespace rule_rc
{
	template<typename T>
	class resource
	{
	public:
		using allocator_type = typename T::allocator_type;
		using unexpected_error_type = kernel_error;
		using value_type = rc_data::rc_sgl<T>;
		using resource_type = T;
		using index_type = typename resource_type::index_this_type;
		using index_this_type = rc_data::rc_index;
		using size_type = std::size_t;
	private:
		using allocator_rc_traits = typename std::allocator_traits<allocator_type>;
		using allocator_rc_set = allocator_rc_traits::template rebind_alloc<value_type>;
		using allocator_rc_set_index_free = allocator_rc_traits::template rebind_alloc<index_type>;
	public:
		using iterator = typename mgr_ns::container_plus<value_type, allocator_rc_set>::iterator;
		using const_iterator = typename mgr_ns::container_plus<value_type, allocator_rc_set>::const_iterator;
		using reverse_iterator = typename mgr_ns::container_plus<value_type, allocator_rc_set>::reverse_iterator;
		using const_reverse_iterator = typename mgr_ns::container_plus<value_type, allocator_rc_set>::const_reverse_iterator;
	private:
		allocator_type alloc_;
		mgr_ns::container_plus<value_type, allocator_rc_set> set;
	private:
		[[nodiscard]] bool test_validity_rc_index(index_type index) const
		{
			if (index == rc_data::INVALID_RC_INDEX)
				return false;

			const_iterator it_res = std::lower_bound
			(
				this->set.begin(),
				this->set.end(),
				index,
				[](const value_type& left, index_type right)->bool
				{
					return std::less<>{}(left.get_index(), right);
				}
			);

			if (it_res == this->set.end())
				return false;

			return it_res->get_index() == index;
		}
		rc_data::rc_res_inst_it<value_type, allocator_rc_set> get_instance_by_rc_index(index_type index, bool is_save_lower)
		{
			if (index == rc_data::INVALID_RC_INDEX)
				return std::unexpected(kernel_error::resource__resource__get_instance_by_rc_index__index_invalid);

			iterator it_end = this->set.end();
			iterator it_res = std::lower_bound
			(
				this->set.begin(),
				it_end,
				index,
				[](const value_type& left, index_type right)->bool
				{
					return std::less<>{}(left.get_index(), right);
				}
			);

			if (it_res == it_end)
				return std::unexpected(kernel_error::resource__resource__get_instance_by_rc_index__index_invalid);

			if (is_save_lower == true)
				return it_res;

			if (it_res->index != index)
				return std::unexpected(kernel_error::resource__resource__get_instance_by_rc_index__index_invalid);

			return it_res;
		}
		rc_data::rc_res_inst_it_const<value_type, allocator_rc_set> get_instance_const_by_rc_index(index_type index, bool is_save_lower) const
		{
			if (index == rc_data::INVALID_RC_INDEX)
				return std::unexpected(kernel_error::resource__resource__get_instance_const_by_rc_index__index_invalid);

			const_iterator it_end = this->set.end();
			const_iterator it_res = std::lower_bound
			(
				this->set.begin(),
				it_end,
				index,
				[](const value_type& left, index_type right)->bool
				{
					return std::less<>{}(left.index, right);
				}
			);

			if (it_res == it_end)
				return std::unexpected(kernel_error::resource__resource__get_instance_const_by_rc_index__index_invalid);

			if (is_save_lower == true)
				return it_res;

			if (it_res->index != index)
				return std::unexpected(kernel_error::resource__resource__get_instance_const_by_rc_index__index_invalid);

			return it_res;
		}
	public:
		resource() = default;
		explicit resource(const allocator_type& allocator_val) :
			alloc_(allocator_val), set(allocator_rc_set(allocator_val))
		{

		}
		resource(const resource& other_val) :
			alloc_(allocator_rc_traits::select_on_container_copy_construction(other_val.alloc_)), set(other_val.set)
		{

		}
		resource& operator=(const resource& other_val)
		{
			if (this == &other_val)
				return *this;

			if constexpr (allocator_rc_traits::propagate_on_container_copy_assignment::value == true)
				this->alloc_ = other_val.alloc_;

			this->set = other_val.set;

			return *this;
		}
		resource(resource&& other_val) :
			alloc_(std::move(other_val.alloc_)), set(std::move(other_val.set))
		{

		}
		resource& operator=(resource&& other_val)
		{
			if (this == &other_val)
				return *this;

			if constexpr (allocator_rc_traits::propagate_on_container_move_assignment::value == true)
				this->alloc_ = std::move(other_val.alloc_);

			this->set = std::move(other_val.set);

			return *this;
		}
	public:
		rc_data::rc_res_inst_ref<value_type> add(index_type index_new, bool is_increase_ref = true)
		{
			bool is_created_ignore = false;

			return this->add(index_new, is_increase_ref, is_created_ignore);
		}
		rc_data::rc_res_inst_ref<value_type> add(index_type index_new, bool is_increase_ref, bool& is_created)
		{
			is_created = false;

			if (index_new == T::index_this_invalid_value)
				return std::unexpected(kernel_error::resource__resource__add__index_invalid);

			rc_data::rc_res_inst_it<value_type, allocator_rc_set> it_res = get_instance_by_rc_index(index_new, true);

			if (it_res.has_value() == false)
			{
				rc_data::rc_res_inst_ref<value_type> emplace_res = this->set.emplace_back(index_new, 1, T(this->alloc_));

				if (emplace_res.has_value() == true)
					is_created = true;

				return emplace_res;
			}

			iterator& it_ref = it_res.value();

			if (it_ref->get_index() == index_new)
			{
				if (is_increase_ref == true) ++it_ref->get_ref();
				return *it_ref;
			}

			typename mgr_ns::container_plus<value_type, allocator_rc_set>::result_iterator insert_res = this->set.insert(it_ref, index_new, 1, T(this->alloc_));

			if (insert_res.has_value() == false)
				return std::unexpected(insert_res.error());

			is_created = true;
			return *insert_res.value();
		}
		template<typename cursor_t>
		rc_data::rc_res_inst_ref<value_type> add_ex(index_type index_new, bool is_increase_ref, index_type cursor_owner, cursor_t& it_cursor)
		{
			bool is_created_ignore = false;

			return this->add_ex(index_new, is_increase_ref, cursor_owner, it_cursor, is_created_ignore);
		}
		template<typename cursor_t>
		rc_data::rc_res_inst_ref<value_type> add_ex(index_type index_new, bool is_increase_ref, index_type cursor_owner, cursor_t& it_cursor, bool& is_created)
		{
			rc_data::rc_res_inst_ref<value_type> add_res = this->add(index_new, is_increase_ref, is_created);
			rc_data::rc_res_inst_ref<value_type> cursor_owner_res = this->get(cursor_owner);

			if (cursor_owner_res.has_value() == true)
				cursor_owner_res.value().get().get_content().rebind(it_cursor);

			return add_res;
		}
		rc_data::rc_res destroy(index_type index)
		{
			rc_data::rc_res_inst_it<value_type, allocator_rc_set> it_res = get_instance_by_rc_index(index, false);

			if (it_res.has_value() == false)
				return std::unexpected(it_res.error());

			iterator& it_ref = it_res.value();

			if (--it_ref->get_ref() == 0)
			{
				mgr_data::manager_result erase_res = this->set.erase(it_res.value());

				if (erase_res.has_value() == false)
					throw kernel_error_throw(erase_res.error(), "kernel | resource | resource | destroy | 引用已归零，删除资源失败，错误不可恢复");
			}

			return {};
		}
		template<typename cursor_t>
		rc_data::rc_res destroy_ex(index_type index, index_type cursor_owner, cursor_t& it_cursor)
		{
			rc_data::rc_res destroy_res = this->destroy(index);
			rc_data::rc_res_inst_ref<value_type> cursor_owner_res = this->get(cursor_owner);

			if (cursor_owner_res.has_value() == true)
				cursor_owner_res.value().get().get_content().rebind(it_cursor);
			else
				it_cursor.invalidate();

			return destroy_res;
		}
		template<typename iterator_type>
		rc_data::rc_res destroy_it(iterator_type&& it)
		{
			if (it == end())
				return std::unexpected(kernel_error::resource__resource__destroy_it__iterator_invalid);

			if (--it->get_ref() == 0)
			{
				mgr_data::manager_result erase_res = this->set.erase(it);

				if (erase_res.has_value() == false)
					throw kernel_error_throw(erase_res.error(), "kernel | resource | resource | destroy_it | 引用已归零，删除资源失败，错误不可恢复");
			}

			return {};
		}
		rc_data::rc_res_inst_ref<value_type> get(index_type index)
		{
			rc_data::rc_res_inst_it<value_type, allocator_rc_set> it_res = get_instance_by_rc_index(index, false);

			if (it_res.has_value() == false)
				return std::unexpected(it_res.error());

			return *it_res.value();
		}
		rc_data::rc_res_inst_ref_const<value_type> get(index_type index) const
		{
			rc_data::rc_res_inst_it_const<value_type, allocator_rc_set> it_res = get_instance_const_by_rc_index(index, false);

			if (it_res.has_value() == false)
				return std::unexpected(it_res.error());

			return *it_res.value();
		}
		size_type size() const noexcept
		{
			return static_cast<size_type>(this->set.size());
		}
		void clear()
		{
			this->set.clear();
			return;
		}
		rc_data::rc_res shrink_to_fit()
		{
			return this->set.shrink_to_fit();
		}
		void swap(resource& other_val) noexcept
		{
			if (this == &other_val)
				return;

			std::swap(this->alloc_, other_val.alloc_);
			this->set.swap(other_val.set);
			return;
		}
		bool empty() const noexcept
		{
			return this->set.empty() == true;
		}
		iterator begin() noexcept
		{
			return iterator(this->set.begin());
		}
		iterator end() noexcept
		{
			return iterator(this->set.end());
		}
		reverse_iterator rbegin() noexcept
		{
			return reverse_iterator(this->set.rbegin());
		}
		reverse_iterator rend() noexcept
		{
			return reverse_iterator(this->set.rend());
		}
		const_iterator begin() const noexcept
		{
			return const_iterator(this->set.begin());
		}
		const_iterator end() const noexcept
		{
			return const_iterator(this->set.end());
		}
		const_iterator cbegin() const noexcept
		{
			return const_iterator(this->set.begin());
		}
		const_iterator cend() const noexcept
		{
			return const_iterator(this->set.end());
		}
		const_reverse_iterator crbegin() const noexcept
		{
			return const_reverse_iterator(this->set.cend());
		}
		const_reverse_iterator crend() const noexcept
		{
			return const_reverse_iterator(this->set.cbegin());
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

	template<typename T, typename rc_variable_allocator = std::allocator<T>>
	using rc_variable = resource<var_mgr_ns::variable_manager<T, rc_variable_allocator>>;

	template<typename rc_command_allocator = std::allocator<cmd_data::cmd_op_single>>
	using rc_command = resource<cmd_mgr_ns::command_manager<rc_command_allocator>>;

}