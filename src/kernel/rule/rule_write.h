#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <array>
#include <utility>
#include <type_traits>
#include <functional>
#include <optional>
#include <span>
#include <variant>
#include <algorithm>
#include <initializer_list>
#include "rule_write_data_def.h"
#include "rule_parse.h"
#include "rule_transform.h"
#include "rule_alloc.h"

namespace rule_write
{
	template<typename alloc_rule, typename outer>
	inline bool del_op_target(rdata::rule_int_info& int_info_save, rdata::rule_cmd_act_param<alloc_rule, outer>& run_param_v, const cmd_data::cmd_target& del_cmd_tgt_v)
	{
		cmd_data::cmd_target cmd_tgt_del = del_cmd_tgt_v;

		if (static_cast<cmd_data::cmd_op_width>(cmd_tgt_del.get_width()) == 0) cmd_tgt_del.set_handle_type(cmd_data::op_cmd_type::cmd_tgt);
		else cmd_tgt_del.set_handle_type(cmd_data::op_cmd_type::cmd_val);

		return ralloc::dealloc_rc<alloc_rule, outer>(int_info_save, run_param_v, cmd_tgt_del);
	}

	template<typename alloc_rule, typename outer>
	inline bool del_op_target(rdata::rule_int_info& int_info_save, rdata::rule_cmd_act_param<alloc_rule, outer>& run_param_v, const cmd_data::var_target& del_var_tgt_v)
	{
		cmd_data::var_target var_tgt_del = del_var_tgt_v;

		var_tgt_del.set_handle_type(cmd_data::op_var_type::var_val);

		return ralloc::dealloc_rc<alloc_rule, outer>(int_info_save, run_param_v, var_tgt_del);
	}

