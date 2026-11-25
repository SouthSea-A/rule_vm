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
#include <random>
#include <limits>
#include <algorithm>
#include "cache_prefix_mapping_data_def.h"

namespace rule_cache_prefix_mapping
{
	template <typename T, typename U, typename allocator_cache = std::allocator<T>>
	class cache_prefix_mapping
	{
		static_assert(std::is_integral_v<T>, "索引类型无效");
		static_assert(std::is_integral_v<U>, "容量类型无效");
	public:
		using sum_type = std::common_type_t<U, std::int64_t>;
		using size_type = std::uint64_t;
		using sum_result = cpmap_data::cache_prefix_mapping_result_value<sum_type>;
		using locate_result = cpmap_data::cache_prefix_mapping_result_value<std::pair<T, sum_type>>;
	private:
		struct cache_prefix_mapping_node
		{
			T key;
			U value;
			sum_type sum;
			size_type size;
			std::uint32_t priority;
			cache_prefix_mapping_node* left;
			cache_prefix_mapping_node* right;
			cache_prefix_mapping_node(T key_val, U value_val, std::uint32_t priority_val) :
				key(key_val),
				value(value_val),
				sum(static_cast<sum_type>(value_val)),
				size(1),
				priority(priority_val),
				left(nullptr),
				right(nullptr)
			{

			}
		};

