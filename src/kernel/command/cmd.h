#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <utility>
#include <type_traits>
#include <bit>
#include <limits>
#include <array>
#include <span>
#include <algorithm>
#include <variant>
#include <expected>
#include "../error/error_code.h"
#include "../variable/var.h"
#include "cmd_data_def.h"

namespace rule_cmd
{
	template <typename T>
	class cmd_mgr_iterator
	{
	public:
		using iterator_category = typename T::iterator_category;
		using iterator_concept = typename T::iterator_concept;
	private:
		static constexpr bool is_const = std::is_const_v<std::remove_reference_t<decltype(*std::declval<T&>())>>;
	public:
		using value_type = cmd_data::cmd_single<is_const>;
		using difference_type = typename T::difference_type;
		using reference = value_type;
		using content_span_type = std::conditional_t<is_const, std::span<const cmd_data::cmd_op_single>, std::span<cmd_data::cmd_op_single>>;
	public:
		struct arrow_proxy
		{
		public:
			value_type view;
		public:
			value_type* operator->() noexcept
			{
				return &this->view;
			}
			const value_type* operator->() const noexcept
			{
				return &this->view;
			}
		};
	public:
		using pointer = arrow_proxy;
	private:
		T it;
	public:
		cmd_mgr_iterator() = default;
		cmd_mgr_iterator(const T& it_val) noexcept
		{
			this->it = it_val;
		}
	public:
		template<typename self_this>
		decltype(auto) get_base_iterator(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.it);
		}
	public:
		bool is_valid() const noexcept
		{
			return it.is_valid();
		}
		void invalidate() noexcept
		{
			this->it.invalidate();
			return;
		}
	public:
		cmd_mgr_iterator& operator++()
		{
			this->it++;
			return *this;
		}
		cmd_mgr_iterator operator++(int)
		{
			cmd_mgr_iterator tmp = *this;
			this->it++;
			return tmp;
		}
		cmd_mgr_iterator& operator--()
		{
			this->it--;
			return *this;
		}
		cmd_mgr_iterator operator--(int)
		{
			cmd_mgr_iterator tmp = *this;
			this->it--;
			return tmp;
		}
		cmd_mgr_iterator& operator+=(difference_type add_val)
		{
			this->it += add_val;
			return *this;
		}
		cmd_mgr_iterator& operator-=(difference_type sub_val)
		{
			this->it -= sub_val;
			return *this;
		}
		cmd_mgr_iterator operator+(difference_type add_val) const
		{
			cmd_mgr_iterator tmp = *this;
			tmp.it += add_val;
			return tmp;
		}
		difference_type operator-(const cmd_mgr_iterator& it_val) const
		{
			return this->it - it_val.it;
		}
		cmd_mgr_iterator operator-(difference_type sub_val) const
		{
			cmd_mgr_iterator tmp = *this;
			tmp.it -= sub_val;
			return tmp;
		}
		reference operator[](difference_type index) const
		{
			cmd_mgr_iterator tmp = *this;
			tmp.it += index;

			auto content_res = tmp.it->content.get();

			if (content_res.has_value() == false)
				throw kernel_error_throw(content_res.error(), "kernel | command | cmd_mgr_iterator | operator[] | 指令内容获取失败");

			return value_type{ static_cast<cmd_data::cmd_index>(tmp.it->index), tmp.it->type, content_span_type(content_res.value().get()) };
		}
		bool operator>=(const cmd_mgr_iterator& it_val) const
		{
			return this->it >= it_val.it;
		}
		bool operator>(const cmd_mgr_iterator& it_val) const
		{
			return this->it > it_val.it;
		}
		bool operator<=(const cmd_mgr_iterator& it_val) const
		{
			return this->it <= it_val.it;
		}
		bool operator<(const cmd_mgr_iterator& it_val) const
		{
			return this->it < it_val.it;
		}
		bool operator==(const cmd_mgr_iterator& it_val) const noexcept
		{
			return this->it == it_val.it;
		}
		bool operator!=(const cmd_mgr_iterator& it_val) const
		{
			return !(this->operator==(it_val));
		}
		reference operator*() const
		{
			auto content_res = this->it->get_content().get();

			if (content_res.has_value() == false)
				throw kernel_error_throw(content_res.error(), "kernel | command | cmd_mgr_iterator | operator* | 指令内容获取失败");

			return reference{ static_cast<cmd_data::cmd_index>(this->it->get_index()), this->it->get_type(), content_span_type(content_res.value().get()) };
		}
		pointer operator->() const
		{
			return pointer{ **this };
		}
	};

	template<typename cmd_mgr_alloc = std::allocator<cmd_data::cmd_op_single>>
	class command_manager
	{
		static_assert(std::is_same_v<typename std::allocator_traits<cmd_mgr_alloc>::value_type, cmd_data::cmd_op_single>, "分配器类型必须为 cmd_op_single");
	private:
		var_mgr_ns::variable_manager<cmd_data::cmd_op_single, cmd_mgr_alloc> cmd_set;
	public:
		using allocator_type = cmd_mgr_alloc;
		using value_type = cmd_data::cmd_single<false>;
		using command_type = value_type;
		using const_value_type = cmd_data::cmd_single<true>;
		using size_type = typename var_mgr_ns::variable_manager<cmd_data::cmd_op_single, cmd_mgr_alloc>::size_type;
		using index_type = cmd_data::cmd_index;
		using index_this_type = cmd_data::cmd_mgr_index;
		using unexpected_error_type = kernel_error;
		using command_define_id_type = cmd_data::cmd_type;
		using iterator = cmd_mgr_iterator<typename var_mgr_ns::variable_manager<cmd_data::cmd_op_single, cmd_mgr_alloc>::iterator>;
		using const_iterator = cmd_mgr_iterator<typename var_mgr_ns::variable_manager<cmd_data::cmd_op_single, cmd_mgr_alloc>::const_iterator>;
		using reverse_iterator = cmd_mgr_iterator<typename var_mgr_ns::variable_manager<cmd_data::cmd_op_single, cmd_mgr_alloc>::reverse_iterator>;
		using const_reverse_iterator = cmd_mgr_iterator<typename var_mgr_ns::variable_manager<cmd_data::cmd_op_single, cmd_mgr_alloc>::const_reverse_iterator>;
	public:
		static constexpr index_this_type index_this_invalid_value = cmd_data::INVALID_CMD_MGR_INDEX;
	public:
		command_manager() = default;
		explicit command_manager(const cmd_mgr_alloc& allocator_val) :
			cmd_set(allocator_val)
		{

		}
		command_manager(const command_manager& other_val) :
			cmd_set(other_val.cmd_set)
		{

		}
		command_manager& operator=(const command_manager& other_val)
		{
			if (this == &other_val)
				return *this;

			this->cmd_set = other_val.cmd_set;

			return *this;
		}
		command_manager(command_manager&& other_val) noexcept :
			cmd_set(std::move(other_val.cmd_set))
		{

		}
		command_manager& operator=(command_manager&& other_val)
		{
			if (this == &other_val)
				return *this;

			this->cmd_set = std::move(other_val.cmd_set);

			return *this;
		}
	public:
		cmd_data::cmd_mgr_res add_set(cmd_data::cmd_index index, cmd_data::cmd_type type, cmd_data::cmd_op_index pos_write_begin, const std::span<const cmd_data::cmd_op_single>& span_op_write, bool is_increase_ref = true)
		{
			return this->cmd_set.add_set(static_cast<var_cont_data::var_index>(index), static_cast<var_cont_data::var_type>(type), static_cast<var_cont_data::var_pos>(pos_write_begin), span_op_write, is_increase_ref, true);
		}
		cmd_data::cmd_mgr_res add_set(cmd_data::cmd_index index, cmd_data::cmd_type type, cmd_data::cmd_op_index pos_write_begin, cmd_data::cmd_op_width width_write_fill, bool is_increase_ref = true)
		{
			bool is_inserted_new_ignore = false;

			return this->add_set(index, type, pos_write_begin, width_write_fill, is_increase_ref, is_inserted_new_ignore);
		}
		cmd_data::cmd_mgr_res add_set(cmd_data::cmd_index index, cmd_data::cmd_type type, cmd_data::cmd_op_index pos_write_begin, cmd_data::cmd_op_width width_write_fill, bool is_increase_ref, bool& is_inserted_new)
		{
			return this->cmd_set.add_set
			(
				static_cast<var_cont_data::var_index>(index),
				static_cast<var_cont_data::var_type>(type),
				static_cast<var_cont_data::var_pos>(pos_write_begin),
				static_cast<var_cont_data::var_read_width>(width_write_fill),
				is_increase_ref,
				true,
				is_inserted_new
			);
		}
		cmd_data::cmd_mgr_res add_set_ex(cmd_data::cmd_index index, cmd_data::cmd_type type, cmd_data::cmd_op_index pos_write_begin, const std::span<const cmd_data::cmd_op_single>& span_op_write, bool is_increase_ref, iterator& it_cursor)
		{
			return this->cmd_set.add_set_ex
			(
				static_cast<var_cont_data::var_index>(index),
				static_cast<var_cont_data::var_type>(type),
				static_cast<var_cont_data::var_pos>(pos_write_begin),
				span_op_write,
				is_increase_ref,
				it_cursor.get_base_iterator(),
				true
			);
		}
		cmd_data::cmd_mgr_res add_set_ex(cmd_data::cmd_index index, cmd_data::cmd_type type, cmd_data::cmd_op_index pos_write_begin, cmd_data::cmd_op_width width_write_fill, bool is_increase_ref, iterator& it_cursor)
		{
			bool is_inserted_new_ignore = false;

			return this->add_set_ex(index, type, pos_write_begin, width_write_fill, is_increase_ref, it_cursor, is_inserted_new_ignore);
		}
		cmd_data::cmd_mgr_res add_set_ex(cmd_data::cmd_index index, cmd_data::cmd_type type, cmd_data::cmd_op_index pos_write_begin, cmd_data::cmd_op_width width_write_fill, bool is_increase_ref, iterator& it_cursor, bool& is_inserted_new)
		{
			return this->cmd_set.add_set_ex
			(
				static_cast<var_cont_data::var_index>(index),
				static_cast<var_cont_data::var_type>(type),
				static_cast<var_cont_data::var_pos>(pos_write_begin),
				static_cast<var_cont_data::var_read_width>(width_write_fill),
				is_increase_ref,
				it_cursor.get_base_iterator(),
				true,
				is_inserted_new
			);
		}
	public:
		void rebind(iterator& it) noexcept
		{
			this->cmd_set.rebind(it.get_base_iterator());
			return;
		}
		cmd_data::cmd_mgr_res_cmd<value_type> get(cmd_data::cmd_index index)
		{
			var_data::var_mgr_res_var_sgl<cmd_data::cmd_op_single, cmd_mgr_alloc> op_info_target = this->cmd_set.get(static_cast<var_cont_data::var_index>(index));

			if (op_info_target.has_value() == false)
				return std::unexpected(op_info_target.error());

			var_data::var_sgl<cmd_data::cmd_op_single, cmd_mgr_alloc>& op_info_target_ref = op_info_target.value().get();
			var_cont_data::var_cont_res_set<cmd_data::cmd_op_single, cmd_mgr_alloc> content_res = op_info_target_ref.get_content().get();

			if (content_res.has_value() == false)
				return std::unexpected(content_res.error());

			return value_type
			{
				static_cast<cmd_data::cmd_index>(op_info_target_ref.get_index()),
				op_info_target_ref.get_type(),
				std::span<cmd_data::cmd_op_single>(content_res.value().get())
			};
		}
		cmd_data::cmd_mgr_res_cmd<const_value_type> get(cmd_data::cmd_index index) const
		{
			var_data::var_mgr_res_var_sgl_const<cmd_data::cmd_op_single, cmd_mgr_alloc> op_info_target = this->cmd_set.get(static_cast<var_cont_data::var_index>(index));

			if (op_info_target.has_value() == false)
				return std::unexpected(op_info_target.error());

			const var_data::var_sgl<cmd_data::cmd_op_single, cmd_mgr_alloc>& op_info_target_ref = op_info_target.value().get();
			var_cont_data::var_cont_res_set_const<cmd_data::cmd_op_single, cmd_mgr_alloc> content_res = op_info_target_ref.get_content().get();

			if (content_res.has_value() == false)
				return std::unexpected(content_res.error());

			return const_value_type
			{
				static_cast<cmd_data::cmd_index>(op_info_target_ref.get_index()),
				op_info_target_ref.get_type(),
				std::span<const cmd_data::cmd_op_single>(content_res.value().get())
			};
		}
		cmd_data::cmd_mgr_res_size<size_type> get_ref(cmd_data::cmd_index index) const
		{
			var_data::var_mgr_res_var_sgl_const<cmd_data::cmd_op_single, cmd_mgr_alloc> op_info_target = this->cmd_set.get(static_cast<var_cont_data::var_index>(index));

			if (op_info_target.has_value() == false)
				return std::unexpected(op_info_target.error());

			return op_info_target.value().get().get_ref();
		}
		cmd_data::cmd_mgr_res_size<size_type> get_op_size(cmd_data::cmd_index index) const
		{
			var_data::var_mgr_res_var_sgl_const<cmd_data::cmd_op_single, cmd_mgr_alloc> op_info_target = this->cmd_set.get(static_cast<var_cont_data::var_index>(index));

			if (op_info_target.has_value() == false)
				return std::unexpected(op_info_target.error());

			return op_info_target.value().get().content.size();
		}
		cmd_data::cmd_mgr_res_cmd_single_it<iterator> get_it(cmd_data::cmd_index index)
		{
			var_data::var_mgr_res_var_sgl_it<typename var_mgr_ns::variable_manager<cmd_data::cmd_op_single, cmd_mgr_alloc>::iterator> it_res = this->cmd_set.get_it(static_cast<var_cont_data::var_index>(index));

			if (it_res.has_value() == false)
				return std::unexpected(it_res.error());

			return iterator(it_res.value());
		}
		cmd_data::cmd_mgr_res_cmd_single_it_const<const_iterator> get_it(cmd_data::cmd_index index) const
		{
			var_data::var_mgr_res_var_sgl_it<typename var_mgr_ns::variable_manager<cmd_data::cmd_op_single, cmd_mgr_alloc>::const_iterator> it_res = this->cmd_set.get_it(static_cast<var_cont_data::var_index>(index));

			if (it_res.has_value() == false)
				return std::unexpected(it_res.error());

			return const_iterator(it_res.value());
		}
		cmd_data::cmd_mgr_res del(cmd_data::cmd_index index)
		{
			return this->cmd_set.del(static_cast<var_cont_data::var_index>(index));
		}
		template<typename iterator_type>
		cmd_data::cmd_mgr_res del_it(iterator_type&& it)
		{
			if (it.get_base_iterator() == end().get_base_iterator())
				return std::unexpected(kernel_error::command__command_manager__del_it__iterator_invalid);

			return this->cmd_set.del_it(it.get_base_iterator());
		}
		cmd_data::cmd_mgr_res del(cmd_data::cmd_index index, cmd_data::cmd_op_index pos_del_begin, cmd_data::cmd_op_width width_del, bool is_del_full_span = false)
		{
			return this->cmd_set.del(static_cast<var_cont_data::var_index>(index), static_cast<var_cont_data::var_pos>(pos_del_begin), static_cast<var_cont_data::var_read_width>(width_del), is_del_full_span);
		}
		cmd_data::cmd_mgr_res del_ex(cmd_data::cmd_index index, iterator& it_cursor)
		{
			return this->cmd_set.del_ex(static_cast<var_cont_data::var_index>(index), it_cursor.get_base_iterator());
		}
		template<typename iterator_type>
		cmd_data::cmd_mgr_res del_it_ex(iterator_type&& it, iterator& it_cursor)
		{
			if (it.get_base_iterator() == end().get_base_iterator())
				return std::unexpected(kernel_error::command__command_manager__del_it__iterator_invalid);

			return this->cmd_set.del_it_ex(it.get_base_iterator(), it_cursor.get_base_iterator());
		}
		cmd_data::cmd_mgr_res del_ex(cmd_data::cmd_index index, cmd_data::cmd_op_index pos_del_begin, cmd_data::cmd_op_width width_del, bool is_del_full_span, iterator& it_cursor)
		{
			return this->cmd_set.del_ex
			(
				static_cast<var_cont_data::var_index>(index),
				static_cast<var_cont_data::var_pos>(pos_del_begin),
				static_cast<var_cont_data::var_read_width>(width_del),
				is_del_full_span,
				it_cursor.get_base_iterator()
			);
		}
		size_type size() const noexcept
		{
			return this->cmd_set.size();
		}
		void clear() noexcept
		{
			this->cmd_set.clear();
		}
		cmd_data::cmd_mgr_res shrink_to_fit()
		{
			return this->cmd_set.shrink_to_fit();
		}
		void swap(command_manager& other_val) noexcept
		{
			if (this == &other_val)
				return;

			this->cmd_set.swap(other_val.cmd_set);
			return;
		}
		bool empty() const noexcept
		{
			return this->cmd_set.empty();
		}
		iterator begin() noexcept
		{
			return iterator(this->cmd_set.begin());
		}
		iterator end() noexcept
		{
			return iterator(this->cmd_set.end());
		}
		reverse_iterator rbegin() noexcept
		{
			return reverse_iterator(this->cmd_set.rbegin());
		}
		reverse_iterator rend() noexcept
		{
			return reverse_iterator(this->cmd_set.rend());
		}
		const_iterator begin() const noexcept
		{
			return const_iterator(this->cmd_set.begin());
		}
		const_iterator end() const noexcept
		{
			return const_iterator(this->cmd_set.end());
		}
		const_iterator cbegin() const noexcept
		{
			return const_iterator(this->cmd_set.begin());
		}
		const_iterator cend() const noexcept
		{
			return const_iterator(this->cmd_set.end());
		}
		const_reverse_iterator crbegin() const noexcept
		{
			return const_reverse_iterator(this->cmd_set.crbegin());
		}
		const_reverse_iterator crend() const noexcept
		{
			return const_reverse_iterator(this->cmd_set.crend());
		}
	public:
		[[nodiscard]] bool is_valid(const iterator& it) const noexcept
		{
			return this->cmd_set.is_valid(it.get_base_iterator());
		}
		[[nodiscard]] bool is_valid(const const_iterator& it) const noexcept
		{
			return this->cmd_set.is_valid(it.get_base_iterator());
		}
		[[nodiscard]] bool is_valid(const reverse_iterator& it) const noexcept
		{
			return this->cmd_set.is_valid(it.get_base_iterator());
		}
		[[nodiscard]] bool is_valid(const const_reverse_iterator& it) const noexcept
		{
			return this->cmd_set.is_valid(it.get_base_iterator());
		}
	};


}
