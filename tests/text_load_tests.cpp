#include <cstdio>
#include <optional>
#include <string>
#include <fstream>
#include "rule_design.h"

namespace desg = rule_design;

static std::string wrap_blocks(std::string_view flat_v)
{
	if (flat_v.find("defcmd") != std::string_view::npos || flat_v.find("defvar") != std::string_view::npos)
		return std::string(flat_v);

	std::string var_v;
	std::string cmd_v;
	std::string_view rest_v = flat_v;

	while (true)
	{
		const std::size_t newline_v = rest_v.find('\n');
		const std::string_view line_v = (newline_v == std::string_view::npos) ? rest_v : rest_v.substr(0, newline_v);

		if (line_v.size() >= 3 && line_v.substr(0, 3) == "var" && (line_v.size() == 3 || line_v[3] == ' ' || line_v[3] == '\t'))
		{
			var_v.append(line_v);
			var_v.push_back('\n');
		}
		else if (line_v.size() >= 3 && line_v.substr(0, 3) == "mgr")
		{
		}
		else if (line_v.empty() == false)
		{
			cmd_v.append(line_v);
			cmd_v.push_back('\n');
		}

		if (newline_v == std::string_view::npos) break;
		rest_v.remove_prefix(newline_v + 1);
	}

	return "defvar 0 {\n" + var_v + "}\ndefcmd 0 {\n" + cmd_v + "}\nstart defcmd 0\n";
}


static bool expect(const char* name, bool ok)
{
	std::printf("%s %s\n", name, ok ? "PASS" : "FAIL");
	return ok;
}

static std::optional<std::uint64_t> read_qword(desg::rule_desg<>& desg, desg::rule_desg_var_index var_index)
{
	auto mgr_res = desg.get_owner().get_owner().get_var_set().get(0);

	if (mgr_res.has_value() == false)
		return std::nullopt;

	auto var_res = mgr_res.value().get().get_content().get(var_index);

	if (var_res.has_value() == false)
		return std::nullopt;

	auto span_res = var_res.value().get().get_content().read(0, 8);

	if (span_res.has_value() == false)
		return std::nullopt;

	std::uint64_t value_v = 0;

	for (std::size_t i = 0; i < 8; i++)
		value_v |= static_cast<std::uint64_t>(span_res.value()[i]) << (i * 8);

	return value_v;
}

static bool test_arithmetic_and_jump()
{
	desg::rule_desg<> desg;
	desg::rule_desg_text_error<std::allocator<int>> error_v;

	const char* text_v =
		"; arithmetic + forward label\n"
		"var a 0:0:0:0:8\n"
		"var b 0:1:0:0:8 = 05 00 00 00 00 00 00 00\n"
		"mgr 0\n"
		"start:\n"
		"    mov a, 0x2A\n"
		"    add a, a, 1\n"
		"    mov a, b\n"
		"    jmp done, uncond, uncond\n"
		"    mov a, 0\n"
		"done:\n"
		"    ret\n";

	if (desg.load_text(wrap_blocks(text_v), error_v) == false)
	{
		std::printf("  error line=%zu col=%zu: %s\n", error_v.line, error_v.column, error_v.message.c_str());
		return false;
	}

	desg.run_start();
	const auto value = read_qword(desg, 0);

	return expect("arithmetic_and_jump", value.has_value() && value.value() == 5);
}

static bool test_call_ret()
{
	desg::rule_desg<> desg;
	desg::rule_desg_text_error<std::allocator<int>> error_v;

	const char* text_v =
		"var a 0:0:0:0:8\n"
		"start:\n"
		"    mov a, 10\n"
		"    call sub\n"
		"    jmp end\n"
		"sub:\n"
		"    add a, a, 5\n"
		"    ret\n"
		"end:\n"
		"    ret\n";

	if (desg.load_text(wrap_blocks(text_v), error_v) == false)
		return false;

	desg.run_start();
	const auto value = read_qword(desg, 0);

	return expect("call_ret", value.has_value() && value.value() == 15);
}

static bool test_alloc_dealloc_and_explicit()
{
	desg::rule_desg<> desg;
	desg::rule_desg_text_error<std::allocator<int>> error_v;

	const char* text_v =
		"var a 0:0:0:0:8\n"
		"alloc var 0:5:0:0:8\n"
		"mov v(0:0:0:0:8), 'A'\n"
		"dealloc var 0:5\n"
		"ret\n";

	if (desg.load_text(wrap_blocks(text_v), error_v) == false)
		return false;

	desg.run_start();
	const auto value = read_qword(desg, 0);
	auto mgr_res = desg.get_owner().get_owner().get_var_set().get(0);
	bool var5_absent = false;

	if (mgr_res.has_value() == true)
		var5_absent = mgr_res.value().get().get_content().get(5).has_value() == false;

	return expect("alloc_dealloc_and_explicit", value.has_value() && value.value() == 65 && var5_absent);
}

