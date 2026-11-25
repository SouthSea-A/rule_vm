#pragma once
#include "kernel/kernel_namespace.h"

#include <array>
#include <charconv>
#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include "rule_design_alias.h"
#include "rule_design_error.h"

namespace rule_design
{
	template<typename alloc_t>
	struct rule_desg_text_alloc
	{
	public:
		using alloc_traits = typename std::allocator_traits<alloc_t>;
		using char_alloc = typename alloc_traits::template rebind_alloc<char>;
		using byte_alloc = typename alloc_traits::template rebind_alloc<rule_kit::rule_kit_var_base>;
		using view_alloc = typename alloc_traits::template rebind_alloc<std::string_view>;
		using op_alloc = typename alloc_traits::template rebind_alloc<rule_kit::rule_kit_cmd_op_sgl>;
		using string_t = std::basic_string<char, std::char_traits<char>, char_alloc>;
		using bytes_t = std::vector<rule_kit::rule_kit_var_base, byte_alloc>;
		using views_t = std::vector<std::string_view, view_alloc>;
		using ops_t = std::vector<rule_kit::rule_kit_cmd_op_sgl, op_alloc>;
	};

	template<typename alloc_t>
	struct rule_desg_text_error
	{
	public:
		using alloc_info = rule_desg_text_alloc<alloc_t>;
		using string_t = typename alloc_info::string_t;
	public:
		std::size_t line = 0;
		std::size_t column = 0;
		rule_design_error code = rule_design_error::none;
		string_t message;
	public:
		explicit rule_desg_text_error(const alloc_t& alloc_v = alloc_t()) :
			message(typename alloc_info::char_alloc(alloc_v))
		{
		}
	};

	inline bool rule_desg_text_utf8_valid(std::string_view text_v, std::size_t& bad_offset_save)
	{
		std::size_t i = 0;

		while (i < text_v.size())
		{
			const unsigned char byte_v = static_cast<unsigned char>(text_v[i]);
			std::size_t length_v = 0;

			if (byte_v < 0x80) { i++; continue; }
			if (byte_v >= 0xC2 && byte_v <= 0xDF) length_v = 2;
			else if (byte_v >= 0xE0 && byte_v <= 0xEF) length_v = 3;
			else if (byte_v >= 0xF0 && byte_v <= 0xF4) length_v = 4;
			else { bad_offset_save = i; return false; }

			if (i + length_v > text_v.size()) { bad_offset_save = i; return false; }

			for (std::size_t j = 1; j < length_v; j++)
			{
				const unsigned char cont_v = static_cast<unsigned char>(text_v[i + j]);
				if (cont_v < 0x80 || cont_v > 0xBF) { bad_offset_save = i; return false; }
			}

			if (length_v == 3)
			{
				const unsigned char second_v = static_cast<unsigned char>(text_v[i + 1]);
				if ((byte_v == 0xE0 && second_v < 0xA0) || (byte_v == 0xED && second_v >= 0xA0)) { bad_offset_save = i; return false; }
			}

			if (length_v == 4)
			{
				const unsigned char second_v = static_cast<unsigned char>(text_v[i + 1]);
				if ((byte_v == 0xF0 && second_v < 0x90) || (byte_v == 0xF4 && second_v > 0x8F)) { bad_offset_save = i; return false; }
			}

			i += length_v;
		}

		return true;
	}

	template<typename alloc_t>
	struct rule_desg_text_var
	{
	public:
		using alloc_info = rule_desg_text_alloc<alloc_t>;
		using string_t = typename alloc_info::string_t;
		using bytes_t = typename alloc_info::bytes_t;
	public:
		string_t name;
		rule_kit::rule_kit_var_mgr_index mgr = 0;
		rule_kit::rule_kit_var_index index = 0;
		rule_kit::rule_kit_var_type type = 0;
		rule_kit::rule_kit_var_pos_index begin = 0;
		rule_kit::rule_kit_var_read_width width = 0;
		bytes_t bytes;
	public:
		explicit rule_desg_text_var(const alloc_t& alloc_v = alloc_t()) :
			name(typename alloc_info::char_alloc(alloc_v)),
			bytes(typename alloc_info::byte_alloc(alloc_v))
		{
		}
	};

	template<typename alloc_t>
	struct rule_desg_text_cmd
	{
	public:
		using alloc_info = rule_desg_text_alloc<alloc_t>;
		using string_t = typename alloc_info::string_t;
		using ops_t = typename alloc_info::ops_t;
	public:
		rule_kit::rule_kit_cmd_mgr_index mgr = 0;
		rule_kit::rule_kit_cmd_index index = 0;
		string_t mnemonic;
		ops_t ops;
	public:
		explicit rule_desg_text_cmd(const alloc_t& alloc_v = alloc_t()) :
			mnemonic(typename alloc_info::char_alloc(alloc_v)),
			ops(typename alloc_info::op_alloc(alloc_v))
		{
		}
	};

	template<typename alloc_t>
	struct rule_desg_text_program
	{
	public:
		using alloc_info = rule_desg_text_alloc<alloc_t>;
		using var_t = rule_desg_text_var<alloc_t>;
		using cmd_t = rule_desg_text_cmd<alloc_t>;
		using alloc_var = typename std::allocator_traits<alloc_t>::template rebind_alloc<var_t>;
		using alloc_cmd = typename std::allocator_traits<alloc_t>::template rebind_alloc<cmd_t>;
		using vars_t = std::vector<var_t, alloc_var>;
		using cmds_t = std::vector<cmd_t, alloc_cmd>;
		using alloc_var_mgr = typename std::allocator_traits<alloc_t>::template rebind_alloc<rule_kit::rule_kit_var_mgr_index>;
		using alloc_cmd_mgr = typename std::allocator_traits<alloc_t>::template rebind_alloc<rule_kit::rule_kit_cmd_mgr_index>;
		using var_blocks_t = std::vector<rule_kit::rule_kit_var_mgr_index, alloc_var_mgr>;
		using cmd_blocks_t = std::vector<rule_kit::rule_kit_cmd_mgr_index, alloc_cmd_mgr>;
	public:
		vars_t vars;
		cmds_t cmds;
		var_blocks_t var_blocks;
		cmd_blocks_t cmd_blocks;
		rule_kit::rule_kit_cmd_mgr_index start_mgr = 0;
		rule_kit::rule_kit_cmd_index start_index = 0;
	public:
		explicit rule_desg_text_program(const alloc_t& alloc_v = alloc_t()) :
			vars(alloc_var(alloc_v)),
			cmds(alloc_cmd(alloc_v)),
			var_blocks(alloc_var_mgr(alloc_v)),
			cmd_blocks(alloc_cmd_mgr(alloc_v))
		{
		}
	};
}
