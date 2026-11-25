#include <cstdio>
#include <limits>
#include <string>
#include "rule_design.h"

namespace desg = rule_design;

static constexpr rule_var_cont_data_def::var_mgr_index const_mgr_v = std::numeric_limits<rule_var_cont_data_def::var_mgr_index>::max() - 1;

static bool expect(const char* name, bool ok)
{
	std::printf("%s %s\n", name, ok ? "PASS" : "FAIL");
	return ok;
}

static std::string read_const(desg::rule_desg<>& desg, rule_var_cont_data_def::var_index index_v, std::size_t width_v)
{
	auto mgr_res = desg.get_owner().get_owner().get_var_set().get(const_mgr_v);
	if (mgr_res.has_value() == false) return {};

	auto var_res = mgr_res.value().get().get_content().get(index_v);
	if (var_res.has_value() == false) return {};

	auto span_res = var_res.value().get().get_content().read(0, static_cast<rule_var_cont_data_def::var_read_width>(width_v));
	if (span_res.has_value() == false) return {};

	return std::string(reinterpret_cast<const char*>(span_res.value().data()), width_v);
}

int main()
{
	bool ok = true;

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const char* text =
			"defvar 0 {\n"
			"var a 0:0:0:0:8\n"
			"var b 0:1:0:0:8\n"
			"}\n"
			"defcmd 0 {\n"
			"mov a, \"same\"\n"
			"mov b, \"same\"\n"
			"ret\n"
			"}\n"
			"start defcmd 0\n";

		const bool loaded = desg.load_text(text, error_v);
		desg.run_start();

		auto const_mgr_res = desg.get_owner().get_owner().get_var_set().get(const_mgr_v);
		const bool no_dedup = const_mgr_res.has_value() &&
			const_mgr_res.value().get().get_content().size() == 2;
		const bool value_ok = read_const(desg, 0, 8) == std::string("same\0\0\0\0", 8) &&
			read_const(desg, 1, 8) == std::string("same\0\0\0\0", 8);

		ok &= expect("const_layer_no_dedup", loaded && no_dedup && value_ok);
	}

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const std::string text_v =
			"defvar 0 {\n"
			"var a 0:0:0:0:8\n"
			"}\n"
			"defcmd 0 {\n"
			"mov a, \"same\"\n"
			"mov v(" + std::to_string(const_mgr_v) + ":0:0:0:8), 7\n"
			"ret\n"
			"}\n"
			"start defcmd 0\n";

		const bool loaded = desg.load_text(text_v, error_v);
		desg.run_start();
		const std::string value_v = read_const(desg, 0, 8);
		const bool writable = value_v.size() == 8 && value_v[0] == 7 && value_v[1] == 0;

		ok &= expect("const_layer_writable", loaded && writable);
	}

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const std::string text_v =
			"defvar 0 {\n"
			"var a 0:0:0:0:8\n"
			"}\n"
			"defcmd 0 {\n"
			"mov a, \"same\"\n"
			"alloc var " + std::to_string(const_mgr_v) + ":5:0:0:8\n"
			"ret\n"
			"}\n"
			"start defcmd 0\n";

		const bool loaded = desg.load_text(text_v, error_v);
		desg.run_start();

		auto const_mgr_res = desg.get_owner().get_owner().get_var_set().get(const_mgr_v);
		const bool alloc_ok = const_mgr_res.has_value() &&
			const_mgr_res.value().get().get_content().get(5).has_value();

		ok &= expect("const_layer_alloc", loaded && alloc_ok);
	}

	{
		desg::rule_desg<> desg;
		desg::rule_desg_text_error<std::allocator<int>> error_v;
		const std::string text_v =
			"defvar 0 {\n"
			"var a 0:0:0:0:8\n"
			"}\n"
			"defcmd 0 {\n"
			"mov a, \"same\"\n"
			"dealloc var " + std::to_string(const_mgr_v) + ":0\n"
			"ret\n"
			"}\n"
			"start defcmd 0\n";

		const bool loaded = desg.load_text(text_v, error_v);
		desg.run_start();

		auto const_mgr_res = desg.get_owner().get_owner().get_var_set().get(const_mgr_v);
		const bool dealloc_ok = const_mgr_res.has_value() == false ||
			const_mgr_res.value().get().get_content().get(0).has_value() == false;

		ok &= expect("const_layer_dealloc", loaded && dealloc_ok);
	}

	std::printf("%s\n", ok ? "ALL PASS" : "SOME FAILED");
	return ok ? 0 : 1;
}
