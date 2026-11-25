#pragma once
#include "serialize_data_def.h"

namespace rule_serialize_stack_data_def
{
	struct stack_record
	{
	private:
		std::uint64_t ownership = 0;
		std::uint64_t index = 0;
	public:
		template<typename self_this>
		decltype(auto) get_ownership(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.ownership);
		}
		template<typename self_this>
		decltype(auto) get_index(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.index);
		}
	public:
		stack_record() = default;
		stack_record(std::uint64_t ownership_v, std::uint64_t index_v) :
			ownership(ownership_v), index(index_v)
		{

		}
	};
}
