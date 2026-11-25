#pragma once
#include "rule_kit_data_def.h"
#include "rule_engine.h"
#include "rule_kit.h"

namespace rule_kit
{
	using rule_kit_error = ::kernel_error;
	using rule_kit_error_throw = ::kernel_error_throw;

	using rule_kit_cmd_handle_type = ::rule_tool::rule_kit_cmd_handle_type;
	using rule_kit_cmd_mgr_index = ::rule_tool::rule_kit_cmd_mgr_index;
	using rule_kit_cmd_type = ::rule_tool::rule_kit_cmd_type;
	using rule_kit_cmd_index = ::rule_tool::rule_kit_cmd_index;
	using rule_kit_cmd_op_index = ::rule_tool::rule_kit_cmd_op_index;

	using rule_kit_var_handle_type = ::rule_tool::rule_kit_var_handle_type;
	using rule_kit_var_mgr_index = ::rule_tool::rule_kit_var_mgr_index;
	using rule_kit_var_type = ::rule_tool::rule_kit_var_type;
	using rule_kit_var_index = ::rule_tool::rule_kit_var_index;
	using rule_kit_var_pos_index = ::rule_tool::rule_kit_var_pos_index;
	using rule_kit_var_read_width = ::rule_var_cont_data_def::var_read_width;

	using rule_kit_var_base = ::rule_tool::rule_kit_var_base;

	using rule_kit_imm_handle_type = ::rule_tool::rule_kit_imm_handle_type;
	using rule_kit_imm_tgt = ::rule_tool::rule_kit_imm_tgt;
	using rule_kit_var_tgt = ::rule_tool::rule_kit_var_tgt;
	using rule_kit_cmd_tgt = ::rule_tool::rule_kit_cmd_tgt;
	using rule_kit_cmd_op_sgl = ::rule_tool::rule_kit_cmd_op_sgl;

	using rule_kit_int_info = ::rule_tool::rule_kit_int_info;
	using rule_kit_int_t = ::rule_tool::rule_kit_int_t;
	using rule_kit_act_err_t = ::rule_act_data_def::act_err_t;
	using rule_kit_act_hook_t = ::rule_act_data_def::act_hook_t;
	using rule_kit_cmd_sgl = ::rule_tool::rule_kit_cmd_sgl;

	using rule_kit_var_read_info = ::rule_tool::rule_kit_var_read_info;
	using rule_kit_cmd_read_info = ::rule_tool::rule_kit_cmd_read_info;

	template<typename alloc_rule_kit>
	using rule_kit_cmd_ip = ::rule_tool::rule_kit_cmd_ip<alloc_rule_kit>;

	template<typename alloc_rule_kit, typename user_outer = std::monostate>
	using rule_kit_cmd_info = ::rule_tool::rule_kit_cmd_info<alloc_rule_kit, user_outer>;

	template<typename alloc_rule_kit, typename user_outer = std::monostate>
	using rule_kit_cmd_param = ::rule_tool::rule_kit_cmd_param<alloc_rule_kit, user_outer>;

	template<typename alloc_rule_kit>
	using rule_kit_cmd_res = ::rule_tool::rule_kit_cmd_res<alloc_rule_kit>;

	template<typename alloc_rule_kit, typename user_outer = std::monostate>
	using rule_kit_cmd_call = ::rule_tool::rule_kit_cmd_call<alloc_rule_kit, user_outer>;

	template<typename alloc_rule_kit, typename user_outer = std::monostate>
	using rule_kit_cmd_proxy_call = ::rule_tool::rule_kit_cmd_proxy_call<alloc_rule_kit, user_outer>;

	template<typename alloc_rule_kit, typename user_outer = std::monostate>
	using rule_kit_cmd_cb_int = ::rule_tool::rule_kit_cmd_cb_int<alloc_rule_kit, user_outer>;

	template<typename alloc_rule_kit, typename user_outer = std::monostate>
	using rule_kit_cmd_cb_rc_err = ::rule_tool::rule_kit_cmd_cb_rc_err<alloc_rule_kit, user_outer>;

	template<typename alloc_rule_kit, typename user_outer = std::monostate>
	using rule_kit_cmd_cb_err = ::rule_tool::rule_kit_cmd_cb_err<alloc_rule_kit, user_outer>;

	template<typename alloc_rule_kit, typename user_outer = std::monostate>
	using rule_kit_cmd_cb_hook = ::rule_tool::rule_kit_cmd_cb_hook<alloc_rule_kit, user_outer>;

	template<typename kit_t, typename user_outer_t>
	using rule_kit_outer = ::rule_tool::rule_kit_outer<kit_t, user_outer_t>;

	using rule_kit_cmd_var_base_type = ::rule_cmd_data_def::cmd_var_base_type;
	using rule_kit_var_cmd_mgr_index = ::rule_cmd_data_def::var_cmd_mgr_index;
	using rule_kit_var_cmd_index = ::rule_cmd_data_def::var_cmd_index;
	using rule_kit_var_cmd_type = ::rule_cmd_data_def::var_cmd_type;
	using rule_kit_var_cmd_begin_pos = ::rule_cmd_data_def::var_cmd_begin_pos;
	using rule_kit_cmd_op_width = ::rule_cmd_data_def::cmd_op_width;
	inline constexpr std::size_t rule_kit_imm_width_byte_max = ::rule_cmd_data_def::imm_width_byte_max;
	inline constexpr std::size_t rule_kit_raw_bytes_size_uint_8 = ::rule_cmd_data_def::RAW_BYTES_SIZE_UINT_8;
	inline constexpr std::size_t rule_kit_raw_bytes_size_var_cmd_type = ::rule_cmd_data_def::RAW_BYTES_SIZE_VAR_CMD_TYPE;
	inline constexpr std::size_t rule_kit_raw_bytes_size_var_target = ::rule_cmd_data_def::RAW_BYTES_SIZE_VAR_TARGET;
	inline constexpr std::size_t rule_kit_raw_bytes_size_cmd_target = ::rule_cmd_data_def::RAW_BYTES_SIZE_CMD_TARGET;
	inline constexpr std::size_t rule_kit_raw_bytes_size_cmd_op = ::rule_cmd_data_def::RAW_BYTES_SIZE_CMD_OP;
	using rule_kit_cmd_cb_hook_t = ::rule_tool::rule_kit_cmd_cb_hook_t;

	template<typename alloc_rule_kit, typename user_outer = std::monostate>
	using rule_kit_context = ::rule_tool::rule_kit<alloc_rule_kit, user_outer>;

	template<typename alloc_rule_kit, typename user_outer = std::monostate>
	using rule_kit_rule = ::rule_tool::rule<alloc_rule_kit, user_outer>;
}
