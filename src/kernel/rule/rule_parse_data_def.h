#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <cstddef>
#include <utility>
#include <type_traits>
#include <functional>
#include <optional>
#include <span>
#include <variant>
#include "rule_data_def.h"

namespace rule_parse_data_def
{
	struct op_imm
	{
	public:
		std::reference_wrapper<cmd_data::cmd_op_single> op_ow;
		std::optional<std::span<rdata::rule_var_base_t>> imm_span;
	public:
		template<typename self_this>
		decltype(auto) get_op_ow(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.op_ow);
		}
		template<typename self_this>
		decltype(auto) get_imm_span(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.imm_span);
		}
	public:
		op_imm() = delete;
		op_imm(cmd_data::cmd_op_single& op_ow_v, const std::optional<std::span<rdata::rule_var_base_t>>& imm_span_v) :
			op_ow(op_ow_v), imm_span(imm_span_v)
		{

		}
	};

	template<typename alloc_rule>
	struct op_var_info
	{
	public:
		using var_mgr = rdata::rule_var_mgr<alloc_rule>;
	public:
		std::reference_wrapper<var_mgr> var_mgr_this;
		std::reference_wrapper<var_cont_data::var_type> var_type_tgt;
		std::span<rdata::rule_var_base_t> var_span;
	public:
		template<typename self_this>
		decltype(auto) get_var_mgr_this(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.var_mgr_this);
		}
		template<typename self_this>
		decltype(auto) get_var_type_tgt(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.var_type_tgt);
		}
		template<typename self_this>
		decltype(auto) get_var_span(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.var_span);
		}
	public:
		op_var_info() = delete;
		op_var_info(var_mgr& var_mgr_this_v, var_cont_data::var_type& var_type_tgt_v, const std::span<rdata::rule_var_base_t>& var_span_v) :
			var_mgr_this(var_mgr_this_v), var_type_tgt(var_type_tgt_v), var_span(var_span_v)
		{

		}
	};

	template<typename alloc_rule>
	struct op_var
	{
	public:
		std::reference_wrapper<cmd_data::cmd_op_single> op_ow;
		std::reference_wrapper<cmd_data::var_target> var_tgt;
		std::optional<op_var_info<alloc_rule>> var_info_val;
		std::optional<op_var_info<alloc_rule>> var_info_ptr;
	public:
		template<typename self_this>
		decltype(auto) get_op_ow(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.op_ow);
		}
		template<typename self_this>
		decltype(auto) get_var_tgt(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.var_tgt);
		}
		template<typename self_this>
		decltype(auto) get_var_info_val(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.var_info_val);
		}
		template<typename self_this>
		decltype(auto) get_var_info_ptr(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.var_info_ptr);
		}
	public:
		op_var() = delete;
		op_var(cmd_data::cmd_op_single& op_ow_v, const std::reference_wrapper<cmd_data::var_target>& var_tgt_v, const std::optional<op_var_info<alloc_rule>>& var_info_val_v, const std::optional<op_var_info<alloc_rule>>& var_info_ptr_val_v) :
			op_ow(op_ow_v), var_tgt(var_tgt_v), var_info_val(var_info_val_v), var_info_ptr(var_info_ptr_val_v)
		{

		}
	};

	template<typename alloc_rule>
	struct op_cmd_info
	{
	public:
		using cmd_mgr = rdata::rule_cmd_mgr<alloc_rule>;
	public:
		std::reference_wrapper<cmd_mgr> cmd_mgr_this;
		std::reference_wrapper<cmd_data::cmd_type> cmd_type_tgt;
		std::span<cmd_data::cmd_op_single> cmd_op_span;
	public:
		template<typename self_this>
		decltype(auto) get_cmd_mgr_this(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.cmd_mgr_this);
		}
		template<typename self_this>
		decltype(auto) get_cmd_type_tgt(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.cmd_type_tgt);
		}
		template<typename self_this>
		decltype(auto) get_cmd_op_span(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.cmd_op_span);
		}
	public:
		op_cmd_info() = delete;
		op_cmd_info(cmd_mgr& cmd_mgr_this_v, cmd_data::cmd_type& cmd_type_tgt_v, const std::span<cmd_data::cmd_op_single>& cmd_op_span_v) :
			cmd_mgr_this(cmd_mgr_this_v), cmd_type_tgt(cmd_type_tgt_v), cmd_op_span(cmd_op_span_v)
		{

		}
	};

	template<typename alloc_rule>
	struct op_cmd
	{
	public:
		std::reference_wrapper<cmd_data::cmd_op_single> op_ow;
		std::reference_wrapper<cmd_data::cmd_target> cmd_tgt;
		std::optional<op_cmd_info<alloc_rule>> cmd_info_val;
		std::optional<op_var_info<alloc_rule>> cmd_info_ptr;
	public:
		template<typename self_this>
		decltype(auto) get_op_ow(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.op_ow);
		}
		template<typename self_this>
		decltype(auto) get_cmd_tgt(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.cmd_tgt);
		}
		template<typename self_this>
		decltype(auto) get_cmd_info_val(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.cmd_info_val);
		}
		template<typename self_this>
		decltype(auto) get_cmd_info_ptr(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.cmd_info_ptr);
		}
	public:
		op_cmd() = delete;
		op_cmd(cmd_data::cmd_op_single& op_ow_v, const std::reference_wrapper<cmd_data::cmd_target>& cmd_tgt_v, const std::optional<op_cmd_info<alloc_rule>>& cmd_info_val_v, const std::optional<op_var_info<alloc_rule>>& cmd_info_ptr_v) :
			op_ow(op_ow_v), cmd_tgt(cmd_tgt_v), cmd_info_val(cmd_info_val_v), cmd_info_ptr(cmd_info_ptr_v)
		{

		}
	};

	template<typename alloc_rule>
	using op = std::variant<std::monostate, op_imm, op_var<alloc_rule>, op_cmd<alloc_rule>>;

	enum class op_t : rdata::rule_data_type_base_8
	{
		none,
		imm,
		var,
		cmd,
		both,
	};

	template<typename alloc_rule>
	struct op_handle_set
	{
	public:
		std::reference_wrapper<const rdata::rule_cmd_sgl> cmd_sgl_tgt;
		cmd_data::cmd_op_index op_tgt_index;
		std::reference_wrapper<op<alloc_rule>> parse_save;
		bool is_alloc_rc;
		bool is_incr_ref;
	public:
		template<typename self_this>
		decltype(auto) get_cmd_sgl_tgt(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.cmd_sgl_tgt);
		}
		template<typename self_this>
		decltype(auto) get_op_tgt_index(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.op_tgt_index);
		}
		template<typename self_this>
		decltype(auto) get_parse_save(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.parse_save);
		}
		template<typename self_this>
		decltype(auto) get_is_alloc_rc(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.is_alloc_rc);
		}
		template<typename self_this>
		decltype(auto) get_is_incr_ref(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.is_incr_ref);
		}
	public:
		void set_cmd_sgl_tgt(const rdata::rule_cmd_sgl& cmd_sgl_tgt_v)
		{
			this->cmd_sgl_tgt = cmd_sgl_tgt_v;
		}
		void set_op_tgt_index(cmd_data::cmd_op_index op_tgt_index_v)
		{
			this->op_tgt_index = op_tgt_index_v;
		}
		void set_parse_save(op<alloc_rule>& parse_save_v)
		{
			this->parse_save = parse_save_v;
		}
		void set_is_alloc_rc(bool is_alloc_rc_v)
		{
			this->is_alloc_rc = is_alloc_rc_v;
		}
		void set_is_incr_ref(bool is_incr_ref_v)
		{
			this->is_incr_ref = is_incr_ref_v;
		}
	public:
		op_handle_set() = delete;
		op_handle_set(const rdata::rule_cmd_sgl& cmd_sgl_tgt_v, cmd_data::cmd_op_index op_tgt_index_v, op<alloc_rule>& parse_save_v, bool is_alloc_rc_v, bool is_incr_ref_v) :
			cmd_sgl_tgt(cmd_sgl_tgt_v), op_tgt_index(op_tgt_index_v), parse_save(parse_save_v), is_alloc_rc(is_alloc_rc_v), is_incr_ref(is_incr_ref_v)
		{

		}
	};

	struct op_del
	{
	public:
		std::variant<std::monostate, std::reference_wrapper<cmd_data::cmd_target>, std::reference_wrapper<cmd_data::var_target>> tgt;
	public:
		template<typename self_this>
		decltype(auto) get_tgt(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.tgt);
		}
	public:
		void set_tgt(cmd_data::cmd_target& cmd_tgt_v)
		{
			this->tgt.template emplace<std::reference_wrapper<cmd_data::cmd_target>>(std::ref(cmd_tgt_v));
			return;
		}
		void set_tgt(cmd_data::var_target& var_tgt_v)
		{
			this->tgt.template emplace<std::reference_wrapper<cmd_data::var_target>>(std::ref(var_tgt_v));
			return;
		}
	public:
		op_del() :
			tgt(std::monostate())
		{

		}
		explicit op_del(cmd_data::cmd_target& cmd_tgt_v) :
			tgt(std::ref(cmd_tgt_v))
		{

		}
		explicit op_del(cmd_data::var_target& var_tgt_v) :
			tgt(std::ref(var_tgt_v))
		{

		}
	};

	using op_span = std::variant<std::monostate, std::span<rdata::rule_var_base_t>, std::span<cmd_data::var_cmd_type>, std::span<cmd_data::var_target>, std::span<cmd_data::cmd_target>, std::span<cmd_data::cmd_op_single>, op_del>;

	enum class op_span_t : rdata::rule_data_type_base_8
	{
		none,
		span_raw_bytes,
		span_var_cmd_t,
		span_var_tgt,
		span_cmd_tgt,
		span_cmd_op,
	};

	template<typename alloc_rule>
	struct op_span_set
	{
	public:
		std::reference_wrapper<op<alloc_rule>> parse_get_tgt;
		std::reference_wrapper<op_span> span_tgt_save;
	public:
		template<typename self_this>
		decltype(auto) get_parse_get_tgt(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.parse_get_tgt);
		}
		template<typename self_this>
		decltype(auto) get_span_tgt_save(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.span_tgt_save);
		}
	public:
		void set_parse_get_tgt(op<alloc_rule>& parse_get_tgt_v)
		{
			this->parse_get_tgt = parse_get_tgt_v;
		}
		void set_span_tgt_save(op_span& span_tgt_save_v)
		{
			this->span_tgt_save = span_tgt_save_v;
		}
	public:
		op_span_set() = delete;
		op_span_set(op<alloc_rule>& parse_get_tgt_v, op_span& span_tgt_save_v) :
			parse_get_tgt(parse_get_tgt_v), span_tgt_save(span_tgt_save_v)
		{

		}
	};
}
