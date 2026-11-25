#pragma once

#include <cstdint>
#include "rule_design_alias.h"

namespace rule_design
{
	enum class rule_desg_compare_type : std::uint8_t
	{
		equal,
		greater,
		greater_equal,
		less,
		less_equal,
		unconditional,
		invalid,
	};

	enum class rule_desg_calc_type : std::uint8_t
	{
		unsigned_integer,
		signed_integer,
		floating_point,
		invalid,
	};

	enum class rule_desg_cmd_def_type : std::uint64_t
	{
		mov,
		cmp,

		jmp,
		call,
		ret,

		add,
		sub,
		mul,
		div,

		alloc,
		dealloc,
	};
}
