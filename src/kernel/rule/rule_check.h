#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <cstddef>
#include <initializer_list>
#include "rule_check_data_def.h"


namespace rule_check
{
	inline bool check_cmd_info(rdata::rule_int_info& int_info_save, const rdata::rule_cmd_sgl& cmd_tgt_v, cmd_data::cmd_type cmd_tgt_t_v, std::size_t op_size_v)
	{
		auto set_int_info_check = [&](rdata::rule_int_t check_int_res_v) -> void
	    {
			rdata::set_int_info
			(
				int_info_save,
				check_int_res_v,
				rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
				rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, cmd_tgt_v, std::nullopt, std::nullopt)
			);

			return;
		};

		if (cmd_tgt_v.get_type().get() != cmd_tgt_t_v)
		{
			set_int_info_check(rdata::rule_int_t::check_cmd_t_no_match);
			return false;
		}

		if (cmd_tgt_v.get_content_span().size() < op_size_v)
		{
			set_int_info_check(rdata::rule_int_t::check_cmd_op_size_no_match);
			return false;
		}

		return true;
	}
	inline bool check_cmd_op_info(rdata::rule_int_info& int_info_save, const rdata::rule_cmd_sgl& rule_cmd_sgl_v, cmd_data::cmd_op_index op_index_check_v, cmd_data::cmd_op_type op_t_check_v, bool is_exc)
	{
		const std::span<cmd_data::cmd_op_single>& cmd_op_span_tgt_ref = rule_cmd_sgl_v.get_content_span();

		if (static_cast<cmd_data::cmd_op_index>(cmd_op_span_tgt_ref.size()) <= op_index_check_v)
		{
			rdata::set_int_info
			(
				int_info_save,
				rdata::rule_int_t::check_cmd_op_size_no_match,
				rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
				rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, rule_cmd_sgl_v, op_index_check_v, std::nullopt)
			);
			return false;
		}

		if (op_t_check_v == cmd_data::cmd_op_type::both)
			return true;

		const cmd_data::cmd_op_single& cmd_op_check_ref = cmd_op_span_tgt_ref[op_index_check_v];

		bool t_comp_res = is_exc ^ (static_cast<cmd_data::cmd_op_type>(cmd_op_check_ref.index()) == op_t_check_v);

		if (t_comp_res == false)
		{
			rdata::set_int_info
			(
				int_info_save,
				rdata::rule_int_t::check_cmd_op_t_no_match,
				rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
				rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, rule_cmd_sgl_v, op_index_check_v, cmd_op_check_ref)
			);
			return false;
		}

		return true;
	}
	inline bool check_cmd_op_info_mul(rdata::rule_int_info& int_info_save, const std::initializer_list<check_data::ck_cmd_op_set>& op_check_set)
	{
		typename std::initializer_list<check_data::ck_cmd_op_set>::const_iterator op_check_set_it = op_check_set.begin();

		if (op_check_set_it == op_check_set.end())
			return true;

		do
		{
			if (check_cmd_op_info(int_info_save, op_check_set_it->get_cmd_sgl_tgt().get(), op_check_set_it->get_op_index(), op_check_set_it->get_op_t(), op_check_set_it->get_is_exc()) == false)
				return false;

		} while (++op_check_set_it != op_check_set.end());

		return true;
	}
}