static bool test_string_variables()
{
	desg::rule_desg<> desg;
	desg::rule_desg_text_error<std::allocator<int>> error_v;

	const char* text_v =
		"var s1 0:0:0:0 = \"hello world\"\n"
		"var s2 0:1:0:0:11\n"
		"var a 0:2:0:0:8\n"
		"mov s2, s1\n"
		"mov a, \"A\"\n"
		"ret\n";

	if (desg.load_text(wrap_blocks(text_v), error_v) == false)
	{
		std::printf("  string load error line=%zu col=%zu: %s\n", error_v.line, error_v.column, error_v.message.c_str());
		return false;
	}

	desg.run_start();

	auto mgr_res = desg.get_owner().get_owner().get_var_set().get(0);
	bool string_ok = false;
	bool hidden_ok = false;

	if (mgr_res.has_value() == true)
	{
		auto& var_mgr_v = mgr_res.value().get().get_content();
		auto s2_res = var_mgr_v.get(1);

		if (s2_res.has_value() == true)
		{
			auto span_res = s2_res.value().get().get_content().read(0, 11);
			string_ok = span_res.has_value() &&
				std::string(reinterpret_cast<const char*>(span_res.value().data()), 11) == "hello world";
		}

		auto a_res = var_mgr_v.get(2);

		if (a_res.has_value() == true)
		{
			auto span_res = a_res.value().get().get_content().read(0, 1);
			hidden_ok = span_res.has_value() && span_res.value()[0] == desg::rule_desg_var_base(65);
		}
	}

	return expect("string_variables", string_ok && hidden_ok);
}

static bool test_errors()
{
	desg::rule_desg<> desg;
	desg::rule_desg_text_error<std::allocator<int>> error_v;

	const bool undefined_label = desg.load_text(wrap_blocks("jmp nowhere\n"), error_v) == false && error_v.message.find("未定义标签") != std::string::npos;
	const bool unknown_cmd = desg.load_text(wrap_blocks("nop\n"), error_v) == false && error_v.message.find("未知指令") != std::string::npos;
	const bool duplicate_var = desg.load_text(wrap_blocks("var a 0:0:0:0:8\nvar a 0:1:0:0:8\n"), error_v) == false && error_v.message.find("变量名重复") != std::string::npos;

	return expect("errors", undefined_label && unknown_cmd && duplicate_var);
}

static bool test_utf8_file()
{
	const char* path_v = "text_utf8_test.rule";
	{
		std::ofstream out_v(path_v, std::ios::binary | std::ios::trunc);
		out_v << "\xEF\xBB\xBF";
		out_v << "; 中文注释\n";
		out_v << "defvar 0 {\n";
		out_v << "var s 0:0:0:0 = \"你好\"\n";
		out_v << "}\n";
		out_v << "defcmd 0 {\n";
		out_v << "ret\n";
		out_v << "}\n";
		out_v << "start defcmd 0\n";
	}

	desg::rule_desg<> desg;
	desg::rule_desg_text_error<std::allocator<int>> error_v;

	if (desg.load_text_file(path_v, error_v) == false)
	{
		std::printf("  utf8 load error line=%zu col=%zu: %s\n", error_v.line, error_v.column, error_v.message.c_str());
		std::remove(path_v);
		return false;
	}

	desg.run_start();
	auto mgr_res = desg.get_owner().get_owner().get_var_set().get(0);
	bool utf8_ok = false;

	if (mgr_res.has_value() == true)
	{
		auto var_res = mgr_res.value().get().get_content().get(0);

		if (var_res.has_value() == true)
		{
			auto span_res = var_res.value().get().get_content().read(0, 6);
			utf8_ok = span_res.has_value() &&
				std::string(reinterpret_cast<const char*>(span_res.value().data()), 6) == "你好";
		}
	}

	std::remove(path_v);

	const char* bad_path_v = "text_bad_utf8.rule";
	{
		std::ofstream out_v(bad_path_v, std::ios::binary | std::ios::trunc);
		const char bad_v[2] = { static_cast<char>(0xFF), 0 };
		out_v.write(bad_v, 1);
	}

	desg::rule_desg<> bad_desg;
	const bool bad_rejected = bad_desg.load_text_file(bad_path_v, error_v) == false &&
		error_v.message.find("UTF-8") != std::string::npos;
	std::remove(bad_path_v);

	const bool missing_rejected = bad_desg.load_text_file("not_exists.rule", error_v) == false &&
		error_v.message.find("无法打开") != std::string::npos;

	return expect("utf8_file", utf8_ok && bad_rejected && missing_rejected);
}

