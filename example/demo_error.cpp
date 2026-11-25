#include <cstdio>

#include "demo_common.h"

int main()
{
	rule_vm_demo::desg::rule_desg<> vm;
	rule_vm_demo::desg::rule_desg_text_error<rule_vm_demo::alloc_t> error_v;

	constexpr std::string_view bad_source_v = R"(
defvar 0 {
    var a 0:0:0:0:8
)";

	if (vm.load_text(bad_source_v, error_v) == true)
		return 1;

	std::printf(
		"expected text error: line=%zu column=%zu message=%s\n",
		error_v.line,
		error_v.column,
		error_v.message.c_str());

	try
	{
		vm.throw_error(
			rule_vm_demo::desg::rule_design_error::exec__internal,
			"demo error");
		return 2;
	}
	catch (const rule_vm_demo::desg::rule_design_error_throw& error_v)
	{
		std::printf(
			"caught rule_design_error: code=%s message=%s\n",
			std::string(rule_vm_demo::desg::rule_design_error_text(error_v.error_code)).c_str(),
			error_v.error_msg.c_str());
	}

	return rule_vm_demo::expect("error_handling", true) ? 0 : 1;
}
