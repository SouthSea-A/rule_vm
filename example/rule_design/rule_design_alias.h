#pragma once
#include "kernel/kernel_namespace.h"

#include "kernel/rule/rule.h"


namespace rule_design
{
	using rule_desg_cmd_op_type = rule_kit::rule_kit_cmd_handle_type;
	using rule_desg_cmd_mgr_index = rule_kit::rule_kit_cmd_mgr_index;
	using rule_desg_cmd_type = rule_kit::rule_kit_cmd_type;
	using rule_desg_cmd_index = rule_kit::rule_kit_cmd_index;
	using rule_desg_cmd_op_index = rule_kit::rule_kit_cmd_op_index;

	using rule_desg_var_op_type = rule_kit::rule_kit_var_handle_type;
	using rule_desg_var_mgr_index = rule_kit::rule_kit_var_mgr_index;
	using rule_desg_var_type = rule_kit::rule_kit_var_type;
	using rule_desg_var_index = rule_kit::rule_kit_var_index;
	using rule_desg_var_pos_index = rule_kit::rule_kit_var_pos_index;

	using rule_desg_var_base = rule_kit::rule_kit_var_base;

	using rule_desg_imm_op_type = rule_kit::rule_kit_imm_handle_type;

	using rule_desg_op_imm = rule_kit::rule_kit_imm_tgt;
	using rule_desg_op_var = rule_kit::rule_kit_var_tgt;
	using rule_desg_op_cmd = rule_kit::rule_kit_cmd_tgt;
	using rule_desg_op = rule_kit::rule_kit_cmd_op_sgl;
}
