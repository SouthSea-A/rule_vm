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
#include "../error/error_code.h"

namespace rule_rc_data_def
{
	using rc_data_type_base_8 = std::uint8_t;
	using rc_data_type_base_16 = std::uint16_t;
	using rc_data_type_base_32 = std::uint32_t;
	using rc_data_type_base_64 = std::uint64_t;

	using rc_index = rc_data_type_base_64;
	using rc_ref = rc_data_type_base_64;

	constexpr rc_index INVALID_RC_INDEX = std::numeric_limits<rc_index>::max();
	constexpr rc_ref INVALID_RC_REF = std::numeric_limits<rc_index>::max();

	template<typename T>
	struct rc_sgl
	{
	public:
		using value_type = T;
		using instance_type = value_type;
		using index_type = typename value_type::index_this_type;
	public:
		index_type index;
		rc_ref ref;
		T content;
	private:
		static T copy_content(const T& content_src)
		{
			try { return T(content_src); }
			catch (const kernel_error_throw&) { throw; }
			catch (...) { throw kernel_error_throw(kernel_error::resource__rc_sgl__rc_sgl__copy_unexpected, "kernel | resource | rc_sgl | rc_sgl | 资源内容拷贝失败，错误不可恢复"); }
		}
	public:
		template<typename this_self>
		decltype(auto) get_index(this this_self&& self) noexcept
		{
			return std::forward_like<this_self>(self.index);
		}
		template<typename this_self>
		decltype(auto) get_ref(this this_self&& self) noexcept
		{
			return std::forward_like<this_self>(self.ref);
		}
		template<typename this_self>
		decltype(auto) get_content(this this_self&& self) noexcept
		{
			return std::forward_like<this_self>(self.content);
		}
	public:
		void set_index(index_type index_v)
		{
			this->index = index_v;
		}
		void set_ref(rc_ref ref_v)
		{
			this->ref = ref_v;
		}
		void set_content(const T& content_v)
		{
			try { this->content = content_v; }
			catch (const kernel_error_throw&) { throw; }
			catch (...) { throw kernel_error_throw(kernel_error::resource__rc_sgl__set_content__copy_unexpected, "kernel | resource | rc_sgl | set_content | 资源内容拷贝失败，错误不可恢复"); }

			return;
		}
	public:
		rc_sgl() :
			index(INVALID_RC_INDEX), ref(0), content()
		{

		}
		rc_sgl(index_type index_v, rc_ref ref_v, const T& content_v) :
			index(index_v), ref(ref_v), content(copy_content(content_v))
		{

		}
	};

	using rc_res = std::expected<void, kernel_error>;
	template<typename T>
	using rc_res_index = std::expected<T, kernel_error>;
	template<typename T>
	using rc_res_inst = std::expected<T, kernel_error>;
	template<typename T>
	using rc_res_inst_ref = std::expected<std::reference_wrapper<T>, kernel_error>;
	template<typename T>
	using rc_res_inst_ref_const = std::expected<std::reference_wrapper<const T>, kernel_error>;
	template<typename T, typename allocator>
	using rc_res_inst_it = std::expected<typename mgr_ns::container_plus<T, allocator>::iterator, kernel_error>;
	template<typename T, typename allocator>
	using rc_res_inst_it_const = std::expected<typename mgr_ns::container_plus<T, allocator>::const_iterator, kernel_error>;
}
