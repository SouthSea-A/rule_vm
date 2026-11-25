#include <cstdio>
#include <string>
#include "rule_design.h"

namespace desg = rule_design;

static bool expect(const char* name, bool ok)
{
	std::printf("%s %s\n", name, ok ? "PASS" : "FAIL");
	return ok;
}

static bool load_fail(const char* name, const char* text, const char* message_part)
{
	desg::rule_desg<> desg;
	desg::rule_desg_text_error<std::allocator<int>> error_v;
	const bool failed = desg.load_text(text, error_v) == false;
	const bool message_ok = message_part == nullptr || error_v.message.find(message_part) != std::string::npos;
	return expect(name, failed && message_ok);
}

static std::optional<std::uint64_t> read_qword(desg::rule_desg<>& desg, desg::rule_desg_var_index index_v)
{
	auto mgr_res = desg.get_owner().get_owner().get_var_set().get(0);
	if (mgr_res.has_value() == false) return std::nullopt;
	auto var_res = mgr_res.value().get().get_content().get(index_v);
	if (var_res.has_value() == false) return std::nullopt;
	auto span_res = var_res.value().get().get_content().read(0, 8);
	if (span_res.has_value() == false) return std::nullopt;
	std::uint64_t value_v = 0;
	for (std::size_t i = 0; i < 8; i++) value_v |= static_cast<std::uint64_t>(span_res.value()[i]) << (i * 8);
	return value_v;
}

int main()
{
	bool ok = true;

	ok &= load_fail("flat_rejected",
		"var a 0:0:0:0:8\nmov a, 1\nret\n", "块外");

	ok &= load_fail("var_outside_block",
		"var a 0:0:0:0:8\ndefcmd 0 {\nret\n}\nstart defcmd 0\n", "块外");

	ok &= load_fail("mgr_rejected",
		"defcmd 0 {\nmgr 1\nret\n}\nstart defcmd 0\n", "不允许 mgr");

	ok &= load_fail("var_in_cmd",
		"defcmd 0 {\nvar a 0:0:0:0:8\nret\n}\nstart defcmd 0\n", "不允许 var");

	ok &= load_fail("unclosed_block",
		"defcmd 0 {\nret\n", "块未闭合");

	ok &= load_fail("missing_start",
		"defvar 0 {}\ndefcmd 0 {\nret\n}\n", "start");

	ok &= load_fail("duplicate_start",
		"defcmd 0 {\nret\n}\nstart defcmd 0\nstart defcmd 0\n", "start 重复");

	ok &= load_fail("start_missing_block",
		"defcmd 0 {\nret\n}\nstart defcmd 1\n", "不存在");

	ok &= load_fail("start_empty_block",
		"defcmd 1 {}\nstart defcmd 1\n", "没有指令");

	ok &= load_fail("duplicate_var_block",
		"defvar 0 {}\ndefvar 0 {}\n", "defvar 块索引重复");

	ok &= load_fail("duplicate_cmd_block",
		"defcmd 0 {\nret\n}\ndefcmd 0 {\nret\n}\n", "defcmd 块索引重复");

	ok &= load_fail("var_mgr_mismatch",
		"defvar 1 {\nvar a 0:0:0:0:8\n}\ndefcmd 0 {\nret\n}\nstart defcmd 0\n", "不一致");

	ok &= load_fail("duplicate_label",
		"defcmd 0 {\nx:\nret\nx:\nret\n}\nstart defcmd 0\n", "标签重复");

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const char* text =
			"defvar 0 {\nvar a 0:0:0:0:8\n}\n"
			"defcmd 0 {\nx:\nmov a, 1\nret\n}\n"
			"defcmd 1 {\nx:\nmov a, 2\nret\n}\n"
			"start defcmd 0\n";
		const bool loaded = desg.load_text(text, error_v);
		desg.run_start();
		ok &= expect("label_scope", loaded && read_qword(desg, 0).value_or(0) == 1);
	}

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const char* text =
			"defvar 0 {\nvar a 0:0:0:0:8\n}\n"
			"defvar 1 {}\n"
			"defcmd 0 {\nmov a, 10\nret\n}\n"
			"defcmd 1 {\nadd a, a, 5\nret\n}\n"
			"start defcmd 0\n";
		const bool loaded = desg.load_text(text, error_v);
		desg.run_start();
		const bool sizes = desg.get_owner().get_owner().get_var_set().size() == 2 &&
			desg.get_owner().get_owner().get_cmd_set().size() == 2;
		ok &= expect("empty_blocks", loaded && sizes && read_qword(desg, 0).value_or(0) == 10);
	}

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const char* text =
			"start defcmd 1\n"
			"defvar 0 {\n}\n"
			"defcmd 0 {\nret\n}\n"
			"defcmd 1 {\nmov v(0:0:0:0:8), 7\njmp c(0:5:0:0:0)\n}\n";
		const bool loaded = desg.load_text(text, error_v);
		desg.run_start();
		const bool jump_created = desg.get_owner().get_owner().get_cmd_set().get(0).has_value() &&
			desg.get_owner().get_owner().get_cmd_set().get(0).value().get().get_content().get(5).has_value();
		ok &= expect("start_before_block_and_auto", loaded && jump_created && read_qword(desg, 0).value_or(0) == 7);
	}

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const char* text =
			"defvar 0 {\nvar a 0:0:0:8\n}\n"
			"defcmd 0 {\nmov a, 33\nret\n}\nstart defcmd 0\n";
		const bool loaded = desg.load_text(text, error_v);
		desg.run_start();
		ok &= expect("compact_var", loaded && read_qword(desg, 0).value_or(0) == 33);
	}

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const char* text =
			"defvar 0 {}\ndefcmd 0 {}\ndefcmd 1 {\nret\n}\nstart defcmd 1\n";
		const bool loaded = desg.load_text(text, error_v);
		ok &= expect("empty_inline", loaded &&
			desg.get_owner().get_owner().get_var_set().size() == 1 &&
			desg.get_owner().get_owner().get_cmd_set().size() == 2);
	}

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const char* text =
			"defvar 0\n"
			"{\n"
			"    var a 0:0:0:0:8\n"
			"    var s 0:1:0:0:11 = \"hello world\"\n"
			"}\n"
			"defcmd 0\n"
			"{\n"
			"    mov a, 42\n"
			"    ret\n"
			"}\n"
			"start defcmd 0\n";
		const bool loaded = desg.load_text(text, error_v);
		desg.run_start();
		ok &= expect("brace_next_line", loaded && read_qword(desg, 0).value_or(0) == 42);
	}

	std::printf("%s\n", ok ? "ALL PASS" : "SOME FAILED");
	return ok ? 0 : 1;
}
