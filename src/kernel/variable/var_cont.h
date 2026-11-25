#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <utility>
#include <type_traits>
#include <span>
#include <vector>
#include <expected>
#include <algorithm>
#include "../error/error_code.h"
#include "var_cont_data_def.h"

namespace rule_var_cont
{
	template<typename T, typename var_block_alloc = std::allocator<T>>
	class var_block
	{
	public:
		using allocator_type = var_block_alloc;
		using value_type = T;
		using size_type = std::size_t;
	private:
		size_type start = 0;
		std::vector<value_type, allocator_type> data;
	public:
		var_block() = default;
		var_block(size_type start_val, const allocator_type& allocator_val) :
			start(start_val),
			data(allocator_val)
		{
		}
	public:
		template<typename self_this>
		decltype(auto) get_start(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.start);
		}
		template<typename self_this>
		decltype(auto) get_data(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.data);
		}
		size_type get_length() const noexcept
		{
			return this->data.size();
		}
		size_type get_capacity() const noexcept
		{
			return this->data.capacity();
		}
	public:
		void set_start(size_type start_val) noexcept
		{
			this->start = start_val;
		}
		void set_data(std::vector<value_type, allocator_type>&& data_val) noexcept
		{
			this->data = std::move(data_val);
		}
	};

	template<typename T, typename var_cont_alloc = std::allocator<T>>
	class var_content
	{
		static_assert(std::is_same_v<typename std::allocator_traits<var_cont_alloc>::value_type, T>, "分配器类型必须与元素类型相同");
	public:
		using allocator_type = var_cont_alloc;
		using value_type = T;
		using size_type = std::size_t;
	private:
		static constexpr size_type block_redundancy_bytes = 128;
		static constexpr size_type invalid_pos = std::numeric_limits<size_type>::max();
	private:
		using block = var_block<value_type, allocator_type>;
		using block_alloc = typename std::allocator_traits<allocator_type>::template rebind_alloc<block>;
	private:
		std::vector<block, block_alloc> blocks;
		allocator_type allocator_;
	private:
		constexpr bool test_validity_var_pos(var_cont_data::var_pos pos) const noexcept
		{
			if (pos == var_cont_data::INVALID_VAR_POS)
				return false;

			return static_cast<size_type>(pos) < this->size();
		}
		constexpr bool test_validity_var_read_width(var_cont_data::var_pos pos_begin, var_cont_data::var_read_width width) const noexcept
		{
			if (test_validity_var_pos(pos_begin) == false)
				return false;

			if (static_cast<size_type>(pos_begin) > std::numeric_limits<size_type>::max() - static_cast<size_type>(width))
				return false;

			return static_cast<size_type>(pos_begin) + static_cast<size_type>(width) <= this->size();
		}
		size_type locate(size_type pos) const noexcept
		{
			typename std::vector<block, block_alloc>::const_iterator it = std::lower_bound
			(
				this->blocks.begin(),
				this->blocks.end(),
				pos,
				[](const block& block_v, size_type pos_v) { return block_v.get_start() < pos_v; }
			);

			if (it != this->blocks.end() && it->get_start() == pos)
				return static_cast<size_type>(it - this->blocks.begin());

			if (it == this->blocks.begin())
				return invalid_pos;

			typename std::vector<block, block_alloc>::const_iterator it_prev = it - 1;

			if (pos < it_prev->get_start() + it_prev->get_length())
				return static_cast<size_type>(it_prev - this->blocks.begin());

			return invalid_pos;
		}
		size_type locate_insert_pos(size_type pos) const noexcept
		{
			typename std::vector<block, block_alloc>::const_iterator it = std::lower_bound
			(
				this->blocks.begin(),
				this->blocks.end(),
				pos,
				[](const block& block_v, size_type pos_v) { return block_v.get_start() < pos_v; }
			);

			return static_cast<size_type>(it - this->blocks.begin());
		}
		size_type locate_first_end_gt(size_type pos) const noexcept
		{
			typename std::vector<block, block_alloc>::const_iterator it = std::lower_bound
			(
				this->blocks.begin(),
				this->blocks.end(),
				pos,
				[](const block& block_v, size_type pos_v) { return block_v.get_start() + block_v.get_length() <= pos_v; }
			);

			return static_cast<size_type>(it - this->blocks.begin());
		}
		size_type calc_capacity(size_type start, size_type length, size_type limit) const noexcept
		{
			const size_type need_bytes = length * sizeof(value_type);
			size_type cap_bytes = need_bytes;

			if (block_redundancy_bytes != 0)
			{
				const size_type rem = need_bytes % block_redundancy_bytes;

				if (rem != 0)
					cap_bytes = need_bytes + (block_redundancy_bytes - rem);
			}

			size_type cap = cap_bytes / sizeof(value_type);

			if (cap < length) cap = length;

			if (limit != invalid_pos)
			{
				const size_type cap_max = limit - start;

				if (cap > cap_max)
					cap = cap_max;
			}

			if (cap < length)
				cap = length;

			return cap;
		}
		bool merge_pair(size_type index)
		{
			block& block_left = this->blocks[index];
			block& block_right = this->blocks[index + 1];
			const size_type length_new = (block_right.get_start() + block_right.get_length()) - block_left.get_start();
			const size_type limit = (index + 2 < this->blocks.size()) ? this->blocks[index + 2].get_start() : invalid_pos;
			block block_merged(block_left.get_start(), this->allocator_);

			try
			{
				block_merged.get_data().reserve(this->calc_capacity(block_left.get_start(), length_new, limit));
				block_merged.get_data().resize(length_new);
			}
			catch (...)
			{
				return false;
			}

			std::copy(block_left.get_data().begin(), block_left.get_data().end(), block_merged.get_data().begin());
			std::copy(block_right.get_data().begin(), block_right.get_data().end(), block_merged.get_data().begin() + static_cast<typename std::vector<value_type, allocator_type>::difference_type>(block_left.get_length()));
			block_left.set_data(std::move(block_merged.get_data()));

			return true;
		}
		bool merge_touching(size_type index)
		{
			while (index > 0 && this->blocks[index - 1].get_start() + this->blocks[index - 1].get_length() == this->blocks[index].get_start())
			{
				if (this->merge_pair(index - 1) == false)
					return false;

				this->blocks.erase(this->blocks.begin() + static_cast<typename std::vector<block, block_alloc>::difference_type>(index));
				--index;
			}

			while (index + 1 < this->blocks.size() && this->blocks[index].get_start() + this->blocks[index].get_length() == this->blocks[index + 1].get_start())
			{
				if (this->merge_pair(index) == false)
					return false;

				this->blocks.erase(this->blocks.begin() + static_cast<typename std::vector<block, block_alloc>::difference_type>(index + 1));
			}

			return true;
		}
		var_cont_data::var_cont_res_span<value_type> ensure_window(size_type pos, size_type width)
		{
			if (width == 0)
			{
				if (this->blocks.empty())
				{
					try
					{
						block block_new(pos, this->allocator_);

						this->blocks.emplace_back(std::move(block_new));
					}
					catch (...)
					{
						return std::unexpected(kernel_error::variable__var_content__ensure__allocate_unexpected);
					}
				}

				return std::span<value_type>();
			}

			if (pos > std::numeric_limits<size_type>::max() - width)
				return std::unexpected(kernel_error::variable__var_content__ensure__range_overflow);

			const size_type index_contain = this->locate(pos);

			if (index_contain != invalid_pos)
			{
				block& block_contain = this->blocks[index_contain];
				const size_type offset = pos - block_contain.get_start();

				if (offset + width <= block_contain.get_length())
					return std::span<value_type>(block_contain.get_data().data() + offset, width);
			}

			const size_type end = pos + width;
			const size_type count = this->blocks.size();
			const size_type index_first = this->locate_first_end_gt(pos);
			const size_type index_last = this->locate_insert_pos(end);

			if (index_first == index_last)
			{
				if (index_first > 0)
				{
					block& block_prev = this->blocks[index_first - 1];
					const size_type prev_end = block_prev.get_start() + block_prev.get_length();
					const size_type limit = (index_first < count) ? this->blocks[index_first].get_start() : invalid_pos;

					if (pos == prev_end)
					{
						const size_type length_need = end - block_prev.get_start();

						if (end > block_prev.get_start() + block_prev.get_capacity())
						{
							size_type cap_bytes = length_need * sizeof(value_type);
							const size_type cap_grow_bytes = block_prev.get_capacity() * sizeof(value_type) * 2;

							if (cap_bytes < cap_grow_bytes)
								cap_bytes = cap_grow_bytes;

							if (block_redundancy_bytes != 0)
							{
								const size_type rem = cap_bytes % block_redundancy_bytes;

								if (rem != 0)
									cap_bytes += block_redundancy_bytes - rem;
							}

							size_type cap_new = cap_bytes / sizeof(value_type);

							if (cap_new < length_need)
								cap_new = length_need;

							if (limit != invalid_pos && cap_new > limit - block_prev.get_start())
								cap_new = limit - block_prev.get_start();

							if (cap_new < length_need)
								cap_new = length_need;

							try
							{
								block_prev.get_data().reserve(cap_new);
							}
							catch (...)
							{
								return std::unexpected(kernel_error::variable__var_content__ensure__allocate_unexpected);
							}
						}

						try
						{
							block_prev.get_data().resize(length_need);
						}
						catch (...)
						{
							return std::unexpected(kernel_error::variable__var_content__ensure__allocate_unexpected);
						}

						if (this->merge_touching(index_first - 1) == false)
							return std::unexpected(kernel_error::variable__var_content__ensure__allocate_unexpected);

						const size_type index_res = this->locate(pos);

						if (index_res == invalid_pos)
							return std::unexpected(kernel_error::variable__var_content__ensure__allocate_unexpected);

						block& block_res = this->blocks[index_res];
						return std::span<value_type>(block_res.get_data().data() + (pos - block_res.get_start()), width);
					}
				}

				const size_type limit = (index_first < count) ? this->blocks[index_first].get_start() : invalid_pos;
				block block_new(pos, this->allocator_);

				try
				{
					block_new.get_data().reserve(this->calc_capacity(pos, width, limit));
					block_new.get_data().resize(width);
				}
				catch (...)
				{
					return std::unexpected(kernel_error::variable__var_content__ensure__allocate_unexpected);
				}

				try
				{
					this->blocks.insert(this->blocks.begin() + static_cast<typename std::vector<block, block_alloc>::difference_type>(index_first), std::move(block_new));
				}
				catch (...)
				{
					return std::unexpected(kernel_error::variable__var_content__ensure__allocate_unexpected);
				}

				if (this->merge_touching(index_first) == false)
					return std::unexpected(kernel_error::variable__var_content__ensure__allocate_unexpected);

				const size_type index_res = this->locate(pos);

				if (index_res == invalid_pos)
					return std::unexpected(kernel_error::variable__var_content__ensure__allocate_unexpected);

				block& block_res = this->blocks[index_res];
				return std::span<value_type>(block_res.get_data().data() + (pos - block_res.get_start()), width);
			}

			const size_type start_union = (pos < this->blocks[index_first].get_start()) ? pos : this->blocks[index_first].get_start();
			const size_type end_last = this->blocks[index_last - 1].get_start() + this->blocks[index_last - 1].get_length();
			const size_type end_union = (end > end_last) ? end : end_last;
			const size_type limit = (index_last < count) ? this->blocks[index_last].get_start() : invalid_pos;
			block block_merged(start_union, this->allocator_);

			try
			{
				block_merged.get_data().reserve(this->calc_capacity(start_union, end_union - start_union, limit));
				block_merged.get_data().resize(end_union - start_union);
			}
			catch (...)
			{
				return std::unexpected(kernel_error::variable__var_content__ensure__allocate_unexpected);
			}

			for (size_type i = index_first; i < index_last; ++i)
			{
				const block& block_ref = this->blocks[i];
				std::copy(block_ref.get_data().begin(), block_ref.get_data().end(), block_merged.get_data().begin() + static_cast<typename std::vector<value_type, allocator_type>::difference_type>(block_ref.get_start() - start_union));
			}

			try
			{
				this->blocks.erase(this->blocks.begin() + static_cast<typename std::vector<block, block_alloc>::difference_type>(index_first), this->blocks.begin() + static_cast<typename std::vector<block, block_alloc>::difference_type>(index_last));
				this->blocks.insert(this->blocks.begin() + static_cast<typename std::vector<block, block_alloc>::difference_type>(index_first), std::move(block_merged));
			}
			catch (...)
			{
				return std::unexpected(kernel_error::variable__var_content__ensure__allocate_unexpected);
			}

			if (this->merge_touching(index_first) == false)
				return std::unexpected(kernel_error::variable__var_content__ensure__allocate_unexpected);

			const size_type index_res = this->locate(pos);

			if (index_res == invalid_pos)
				return std::unexpected(kernel_error::variable__var_content__ensure__allocate_unexpected);

			block& block_res = this->blocks[index_res];
			return std::span<value_type>(block_res.get_data().data() + (pos - block_res.get_start()), width);
		}
	public:
		var_content() :
			blocks(block_alloc(allocator_type())),
			allocator_()
		{
		}
		var_content(size_type size, value_type var_content_val = value_type()) :
			blocks(block_alloc(allocator_type())),
			allocator_()
		{
			if (size == 0)
				return;

			try
			{
				block block_new(0, this->allocator_);

				block_new.get_data().reserve(this->calc_capacity(0, size, invalid_pos));
				block_new.get_data().resize(size, var_content_val);
				this->blocks.emplace_back(std::move(block_new));
			}
			catch (...)
			{
				throw kernel_error_throw(kernel_error::variable__var_content__var_content__resize_unexpected, "kernel | variable | var_content | var_content | 内容初始化失败，错误不可恢复");
			}
		}
		explicit var_content(const allocator_type& allocator_val) :
			blocks(block_alloc(allocator_val)),
			allocator_(allocator_val)
		{
		}
		explicit var_content(const var_content& other_val, const allocator_type& allocator_val = allocator_type()) :
			blocks(block_alloc(allocator_val)),
			allocator_(allocator_val)
		{
			try
			{
				this->blocks.reserve(other_val.blocks.size());

				for (const block& block_ref : other_val.blocks)
				{
					block block_new(block_ref.get_start(), allocator_val);

					block_new.get_data().assign(block_ref.get_data().begin(), block_ref.get_data().end());
					this->blocks.emplace_back(std::move(block_new));
				}
			}
			catch (const kernel_error_throw&)
			{
				throw;
			}
			catch (...)
			{
				throw kernel_error_throw(kernel_error::variable__var_content__copy_construct__copy_unexpected, "kernel | variable | var_content | copy_construct | 内容拷贝失败，错误不可恢复");
			}
		}
		var_content& operator=(const var_content& other_val)
		{
			if (this == &other_val)
				return *this;

			try
			{
				var_content content_new(other_val, this->allocator_);

				this->blocks = std::move(content_new.blocks);
			}
			catch (const kernel_error_throw&)
			{
				throw;
			}
			catch (...)
			{
				throw kernel_error_throw(kernel_error::variable__var_content__copy_assign__copy_unexpected, "kernel | variable | var_content | operator= | 内容拷贝赋值失败，错误不可恢复");
			}

			return *this;
		}
		var_content(var_content&& other_val) noexcept :
			blocks(std::move(other_val.blocks)),
			allocator_(std::move(other_val.allocator_))
		{
		}
		var_content& operator=(var_content&& other_val) noexcept
		{
			if (this == &other_val)
				return *this;

			this->blocks = std::move(other_val.blocks);
			this->allocator_ = std::move(other_val.allocator_);

			return *this;
		}
	public:
		var_cont_data::var_cont_res_span<value_type> read(var_cont_data::var_pos pos_begin, var_cont_data::var_read_width width)
		{
			if (test_validity_var_pos(pos_begin) == false)
				return std::unexpected(kernel_error::variable__var_content__read__var_pos_invalid);

			if (test_validity_var_read_width(pos_begin, width) == false)
				return std::unexpected(kernel_error::variable__var_content__read__var_width_invalid);

			const size_type index = this->locate(static_cast<size_type>(pos_begin));

			if (index == invalid_pos)
				return std::unexpected(kernel_error::variable__var_content__read__range_not_allocated);

			block& block_ref = this->blocks[index];
			const size_type offset = static_cast<size_type>(pos_begin) - block_ref.get_start();

			if (offset + static_cast<size_type>(width) > block_ref.get_length())
				return std::unexpected(kernel_error::variable__var_content__read__range_not_allocated);

			return std::span<value_type>(block_ref.get_data().data() + offset, static_cast<size_type>(width));
		}
		var_cont_data::var_cont_res_span<value_type> ensure(var_cont_data::var_pos pos_begin, var_cont_data::var_read_width width)
		{
			return this->ensure_window(static_cast<size_type>(pos_begin), static_cast<size_type>(width));
		}
		var_cont_data::var_cont_res ensure_dense(var_cont_data::var_read_width width_new)
		{
			var_cont_data::var_cont_res_span<value_type> span_res = this->ensure_window(0, static_cast<size_type>(width_new));

			if (span_res.has_value() == false)
				return std::unexpected(span_res.error());

			return {};
		}
		var_cont_data::var_cont_res del(var_cont_data::var_pos pos_begin, var_cont_data::var_read_width width)
		{
			if (test_validity_var_pos(pos_begin) == false)
				return std::unexpected(kernel_error::variable__var_content__del__var_pos_invalid);

			if (test_validity_var_read_width(pos_begin, width) == false)
				return std::unexpected(kernel_error::variable__var_content__del__var_width_invalid);

			const size_type pos = static_cast<size_type>(pos_begin);
			const size_type width_num = static_cast<size_type>(width);
			const size_type end = pos + width_num;

			try
			{
				std::vector<block, block_alloc> blocks_new(this->blocks.get_allocator());

				blocks_new.reserve(this->blocks.size() + 2);

				for (const block& block_ref : this->blocks)
				{
					const size_type start = block_ref.get_start();
					const size_type block_end = block_ref.get_start() + block_ref.get_length();

					if (block_end <= pos)
					{
						blocks_new.emplace_back(block_ref);
						continue;
					}

					if (start >= end)
					{
						block block_new(block_ref);

						block_new.set_start(start - width_num);
						blocks_new.emplace_back(std::move(block_new));
						continue;
					}

					if (start < pos)
					{
						block block_left(start, this->allocator_);

						block_left.get_data().assign(block_ref.get_data().begin(), block_ref.get_data().begin() + static_cast<typename std::vector<value_type, allocator_type>::difference_type>(pos - start));
						blocks_new.emplace_back(std::move(block_left));
					}

					if (block_end > end)
					{
						const size_type offset = end - start;
						block block_right(pos, this->allocator_);

						block_right.get_data().assign(block_ref.get_data().begin() + static_cast<typename std::vector<value_type, allocator_type>::difference_type>(offset), block_ref.get_data().end());
						blocks_new.emplace_back(std::move(block_right));
					}
				}

				for (size_type i = 0; i + 1 < blocks_new.size();)
				{
					block& block_left = blocks_new[i];
					block& block_right = blocks_new[i + 1];

					if (block_left.get_start() + block_left.get_length() != block_right.get_start())
					{
						++i;
						continue;
					}

					const size_type length_new = (block_right.get_start() + block_right.get_length()) - block_left.get_start();
					block block_merged(block_left.get_start(), this->allocator_);

					block_merged.get_data().reserve(this->calc_capacity(block_left.get_start(), length_new, invalid_pos));
					block_merged.get_data().resize(length_new);
					std::copy(block_left.get_data().begin(), block_left.get_data().end(), block_merged.get_data().begin());
					std::copy(block_right.get_data().begin(), block_right.get_data().end(), block_merged.get_data().begin() + static_cast<typename std::vector<value_type, allocator_type>::difference_type>(block_left.get_length()));
					block_left.set_data(std::move(block_merged.get_data()));
					blocks_new.erase(blocks_new.begin() + static_cast<typename std::vector<block, block_alloc>::difference_type>(i + 1));
				}

				if (blocks_new.empty() == true)
				{
					block block_empty(0, this->allocator_);

					blocks_new.emplace_back(std::move(block_empty));
				}

				this->blocks = std::move(blocks_new);
			}
			catch (...)
			{
				return std::unexpected(kernel_error::variable__var_content__del__erase_unexpected);
			}

			return {};
		}
		var_cont_data::var_cont_res_set<value_type, allocator_type> get()
		{
			if (this->blocks.size() != 1 || this->blocks[0].get_start() != 0)
				return std::unexpected(kernel_error::variable__var_content__get__content_not_contiguous);

			return std::ref(this->blocks[0].get_data());
		}
		var_cont_data::var_cont_res_set_const<value_type, allocator_type> get() const
		{
			if (this->blocks.size() != 1 || this->blocks[0].get_start() != 0)
				return std::unexpected(kernel_error::variable__var_content__get__content_not_contiguous);

			return std::cref(this->blocks[0].get_data());
		}
		size_type size() const
		{
			if (this->blocks.empty())
				return 0;

			const block& block_last = this->blocks.back();
			return block_last.get_start() + block_last.get_length();
		}
		bool empty() const
		{
			return this->blocks.empty();
		}
	public:
		auto get_blocks() const noexcept
		{
			return std::span<const block>(this->blocks);
		}
		bool restore_block(var_cont_data::var_pos pos, std::span<const value_type> data)
		{			
			if (data.empty() == true)
				return true;

			const size_type start = static_cast<size_type>(pos);
			const size_type length = data.size();

			if (start > std::numeric_limits<size_type>::max() - length)
				return false;

			auto it = std::lower_bound
			(
				this->blocks.begin(), 
				this->blocks.end(), 
				start,
				[](const block& block_v, size_type start_v) { return block_v.get_start() < start_v; }
			);

			bool merge_left = false;
			bool merge_right = false;

			if (it != this->blocks.begin())
			{
				const block& block_prev = *(it - 1);
				const size_type prev_end = block_prev.get_start() + block_prev.get_length();

				if (prev_end > start)
					return false;

				merge_left = (prev_end == start);
			}

			if (it != this->blocks.end())
			{
				if (start + length > it->get_start())
					return false;

				merge_right = (start + length == it->get_start());
			}

			try
			{
				if (merge_left == true)
				{
					block& block_prev = *(it - 1);
					block_prev.get_data().insert(block_prev.get_data().end(), data.begin(), data.end());

					if (merge_right == true)
					{
						block& block_next = *it;
						block_prev.get_data().insert(block_prev.get_data().end(), block_next.get_data().begin(), block_next.get_data().end());
						this->blocks.erase(it);
					}

					return true;
				}

				if (merge_right == true)
				{
					block& block_next = *it;
					block_next.get_data().insert(block_next.get_data().begin(), data.begin(), data.end());
					block_next.set_start(start);
					return true;
				}

				block block_new(start, this->allocator_);
				block_new.get_data().assign(data.begin(), data.end());
				this->blocks.insert(it, std::move(block_new));
				return true;
			}
			catch (...)
			{
				return false;
			}
		}
		std::span<value_type> restore_block_create(var_cont_data::var_pos pos, size_type length)
		{
			if (length == 0)
				return {};

			const size_type start = static_cast<size_type>(pos);

			if (start > std::numeric_limits<size_type>::max() - length)
				return {};

			auto it = std::lower_bound
			(
				this->blocks.begin(), this->blocks.end(), start,
				[](const block& block_v, size_type start_v) { return block_v.get_start() < start_v; }
			);

			if (it != this->blocks.begin())
			{
				const block& block_prev = *(it - 1);

				if (block_prev.get_start() + block_prev.get_length() > start)
					return {};
			}

			if (it != this->blocks.end() && start + length > it->get_start())
				return {};

			try
			{
				block block_new(start, this->allocator_);
				block_new.get_data().resize(length);
				auto it_new = this->blocks.insert(it, std::move(block_new));
				return std::span<value_type>(it_new->get_data());

			}
			catch (...)
			{
				return {};
			}
		}
	};
}