	template<typename alloc_rule, typename outer>
	inline bool write_op_span(rdata::rule_int_info& int_info_save, rdata::rule_cmd_act_param<alloc_rule, outer>& run_param_v, parse_data::op<alloc_rule>& write_op_parse_tgt, parse_data::op<alloc_rule>& write_op_parse_src)
	{
		parse_data::op_span span_tgt;
		parse_data::op_span span_src;

		if (rparse::get_op_parse_span_mul<alloc_rule>({ parse_data::op_span_set<alloc_rule>(write_op_parse_tgt, span_tgt), parse_data::op_span_set<alloc_rule>(write_op_parse_src, span_src) }) == false)
		{
			rdata::set_int_info
			(
				int_info_save,
				rdata::rule_int_t::write_get_span_error,
				rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
				rdata::rule_int_info_cont(std::in_place_type<std::monostate>, std::monostate())
			);
			return false;
		}

		auto write_span = [&]<typename write_tgt_t, typename write_src_t>(const std::span<write_tgt_t>&span_write_tgt_v, const std::span<write_src_t>&span_write_src_v) -> bool
		{
			auto get_tgt_t_raw_bytes_size = []<typename match_tgt_t>(const std::span<match_tgt_t>&span_match_tgt_v, std::size_t & tgt_t_size_save) -> bool
			{
				if constexpr (std::is_same_v<match_tgt_t, rdata::rule_var_base_t>)
				{
					tgt_t_size_save = 1;
					return true;
				}
				else if constexpr (std::is_same_v<match_tgt_t, cmd_data::var_cmd_type>)
				{
					tgt_t_size_save = cmd_data::RAW_BYTES_SIZE_VAR_CMD_TYPE;
					return true;
				}
				else if constexpr (std::is_same_v<match_tgt_t, cmd_data::imm_target>)
				{
					tgt_t_size_save = cmd_data::RAW_BYTES_SIZE_IMM_TARGET;
					return true;
				}
				else if constexpr (std::is_same_v<match_tgt_t, cmd_data::var_target>)
				{
					tgt_t_size_save = cmd_data::RAW_BYTES_SIZE_VAR_TARGET;
					return true;
				}
				else if constexpr (std::is_same_v<match_tgt_t, cmd_data::cmd_target>)
				{
					tgt_t_size_save = cmd_data::RAW_BYTES_SIZE_CMD_TARGET;
					return true;
				}
				else if constexpr (std::is_same_v<match_tgt_t, cmd_data::cmd_op_single>)
				{
					tgt_t_size_save = cmd_data::RAW_BYTES_SIZE_CMD_OP;
					return true;
				}
				else
				{
					tgt_t_size_save = 0;
					return false;
				}
			};

			auto size_t_match = [&]<typename match_tgt_t, typename match_src_t>(const std::span<match_tgt_t>&span_match_tgt_v, const std::span<match_src_t>&span_match_src_v, std::size_t & loop_len_save, std::size_t & raw_size_save) -> bool
			{
				if (span_match_src_v.size_bytes() == 0 && span_match_tgt_v.size_bytes() == 0)
				{
					raw_size_save = 0;
					loop_len_save = 0;
					return true;
				}

				if constexpr (std::is_same_v<match_tgt_t, match_src_t>)
				{
					if (get_tgt_t_raw_bytes_size(span_match_tgt_v, raw_size_save) == false)
					{
						raw_size_save = 0;
						loop_len_save = 0;

						rdata::set_int_info
						(
							int_info_save,
							rdata::rule_int_t::write_tgt_src_t_no_match,
							rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
							rdata::rule_int_info_cont(std::in_place_type<std::monostate>, std::monostate())
						);
						return false;
					}

					if (span_match_src_v.size() < span_match_tgt_v.size())
					{
						raw_size_save = 0;
						loop_len_save = 0;

						rdata::set_int_info
						(
							int_info_save,
							rdata::rule_int_t::write_span_src_size_no_match,
							rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
							rdata::rule_int_info_cont(std::in_place_type<std::monostate>, std::monostate())
						);
						return false;
					}

					loop_len_save = span_match_tgt_v.size();
					return true;
				}
				else if constexpr (std::is_same_v<match_tgt_t, rdata::rule_var_base_t> && std::is_same_v<match_src_t, rdata::rule_var_base_t> == false)
				{
					if (get_tgt_t_raw_bytes_size(span_match_src_v, raw_size_save) == false)
					{
						raw_size_save = 0;
						loop_len_save = 0;

						rdata::set_int_info
						(
							int_info_save,
							rdata::rule_int_t::write_src_t_no_match,
							rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
							rdata::rule_int_info_cont(std::in_place_type<std::monostate>, std::monostate())
						);
						return false;
					}

					if (span_match_tgt_v.size_bytes() < raw_size_save)
					{
						raw_size_save = 0;
						loop_len_save = 0;

						rdata::set_int_info
						(
							int_info_save,
							rdata::rule_int_t::write_span_tgt_size_no_match,
							rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
							rdata::rule_int_info_cont(std::in_place_type<std::monostate>, std::monostate())
						);
						return false;
					}

					loop_len_save = std::min({ span_match_tgt_v.size_bytes() / raw_size_save , span_match_src_v.size() });
					return true;
				}
				else if constexpr (std::is_same_v<match_src_t, rdata::rule_var_base_t> && std::is_same_v<match_tgt_t, rdata::rule_var_base_t> == false)
				{
					if (get_tgt_t_raw_bytes_size(span_match_tgt_v, raw_size_save) == false)
					{
						raw_size_save = 0;
						loop_len_save = 0;

						rdata::set_int_info
						(
							int_info_save,
							rdata::rule_int_t::write_tgt_t_no_match,
							rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
							rdata::rule_int_info_cont(std::in_place_type<std::monostate>, std::monostate())
						);
						return false;
					}

					if (span_match_src_v.size_bytes() < raw_size_save)
					{
						raw_size_save = 0;
						loop_len_save = 0;

						rdata::set_int_info
						(
							int_info_save,
							rdata::rule_int_t::write_span_src_size_no_match,
							rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
							rdata::rule_int_info_cont(std::in_place_type<std::monostate>, std::monostate())
						);
						return false;
					}

					loop_len_save = std::min({ span_match_src_v.size_bytes() / raw_size_save , span_match_tgt_v.size() });
					return true;
				}
				else
				{
					rdata::set_int_info
					(
						int_info_save,
						rdata::rule_int_t::write_tgt_src_t_no_match,
						rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
						rdata::rule_int_info_cont(std::in_place_type<std::monostate>, std::monostate())
					);
					loop_len_save = 0;
					return false;
				}
			};

			std::size_t write_loop_len = 0;
			std::size_t write_loop_index_current = 0;
			std::size_t tgt_t_to_raw_size = 0;

			if (size_t_match(span_write_tgt_v, span_write_src_v, write_loop_len, tgt_t_to_raw_size) == false)
				return false;

			if (write_loop_len == 0)
				return true;

			else if constexpr (std::is_same_v<write_tgt_t, rdata::rule_var_base_t> && std::is_same_v<write_src_t, rdata::rule_var_base_t> == false)
			{
				std::array<rdata::rule_var_base_t, cmd_data::RAW_BYTES_SIZE_CMD_OP> write_tgt_precheck_buf{};

				for (std::size_t write_precheck_index = 0; write_precheck_index < write_loop_len; write_precheck_index++)
				{
					std::span<rdata::rule_var_base_t> write_tgt_precheck_span(write_tgt_precheck_buf.data(), tgt_t_to_raw_size);

					if (trans::transform_tgt_to_raw_bytes(int_info_save, span_write_src_v[write_precheck_index], write_tgt_precheck_span) == false)
						return false;
				}

				do
				{
					write_src_t& write_src_transform_ref = span_write_src_v[write_loop_index_current];
					std::span<rdata::rule_var_base_t> write_tgt_span_transform = span_write_tgt_v.subspan(write_loop_index_current * tgt_t_to_raw_size, tgt_t_to_raw_size);

					if (trans::transform_tgt_to_raw_bytes(int_info_save, write_src_transform_ref, write_tgt_span_transform) == false)
						return false;

				} while (++write_loop_index_current < write_loop_len);

				return true;
			}
			else if constexpr (std::is_same_v<write_src_t, rdata::rule_var_base_t> && std::is_same_v<write_tgt_t, rdata::rule_var_base_t> == false)
			{
				for (std::size_t write_precheck_index = 0; write_precheck_index < write_loop_len; write_precheck_index++)
				{
					write_tgt_t write_tgt_precheck{};
					std::span<rdata::rule_var_base_t> write_src_span_precheck = span_write_src_v.subspan(write_precheck_index * tgt_t_to_raw_size, tgt_t_to_raw_size);

					if (trans::transform_raw_bytes_to(int_info_save, write_src_span_precheck, write_tgt_precheck) == false)
						return false;
				}

				do
				{
					write_tgt_t write_tgt_transform;
					std::span<rdata::rule_var_base_t> write_src_span_transform = span_write_src_v.subspan(write_loop_index_current * tgt_t_to_raw_size, tgt_t_to_raw_size);

					if (trans::transform_raw_bytes_to(int_info_save, write_src_span_transform, write_tgt_transform) == false)
						return false;

					span_write_tgt_v[write_loop_index_current] = write_tgt_transform;

				} while (++write_loop_index_current < write_loop_len);

				return true;
			}
			else
			{
				std::memmove(span_write_tgt_v.data(), span_write_src_v.data(), span_write_tgt_v.size_bytes());
				return true;
			}

			return false;
		};

		auto handle_op_parse_span = [&]<typename write_tgt_parse_t, typename write_src_parse_t>(write_tgt_parse_t & tgt, write_src_parse_t & src) -> bool
		{
			if constexpr (std::is_same_v<write_tgt_parse_t, parse_data::op_del>)
			{
				return std::visit
				(
					[&]<typename del_tgt_t>(del_tgt_t & del_tgt_ref) -> bool
				    {
					    if constexpr (std::is_same_v<std::decay_t<del_tgt_t>, std::monostate>)
					    {
						    rdata::set_int_info
						    (
							    int_info_save,
							    rdata::rule_int_t::write_tgt_src_t_no_match,
							    rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
							    rdata::rule_int_info_cont(std::in_place_type<std::monostate>, std::monostate())
						    );
						    return false;
					    }
					    else
					    {
						    return del_op_target<alloc_rule>(int_info_save, run_param_v, del_tgt_ref.get());
					    }
				    },
					tgt.tgt
				);
			}
			else if constexpr (std::is_same_v<write_tgt_parse_t, std::monostate> || std::is_same_v<write_src_parse_t, std::monostate> || std::is_same_v<write_src_parse_t, parse_data::op_del>)
			{
				rdata::set_int_info
				(
					int_info_save,
					rdata::rule_int_t::write_tgt_src_t_no_match,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont(std::in_place_type<std::monostate>, std::monostate())
				);
				return false;
			}
			else
			{
				return write_span(tgt, src);
			}
		};

		return std::visit(handle_op_parse_span, span_tgt, span_src);
	}

	template<typename alloc_rule, typename outer>
	inline bool write_op_span_mul(rdata::rule_int_info& int_info_save, rdata::rule_cmd_act_param<alloc_rule, outer>& run_param_v, const std::initializer_list<write_data::write_op_set<alloc_rule>>& write_set)
	{
		typename std::initializer_list<write_data::write_op_set<alloc_rule>>::const_iterator write_it = write_set.begin();

		if (write_it == write_set.end())
			return true;

		do
		{
			if (write_op_span<alloc_rule, outer>(int_info_save, run_param_v, write_it->get_write_tgt(), write_it->get_write_src()) == false)
				return false;

		} while (++write_it != write_set.end());

		return true;
	}
}
