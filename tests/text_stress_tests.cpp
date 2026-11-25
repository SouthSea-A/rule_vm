#include <chrono>
#include <cstdio>
#include <optional>
#include <string>
#include "rule_design.h"

namespace desg = rule_design;

static std::string wrap_blocks(std::string_view flat_v)
{
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

using steady_t = std::chrono::steady_clock;

static bool expect(const char* name, bool ok, double ms_v)
{
	std::printf("%s %s (%.1f ms)\n", name, ok ? "PASS" : "FAIL", ms_v);
	return ok;
}

static std::optional<std::uint64_t> read_qword(desg::rule_desg<>& desg, desg::rule_desg_var_index var_index, desg::rule_desg_var_mgr_index mgr_index = 0)
{
	auto mgr_res = desg.get_owner().get_owner().get_var_set().get(mgr_index);

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

static double elapsed_ms(steady_t::time_point begin_v)
{
	return std::chrono::duration<double, std::milli>(steady_t::now() - begin_v).count();
}

static bool test_large_chain(std::size_t count_v)
{
	std::string text_v;
	text_v.reserve(count_v * 16 + 64);
	text_v += "var a 0:0:0:0:8\n";
	text_v += "mov a, 0\n";

	for (std::size_t i = 0; i < count_v; i++)
		text_v += "add a, a, 1\n";

	text_v += "ret\n";

	const auto begin_v = steady_t::now();
	desg::rule_desg<> desg;
	desg::rule_desg_text_error<std::allocator<int>> error_v;

	if (desg.load_text(wrap_blocks(text_v), error_v) == false)
	{
		std::printf("  chain load error line=%zu: %s\n", error_v.line, error_v.message.c_str());
		return false;
	}

	const double load_ms_v = elapsed_ms(begin_v);
	const auto run_begin_v = steady_t::now();
	desg.run_start();
	const double run_ms_v = elapsed_ms(run_begin_v);
	const auto value = read_qword(desg, 0);

	std::printf("  chain count=%zu load=%.1f ms run=%.1f ms\n", count_v, load_ms_v, run_ms_v);
	return expect("large_chain", value.has_value() && value.value() == count_v, load_ms_v + run_ms_v);
}

static bool test_large_auto_alloc(std::size_t count_v)
{
	std::string text_v;
	text_v.reserve(count_v * 32);

	for (std::size_t i = 0; i < count_v; i++)
	{
		text_v += "mov v(0:";
		text_v += std::to_string(i);
		text_v += ":0:0:8), ";
		text_v += std::to_string(i * 3 + 1);
		text_v += "\n";
	}

	text_v += "ret\n";

	const auto begin_v = steady_t::now();
	desg::rule_desg<> desg;
	desg::rule_desg_text_error<std::allocator<int>> error_v;

	if (desg.load_text(wrap_blocks(text_v), error_v) == false)
	{
		std::printf("  auto load error line=%zu: %s\n", error_v.line, error_v.message.c_str());
		return false;
	}

	const double load_ms_v = elapsed_ms(begin_v);
	const auto run_begin_v = steady_t::now();
	desg.run_start();
	const double run_ms_v = elapsed_ms(run_begin_v);

	bool all_ok = true;

	for (std::size_t i = 0; i < count_v; i++)
	{
		const auto value = read_qword(desg, static_cast<desg::rule_desg_var_index>(i));

		if (value.has_value() == false || value.value() != i * 3 + 1)
		{
			all_ok = false;
			break;
		}
	}

	desg.release_auto_alloc();
	bool released = false;
	auto released_mgr_res = desg.get_owner().get_owner().get_var_set().get(0);

	if (released_mgr_res.has_value() == true)
		released = released_mgr_res.value().get().get_content().get(0).has_value() == false;

	std::printf("  auto count=%zu load=%.1f ms run=%.1f ms\n", count_v, load_ms_v, run_ms_v);
	return expect("large_auto_alloc", all_ok && released, load_ms_v + run_ms_v);
}

static bool test_large_self_modify(std::size_t count_v)
{
	if (count_v < 4)
		return false;

	std::string text_v;
	text_v.reserve(count_v * 16);
	text_v += "var a 0:0:0:0:8\n";
	text_v += "alloc cmd 0:" + std::to_string(count_v - 2) + ":4:0:0\n";

	for (std::size_t i = 1; i + 2 < count_v; i++)
		text_v += "add a, a, 1\n";

	text_v += "mov a, 99\n";
	text_v += "ret\n";

	const auto begin_v = steady_t::now();
	desg::rule_desg<> desg;
	desg::rule_desg_text_error<std::allocator<int>> error_v;

	if (desg.load_text(wrap_blocks(text_v), error_v) == false)
		return false;

	const double load_ms_v = elapsed_ms(begin_v);
	const auto run_begin_v = steady_t::now();
	desg.run_start();
	const double run_ms_v = elapsed_ms(run_begin_v);
	const auto value = read_qword(desg, 0);
	const std::uint64_t expected_v = count_v - 3;

	auto cmd_mgr_res = desg.get_owner().get_owner().get_cmd_set().get(0);
	bool type_ok = false;

	if (cmd_mgr_res.has_value() == true)
	{
		auto cmd_res = cmd_mgr_res.value().get().get_content().get(static_cast<desg::rule_desg_cmd_index>(count_v - 2));

		if (cmd_res.has_value() == true)
			type_ok = static_cast<desg::rule_desg_cmd_def_type>(cmd_res.value().get_type().get()) == desg::rule_desg_cmd_def_type::ret;
	}

	std::printf("  selfmod count=%zu load=%.1f ms run=%.1f ms\n", count_v, load_ms_v, run_ms_v);
	return expect("large_self_modify", value.has_value() && value.value() == expected_v && type_ok, load_ms_v + run_ms_v);
}

static bool test_large_string(std::size_t size_v)
{
	std::string payload_v;
	payload_v.reserve(size_v);

	std::uint64_t hash_expected = 0;

	for (std::size_t i = 0; i < size_v; i++)
	{
		const char char_v = static_cast<char>('A' + (i % 26));
		payload_v.push_back(char_v);
		hash_expected += static_cast<unsigned char>(char_v);
	}

	std::string text_v;
	text_v.reserve(size_v + 128);
	text_v += "var s1 0:0:0:0 = \"";
	text_v += payload_v;
	text_v += "\"\n";
	text_v += "var s2 0:1:0:0:" + std::to_string(size_v) + "\n";
	text_v += "mov s2, s1\n";
	text_v += "ret\n";

	const auto begin_v = steady_t::now();
	desg::rule_desg<> desg;
	desg::rule_desg_text_error<std::allocator<int>> error_v;

	if (desg.load_text(wrap_blocks(text_v), error_v) == false)
		return false;

	const double load_ms_v = elapsed_ms(begin_v);
	const auto run_begin_v = steady_t::now();
	desg.run_start();
	const double run_ms_v = elapsed_ms(run_begin_v);

	auto mgr_res = desg.get_owner().get_owner().get_var_set().get(0);
	bool copy_ok = false;

	if (mgr_res.has_value() == true)
	{
		auto var_res = mgr_res.value().get().get_content().get(1);

		if (var_res.has_value() == true)
		{
			auto span_res = var_res.value().get().get_content().read(0, static_cast<rule_var_cont_data_def::var_read_width>(size_v));
			std::uint64_t hash_v = 0;

			if (span_res.has_value() == true)
				for (const auto byte_v : span_res.value())
					hash_v += static_cast<unsigned char>(byte_v);

			copy_ok = span_res.has_value() && hash_v == hash_expected;
		}
	}

	std::printf("  string size=%zu load=%.1f ms run=%.1f ms\n", size_v, load_ms_v, run_ms_v);
	return expect("large_string", copy_ok, load_ms_v + run_ms_v);
}

int main()
{
	bool ok = true;

	ok &= test_large_chain(10000);
	ok &= test_large_chain(100000);
	ok &= test_large_auto_alloc(10000);
	ok &= test_large_auto_alloc(100000);
	ok &= test_large_self_modify(10000);
	ok &= test_large_self_modify(100000);
	ok &= test_large_string(1024 * 1024);

	std::printf("%s\n", ok ? "ALL PASS" : "SOME FAILED");
	return ok ? 0 : 1;
}
