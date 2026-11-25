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
    mov c(0:3:0:0:2), c(0:2:0:0:2)
    jmp c(0:3:0:0:0)
    mov a, 7
    mov a, 99
    ret
}

start defcmd 0
)";

	if (rule_vm_demo::load_text(vm, source_v, error_v) == false)
		return 1;

	vm.run_start();

	const std::optional<std::uint64_t> value_v = rule_vm_demo::read_qword(vm, 0, 0);
	const bool ok_v = value_v.has_value() && value_v.value() == 7;

	return rule_vm_demo::expect("self_modify_command", ok_v) ? 0 : 1;
}
