#pragma once
#include "kernel/kernel_namespace.h"

#include "rule_design_text_data_def.h"

namespace rule_design
{
	template<typename alloc_t>
	class rule_desg_text_parser
	{
	private:
		using alloc_info = rule_desg_text_alloc<alloc_t>;
		using string_t = typename alloc_info::string_t;
		using bytes_t = typename alloc_info::bytes_t;
		using views_t = typename alloc_info::views_t;
		using ops_t = typename alloc_info::ops_t;
		using var_t = rule_desg_text_var<alloc_t>;
		using cmd_t = rule_desg_text_cmd<alloc_t>;
		using program_t = rule_desg_text_program<alloc_t>;
		using error_t = rule_desg_text_error<alloc_t>;
		using label_value_t = std::pair<rule_kit::rule_kit_cmd_mgr_index, rule_kit::rule_kit_cmd_index>;
		using alloc_label = typename std::allocator_traits<alloc_t>::template rebind_alloc<std::pair<const string_t, label_value_t>>;
		using alloc_next = typename std::allocator_traits<alloc_t>::template rebind_alloc<std::pair<const rule_kit::rule_kit_cmd_mgr_index, rule_kit::rule_kit_cmd_index>>;
		using alloc_var = typename std::allocator_traits<alloc_t>::template rebind_alloc<std::pair<const string_t, rule_kit::rule_kit_var_tgt>>;
		using label_map_t = std::map<string_t, label_value_t, std::less<string_t>, alloc_label>;
		using next_map_t = std::map<rule_kit::rule_kit_cmd_mgr_index, rule_kit::rule_kit_cmd_index, std::less<>, alloc_next>;
		using var_map_t = std::map<string_t, rule_kit::rule_kit_var_tgt, std::less<string_t>, alloc_var>;
	private:
		struct pending_target
		{
		public:
			std::size_t cmd_pos = 0;
			std::size_t op_pos = 0;
			string_t name;
		public:
			explicit pending_target(const alloc_t& alloc_v) : name(typename alloc_info::char_alloc(alloc_v)) {}
		};
		using alloc_pending = typename std::allocator_traits<alloc_t>::template rebind_alloc<pending_target>;
		using pending_list_t = std::vector<pending_target, alloc_pending>;
	private:
		static constexpr rule_kit::rule_kit_var_mgr_index hidden_var_mgr = std::numeric_limits<rule_kit::rule_kit_var_mgr_index>::max() - 1;
	private:
		enum class block_kind : std::uint8_t
		{
			none,
			var_block,
			cmd_block,
		};
	private:
		alloc_t allocator;
		std::size_t line_no = 0;
		rule_kit::rule_kit_var_index hidden_var_next = 0;
		rule_kit::rule_kit_cmd_mgr_index current_mgr = 0;
		rule_kit::rule_kit_cmd_mgr_index block_mgr = 0;
		block_kind block_current = block_kind::none;
		block_kind block_pending = block_kind::none;
		bool start_seen = false;
		rule_kit::rule_kit_cmd_mgr_index start_mgr = 0;
		next_map_t next_index;
		label_map_t labels;
		var_map_t vars;
		pending_list_t pending;
		program_t program;
		error_t* error = nullptr;
	private:
		bool fail(std::string_view message_v)
		{
			if (this->error != nullptr)
			{
				rule_design_error code_v = rule_design_error::text__syntax;

				if (message_v.find("标签") != std::string_view::npos)
					code_v = rule_design_error::text__label;
				else if (message_v.find("start") != std::string_view::npos)
					code_v = rule_design_error::text__start;
				else if (message_v.find("块") != std::string_view::npos || message_v.find("defvar") != std::string_view::npos || message_v.find("defcmd") != std::string_view::npos)
					code_v = rule_design_error::text__block;
				else if (message_v.find("var") != std::string_view::npos)
					code_v = rule_design_error::text__var;
				else if (message_v.find("指令") != std::string_view::npos)
					code_v = rule_design_error::text__cmd;

				this->error->line = this->line_no;
				this->error->column = 1;
				this->error->code = code_v;
				this->error->message.assign(message_v.begin(), message_v.end());
			}
			return false;
		}
		static std::string_view trim(std::string_view text_v)
		{
			while (!text_v.empty() && (text_v.front() == ' ' || text_v.front() == '\t' || text_v.front() == '\r'))
				text_v.remove_prefix(1);
			while (!text_v.empty() && (text_v.back() == ' ' || text_v.back() == '\t' || text_v.back() == '\r'))
				text_v.remove_suffix(1);
			return text_v;
		}
		static std::string_view strip_comment(std::string_view text_v)
		{
			bool in_char = false;
			for (std::size_t i = 0; i < text_v.size(); i++)
			{
				if (text_v[i] == '\'') in_char = !in_char;
				else if (in_char == false && (text_v[i] == ';' || text_v[i] == '#'))
					return text_v.substr(0, i);
			}
			return text_v;
		}
		views_t split(std::string_view text_v, char sep_v) const
		{
			views_t parts(typename alloc_info::view_alloc(this->allocator));
			std::size_t begin_v = 0;
			for (std::size_t i = 0; i <= text_v.size(); i++)
			{
				if (i == text_v.size() || text_v[i] == sep_v)
				{
					parts.push_back(trim(text_v.substr(begin_v, i - begin_v)));
					begin_v = i + 1;
				}
			}
			return parts;
		}
		views_t split_words(std::string_view text_v) const
		{
			views_t words(typename alloc_info::view_alloc(this->allocator));
			std::size_t begin_v = 0;
			bool in_quote = false;
			bool escape = false;
			for (std::size_t i = 0; i <= text_v.size(); i++)
			{
				const char char_v = (i == text_v.size()) ? ' ' : text_v[i];
				if (escape == true) escape = false;
				else if (in_quote == true && char_v == '\\') escape = true;
				else if (char_v == '"') in_quote = !in_quote;
				else if (char_v == ' ' && in_quote == false)
				{
					if (i > begin_v) words.push_back(trim(text_v.substr(begin_v, i - begin_v)));
					begin_v = i + 1;
				}
			}
			return words;
		}
		static bool parse_u64(std::string_view token_v, std::uint64_t& value_save)
		{
			token_v = trim(token_v);
			if (token_v.size() == 3 && token_v.front() == '\'' && token_v.back() == '\'')
			{
				value_save = static_cast<std::uint8_t>(token_v[1]);
				return true;
			}
			if (token_v.size() > 2 && token_v[0] == '0' && (token_v[1] == 'x' || token_v[1] == 'X'))
			{
				const auto result_v = std::from_chars(token_v.data() + 2, token_v.data() + token_v.size(), value_save, 16);
				return result_v.ec == std::errc() && result_v.ptr == token_v.data() + token_v.size();
			}
			const auto result_v = std::from_chars(token_v.data(), token_v.data() + token_v.size(), value_save, 10);
			return result_v.ec == std::errc() && result_v.ptr == token_v.data() + token_v.size();
		}
		bool decode_string(std::string_view token_v, bytes_t& bytes_save) const
		{
			if (token_v.size() < 2 || token_v.front() != '"' || token_v.back() != '"')
				return false;
			bytes_save.clear();
			for (std::size_t i = 1; i + 1 < token_v.size(); i++)
			{
				char char_v = token_v[i];
				if (char_v == '\\' && i + 2 < token_v.size())
				{
					const char esc_v = token_v[++i];
					switch (esc_v)
					{
					case 'n': char_v = '\n'; break;
					case 't': char_v = '\t'; break;
					case 'r': char_v = '\r'; break;
					case '0': char_v = '\0'; break;
					case '\\': char_v = '\\'; break;
					case '"': char_v = '"'; break;
					case 'x':
					{
						std::uint64_t value_v = 0;
						if (i + 2 >= token_v.size() || parse_u64(token_v.substr(i + 1, 2), value_v) == false || value_v > 0xFF)
							return false;
						char_v = static_cast<char>(value_v);
						i += 2;
						break;
					}
					default: return false;
					}
				}
				bytes_save.push_back(static_cast<rule_kit::rule_kit_var_base>(static_cast<unsigned char>(char_v)));
			}
			return true;
		}
		static bool parse_var_handle(std::string_view token_v, rule_kit::rule_kit_var_handle_type& handle_save)
		{
			if (token_v == "var_type" || token_v == "type") { handle_save = rule_kit::rule_kit_var_handle_type::var_type; return true; }
			if (token_v == "var_self" || token_v == "self") { handle_save = rule_kit::rule_kit_var_handle_type::var_self; return true; }
			if (token_v == "var_tgt" || token_v == "tgt") { handle_save = rule_kit::rule_kit_var_handle_type::var_tgt; return true; }
			if (token_v == "var_val" || token_v == "val") { handle_save = rule_kit::rule_kit_var_handle_type::var_val; return true; }
			if (token_v == "var_ptr" || token_v == "ptr") { handle_save = rule_kit::rule_kit_var_handle_type::var_ptr; return true; }
			if (token_v == "var_del" || token_v == "del") { handle_save = rule_kit::rule_kit_var_handle_type::var_del; return true; }
			if (token_v == "invalid") { handle_save = rule_kit::rule_kit_var_handle_type::invalid; return true; }
			return false;
		}
		static bool parse_cmd_handle(std::string_view token_v, rule_kit::rule_kit_cmd_handle_type& handle_save)
		{
			if (token_v == "cmd_type" || token_v == "type") { handle_save = rule_kit::rule_kit_cmd_handle_type::cmd_type; return true; }
			if (token_v == "cmd_self" || token_v == "self") { handle_save = rule_kit::rule_kit_cmd_handle_type::cmd_self; return true; }
			if (token_v == "cmd_tgt" || token_v == "tgt") { handle_save = rule_kit::rule_kit_cmd_handle_type::cmd_tgt; return true; }
			if (token_v == "cmd_val" || token_v == "val") { handle_save = rule_kit::rule_kit_cmd_handle_type::cmd_val; return true; }
			if (token_v == "cmd_ptr" || token_v == "ptr") { handle_save = rule_kit::rule_kit_cmd_handle_type::cmd_ptr; return true; }
			if (token_v == "cmd_del" || token_v == "del") { handle_save = rule_kit::rule_kit_cmd_handle_type::cmd_del; return true; }
			if (token_v == "invalid") { handle_save = rule_kit::rule_kit_cmd_handle_type::invalid; return true; }
			return false;
		}
		bool parse_var_target(std::string_view token_v, rule_kit::rule_kit_var_handle_type handle_v, rule_kit::rule_kit_var_tgt& target_save)
		{
			views_t parts(typename alloc_info::view_alloc(this->allocator));
			std::size_t begin_v = 0;
			for (std::size_t i = 0; i <= token_v.size(); i++)
			{
				if (i == token_v.size() || token_v[i] == ':')
				{
					parts.push_back(token_v.substr(begin_v, i - begin_v));
					begin_v = i + 1;
				}
			}
			if (parts.size() != 5 && parts.size() != 6)
				return false;
			std::size_t offset_v = 0;
			if (parts.size() == 6)
			{
				if (parse_var_handle(parts[0], handle_v) == false)
					return false;
				offset_v = 1;
			}
			std::uint64_t values_v[5] = {};
			for (std::size_t i = 0; i < 5; i++)
			{
				if (parse_u64(parts[i + offset_v], values_v[i]) == false)
					return false;
			}
			target_save = rule_kit::rule_kit_var_tgt(handle_v, static_cast<rule_kit::rule_kit_var_mgr_index>(values_v[0]), static_cast<rule_kit::rule_kit_var_index>(values_v[1]), static_cast<rule_kit::rule_kit_var_type>(values_v[2]), static_cast<rule_kit::rule_kit_var_pos_index>(values_v[3]), static_cast<rule_kit::rule_kit_var_read_width>(values_v[4]));
			return true;
		}
		bool parse_cmd_target(std::string_view token_v, rule_kit::rule_kit_cmd_handle_type handle_v, rule_kit::rule_kit_cmd_tgt& target_save)
		{
			views_t parts(typename alloc_info::view_alloc(this->allocator));
			std::size_t begin_v = 0;
			for (std::size_t i = 0; i <= token_v.size(); i++)
			{
				if (i == token_v.size() || token_v[i] == ':')
				{
					parts.push_back(token_v.substr(begin_v, i - begin_v));
					begin_v = i + 1;
				}
			}
			if (parts.size() != 5 && parts.size() != 6)
				return false;
			std::size_t offset_v = 0;
			if (parts.size() == 6)
			{
				if (parse_cmd_handle(parts[0], handle_v) == false)
					return false;
				offset_v = 1;
			}
			std::uint64_t values_v[5] = {};
			for (std::size_t i = 0; i < 5; i++)
			{
				if (parse_u64(parts[i + offset_v], values_v[i]) == false)
					return false;
			}
			target_save = rule_kit::rule_kit_cmd_tgt(handle_v, static_cast<rule_kit::rule_kit_var_cmd_mgr_index>(values_v[0]), static_cast<rule_kit::rule_kit_var_cmd_index>(values_v[1]), static_cast<rule_kit::rule_kit_var_cmd_type>(values_v[2]), static_cast<rule_kit::rule_kit_var_cmd_begin_pos>(values_v[3]), static_cast<rule_kit::rule_kit_cmd_op_width>(values_v[4]));
			return true;
		}
		static bool imm_target_to_u64(const rule_kit::rule_kit_imm_tgt& imm_v, std::uint64_t& value_save)
		{
			std::size_t width_v = 0;

			switch (imm_v.get_type())
			{
			case rule_kit::rule_kit_imm_handle_type::byte: width_v = 1; break;
			case rule_kit::rule_kit_imm_handle_type::word: width_v = 2; break;
			case rule_kit::rule_kit_imm_handle_type::dword: width_v = 4; break;
			case rule_kit::rule_kit_imm_handle_type::qword: width_v = 8; break;
			default: return false;
			}

			value_save = 0;
			for (std::size_t i = 0; i < width_v; i++)
				value_save |= static_cast<std::uint64_t>(imm_v.get_data()[i]) << (i * 8);

			return true;
		}
		static void make_cmd_type_imm(const rule_kit::rule_kit_imm_tgt& imm_v, rule_kit::rule_kit_cmd_op_sgl& op_save)
		{
			std::uint64_t value_v = 0;

			if (imm_target_to_u64(imm_v, value_v) == false)
				return;

			std::array<rule_kit::rule_kit_cmd_var_base_type, rule_kit::rule_kit_imm_width_byte_max> data_v{};
			for (std::size_t i = 0; i < 8; i++)
				data_v[7 - i] = static_cast<rule_kit::rule_kit_cmd_var_base_type>((value_v >> (i * 8)) & 0xff);

			op_save.emplace<rule_kit::rule_kit_imm_tgt>(rule_kit::rule_kit_imm_handle_type::qword, data_v);
			return;
		}
		static bool parse_imm(std::string_view token_v, rule_kit::rule_kit_cmd_op_sgl& op_save)
		{
			std::string_view value_v = token_v;
			std::size_t width_v = 8;
			const std::size_t colon_v = token_v.rfind(':');
			if (colon_v != std::string_view::npos)
			{
				std::uint64_t width_u = 0;
				if (parse_u64(token_v.substr(colon_v + 1), width_u) == true)
				{
					width_v = static_cast<std::size_t>(width_u);
					value_v = token_v.substr(0, colon_v);
				}
			}
			if (width_v != 1 && width_v != 2 && width_v != 4 && width_v != 8)
				return false;
			std::uint64_t value_u = 0;
			if (parse_u64(value_v, value_u) == false)
				return false;
			if (width_v < 8 && value_u >= (std::uint64_t(1) << (width_v * 8)))
				return false;
			std::array<rule_kit::rule_kit_cmd_var_base_type, rule_kit::rule_kit_imm_width_byte_max> data_v{};
			for (std::size_t i = 0; i < width_v; i++)
			{
				data_v[i] = static_cast<rule_kit::rule_kit_cmd_var_base_type>((value_u >> (i * 8)) & 0xff);
			}
			rule_kit::rule_kit_imm_handle_type type_v = rule_kit::rule_kit_imm_handle_type::qword;
			if (width_v == 1) type_v = rule_kit::rule_kit_imm_handle_type::byte;
			else if (width_v == 2) type_v = rule_kit::rule_kit_imm_handle_type::word;
			else if (width_v == 4) type_v = rule_kit::rule_kit_imm_handle_type::dword;
			op_save.emplace<rule_kit::rule_kit_imm_tgt>(type_v, data_v);
			return true;
		}
		static bool parse_compare(std::string_view token_v, rule_kit::rule_kit_cmd_op_sgl& op_save)
		{
			if (token_v == "eq")
				return parse_imm("0:1", op_save);
			if (token_v == "gt")
				return parse_imm("1:1", op_save);
			if (token_v == "ge")
				return parse_imm("2:1", op_save);
			if (token_v == "lt")
				return parse_imm("3:1", op_save);
			if (token_v == "le")
				return parse_imm("4:1", op_save);
			if (token_v == "uncond" || token_v == "unconditional")
				return parse_imm("5:1", op_save);
			return parse_imm(token_v, op_save);
		}
		bool parse_var_operand(std::string_view token_v, rule_kit::rule_kit_cmd_op_sgl& op_save, rule_kit::rule_kit_var_read_width pad_width_v = 0)
		{
			if (token_v.size() > 3 && token_v.front() == 'v' && token_v[1] == '(' && token_v.back() == ')')
			{
				rule_kit::rule_kit_var_tgt target_v;
				if (parse_var_target(token_v.substr(2, token_v.size() - 3), rule_kit::rule_kit_var_handle_type::var_val, target_v) == false)
					return false;
				op_save.emplace<rule_kit::rule_kit_var_tgt>(target_v);
				return true;
			}
			if (token_v.size() >= 2 && token_v.front() == '"' && token_v.back() == '"')
			{
				bytes_t bytes_v(typename alloc_info::byte_alloc(this->allocator));
				if (decode_string(token_v, bytes_v) == false || bytes_v.empty() == true)
					return false;
				if (bytes_v.size() < static_cast<std::size_t>(pad_width_v)) bytes_v.resize(static_cast<std::size_t>(pad_width_v), rule_kit::rule_kit_var_base(0));

				var_t hidden_v(this->allocator);
				hidden_v.name.assign(token_v.begin(), token_v.end());
				hidden_v.mgr = hidden_var_mgr;
				hidden_v.index = this->hidden_var_next++;
				hidden_v.width = static_cast<rule_kit::rule_kit_var_read_width>(bytes_v.size());
				hidden_v.bytes = std::move(bytes_v);
				this->program.vars.push_back(std::move(hidden_v));
				const var_t& var_v = this->program.vars.back();
				op_save.emplace<rule_kit::rule_kit_var_tgt>(rule_kit::rule_kit_var_tgt(rule_kit::rule_kit_var_handle_type::var_val, var_v.mgr, var_v.index, 0, 0, var_v.width));
				return true;
			}
			const auto it = this->vars.find(string_t(token_v.begin(), token_v.end(), typename alloc_info::char_alloc(this->allocator)));
			if (it == this->vars.end())
				return false;
			op_save.emplace<rule_kit::rule_kit_var_tgt>(it->second);
			return true;
		}
		string_t make_label_key(rule_kit::rule_kit_cmd_mgr_index mgr_v, std::string_view name_v) const
		{
			string_t key_v(typename alloc_info::char_alloc(this->allocator));
			char buffer_v[32] = {};
			const auto result_v = std::to_chars(buffer_v, buffer_v + sizeof(buffer_v) - 1, mgr_v);
			key_v.append(buffer_v, result_v.ptr);
			key_v.push_back(':');
			key_v.append(name_v.begin(), name_v.end());
			return key_v;
		}
		bool parse_label_operand(std::string_view token_v, rule_kit::rule_kit_cmd_op_sgl& op_save, std::size_t cmd_pos_v, std::size_t op_pos_v)
		{
			if (token_v.size() > 3 && token_v.front() == 'c' && token_v[1] == '(' && token_v.back() == ')')
			{
				rule_kit::rule_kit_cmd_tgt target_v;
				if (parse_cmd_target(token_v.substr(2, token_v.size() - 3), rule_kit::rule_kit_cmd_handle_type::cmd_tgt, target_v) == false)
					return false;
				op_save.emplace<rule_kit::rule_kit_cmd_tgt>(target_v);
				return true;
			}
			const string_t name_v = this->make_label_key(this->current_mgr, token_v);
			const auto it = this->labels.find(name_v);
			if (it != this->labels.end())
			{
				op_save.emplace<rule_kit::rule_kit_cmd_tgt>(rule_kit::rule_kit_cmd_tgt(rule_kit::rule_kit_cmd_handle_type::cmd_tgt, it->second.first, it->second.second, 0, 0, 0));
				return true;
			}
			op_save.emplace<rule_kit::rule_kit_cmd_tgt>(rule_kit::rule_kit_cmd_tgt(rule_kit::rule_kit_cmd_handle_type::cmd_tgt, this->current_mgr, 0, 0, 0, 0));
			this->pending.push_back(pending_target(this->allocator));
			this->pending.back().cmd_pos = cmd_pos_v;
			this->pending.back().op_pos = op_pos_v;
			this->pending.back().name = name_v;
			return true;
		}
		bool parse_operand(std::string_view token_v, rule_kit::rule_kit_cmd_op_sgl& op_save, std::size_t cmd_pos_v, std::size_t op_pos_v, rule_kit::rule_kit_var_read_width pad_width_v = 0)
		{
			if (parse_var_operand(token_v, op_save, pad_width_v) == true)
				return true;
			if (token_v.size() > 3 && token_v.front() == 'c' && token_v[1] == '(' && token_v.back() == ')')
			{
				rule_kit::rule_kit_cmd_tgt target_v;
				if (parse_cmd_target(token_v.substr(2, token_v.size() - 3), rule_kit::rule_kit_cmd_handle_type::cmd_val, target_v) == false)
					return false;
				op_save.emplace<rule_kit::rule_kit_cmd_tgt>(target_v);
				return true;
			}
			return parse_imm(token_v, op_save);
		}
		bool emit_cmd(std::string_view mnemonic_v, ops_t ops_v)
		{
			cmd_t cmd_v(this->allocator);
			cmd_v.mgr = this->current_mgr;
			cmd_v.index = this->next_index[this->current_mgr];
			cmd_v.mnemonic.assign(mnemonic_v.begin(), mnemonic_v.end());
			cmd_v.ops = std::move(ops_v);
			this->next_index[this->current_mgr]++;
			this->program.cmds.push_back(std::move(cmd_v));
			return true;
		}
		bool parse_var_line(std::string_view line_v)
		{
			if (this->block_current != block_kind::var_block)
				return fail("var 只能出现在 defvar 块内");

			const views_t words = split_words(line_v);
			if (words.size() < 3)
				return fail("var 语法: var <name> <index>:<type>:<begin>[:<width>] [= bytes|\"string\"]");
			var_t var_v(this->allocator);
			var_v.name.assign(words[1].begin(), words[1].end());
			const views_t fields_v = split(words[2], ':');
			if (fields_v.size() != 4 && fields_v.size() != 5)
				return fail("var 目标必须是 index:type:begin[:width] 或 manager:index:type:begin:width");
			std::uint64_t values_v[5] = {};
			for (std::size_t i = 0; i < fields_v.size(); i++)
			{
				if (parse_u64(fields_v[i], values_v[i]) == false)
					return fail("var 目标字段不是合法整数");
			}

			if (fields_v.size() == 5)
			{
				if (static_cast<rule_kit::rule_kit_var_mgr_index>(values_v[0]) != static_cast<rule_kit::rule_kit_var_mgr_index>(this->block_mgr))
					return fail("var 的 manager 与 defvar 块不一致");

				var_v.index = static_cast<rule_kit::rule_kit_var_index>(values_v[1]);
				var_v.type = static_cast<rule_kit::rule_kit_var_type>(values_v[2]);
				var_v.begin = static_cast<rule_kit::rule_kit_var_pos_index>(values_v[3]);
				var_v.width = static_cast<rule_kit::rule_kit_var_read_width>(values_v[4]);
			}
			else
			{
				var_v.index = static_cast<rule_kit::rule_kit_var_index>(values_v[0]);
				var_v.type = static_cast<rule_kit::rule_kit_var_type>(values_v[1]);
				var_v.begin = static_cast<rule_kit::rule_kit_var_pos_index>(values_v[2]);
				var_v.width = static_cast<rule_kit::rule_kit_var_read_width>(values_v[3]);
			}

			var_v.mgr = static_cast<rule_kit::rule_kit_var_mgr_index>(this->block_mgr);
			if (words.size() > 3)
			{
				if (words[3] != "=")
					return fail("var 初值必须写成 = byte|\"string\" ...");
				for (std::size_t i = 4; i < words.size(); i++)
				{
					if (words[i].size() >= 2 && words[i].front() == '"' && words[i].back() == '"')
					{
						bytes_t string_bytes_v(typename alloc_info::byte_alloc(this->allocator));
						if (decode_string(words[i], string_bytes_v) == false)
							return fail("var 字符串字面量非法");
						var_v.bytes.insert(var_v.bytes.end(), string_bytes_v.begin(), string_bytes_v.end());
					}
					else
					{
						std::uint64_t byte_v = 0;
						if (parse_u64(words[i], byte_v) == false || byte_v > 0xFF)
							return fail("var 初值必须是 0..255 或字符串");
						var_v.bytes.push_back(static_cast<rule_kit::rule_kit_var_base>(byte_v));
					}
				}
			}
			if (var_v.width == 0) var_v.width = static_cast<rule_kit::rule_kit_var_read_width>(var_v.bytes.size());
			if (var_v.width == 0 || var_v.width > (1u << 24))
				return fail("var width 非法");
			if (var_v.bytes.size() > var_v.width)
				return fail("var 初值长度超过 width");
			var_v.bytes.resize(static_cast<std::size_t>(var_v.width), rule_kit::rule_kit_var_base(0));
			if (this->vars.find(var_v.name) != this->vars.end())
				return fail("变量名重复");
			this->vars.emplace(var_v.name, rule_kit::rule_kit_var_tgt(rule_kit::rule_kit_var_handle_type::var_val, var_v.mgr, var_v.index, var_v.type, var_v.begin, var_v.width));
			this->program.vars.push_back(std::move(var_v));
			return true;
		}
		bool parse_alloc_line(std::string_view line_v, bool is_alloc)
		{
			const views_t words = split_words(line_v);
			ops_t ops_v(typename alloc_info::op_alloc(this->allocator));
			if (is_alloc == true)
			{
				if (words.size() != 3)
					return fail("alloc 语法: alloc var|varptr|cmd|cmdptr <mgr>:<index>:<type>:<begin>:<width>");
				rule_kit::rule_kit_cmd_op_sgl op_v;
				if (words[1] == "var" || words[1] == "varptr")
				{
					rule_kit::rule_kit_var_tgt target_v;
					const rule_kit::rule_kit_var_handle_type handle_v = (words[1] == "var") ? rule_kit::rule_kit_var_handle_type::var_val : rule_kit::rule_kit_var_handle_type::var_ptr;
					if (parse_var_target(words[2], handle_v, target_v) == false)
						return fail("alloc var 目标非法");
					op_v.emplace<rule_kit::rule_kit_var_tgt>(target_v);
				}
				else if (words[1] == "cmd" || words[1] == "cmdptr")
				{
					rule_kit::rule_kit_cmd_tgt target_v;
					const rule_kit::rule_kit_cmd_handle_type handle_v = (words[1] == "cmd") ? rule_kit::rule_kit_cmd_handle_type::cmd_val : rule_kit::rule_kit_cmd_handle_type::cmd_ptr;
					if (parse_cmd_target(words[2], handle_v, target_v) == false)
						return fail("alloc cmd 目标非法");
					op_v.emplace<rule_kit::rule_kit_cmd_tgt>(target_v);
				}
				else return fail("alloc 类型只能是 var|varptr|cmd|cmdptr");
				ops_v.push_back(op_v);
				return this->emit_cmd("alloc", std::move(ops_v));
			}
			if (words.size() != 3)
				return fail("dealloc 语法: dealloc var|varptr|cmd|cmdtgt|cmdptr <mgr>:<index>");
			const views_t target_v = split(words[2], ':');
			if (target_v.size() != 2)
				return fail("dealloc 目标必须是 mgr:index");
			std::uint64_t mgr_v = 0, index_v = 0;
			if (parse_u64(target_v[0], mgr_v) == false || parse_u64(target_v[1], index_v) == false)
				return fail("dealloc 目标字段非法");
			rule_kit::rule_kit_cmd_op_sgl op_v;
			if (words[1] == "var" || words[1] == "varptr")
			{
				op_v.emplace<rule_kit::rule_kit_var_tgt>(rule_kit::rule_kit_var_tgt(words[1] == "var" ? rule_kit::rule_kit_var_handle_type::var_val : rule_kit::rule_kit_var_handle_type::var_ptr, static_cast<rule_kit::rule_kit_var_mgr_index>(mgr_v), static_cast<rule_kit::rule_kit_var_index>(index_v), 0, 0, 0));
			}
			else if (words[1] == "cmd" || words[1] == "cmdtgt" || words[1] == "cmdptr")
			{
				rule_kit::rule_kit_cmd_handle_type handle_v = rule_kit::rule_kit_cmd_handle_type::cmd_val;
				if (words[1] == "cmdtgt") handle_v = rule_kit::rule_kit_cmd_handle_type::cmd_tgt;
				else if (words[1] == "cmdptr") handle_v = rule_kit::rule_kit_cmd_handle_type::cmd_ptr;
				op_v.emplace<rule_kit::rule_kit_cmd_tgt>(rule_kit::rule_kit_cmd_tgt(handle_v, static_cast<rule_kit::rule_kit_var_cmd_mgr_index>(mgr_v), static_cast<rule_kit::rule_kit_var_cmd_index>(index_v), 0, 0, 0));
			}
			else return fail("dealloc 类型非法");
			ops_v.push_back(op_v);
			return this->emit_cmd("dealloc", std::move(ops_v));
		}
		bool parse_instruction_line(std::string_view line_v)
		{
			const views_t segments = split(line_v, ',');
			if (segments.empty())
				return true;
			std::string_view first_v = segments[0];
			const std::size_t space_v = first_v.find_first_of(" \t");
			const std::string_view mnemonic_v = (space_v == std::string_view::npos) ? first_v : first_v.substr(0, space_v);
			views_t operands(typename alloc_info::view_alloc(this->allocator));
			if (space_v != std::string_view::npos) operands.push_back(trim(first_v.substr(space_v + 1)));
			for (std::size_t i = 1; i < segments.size(); i++)
			{
				operands.push_back(segments[i]);
			}
			const auto is = [&](std::string_view name_v) { return mnemonic_v == name_v; };
			const std::size_t cmd_pos_v = this->program.cmds.size();
			ops_t ops_v(typename alloc_info::op_alloc(this->allocator));
			if (is("ret"))
			{
				if (operands.empty() == false)
					return fail("ret 不接受操作数");
				return this->emit_cmd("ret", std::move(ops_v));
			}
			if (is("mov"))
			{
				if (operands.size() != 2)
					return fail("mov 需要 2 个操作数");
				rule_kit::rule_kit_cmd_op_sgl dst_v;
				if (parse_operand(operands[0], dst_v, cmd_pos_v, 0) == false)
					return fail("mov 目标非法");
				rule_kit::rule_kit_var_read_width pad_v = 0;
				if (const auto* dst_tgt_v = std::get_if<rule_kit::rule_kit_var_tgt>(&dst_v)) pad_v = dst_tgt_v->get_width();
				rule_kit::rule_kit_cmd_op_sgl src_v;
				if (parse_operand(operands[1], src_v, cmd_pos_v, 1, pad_v) == false)
					return fail("mov 源操作数非法");

				if (const auto* dst_cmd_v = std::get_if<rule_kit::rule_kit_cmd_tgt>(&dst_v))
				{
					if (dst_cmd_v->get_handle_type() == rule_kit::rule_kit_cmd_handle_type::cmd_type)
					{
						if (const auto* src_imm_v = std::get_if<rule_kit::rule_kit_imm_tgt>(&src_v))
							make_cmd_type_imm(*src_imm_v, src_v);
					}
				}

				if (const auto* dst_var_v = std::get_if<rule_kit::rule_kit_var_tgt>(&dst_v))
				{
					if (dst_var_v->get_handle_type() == rule_kit::rule_kit_var_handle_type::var_type)
					{
						if (const auto* src_imm_v = std::get_if<rule_kit::rule_kit_imm_tgt>(&src_v))
							make_cmd_type_imm(*src_imm_v, src_v);
					}
				}

				ops_v.push_back(dst_v); ops_v.push_back(src_v);
				return this->emit_cmd("mov", std::move(ops_v));
			}
			if (is("cmp") || is("add") || is("sub") || is("mul") || is("div"))
			{
				if (operands.size() != 3)
					return fail("该指令需要 3 个操作数");
				rule_kit::rule_kit_cmd_op_sgl dst_v;
				if (parse_operand(operands[0], dst_v, cmd_pos_v, 0) == false)
					return fail("目标操作数非法");
				rule_kit::rule_kit_var_read_width pad_v = 0;
				if (const auto* dst_tgt_v = std::get_if<rule_kit::rule_kit_var_tgt>(&dst_v)) pad_v = dst_tgt_v->get_width();
				ops_v.push_back(dst_v);
				for (std::size_t i = 1; i < operands.size(); i++)
				{
					rule_kit::rule_kit_cmd_op_sgl op_v;
					if (parse_operand(operands[i], op_v, cmd_pos_v, i, pad_v) == false)
						return fail("操作数非法");
					ops_v.push_back(op_v);
				}
				return this->emit_cmd(mnemonic_v, std::move(ops_v));
			}
			if (is("jmp"))
			{
				if (operands.empty() || operands.size() == 2 || operands.size() > 3)
					return fail("jmp 语法: jmp <label>[, cond, cmp]");
				rule_kit::rule_kit_cmd_op_sgl target_v;
				if (parse_label_operand(operands[0], target_v, cmd_pos_v, 0) == false)
					return fail("jmp 目标非法");
				ops_v.push_back(target_v);
				if (operands.size() == 1)
				{
					rule_kit::rule_kit_cmd_op_sgl cond_v, cmp_v;
					if (parse_imm("5:1", cond_v) == false || parse_imm("5:1", cmp_v) == false)
						return fail("jmp 默认条件构造失败");
					ops_v.push_back(cond_v); ops_v.push_back(cmp_v);
				}
				else
				{
					for (std::size_t i = 1; i < 3; i++)
					{
						rule_kit::rule_kit_cmd_op_sgl op_v;

						if (parse_compare(operands[i], op_v) == false && parse_operand(operands[i], op_v, cmd_pos_v, i) == false)
							return fail("jmp 条件/比较值非法");

						ops_v.push_back(op_v);
					}
				}
				return this->emit_cmd("jmp", std::move(ops_v));
			}
			if (is("call"))
			{
				if (operands.size() != 1)
					return fail("call 需要 1 个目标");
				rule_kit::rule_kit_cmd_op_sgl op_v;
				if (parse_label_operand(operands[0], op_v, cmd_pos_v, 0) == false)
					return fail("call 目标非法");
				ops_v.push_back(op_v);
				return this->emit_cmd("call", std::move(ops_v));
			}
			if (is("alloc") || is("dealloc"))
				return this->parse_alloc_line(line_v, is("alloc"));
			if (is("mgr"))
				return fail("defcmd 块内不允许 mgr");
			return fail("未知指令");
		}
		bool open_block(std::string_view line_v, bool is_var_block)
		{
			const views_t words = split_words(line_v);

			if (words.size() != 2 && words.size() != 3)
				return fail(is_var_block ? "语法: defvar <index> {" : "语法: defcmd <index> {");

			if (words.size() == 3 && words[2] != "{" && words[2] != "{}")
				return fail(is_var_block ? "语法: defvar <index> {" : "语法: defcmd <index> {");

			const bool no_brace_v = words.size() == 2;
			const bool empty_block_v = words.size() == 3 && words[2] == "{}";
			std::uint64_t index_v = 0;

			if (parse_u64(words[1], index_v) == false)
				return fail("块索引非法");

			if (is_var_block == true)
			{
				const rule_kit::rule_kit_var_mgr_index mgr_v = static_cast<rule_kit::rule_kit_var_mgr_index>(index_v);

				for (const rule_kit::rule_kit_var_mgr_index exists_v : this->program.var_blocks)
					if (exists_v == mgr_v)
						return fail("defvar 块索引重复");

				this->program.var_blocks.push_back(mgr_v);
				this->block_mgr = static_cast<rule_kit::rule_kit_cmd_mgr_index>(mgr_v);
			}
			else
			{
				const rule_kit::rule_kit_cmd_mgr_index mgr_v = static_cast<rule_kit::rule_kit_cmd_mgr_index>(index_v);

				for (const rule_kit::rule_kit_cmd_mgr_index exists_v : this->program.cmd_blocks)
					if (exists_v == mgr_v)
						return fail("defcmd 块索引重复");

				this->program.cmd_blocks.push_back(mgr_v);
				this->block_mgr = mgr_v;
				this->current_mgr = mgr_v;
				this->next_index.emplace(mgr_v, 0);
			}

			this->block_current = (empty_block_v == true) ? block_kind::none : (is_var_block ? block_kind::var_block : block_kind::cmd_block);
			this->block_pending = (no_brace_v == true) ? (is_var_block ? block_kind::var_block : block_kind::cmd_block) : block_kind::none;
			this->block_current = (no_brace_v == true) ? block_kind::none : this->block_current;
			return true;
		}
		bool parse_top_level_line(std::string_view line_v)
		{
			if (line_v.substr(0, 6) == "defvar")
				return this->open_block(line_v, true);

			if (line_v.substr(0, 6) == "defcmd")
				return this->open_block(line_v, false);

			if (line_v.substr(0, 5) == "start")
			{
				if (this->start_seen == true)
					return fail("start 重复");

				const views_t words = split_words(line_v);

				if (words.size() != 3 || words[1] != "defcmd")
					return fail("语法: start defcmd <index>");

				std::uint64_t index_v = 0;

				if (parse_u64(words[2], index_v) == false)
					return fail("start 块索引非法");

				this->start_seen = true;
				this->start_mgr = static_cast<rule_kit::rule_kit_cmd_mgr_index>(index_v);
				return true;
			}

			return fail("块外只允许 defvar/defcmd/start");
		}
		bool parse_var_block_line(std::string_view line_v)
		{
			if (line_v.substr(0, 3) != "var" || (line_v.size() > 3 && line_v[3] != ' ' && line_v[3] != '\t'))
				return fail("defvar 块内只允许 var 声明");

			return this->parse_var_line(line_v);
		}
		bool parse_cmd_block_line(std::string_view line_v)
		{
			if (line_v.substr(0, 3) == "var" && (line_v.size() == 3 || line_v[3] == ' ' || line_v[3] == '\t'))
				return fail("defcmd 块内不允许 var");

			const std::size_t colon_v = line_v.find(':');
			const std::size_t space_v = line_v.find_first_of(" \t");

			if (colon_v != std::string_view::npos && (space_v == std::string_view::npos || colon_v < space_v))
			{
				const std::string_view label_v = trim(line_v.substr(0, colon_v));

				if (label_v.empty())
					return fail("标签名为空");

				const string_t label_name_v = this->make_label_key(this->current_mgr, label_v);

				if (this->labels.find(label_name_v) != this->labels.end())
					return fail("标签重复");

				this->labels.emplace(label_name_v, label_value_t(this->current_mgr, this->next_index[this->current_mgr]));
				line_v = trim(line_v.substr(colon_v + 1));

				if (line_v.empty())
					return true;
			}

			return this->parse_instruction_line(line_v);
		}
		bool parse_one_line(std::string_view line_v)
		{
			line_v = trim(line_v);

			if (line_v.empty())
				return true;

			if (this->block_pending != block_kind::none)
			{
				if (line_v != "{")
					return fail("块声明后必须单独写 {");

				this->block_current = this->block_pending;
				this->block_pending = block_kind::none;
				return true;
			}

			if (line_v == "}")
			{
				if (this->block_current == block_kind::none)
					return fail("多余的 }");

				this->block_current = block_kind::none;
				return true;
			}

			if (this->block_current == block_kind::none)
				return this->parse_top_level_line(line_v);

			if (this->block_current == block_kind::var_block)
				return this->parse_var_block_line(line_v);

			return this->parse_cmd_block_line(line_v);
		}
	public:
		explicit rule_desg_text_parser(const alloc_t& alloc_v = alloc_t()) :
			allocator(alloc_v),
			next_index(typename next_map_t::key_compare(), alloc_next(alloc_v)),
			labels(typename label_map_t::key_compare(), alloc_label(alloc_v)),
			vars(typename var_map_t::key_compare(), alloc_var(alloc_v)),
			pending(alloc_pending(alloc_v)),
			program(alloc_v)
		{
		}
		bool parse(std::string_view text_v, program_t& program_save, error_t& error_save)
		{
			try
			{
				this->line_no = 0;
				this->current_mgr = 0;
				this->block_mgr = 0;
				this->block_current = block_kind::none;
				this->block_pending = block_kind::none;
				this->start_seen = false;
				this->start_mgr = 0;
				this->hidden_var_next = 0;
				this->next_index.clear();
				this->labels.clear();
				this->vars.clear();
				this->pending.clear();
				this->program = program_t(this->allocator);
				this->error = &error_save;
				std::string_view rest_v = text_v;
				while (true)
				{
					const std::size_t newline_v = rest_v.find('\n');
					const std::string_view line_v = (newline_v == std::string_view::npos) ? rest_v : rest_v.substr(0, newline_v);
					this->line_no++;
					if (this->parse_one_line(strip_comment(line_v)) == false)
						return false;
					if (newline_v == std::string_view::npos)
						break;
					rest_v.remove_prefix(newline_v + 1);
				}
				for (const pending_target& pending_v : this->pending)
				{
					const auto it = this->labels.find(pending_v.name);
					if (it == this->labels.end())
						return fail("未定义标签");
					rule_kit::rule_kit_cmd_op_sgl& op_v = this->program.cmds[pending_v.cmd_pos].ops[pending_v.op_pos];
					if (auto* target_v = std::get_if<rule_kit::rule_kit_cmd_tgt>(&op_v))
						*target_v = rule_kit::rule_kit_cmd_tgt(rule_kit::rule_kit_cmd_handle_type::cmd_tgt, it->second.first, it->second.second, 0, 0, 0);
					else return fail("内部错误: 待解析目标不是 cmd_target");
				}
	
				if (this->block_current != block_kind::none || this->block_pending != block_kind::none)
					return fail("块未闭合");
	
				if (this->start_seen == false)
					return fail("缺少 start defcmd <index>");
	
				bool start_block_exist = false;
				bool start_block_has_cmd = false;
	
				for (const rule_kit::rule_kit_cmd_mgr_index mgr_v : this->program.cmd_blocks)
					if (mgr_v == this->start_mgr)
						start_block_exist = true;
	
				for (const cmd_t& cmd_v : this->program.cmds)
					if (cmd_v.mgr == this->start_mgr)
						start_block_has_cmd = true;
	
				if (start_block_exist == false)
					return fail("start 指向的 defcmd 块不存在");
	
				if (start_block_has_cmd == false)
					return fail("start 指向的 defcmd 块没有指令");
	
				this->program.start_mgr = this->start_mgr;
				this->program.start_index = 0;
				program_save = std::move(this->program);
				return true;
				}
			catch (const rule_design_error_throw&)
			{
				throw;
			}
			catch (const std::bad_alloc&)
			{
				throw rule_design_error_throw(rule_design_error::internal__container, "rule_design | text_parser | 解析容器内存分配失败，错误不可恢复");
			}
			catch (const std::exception& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__parser, std::string("rule_design | text_parser | 解析异常: ") + error_v.what());
			}
			catch (...)
			{
				throw rule_design_error_throw(rule_design_error::internal__parser, "rule_design | text_parser | 未知解析异常，错误不可恢复");
			}
		}
	};
}
