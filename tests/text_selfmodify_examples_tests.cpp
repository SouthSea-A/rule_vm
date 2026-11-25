#include <cstdio>
#include <limits>
#include <string>
#include "rule_design.h"

namespace desg = rule_design;

static bool expect(const char* name, bool ok)
{
	std::printf("%s %s\n", name, ok ? "PASS" : "FAIL");
	return ok;
}

static std::uint64_t read_qword(desg::rule_desg<>& desg, desg::rule_desg_var_index index_v)
{
	auto mgr_res = desg.get_owner().get_owner().get_var_set().get(0);
	if (mgr_res.has_value() == false) return 0;
	auto var_res = mgr_res.value().get().get_content().get(index_v);
	if (var_res.has_value() == false) return 0;
	auto span_res = var_res.value().get().get_content().read(0, 8);
	if (span_res.has_value() == false) return 0;
	std::uint64_t value_v = 0;
	for (std::size_t i = 0; i < 8; i++) value_v |= static_cast<std::uint64_t>(span_res.value()[i]) << (i * 8);
	return value_v;
}

int main()
{
	bool ok = true;

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const char* text =
			"defvar 0 {}\n"
			"defcmd 0 {\n"
			"mov v(0:5:0:0:8), 42\n"
			"mov v(0:6:0:0:8), v(0:5:0:0:8)\n"
			"ret\n"
			"}\n"
			"start defcmd 0\n";

		const bool loaded = desg.load_text(text, error_v);
		desg.run_start();
		ok &= expect("mov_creates_variables", loaded && read_qword(desg, 5) == 42 && read_qword(desg, 6) == 42);
	}

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const char* text =
			"defvar 0 {\n"
			"var a 0:0:0:0:8\n"
			"}\n"
			"defcmd 0 {\n"
			"mov c(0:3:0:0:2), c(0:2:0:0:2)\n"
			"jmp c(0:3:0:0:0)\n"
			"mov a, 7\n"
			"mov a, 99\n"
			"ret\n"
			"}\n"
			"start defcmd 0\n";

		const bool loaded = desg.load_text(text, error_v);
		desg.run_start();
		ok &= expect("mov_rewrites_existing_command", loaded && read_qword(desg, 0) == 7);
	}

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const char* text =
			"defvar 0 {\n"
			"var a 0:0:0:0:8\n"
			"}\n"
			"defcmd 0 {\n"
			"mov c(0:10:0:0:2), c(0:2:0:0:2)\n"
			"jmp c(0:10:0:0:0)\n"
			"mov a, 5\n"
			"ret\n"
			"}\n"
			"start defcmd 0\n";

		const bool loaded = desg.load_text(text, error_v);
		desg.run_start();

		auto cmd_mgr_res = desg.get_owner().get_owner().get_cmd_set().get(0);
		const bool generated = cmd_mgr_res.has_value() &&
			cmd_mgr_res.value().get().get_content().get(10).has_value();
		ok &= expect("mov_generates_command", loaded && generated && read_qword(desg, 0) == 5);
	}

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const char* text =
			"defvar 0 {}\n"
			"defcmd 0 {\n"
			"mov v(0:5:0:0:8), 123\n"
			"dealloc var 0:5\n"
			"mov v(0:5:0:0:8), 7\n"
			"ret\n"
			"}\n"
			"start defcmd 0\n";

		const bool loaded = desg.load_text(text, error_v);
		desg.run_start();
		ok &= expect("dealloc_and_recreate_variable", loaded && read_qword(desg, 5) == 7);
	}

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const char* text =
			"defvar 0 {}\n"
			"defcmd 0 {\n"
			"alloc cmd 0:10:4:0:0\n"
			"dealloc cmdtgt 0:10\n"
			"ret\n"
			"}\n"
			"start defcmd 0\n";

		const bool loaded = desg.load_text(text, error_v);
		desg.run_start();

		auto cmd_mgr_res = desg.get_owner().get_owner().get_cmd_set().get(0);
		const bool deleted = cmd_mgr_res.has_value() == false ||
			cmd_mgr_res.value().get().get_content().get(10).has_value() == false;
		ok &= expect("dealloc_command", loaded && deleted);
	}

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const char* text =
			"defvar 0 {\n"
			"var a 0:0:0:0:8\n"
			"}\n"
			"defcmd 0 {\n"
			"mov v(var_val:0:5:0:0:8), 42\n"
			"mov c(cmd_val:0:10:0:0:2), c(cmd_val:0:3:0:0:2)\n"
			"jmp c(cmd_tgt:0:10:0:0:0)\n"
			"mov a, 5\n"
			"ret\n"
			"}\n"
			"start defcmd 0\n";

		const bool loaded = desg.load_text(text, error_v);
		desg.run_start();

		auto cmd_mgr_res = desg.get_owner().get_owner().get_cmd_set().get(0);
		const bool generated = cmd_mgr_res.has_value() &&
			cmd_mgr_res.value().get().get_content().get(10).has_value();
		ok &= expect("explicit_handles", loaded && generated && read_qword(desg, 0) == 5 && read_qword(desg, 5) == 42);
	}

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const char* text =
			"defvar 0 {\n"
			"var a 0:0:0:0:8\n"
			"}\n"
			"defcmd 0 {\n"
			"mov c(cmd_type:0:3:0:0:0), 4\n"
			"jmp done\n"
			"ret\n"
			"mov a, 5\n"
			"done:\n"
			"ret\n"
			"}\n"
			"start defcmd 0\n";

		const bool loaded = desg.load_text(text, error_v);
		desg.run_start();

		auto cmd_mgr_res = desg.get_owner().get_owner().get_cmd_set().get(0);
		std::uint64_t type_v = 99;
		if (cmd_mgr_res.has_value())
		{
			auto cmd_res = cmd_mgr_res.value().get().get_content().get(3);
			if (cmd_res.has_value()) type_v = cmd_res.value().get_type().get();
		}

		ok &= expect("mov_writes_cmd_type", loaded && type_v == 4);
	}

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const char* text =
			"defvar 0 {\n"
			"var a 0:0:0:0:8\n"
			"}\n"
			"defcmd 0 {\n"
			"mov a, c(cmd_type:0:2:0:0:0)\n"
			"ret\n"
			"mov a, 5\n"
			"}\n"
			"start defcmd 0\n";

		const bool loaded = desg.load_text(text, error_v);
		desg.run_start();
		ok &= expect("mov_reads_cmd_type", loaded && read_qword(desg, 0) == 0);
	}

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const char* text =
			"defvar 0 {\n"
			"var a 0:0:0:0:8\n"
			"}\n"
			"defcmd 0 {\n"
			"mov v(var_type:0:0:0:0:0), 1\n"
			"ret\n"
			"}\n"
			"start defcmd 0\n";

		const bool loaded = desg.load_text(text, error_v);
		desg.run_start();

		auto mgr_res = desg.get_owner().get_owner().get_var_set().get(0);
		std::uint64_t type_v = 99;
		if (mgr_res.has_value())
		{
			auto var_res = mgr_res.value().get().get_content().get(0);
			if (var_res.has_value()) type_v = var_res.value().get().get_type();
		}

		ok &= expect("mov_writes_var_type", loaded && type_v == 1);
	}

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const char* text =
			"defvar 0 {}\n"
			"defcmd 0 {\n"
			"alloc cmd 0:10:4:0:0\n"
			"mov c(cmd_del:0:10:0:0:0), 0\n"
			"ret\n"
			"}\n"
			"start defcmd 0\n";

		const bool loaded = desg.load_text(text, error_v);
		desg.run_start();

		auto cmd_mgr_res = desg.get_owner().get_owner().get_cmd_set().get(0);
		const bool deleted = cmd_mgr_res.has_value() == false ||
			cmd_mgr_res.value().get().get_content().get(10).has_value() == false;
		ok &= expect("mov_deletes_command", loaded && deleted);
	}

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const char* text =
			"defvar 0 {}\n"
			"defcmd 0 {\n"
			"mov v(var_val:0:5:0:0:8), 123\n"
			"mov v(var_del:0:5:0:0:0), 0\n"
			"ret\n"
			"}\n"
			"start defcmd 0\n";

		const bool loaded = desg.load_text(text, error_v);
		desg.run_start();

		auto var_mgr_res = desg.get_owner().get_owner().get_var_set().get(0);
		const bool deleted = var_mgr_res.has_value() == false ||
			var_mgr_res.value().get().get_content().get(5).has_value() == false;
		ok &= expect("mov_deletes_variable", loaded && deleted);
	}


	std::printf("%s\n", ok ? "ALL PASS" : "SOME FAILED");
	return ok ? 0 : 1;
}
