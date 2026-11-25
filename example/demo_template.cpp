#include <cstdio>
#include <filesystem>
#include <string>

#include "demo_common.h"

#ifndef RULE_VM_EXAMPLE_DIR
#define RULE_VM_EXAMPLE_DIR "example/programs"
#endif

int main(int argc, char** argv)
{
	rule_vm_demo::desg::rule_desg<> vm;
	rule_vm_demo::desg::rule_desg_text_error<rule_vm_demo::alloc_t> error_v;

	const std::filesystem::path program_v =
		argc > 1
			? std::filesystem::path(argv[1])
			: std::filesystem::path(RULE_VM_EXAMPLE_DIR) / "jit_expr.rule";

	if (vm.load_text_file(program_v, error_v) == false)
	{
		std::printf(
			"load_text_file failed: %s line=%zu column=%zu message=%s\n",
			program_v.string().c_str(),
			error_v.line,
			error_v.column,
			error_v.message.c_str());
		return 1;
	}

	vm.run_start();

	// jit_expr.rule 的模板：a=6, b=2, c=3, op1=add, op2=mul
	// 它在运行时把选中的指令复制到 100/101，再执行，结果为 24。
	const std::optional<std::uint64_t> result_v =
		rule_vm_demo::read_qword(vm, 0, 4);

	const bool ok_v = result_v.has_value() && result_v.value() == 24;

	return rule_vm_demo::expect("template_codegen", ok_v) ? 0 : 1;
}