		using allocator_traits = typename std::allocator_traits<allocator_cache>;
		using allocator_node = allocator_traits::template rebind_alloc<cache_prefix_mapping_node>;
		using allocator_rng_mt19937 = allocator_traits::template rebind_alloc<std::mt19937>;
		using allocator_node_traits = std::allocator_traits<allocator_node>;
		using allocator_rng_mt19937_traits = std::allocator_traits<allocator_rng_mt19937>;
	private:
		allocator_node allocator_cache_node;
		allocator_rng_mt19937 allocator_cache_mt19937;
		cache_prefix_mapping_node* root_;
		std::mt19937* rng_;
	private:
		constexpr static sum_type get_sum(const cache_prefix_mapping_node* node) noexcept
		{
			return node == nullptr ? static_cast<sum_type>(0) : node->sum;
		}
		constexpr static size_type get_size(const cache_prefix_mapping_node* node) noexcept
		{
			return node == nullptr ? static_cast<size_type>(0) : node->size;
		}
		constexpr static void pull(cache_prefix_mapping_node* node) noexcept
		{
			if (node == nullptr)
				return;

			node->size = get_size(node->left) + 1 + get_size(node->right);
			node->sum = get_sum(node->left) + static_cast<sum_type>(node->value) + get_sum(node->right);
			return;
		}
		[[nodiscard]] static constexpr bool test_validity_value(const U& value) noexcept
		{
			if constexpr (std::is_signed_v<U>)
				return value >= 0;

			return true;
		}
		[[nodiscard]] static constexpr bool test_validity_index(const sum_type& index) noexcept
		{
			if constexpr (std::is_signed_v<sum_type>)
				return index >= 0;

			return true;
		}
		constexpr static cache_prefix_mapping_node* merge(cache_prefix_mapping_node* left, cache_prefix_mapping_node* right) noexcept
		{
			if (left == nullptr)
				return right;

			if (right == nullptr)
				return left;


			if (left->priority > right->priority)
			{
				left->right = merge(left->right, right);
				pull(left);
				return left;
			}

			right->left = merge(left, right->left);
			pull(right);
			return right;
		}
		constexpr static void split(cache_prefix_mapping_node* node, size_type left_count, cache_prefix_mapping_node*& left, cache_prefix_mapping_node*& right) noexcept
		{
			if (node == nullptr)
			{
				left = nullptr;
				right = nullptr;
				return;
			}

			const size_type left_size = get_size(node->left);

			if (left_count <= left_size)
			{
				split(node->left, left_count, left, node->left);
				right = node;
				pull(right);
			}
			else
			{
				split(node->right, left_count - left_size - 1, node->right, right);
				left = node;
				pull(left);
			}

			return;
		}
		static void destroy(allocator_node* allocator, cache_prefix_mapping_node* node) noexcept
		{
			if (node == nullptr)
				return;

			destroy(allocator, node->left);
			destroy(allocator, node->right);

			std::destroy_at(node);
			allocator_node_traits::deallocate(*allocator, node, 1);

			return;
		}
		constexpr static cache_prefix_mapping_node* kth_node(cache_prefix_mapping_node* node, size_type pos) noexcept
		{
			while (node != nullptr)
			{
				const size_type left_size = get_size(node->left);

				if (pos < left_size)
				{
					node = node->left;
				}
				else if (pos == left_size)
				{
					return node;
				}
				else
				{
					pos -= left_size + 1;
					node = node->right;
				}
			}

			return nullptr;
		}
		constexpr static const cache_prefix_mapping_node* kth_node(const cache_prefix_mapping_node* node, size_type pos) noexcept
		{
			while (node != nullptr)
			{
				const size_type left_size = get_size(node->left);

				if (pos < left_size)
				{
					node = node->left;
				}
				else if (pos == left_size)
				{
					return node;
				}
				else
				{
					pos -= left_size + 1;
					node = node->right;
				}
			}

			return nullptr;
		}
		constexpr static bool update_node(cache_prefix_mapping_node* node, size_type pos, const U& value) noexcept
		{
			if (node == nullptr)
				return false;

			const size_type left_size = get_size(node->left);
			bool ok = false;

			if (pos < left_size)
			{
				ok = update_node(node->left, pos, value);
			}
			else if (pos == left_size)
			{
				node->value = value;
				ok = true;
			}
			else
			{
				ok = update_node(node->right, pos - left_size - 1, value);
			}

			pull(node);
			return ok;
		}
		static cache_prefix_mapping_node* clone_tree(allocator_node* allocator, const cache_prefix_mapping_node* node)
		{
			if (node == nullptr)
				return nullptr;

			cache_prefix_mapping_node* new_node = allocator_node_traits::allocate(*allocator, 1);

			try
			{
				std::construct_at(new_node, node->key, node->value, node->priority);

				new_node->sum = node->sum;
				new_node->size = node->size;

				new_node->left = clone_tree(allocator, node->left);
				new_node->right = clone_tree(allocator, node->right);
			}
			catch (...)
			{
				destroy(allocator, new_node);
				throw;
			}

			return new_node;
		}
		cpmap_data::cache_prefix_mapping_result init_rng()
		{
			if (this->rng_ != nullptr)
				return {};

			std::mt19937* rng_new = nullptr;

			try { rng_new = allocator_rng_mt19937_traits::allocate(this->allocator_cache_mt19937, 1); }
			catch (...) { return std::unexpected(kernel_error::manager__cache_prefix_mapping__init_rng__allocate_unexpected); }

			try { std::construct_at(rng_new, std::random_device{}()); }
			catch (...)
			{
				allocator_rng_mt19937_traits::deallocate(this->allocator_cache_mt19937, rng_new, 1);
				return std::unexpected(kernel_error::manager__cache_prefix_mapping__init_rng__construct_unexpected);
			}

			this->rng_ = rng_new;
			return {};
		}
	public:
		cache_prefix_mapping() :
			allocator_cache_node(), allocator_cache_mt19937(), root_(nullptr), rng_(nullptr)
		{

		}
		explicit cache_prefix_mapping(const allocator_cache& allocator_val) :
			allocator_cache_node(allocator_node(allocator_val)),
			allocator_cache_mt19937(allocator_rng_mt19937(allocator_val)),
			root_(nullptr),
			rng_(nullptr)
		{

		}
		cache_prefix_mapping(const cache_prefix_mapping& other_val) :
			allocator_cache_node(allocator_node_traits::select_on_container_copy_construction(other_val.allocator_cache_node)),
			allocator_cache_mt19937(allocator_rng_mt19937_traits::select_on_container_copy_construction(other_val.allocator_cache_mt19937)),
			root_(nullptr),
			rng_(nullptr)
		{
			cache_prefix_mapping_node* new_root = nullptr;
			std::mt19937* new_rng = nullptr;

			try
			{
				new_root = clone_tree(&this->allocator_cache_node, other_val.root_);

				if (other_val.rng_ != nullptr)
				{
					new_rng = allocator_rng_mt19937_traits::allocate(this->allocator_cache_mt19937, 1);

					std::construct_at(new_rng, *other_val.rng_);
				}
			}
			catch (...)
			{
				destroy(&this->allocator_cache_node, new_root);

				if (new_rng != nullptr)
				{
					std::destroy_at(new_rng);

					allocator_rng_mt19937_traits::deallocate(this->allocator_cache_mt19937, new_rng, 1);
				}

				throw kernel_error_throw(kernel_error::manager__cache_prefix_mapping__copy_construct__clone_unexpected, "kernel | manager | cache_prefix_mapping | copy_construct | 树节点克隆失败，错误不可恢复");
			}

			this->root_ = new_root;
			this->rng_ = new_rng;
		}
		cache_prefix_mapping& operator=(const cache_prefix_mapping& other_val)
		{
			if (this == &other_val)
				return *this;

			destroy(&this->allocator_cache_node, this->root_);

			if (this->rng_ != nullptr)
			{
				std::destroy_at(this->rng_);

				allocator_rng_mt19937_traits::deallocate(this->allocator_cache_mt19937, this->rng_, 1);
			}

			if constexpr (allocator_rng_mt19937_traits::propagate_on_container_copy_assignment::value == true)
				this->allocator_cache_mt19937 = other_val.allocator_cache_mt19937;

			if constexpr (allocator_node_traits::propagate_on_container_copy_assignment::value == true)
				this->allocator_cache_node = other_val.allocator_cache_node;

			cache_prefix_mapping_node* new_root = nullptr;
			std::mt19937* new_rng = nullptr;

			try
			{
				new_root = clone_tree(&this->allocator_cache_node, other_val.root_);

				if (other_val.rng_ != nullptr)
				{
					new_rng = allocator_rng_mt19937_traits::allocate(this->allocator_cache_mt19937, 1);

					std::construct_at(new_rng, *other_val.rng_);
				}
			}
			catch (...)
			{
				destroy(&this->allocator_cache_node, new_root);

				if (new_rng != nullptr)
				{
					std::destroy_at(new_rng);

					allocator_rng_mt19937_traits::deallocate(this->allocator_cache_mt19937, new_rng, 1);
				}

				throw kernel_error_throw(kernel_error::manager__cache_prefix_mapping__copy_assign__clone_unexpected, "kernel | manager | cache_prefix_mapping | copy_assign | 树节点克隆失败，错误不可恢复");
			}

			this->root_ = new_root;
			this->rng_ = new_rng;

			return *this;
		}
		cache_prefix_mapping(cache_prefix_mapping&& other_val) noexcept :
			allocator_cache_node(std::move(other_val.allocator_cache_node)),
			allocator_cache_mt19937(std::move(other_val.allocator_cache_mt19937)),
			root_(other_val.root_),
			rng_(other_val.rng_)
		{
			other_val.root_ = nullptr;
			other_val.rng_ = nullptr;
		}
		cache_prefix_mapping& operator=(cache_prefix_mapping&& other_val)
		{
			if (this == &other_val)
				return *this;

			destroy(&this->allocator_cache_node, this->root_);
			this->root_ = nullptr;

			if (this->rng_ != nullptr)
			{
				std::destroy_at(this->rng_);
				allocator_rng_mt19937_traits::deallocate(this->allocator_cache_mt19937, this->rng_, 1);

				this->rng_ = nullptr;
			}

			if constexpr (allocator_node_traits::propagate_on_container_move_assignment::value == true)
			{
				this->allocator_cache_node = std::move(other_val.allocator_cache_node);
				this->root_ = std::exchange(other_val.root_, nullptr);
			}
			else
			{
				if (this->allocator_cache_node == other_val.allocator_cache_node)
				{
					this->root_ = std::exchange(other_val.root_, nullptr);
				}
				else
				{
					cache_prefix_mapping_node* new_root = nullptr;

					try
					{
						new_root = clone_tree(&this->allocator_cache_node, other_val.root_);
					}
					catch (...)
					{
						destroy(&this->allocator_cache_node, new_root);
						throw kernel_error_throw(kernel_error::manager__cache_prefix_mapping__move_assign__clone_unexpected, "kernel | manager | cache_prefix_mapping | move_assign | 树节点克隆失败，错误不可恢复");
					}

					this->root_ = new_root;

					destroy(&other_val.allocator_cache_node, other_val.root_);
					other_val.root_ = nullptr;
				}
			}

			if constexpr (allocator_rng_mt19937_traits::propagate_on_container_move_assignment::value == true)
			{
				this->allocator_cache_mt19937 = std::move(other_val.allocator_cache_mt19937);
				this->rng_ = std::exchange(other_val.rng_, nullptr);
			}
			else
			{
				if (this->allocator_cache_mt19937 == other_val.allocator_cache_mt19937)
				{
					this->rng_ = std::exchange(other_val.rng_, nullptr);
				}
				else
				{
					std::mt19937* new_rng = nullptr;

					try
					{
						if (other_val.rng_ != nullptr)
						{
							new_rng = allocator_rng_mt19937_traits::allocate(this->allocator_cache_mt19937, 1);
							std::construct_at(new_rng, *other_val.rng_);
						}
					}
					catch (...)
					{
						if (new_rng != nullptr)
						{
							std::destroy_at(new_rng);
							allocator_rng_mt19937_traits::deallocate(this->allocator_cache_mt19937, new_rng, 1);
						}

						throw kernel_error_throw(kernel_error::manager__cache_prefix_mapping__move_assign__clone_unexpected, "kernel | manager | cache_prefix_mapping | move_assign | 随机数发生器拷贝失败，错误不可恢复");
					}

					this->rng_ = new_rng;

					if (other_val.rng_ != nullptr)
					{
						std::destroy_at(other_val.rng_);
						allocator_rng_mt19937_traits::deallocate(&other_val.allocator_cache_mt19937, other_val.rng_, 1);

						other_val.rng_ = nullptr;
					}
				}
			}

			return *this;
		}
		~cache_prefix_mapping() noexcept
		{
			destroy(&this->allocator_cache_node, this->root_);

			if (this->rng_ != nullptr)
			{
				std::destroy_at(this->rng_);
				allocator_rng_mt19937_traits::deallocate(this->allocator_cache_mt19937, this->rng_, 1);
			}
		}
	public:
		constexpr bool empty() const noexcept
		{
			return this->root_ == nullptr;
		}
		constexpr size_type size() const noexcept
		{
			return get_size(this->root_);
		}
		constexpr sum_type total_sum() const noexcept
		{
			return get_sum(this->root_);
		}
		void clear() noexcept
		{
			destroy(&this->allocator_cache_node, this->root_);
			this->root_ = nullptr;
			return;
		}
		cpmap_data::cache_prefix_mapping_result push_back(const T& key, const U& value)
		{
			return insert(size(), key, value);
		}
		cpmap_data::cache_prefix_mapping_result insert(size_type pos, const T& key, const U& value)
		{
			if (test_validity_value(value) == false)
				return std::unexpected(kernel_error::manager__cache_prefix_mapping__insert__value_invalid);

			if (pos > size())
				return std::unexpected(kernel_error::manager__cache_prefix_mapping__insert__pos_out_of_range);

			cpmap_data::cache_prefix_mapping_result rng_res = init_rng();

			if (rng_res.has_value() == false)
				return rng_res;

			cache_prefix_mapping_node* left = nullptr;
			cache_prefix_mapping_node* right = nullptr;
			split(this->root_, pos, left, right);

			cache_prefix_mapping_node* mid = nullptr;

			try { mid = allocator_node_traits::allocate(this->allocator_cache_node, 1); }
			catch (...)
			{
				this->root_ = merge(left, right);
				return std::unexpected(kernel_error::manager__cache_prefix_mapping__insert__node_allocate_unexpected);
			}

			try { std::construct_at(mid, key, value, (*this->rng_)()); }
			catch (...)
			{
				allocator_node_traits::deallocate(this->allocator_cache_node, mid, 1);
				this->root_ = merge(left, right);
				return std::unexpected(kernel_error::manager__cache_prefix_mapping__insert__node_construct_unexpected);
			}

			this->root_ = merge(merge(left, mid), right);

			return {};
		}
		bool erase(size_type pos) noexcept
		{
			if (pos >= size())
				return false;

			cache_prefix_mapping_node* left = nullptr;
			cache_prefix_mapping_node* mid_right = nullptr;
			cache_prefix_mapping_node* mid = nullptr;
			cache_prefix_mapping_node* right = nullptr;

			split(this->root_, pos, left, mid_right);
			split(mid_right, 1, mid, right);

			destroy(&this->allocator_cache_node, mid);
			this->root_ = merge(left, right);
			return true;
		}
		cpmap_data::cache_prefix_mapping_result update(size_type pos, const U& value)
		{
			if (test_validity_value(value) == false)
				return std::unexpected(kernel_error::manager__cache_prefix_mapping__update__value_invalid);

			if (pos >= size())
				return std::unexpected(kernel_error::manager__cache_prefix_mapping__update__pos_out_of_range);

			if (update_node(this->root_, pos, value) == false)
				return std::unexpected(kernel_error::manager__cache_prefix_mapping__update__node_missing);

			return {};
		}
		constexpr bool get(size_type pos, T& out_key, U& out_value) const noexcept
		{
			const cache_prefix_mapping_node* node = kth_node(this->root_, pos);

			if (node == nullptr)
				return false;

			out_key = node->key;
			out_value = node->value;
			return true;
		}
		sum_result prefix_before_pos(size_type pos) const
		{
			if (pos > size())
				return std::unexpected(kernel_error::manager__cache_prefix_mapping__prefix_before_pos__pos_out_of_range);

			const cache_prefix_mapping_node* node = this->root_;
			sum_type result = 0;

			while (node != nullptr)
			{
				const size_type left_size = get_size(node->left);

				if (pos < left_size)
				{
					node = node->left;
				}
				else if (pos == left_size)
				{
					result += get_sum(node->left);
					return result;
				}
				else
				{
					result += get_sum(node->left) + static_cast<sum_type>(node->value);
					pos -= left_size + 1;
					node = node->right;
				}
			}

			return result;
		}
		locate_result locate_by_index(sum_type index, size_type& pos) const
		{
			if (test_validity_index(index) == false)
				return std::unexpected(kernel_error::manager__cache_prefix_mapping__locate_by_index__index_negative);

			if (index >= total_sum())
				return std::unexpected(kernel_error::manager__cache_prefix_mapping__locate_by_index__index_out_of_range);

			const cache_prefix_mapping_node* node = this->root_;
			sum_type prefix = 0;
			size_type skipped_size = 0;

			while (node != nullptr)
			{
				const sum_type left_sum = get_sum(node->left);
				const size_type left_size = get_size(node->left);

				const sum_type current_begin = prefix + left_sum;
				const sum_type current_end = current_begin + static_cast<sum_type>(node->value);

				if (index < current_begin)
				{
					node = node->left;
				}
				else if (index < current_end)
				{
					pos = skipped_size + left_size;
					return std::make_pair(node->key, current_begin);
				}
				else
				{
					prefix = current_end;
					skipped_size += left_size + 1;
					node = node->right;
				}
			}

			return std::unexpected(kernel_error::manager__cache_prefix_mapping__locate_by_index__node_missing);
		}
		locate_result locate_by_index(sum_type index) const
		{
			size_type ignored_pos = 0;
			return locate_by_index(index, ignored_pos);
		}
		void swap(cache_prefix_mapping& other)
		{
			if (this == &other)
				return;

			std::swap(this->allocator_cache_node, other.allocator_cache_node);
			std::swap(this->allocator_cache_mt19937, other.allocator_cache_mt19937);
			std::swap(this->root_, other.root_);
			std::swap(this->rng_, other.rng_);

			return;
		}
	};
}
