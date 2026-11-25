#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

namespace rule_design
{
	enum class rule_design_error : std::uint64_t
	{
		none,

		text__file_open,
		text__file_read,
		text__utf8,
		text__syntax,
		text__block,
		text__var,
		text__cmd,
		text__label,
		text__start,
		text__var_load,
		text__cmd_load,

		exec__check_cmd,
		exec__parse_op,
		exec__write_op,
		exec__alloc,
		exec__dealloc,
		exec__jump_target,
		exec__unsupported_handle,
		exec__internal,

		internal__container,
		internal__kernel,
		internal__serialize,
		internal__parser,
		internal__exception,
	};

	inline std::string_view rule_design_error_text(rule_design_error error_v)
	{
		switch (error_v)
		{
		case rule_design_error::none:
		{
			return "none";
		}
		case rule_design_error::text__file_open:
		{
			return "text file open";
		}
		case rule_design_error::text__file_read:
		{
			return "text file read";
		}
		case rule_design_error::text__utf8:
		{
			return "text utf8";
		}
		case rule_design_error::text__syntax:
		{
			return "text syntax";
		}
		case rule_design_error::text__block:
		{
			return "text block";
		}
		case rule_design_error::text__var:
		{
			return "text var";
		}
		case rule_design_error::text__cmd:
		{
			return "text cmd";
		}
		case rule_design_error::text__label:
		{
			return "text label";
		}
		case rule_design_error::text__start:
		{
			return "text start";
		}
		case rule_design_error::text__var_load:
		{
			return "text var load";
		}
		case rule_design_error::text__cmd_load:
		{
			return "text cmd load";
		}
		case rule_design_error::exec__check_cmd:
		{
			return "exec check cmd";
		}
		case rule_design_error::exec__parse_op:
		{
			return "exec parse op";
		}
		case rule_design_error::exec__write_op:
		{
			return "exec write op";
		}
		case rule_design_error::exec__alloc:
		{
			return "exec alloc";
		}
		case rule_design_error::exec__dealloc:
		{
			return "exec dealloc";
		}
		case rule_design_error::exec__jump_target:
		{
			return "exec jump target";
		}
		case rule_design_error::exec__unsupported_handle:
		{
			return "exec unsupported handle";
		}
		case rule_design_error::exec__internal:
		{
			return "exec internal";
		}
		case rule_design_error::internal__container:
		{
			return "internal container";
		}
		case rule_design_error::internal__kernel:
		{
			return "internal kernel";
		}
		case rule_design_error::internal__serialize:
		{
			return "internal serialize";
		}
		case rule_design_error::internal__parser:
		{
			return "internal parser";
		}
		case rule_design_error::internal__exception:
		{
			return "internal exception";
		}
		default:
		{
			return "unknown";
		}
		}
	}

	template<typename alloc_t>
	struct rule_design_error_info
	{
	public:
		using alloc_traits = typename std::allocator_traits<alloc_t>;
		using char_alloc = typename alloc_traits::template rebind_alloc<char>;
		using string_t = std::basic_string<char, std::char_traits<char>, char_alloc>;
	public:
		rule_design_error code = rule_design_error::none;
		string_t message;
	public:
		explicit rule_design_error_info(const alloc_t& alloc_v = alloc_t()) :
			message(char_alloc(alloc_v))
		{
		}
		bool has_error() const noexcept
		{
			return this->code != rule_design_error::none;
		}
		void clear()
		{
			this->code = rule_design_error::none;
			this->message.clear();
			return;
		}
		void set(rule_design_error code_v, std::string_view message_v)
		{
			this->code = code_v;
			this->message.assign(message_v.begin(), message_v.end());
			return;
		}
	};

	struct rule_design_error_throw
	{
	public:
		rule_design_error error_code = rule_design_error::none;
		std::string error_msg;
	public:
		rule_design_error_throw() = delete;
		rule_design_error_throw(rule_design_error error_code_v, const std::string& error_msg_v) :
			error_code(error_code_v), error_msg(error_msg_v)
		{
		}
	};
}
