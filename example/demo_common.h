#pragma once

#include <array>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <span>
#include <string>
#include <string_view>

#include "rule_design.h"

namespace rule_vm_demo
{
	namespace desg = rule_design;
	using alloc_t = std::allocator<int>;

	inline std::array<desg::rule_desg_var_base, 8> make_qword(std::uint64_t value_v)
	{
		std::array<desg::rule_desg_var_base, 8> bytes{};

		for (std::size_t i = 0; i < bytes.size(); i++)
			bytes[i] = static_cast<desg::rule_desg_var_base>((value_v >> (i * 8)) & 0xff);

		return bytes;
	}

	inline std::optional<std::uint64_t> read_qword(
		desg::rule_desg<>& vm,
		desg::rule_desg_var_mgr_index mgr_index_v,
		desg::rule_desg_var_index var_index_v)
	{
		auto var_mgr_res = vm.get_owner().get_owner().get_var_set().get(mgr_index_v);

		if (var_mgr_res.has_value() == false)
			return std::nullopt;

		auto var_res = var_mgr_res.value().get().get_content().get(var_index_v);

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

	inline bool load_text(
		desg::rule_desg<>& vm,
		std::string_view source_v,
		desg::rule_desg_text_error<alloc_t>& error_v)
	{
		if (vm.load_text(source_v, error_v) == true)
			return true;

		std::printf(
			"load_text failed: line=%zu column=%zu code=%s message=%s\n",
			error_v.line,
			error_v.column,
			std::string(desg::rule_design_error_text(error_v.code)).c_str(),
			error_v.message.c_str());

		return false;
	}

	inline bool expect(const char* name_v, bool ok_v)
	{
		std::printf("%s %s\n", name_v, ok_v ? "PASS" : "FAIL");
		return ok_v;
	}
}
