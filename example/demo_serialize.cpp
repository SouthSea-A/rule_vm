#include <cstdio>
#include <filesystem>

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

	origin.run_start();

	const std::filesystem::path path_v =
		std::filesystem::temp_directory_path() / "rule_vm_demo_state.rvm";

	if (origin.save_file(path_v, 0, 0) == false)
		return 2;

	rule_vm_demo::desg::rule_desg<> restored;
	rule_vm_demo::desg::rule_desg_cmd_mgr_index mgr_index_v = 0;
	rule_vm_demo::desg::rule_desg_cmd_index cmd_index_v = 0;

	if (restored.load_file(path_v, mgr_index_v, cmd_index_v) == false)
		return 3;

	std::error_code remove_error_v;
	std::filesystem::remove(path_v, remove_error_v);

	restored.resume(mgr_index_v, cmd_index_v);

	const std::optional<std::uint64_t> value_v =
		rule_vm_demo::read_qword(restored, 0, 0);
	const bool ok_v = value_v.has_value() && value_v.value() == 42;

	return rule_vm_demo::expect("save_load_resume", ok_v) ? 0 : 1;
}
