#include "demo_common.h"

int main()
{
	rule_vm_demo::desg::rule_desg<> vm;
	rule_vm_demo::desg::rule_desg_text_error<rule_vm_demo::alloc_t> error_v;

	constexpr std::string_view source_v = R"(
defvar 0 {
    var a 0:0:0:0:8
}

defcmd 0 {
    mov c(0:10:0:0:2), c(0:2:0:0:2)
    jmp c(0:10:0:0:0)
    mov a, 5
    ret
}

start defcmd 0
)";

	if (rule_vm_demo::load_text(vm, source_v, error_v) == false)
		return 1;

	vm.run_start();

	const std::optional<std::uint64_t> value_v = rule_vm_demo::read_qword(vm, 0, 0);
	auto cmd_mgr_res = vm.get_owner().get_owner().get_cmd_set().get(0);
	const bool generated_v = cmd_mgr_res.has_value() &&
		cmd_mgr_res.value().get().get_content().get(10).has_value();
	const bool ok_v = generated_v && value_v.has_value() && value_v.value() == 5;

	return rule_vm_demo::expect("generated_command", ok_v) ? 0 : 1;
}
