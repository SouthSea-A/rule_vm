#pragma once
#include "serialize_data_def.h"

namespace rule_serialize_cmd_data_def
{
	struct cmd_record
	{
	private:
		std::uint64_t index = 0;
		std::uint64_t type = 0;
		std::uint64_t ref = 0;
		std::uint64_t op_count = 0;
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
		decltype(auto) get_op_count(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.op_count);
		}
	public:
		void set_index(std::uint64_t index_v) noexcept
		{
			this->index = index_v;
			return;
		}
		void set_type(std::uint64_t type_v) noexcept
		{
			this->type = type_v;
			return;
		}
		void set_ref(std::uint64_t ref_v) noexcept
		{
			this->ref = ref_v;
			return;
		}
		void set_op_count(std::uint64_t op_count_v) noexcept
		{
			this->op_count = op_count_v;
			return;
		}
	public:
		cmd_record() = default;
	};
}
