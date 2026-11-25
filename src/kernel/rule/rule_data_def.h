#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <cstddef>
#include <utility>
#include <type_traits>
#include <functional>
#include <variant>
#include <optional>
#include <span>
#include <memory>
#include <vector>
#include <unordered_map>
#include "../variable/var.h"
#include "../command/cmd.h"
#include "../resource/rc.h"
#include "../actuator/act.h"

namespace rule_tool
{
	template<typename alloc_rule = std::allocator<int>, typename user_outer = std::monostate>
	class rule;

	template<typename alloc_rule_kit = std::allocator<int>, typename user_outer = std::monostate>
	class rule_kit;

	template<typename kit_t, typename user_outer_t>
	struct rule_kit_outer
	{
	public:
		using kit_type = kit_t;
		using user_type = user_outer_t;
	public:
		kit_t* kit = nullptr;
		user_outer_t* user = nullptr;
	public:
		template<typename self_this>
		decltype(auto) get_kit(this self_this&& self) noexcept
		{
			return *self.kit;
		}
		template<typename self_this>
		decltype(auto) get_user(this self_this&& self) noexcept
		{
			return *self.user;
		}
	public:
		rule_kit_outer() = default;
		rule_kit_outer(kit_t& kit_v, user_outer_t& user_v) :
			kit(&kit_v), user(&user_v)
		{

		}
	};
}


namespace rule_data_def
{
	using rule_data_type_base_8 = std::uint8_t;
	using rule_data_type_base_16 = std::uint16_t;
	using rule_data_type_base_32 = std::uint32_t;
	using rule_data_type_base_64 = std::uint64_t;

	using rule_var_base_t = cmd_data::cmd_var_base_type;

	using rule_cmd_sgl = cmd_data::cmd_single<false>;

	template<typename... t_case>
	struct t_case_distribute : public t_case... { using t_case::operator()...; };

	template<typename... t_case>
	t_case_distribute(t_case...) -> t_case_distribute<t_case...>;

	enum class rule_int_t : rule_data_type_base_64
	{
		none,

		check_cmd_t_no_match,
		check_cmd_op_size_no_match,
		check_cmd_op_t_no_match,

		trans_t_invalid,

		trans_raw_to_imm_bytes_size_no_match,
		trans_raw_to_var_bytes_size_no_match,
		trans_raw_to_cmd_bytes_size_no_match,
		trans_raw_to_cmd_op_bytes_size_no_match,

		trans_raw_to_imm_t_no_match,
		trans_raw_to_var_t_no_match,
		trans_raw_to_cmd_t_no_match,
		trans_raw_to_cmd_op_t_no_match,

		trans_imm_to_raw_bytes_size_no_match,
		trans_var_to_raw_bytes_size_no_match,
		trans_cmd_to_raw_bytes_size_no_match,
		trans_cmd_op_to_raw_bytes_size_no_match,

		alloc_var_mgr_error,
		alloc_cmd_mgr_error,
		alloc_var_mgr_span_error,
		alloc_cmd_mgr_span_error,
		alloc_t_no_match,

		dealloc_var_mgr_error,
		dealloc_cmd_mgr_error,
		dealloc_var_mgr_span_error,
		dealloc_cmd_mgr_span_error,
		dealloc_t_no_match,

		parse_run_param_error,

		parse_op_imm_t_no_match,
		parse_op_var_t_no_match,
		parse_op_cmd_t_no_match,

		parse_op_var_get_error,
		parse_op_var_mgr_get_error,
		parse_op_var_mgr_sgl_get_error,
		parse_op_var_mgr_sgl_span_get_error,

		parse_op_var_ptr_get_error,
		parse_op_var_mgr_ptr_get_error,
		parse_op_var_mgr_sgl_ptr_get_error,
		parse_op_var_mgr_sgl_span_ptr_get_error,

		parse_op_cmd_mgr_get_error,
		parse_op_cmd_mgr_sgl_get_error,
		parse_op_cmd_mgr_sgl_span_get_range_overflow,
		parse_op_cmd_mgr_sgl_span_get_error,

		parse_op_cmd_var_mgr_ptr_get_error,
		parse_op_cmd_var_mgr_sgl_ptr_get_error,
		parse_op_cmd_var_mgr_sgl_span_ptr_get_error,

		parse_op_cmd_ptr_get_error,
		parse_op_cmd_mgr_ptr_get_error,
		parse_op_cmd_mgr_sgl_ptr_get_error,
		parse_op_cmd_mgr_sgl_span_ptr_get_error,
		parse_op_cmd_mgr_sgl_span_ptr_get_range_overflow,

		write_get_span_error,
		write_tgt_src_t_no_match,
		write_src_t_no_match,
		write_tgt_t_no_match,
		write_span_tgt_size_no_match,
		write_span_src_size_no_match,

		kit_var_read_mgr_no_found,
		kit_var_read_mgr_index_no_found,
		kit_var_read_mgr_begin_width_error,

