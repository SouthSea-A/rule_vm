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
#include "var_cont_data_def.h"
#include "var_cont.h"

namespace rule_var_data_def
{
	template<typename T, typename var_sgl_allocator = std::allocator<T>>
	struct var_sgl
	{
	public:
		var_cont_data::var_index index = var_cont_data::INVALID_VAR_INDEX;
		var_cont_data::var_type type = 0;
		var_cont_data::var_ref ref = 0;
		vcont::var_content<T, var_sgl_allocator> content;
	public:
		template<typename self_this>
		decltype(auto) get_index(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.index);
		}
		template<typename self_this>
		decltype(auto) get_type(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.type);
		}
		template<typename self_this>
		decltype(auto) get_ref(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.ref);
		}
		template<typename self_this>
		decltype(auto) get_content(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.content);
		}
	public:
		void set_index(var_cont_data::var_index index_val)
		{
			this->index = index_val;
		}
		void set_type(var_cont_data::var_type type_val)
		{
			this->type = type_val;
		}
		void set_ref(var_cont_data::var_ref ref_val)
		{
			this->ref = ref_val;
		}
		void set_content(vcont::var_content<T, var_sgl_allocator>& content_val)
		{
			try { this->content = content_val; }
			catch (const kernel_error_throw&) { throw; }
			catch (...) { throw kernel_error_throw(kernel_error::variable__var_sgl__set_content__copy_unexpected, "kernel | variable | var_sgl | set_content | 内容拷贝失败，错误不可恢复"); }

			return;
		}
	};

	using var_mgr_res = std::expected<void, kernel_error>;
	template<typename size_type>
	using var_mgr_res_size = std::expected<size_type, kernel_error>;
	template<typename T, typename var_allocator>
	using var_mgr_res_var_sgl = std::expected<std::reference_wrapper<var_sgl<T, var_allocator>>, kernel_error>;
	template<typename T, typename var_allocator>
	using var_mgr_res_var_sgl_const = std::expected<std::reference_wrapper<const var_sgl<T, var_allocator>>, kernel_error>;
	template<typename T>
	using var_mgr_res_var_sgl_it = std::expected<T, kernel_error>;
	template<typename T>
	using var_mgr_res_span = std::expected<std::span<T>, kernel_error>;

}