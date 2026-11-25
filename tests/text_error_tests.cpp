#include <cstdio>
#include <memory>
#include <string>
#include "rule_design.h"

namespace desg = rule_design;

static bool expect(const char* name, bool ok)
{
	std::printf("%s %s\n", name, ok ? "PASS" : "FAIL");
	return ok;
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
			"ret\n"
			"}\n";

		const bool loaded = desg.load_text(text, error_v);
		const bool code_ok = error_v.code == desg::rule_design_error::text__start &&
			desg.get_error().code == desg::rule_design_error::text__start;
		const bool message_ok = error_v.message.find("start") != std::string::npos &&
			desg.get_error().message.find("start") != std::string::npos;

		ok &= expect("text_start_error_code", loaded == false && code_ok && message_ok);
	}

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;

		const char* text =
			"defvar 0 {\n"
			"var a 0:0:0:0:8\n"
			"}\n"
			"defcmd 0 {\n"
			"mov c(cmd_self:0:0:0:0:0), 1\n"
			"ret\n"
			"}\n"
			"start defcmd 0\n";

		const bool loaded = desg.load_text(text, error_v);
		desg.run_start();

		const bool exec_error = desg.has_error() &&
			desg.get_error().code != desg::rule_design_error::none &&
			desg.get_error().message.empty() == false;

		ok &= expect("exec_error_code", loaded && exec_error);
	}

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;

		const char* text =
			"defvar 0 {}\n"
			"defcmd 0 {\n"
			"mov v(var_val:0:5:0:0:8), 42\n"
			"ret\n"
			"}\n"
			"start defcmd 0\n";

		const bool loaded = desg.load_text(text, error_v);
		desg.run_start();
		bool release_ok = true;

		try
		{
			desg.release_auto_alloc();
		}
		catch (const desg::rule_design_error_throw&)
		{
			release_ok = false;
		}

		ok &= expect("release_auto_no_throw", loaded && release_ok);
	}

	std::printf("%s\n", ok ? "ALL PASS" : "SOME FAILED");
	return ok ? 0 : 1;
}
