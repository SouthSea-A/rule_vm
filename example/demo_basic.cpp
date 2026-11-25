#include "demo_common.h"

int main()
{
	rule_vm_demo::desg::rule_desg<> vm;
	rule_vm_demo::desg::rule_desg_text_error<rule_vm_demo::alloc_t> error_v;

	constexpr std::string_view source_v = R"(
defvar 0 {
    var a 0:0:0:0:8 = 6
}

defcmd 0 {
    mov a, 42
    add a, a, 1
    ret
}

start defcmd 0
)";

	if (rule_vm_demo::load_text(vm, source_v, error_v) == false)
		return 1;

	vm.run_start();

	const std::optional<std::uint64_t> value_v = rule_vm_demo::read_qword(vm, 0, 0);
	const bool ok_v = value_v.has_value() && value_v.value() == 43;

	return rule_vm_demo::expect("basic_text_program", ok_v) ? 0 : 1;
}
