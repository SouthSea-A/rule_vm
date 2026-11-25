#include "demo_common.h"

int main()
{
	rule_vm_demo::desg::rule_desg<> origin;
	rule_vm_demo::desg::rule_desg_text_error<rule_vm_demo::alloc_t> error_v;

	constexpr std::string_view source_v = R"(
defvar 0 {
    var a 0:0:0:0:8
}

defcmd 0 {
    mov a, 42
    ret
}

start defcmd 0
)";

	if (rule_vm_demo::load_text(origin, source_v, error_v) == false)
		return 1;

	rule_vm_demo::desg::rule_desg<> clone;
	clone.copy_from(origin);
	clone.run(0, 0);

	const std::optional<std::uint64_t> clone_value_v =
		rule_vm_demo::read_qword(clone, 0, 0);
	const std::optional<std::uint64_t> origin_value_v =
		rule_vm_demo::read_qword(origin, 0, 0);

	const bool ok_v = clone_value_v.has_value() && clone_value_v.value() == 42 &&
		origin_value_v.has_value() && origin_value_v.value() == 0;

	return rule_vm_demo::expect("copy_snapshot", ok_v) ? 0 : 1;
}
