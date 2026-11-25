#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <cstddef>
#include <utility>
#include <type_traits>
#include <functional>
#include "rule_parse_data_def.h"

namespace rule_write_data_def
{
	template<typename alloc_rule>
	struct write_op_set
	{
	public:
		std::reference_wrapper<parse_data::op<alloc_rule>> write_tgt;
		std::reference_wrapper<parse_data::op<alloc_rule>> write_src;
	public:
		template<typename self_this>
		decltype(auto) get_write_tgt(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.write_tgt);
		}
		template<typename self_this>
		decltype(auto) get_write_src(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.write_src);
		}
	public:
		void set_write_tgt(parse_data::op<alloc_rule>& write_tgt_v)
		{
			this->write_tgt = write_tgt_v;
		}
		void set_write_src(parse_data::op<alloc_rule>& write_src_v)
		{
			this->write_src = write_src_v;
		}
	public:
		write_op_set() = delete;
		write_op_set(parse_data::op<alloc_rule>& write_tgt_v, parse_data::op<alloc_rule>& write_src_v) :
			write_tgt(write_tgt_v), write_src(write_src_v)
		{

		}
	};
}