static bool test_self_modify()
{
	desg::rule_desg<> desg;
	desg::rule_desg_text_error<std::allocator<int>> error_v;

	const char* text_v =
		"var a 0:0:0:0:8\n"
		"alloc cmd 0:2:4:0:0\n"
		"jmp skip\n"
		"skip:\n"
		"    mov a, 99\n"
		"    ret\n";

	if (desg.load_text(wrap_blocks(text_v), error_v) == false)
		return false;

	desg.run_start();
	const auto value = read_qword(desg, 0);

	auto cmd_mgr_res = desg.get_owner().get_owner().get_cmd_set().get(0);
	bool type_ok = false;

	if (cmd_mgr_res.has_value() == true)
	{
		auto cmd_res = cmd_mgr_res.value().get().get_content().get(2);

		if (cmd_res.has_value() == true)
			type_ok = static_cast<desg::rule_desg_cmd_def_type>(cmd_res.value().get_type().get()) == desg::rule_desg_cmd_def_type::ret;
	}

	return expect("self_modify", value.has_value() && value.value() == 0 && type_ok);
}

static bool test_auto_alloc()
{
	desg::rule_desg<> desg;
	desg::rule_desg_text_error<std::allocator<int>> error_v;

	const char* text_v =
		"mov v(0:0:0:0:8), 42\n"
		"ret\n";

	if (desg.load_text(wrap_blocks(text_v), error_v) == false)
		return false;

	desg.run_start();
	const auto value = read_qword(desg, 0);

	desg.release_auto_alloc();
	bool released = false;
	auto released_mgr_res = desg.get_owner().get_owner().get_var_set().get(0);

	if (released_mgr_res.has_value() == true)
		released = released_mgr_res.value().get().get_content().get(0).has_value() == false;

	desg::rule_desg<> jump_desg;

	const char* jump_text_v =
		"jmp c(0:5:0:0:0)\n"
		"ret\n";

	if (jump_desg.load_text(wrap_blocks(jump_text_v), error_v) == false)
		return false;

	jump_desg.run_start();

	bool jump_created = false;
	auto cmd_mgr_res = jump_desg.get_owner().get_owner().get_cmd_set().get(0);

	if (cmd_mgr_res.has_value() == true)
		jump_created = cmd_mgr_res.value().get().get_content().get(5).has_value() == true;

	return expect("auto_alloc", value.has_value() && value.value() == 42 && released && jump_created);
}

static bool test_blocks()
{
	desg::rule_desg<> desg;
	desg::rule_desg_text_error<std::allocator<int>> error_v;

	const char* text_v =
		"defvar 0 {\n"
		"    var a 0:0:0:0:8\n"
		"}\n"
		"defvar 1 {\n"
		"}\n"
		"defcmd 0 {\n"
		"    mov a, 42\n"
		"    call c(1:0:0:0:0)\n"
		"    ret\n"
		"}\n"
		"defcmd 1 {\n"
		"    add a, a, 1\n"
		"    ret\n"
		"}\n"
		"start defcmd 0\n";

	if (desg.load_text(wrap_blocks(text_v), error_v) == false)
	{
		std::printf("  block load error line=%zu: %s\n", error_v.line, error_v.message.c_str());
		return false;
	}

	desg.run_start();
	const auto value = read_qword(desg, 0);
	const bool var_blocks_ok = desg.get_owner().get_owner().get_var_set().size() == 2;
	const bool cmd_blocks_ok = desg.get_owner().get_owner().get_cmd_set().size() == 2;

	desg::rule_desg<> missing_start;
	const bool start_required = missing_start.load_text("defvar 0 {\n}\ndefcmd 0 {\nret\n}\n", error_v) == false &&
		error_v.message.find("start") != std::string::npos;

	return expect("blocks", value.has_value() && value.value() == 43 && var_blocks_ok && cmd_blocks_ok && start_required);
}

int main()
{
	bool ok = true;

	ok &= test_arithmetic_and_jump();
	ok &= test_call_ret();
	ok &= test_alloc_dealloc_and_explicit();
	ok &= test_string_variables();
	ok &= test_utf8_file();
	ok &= test_self_modify();
	ok &= test_auto_alloc();
	ok &= test_blocks();
	ok &= test_errors();

	std::printf("%s\n", ok ? "ALL PASS" : "SOME FAILED");
	return ok ? 0 : 1;
}
