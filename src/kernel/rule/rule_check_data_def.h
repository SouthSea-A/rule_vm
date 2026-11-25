#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <cstddef>
#include <functional>
#include "rule_data_def.h"

namespace rule_check_data_def
{
	struct ck_cmd_op_set
	{
	public:
		std::reference_wrapper<const rdata::rule_cmd_sgl> cmd_sgl_tgt;
		cmd_data::cmd_op_index op_index;
		cmd_data::cmd_op_type op_t;
		bool is_exc;
	public:
		template<typename self_this>
		decltype(auto) get_cmd_sgl_tgt(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.cmd_sgl_tgt);
		}
		template<typename self_this>
		decltype(auto) get_op_index(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.op_index);
		}
		template<typename self_this>
		decltype(auto) get_op_t(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.op_t);
		}
		template<typename self_this>
		decltype(auto) get_is_exc(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.is_exc);
		}
	public:
		void set_cmd_sgl_tgt(rdata::rule_cmd_sgl& cmd_sgl_tgt_v)
		{
			this->cmd_sgl_tgt = cmd_sgl_tgt_v;
		}
		void set_op_index(cmd_data::cmd_op_index op_index_v)
		{
			this->op_index = op_index_v;
		}
		void set_op_t(cmd_data::cmd_op_type op_t_v)
		{
			this->op_t = op_t_v;
		}
		void set_is_exc(bool is_exc_v)
		{
			this->is_exc = is_exc_v;
		}
	public:
		ck_cmd_op_set() = delete;
		ck_cmd_op_set(const rdata::rule_cmd_sgl& cmd_sgl_tgt_v, cmd_data::cmd_op_index op_index_v, cmd_data::cmd_op_type op_t_v, bool is_exc_v) :
			cmd_sgl_tgt(cmd_sgl_tgt_v), op_index(op_index_v), op_t(op_t_v), is_exc(is_exc_v)
		{

		}
	};
}
