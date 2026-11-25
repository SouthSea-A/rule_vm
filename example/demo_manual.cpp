#include <array>
#include <span>

#include "demo_common.h"

int main()
{
	rule_vm_demo::desg::rule_desg<> vm;

	std::array<rule_vm_demo::desg::rule_desg_var_base, 8> data{};
	const bool var_ok = vm.add_set_var(
		0, 0, 0, 0,
		std::span<rule_vm_demo::desg::rule_desg_var_base>(data));

	const std::array<
		rule_vm_demo::desg::rule_desg_var_base,
		rule_cmd_data_def::imm_width_byte_max> imm_data = rule_vm_demo::make_qword(42);

	const bool cmd_ok = vm.add_set_cmd(
		0, 0,
		rule_vm_demo::desg::rule_desg_cmd_def_type::mov,
		0,
		{
			rule_vm_demo::desg::rule_desg_op(
				rule_vm_demo::desg::rule_desg_op_var(
					rule_vm_demo::desg::rule_desg_var_op_type::var_val,
					0, 0, 0, 0, 8)),
			rule_vm_demo::desg::rule_desg_op(
				rule_vm_demo::desg::rule_desg_op_imm(
					rule_vm_demo::desg::rule_desg_imm_op_type::qword,
					imm_data)),
		});

	if (var_ok == false || cmd_ok == false)
		return 1;

	vm.run(0, 0);

	const std::optional<std::uint64_t> value_v = rule_vm_demo::read_qword(vm, 0, 0);
	const bool ok_v = value_v.has_value() && value_v.value() == 42;

	return rule_vm_demo::expect("manual_assembly_program", ok_v) ? 0 : 1;
}