		kit_cmd_read_mgr_no_found,
		kit_cmd_read_mgr_index_no_found,
		kit_cmd_read_mgr_begin_width_error,
	};

	using rule_int_info_base_error_t = std::variant<std::monostate, kernel_error>;

	struct rule_int_info_cont_var
	{
	public:
		std::optional<var_cont_data::var_mgr_index> mgr_index;
		std::optional<var_cont_data::var_index> index;
		std::optional<var_cont_data::var_type> type;
		std::optional<var_cont_data::var_pos> pos;
		std::optional<var_cont_data::var_read_width> width;
	public:
		template<typename self_this>
		decltype(auto) get_mgr_index(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.mgr_index);
		}
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
		decltype(auto) get_pos(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.pos);
		}
		template<typename self_this>
		decltype(auto) get_width(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.width);
		}
	public:
		void set_mgr_index(const std::optional<var_cont_data::var_mgr_index>& mgr_index_v)
		{
			this->mgr_index = mgr_index_v;
		}
		void set_index(const std::optional<var_cont_data::var_index>& index_v)
		{
			this->index = index_v;
		}
		void set_type(const std::optional<var_cont_data::var_type>& type_v)
		{
			this->type = type_v;
		}
		void set_pos(const std::optional<var_cont_data::var_pos>& pos_v)
		{
			this->pos = pos_v;
		}
		void set_width(const std::optional<var_cont_data::var_read_width>& width_v)
		{
			this->width = width_v;
		}
	public:
		rule_int_info_cont_var() :
			mgr_index(std::nullopt), index(std::nullopt), type(std::nullopt), pos(std::nullopt), width(std::nullopt)
		{

		}
		rule_int_info_cont_var(const std::optional<var_cont_data::var_mgr_index>& mgr_index_v, const std::optional<var_cont_data::var_index>& index_v, const std::optional<var_cont_data::var_type>& type_v, const std::optional<var_cont_data::var_pos>& pos_v, const std::optional<var_cont_data::var_read_width>& width_v) :
			mgr_index(mgr_index_v), index(index_v), type(type_v), pos(pos_v), width(width_v)
		{

		}
	};

	struct rule_int_info_cont_cmd
	{
	public:
		std::optional<cmd_data::cmd_mgr_index> mgr_index;
		std::optional<cmd_data::cmd_index> index;
		std::optional<cmd_data::cmd_type> type;
		std::optional<cmd_data::cmd_op_index> op_index;
		std::optional<cmd_data::cmd_op_width> op_width;
	public:
		template<typename self_this>
		decltype(auto) get_mgr_index(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.mgr_index);
		}
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
		decltype(auto) get_op_index(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.op_index);
		}
		template<typename self_this>
		decltype(auto) get_op_width(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.op_width);
		}
	public:
		void set_mgr_index(const std::optional<cmd_data::cmd_mgr_index>& mgr_index_v)
		{
			this->mgr_index = mgr_index_v;
		}
		void set_index(const std::optional<cmd_data::cmd_index>& index_v)
		{
			this->index = index_v;
		}
		void set_type(const std::optional<cmd_data::cmd_type>& type_v)
		{
			this->type = type_v;
		}
		void set_op_index(const std::optional<cmd_data::cmd_op_index>& op_index_v)
		{
			this->op_index = op_index_v;
		}
		void set_op_width(const std::optional<cmd_data::cmd_op_width>& op_width_v)
		{
			this->op_width = op_width_v;
		}
	public:
		rule_int_info_cont_cmd() :
			mgr_index(std::nullopt), index(std::nullopt), type(std::nullopt), op_index(std::nullopt), op_width(std::nullopt)
		{

		}
		rule_int_info_cont_cmd(const std::optional<cmd_data::cmd_mgr_index>& mgr_index_v, const std::optional<cmd_data::cmd_index>& index_v, const std::optional<cmd_data::cmd_type>& type_v, const std::optional<cmd_data::cmd_op_index>& op_index_v, const std::optional<cmd_data::cmd_op_width>& op_width_v) :
			mgr_index(mgr_index_v), index(index_v), type(type_v), op_index(op_index_v), op_width(op_width_v)
		{

		}
	};

	struct rule_int_info_cont_act
	{
	public:
		std::optional<rule_cmd_sgl> cmd_int;
		std::optional<cmd_data::cmd_op_index> op_index_int;
		std::optional<cmd_data::cmd_op_single> op_int;
	public:
		template<typename self_this>
		decltype(auto) get_cmd_int(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.cmd_int);
		}
		template<typename self_this>
		decltype(auto) get_op_index_int(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.op_index_int);
		}
		template<typename self_this>
		decltype(auto) get_op_int(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.op_int);
		}
	public:
		void set_cmd_int(const std::optional<rule_cmd_sgl>& cmd_int_v)
		{
			this->cmd_int = cmd_int_v;
		}
		void set_op_index_int(const std::optional<cmd_data::cmd_op_index>& op_index_int_v)
		{
			this->op_index_int = op_index_int_v;
		}
		void set_op_int(const std::optional<cmd_data::cmd_op_single>& op_int_v)
		{
			this->op_int = op_int_v;
		}
	public:
		rule_int_info_cont_act() :
			cmd_int(std::nullopt), op_index_int(std::nullopt), op_int(std::nullopt)
		{

		}
		rule_int_info_cont_act(const std::optional<rule_cmd_sgl>& cmd_int_v, const std::optional<cmd_data::cmd_op_index>& op_index_int_v, const std::optional<cmd_data::cmd_op_single>& op_int_v) :
			cmd_int(cmd_int_v), op_index_int(op_index_int_v), op_int(op_int_v)
		{

		}
	};

	using rule_int_info_cont = std::variant<std::monostate, rule_int_info_cont_var, rule_int_info_cont_cmd, rule_int_info_cont_act>;

	struct rule_int_info
	{
	public:
		rule_int_t type_int;
		rule_int_info_base_error_t type_base_int;
		rule_int_info_cont cont_int;
	public:
		template<typename self_this>
		decltype(auto) get_type_int(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.type_int);
		}
		template<typename self_this>
		decltype(auto) get_type_base_int(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.type_base_int);
		}
		template<typename self_this>
		decltype(auto) get_cont_int(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.cont_int);
		}
	public:
		void set_type_int(rule_int_t type_int_v)
		{
			this->type_int = type_int_v;
		}
		void set_type_base_int(const rule_int_info_base_error_t& type_base_int_v)
		{
			this->type_base_int = type_base_int_v;
		}
		void set_cont_int(const rule_int_info_cont& cont_int_v)
		{
			this->cont_int = cont_int_v;
		}
	public:
		rule_int_info() :
			type_int(rule_int_t::none), type_base_int(), cont_int()
		{

		}
		rule_int_info(rule_int_t type_int_v, const rule_int_info_base_error_t& type_base_int_v, const rule_int_info_cont& cont_int_v) :
			type_int(type_int_v), type_base_int(type_base_int_v), cont_int(cont_int_v)
		{

		}
	};

	inline void set_int_info(rule_int_info& int_info_save, rule_int_t int_t_v, const rule_int_info_base_error_t& type_base_int_v, const rule_int_info_cont& cont_int_v)
	{
		int_info_save.set_type_int(int_t_v);
		int_info_save.set_type_base_int(type_base_int_v);
		int_info_save.set_cont_int(cont_int_v);
		return;
	}

	template<typename alloc_rule>
	using rule_alloc_traits = typename std::allocator_traits<alloc_rule>;

	template<typename alloc_rule>
	using rule_alloc_var = typename rule_alloc_traits<alloc_rule>::template rebind_alloc<rule_var_base_t>;

	template<typename alloc_rule>
	using rule_alloc_cmd = typename rule_alloc_traits<alloc_rule>::template rebind_alloc<cmd_data::cmd_op_single>;

	template<typename alloc_rule>
	using rule_var_mgr = var_mgr_ns::variable_manager<rule_var_base_t, rule_alloc_var<alloc_rule>>;

	template<typename alloc_rule>
	using rule_cmd_mgr = cmd_mgr_ns::command_manager<rule_alloc_cmd<alloc_rule>>;

	template<typename alloc_rule>
	using rule_var_act = rc_ns::rc_variable<rule_var_base_t, rule_alloc_var<alloc_rule>>;

	template<typename alloc_rule>
	using rule_cmd_act = rc_ns::rc_command<rule_alloc_cmd<alloc_rule>>;

	template<typename alloc_rule>
	using rule_run_ip = act_data::act_cmd_ip<rule_cmd_act<alloc_rule>>;

	template<typename alloc_rule>
	using rule_alloc_stk = typename rule_alloc_traits<alloc_rule>::template rebind_alloc<rule_run_ip<alloc_rule>>;

	template<typename alloc_rule>
	using rule_stk_act = std::vector<rule_run_ip<alloc_rule>, rule_alloc_stk<alloc_rule>>;

	template<typename alloc_rule, typename outer>
	using rule_cmd_act_param = act_data::act_run_param<rule_cmd_act<alloc_rule>, outer>;

	template<typename alloc_rule>
	using rule_cmd_act_res = act_data::act_run_int<rule_cmd_act<alloc_rule>, rule_int_info>;

	template<typename rc_cmd, typename outer>
	using rule_cmd_def = act_data::act_run_int<rc_cmd, rule_int_info> (*)(act_data::act_run_param<rc_cmd, outer>&);

	template<typename rc_cmd, typename outer, typename alloc_rule_cmd_def_set>
	using rule_cmd_def_set = std::unordered_map<cmd_data::cmd_type, rule_cmd_def<rc_cmd, outer>, std::hash<cmd_data::cmd_type>, std::equal_to<cmd_data::cmd_type>, alloc_rule_cmd_def_set>;
}
