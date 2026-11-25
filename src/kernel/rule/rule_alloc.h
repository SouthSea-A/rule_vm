#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <cstddef>
#include <utility>
#include <variant>
#include <optional>
#include "rule_data_def.h"

namespace rule_alloc
{
	template<typename alloc_rule, typename outer, typename rc_tgt_t>
	inline bool alloc_rc(rdata::rule_int_info& int_info_save, rdata::rule_cmd_act_param<alloc_rule, outer>& run_param_v, const rc_tgt_t& rc_tgt_v, bool is_incr_ref_ow, bool is_incr_ref_tgt)
	{
		using var_act = rdata::rule_var_act<alloc_rule>;
		using cmd_act = rdata::rule_cmd_act<alloc_rule>;
		using var_mgr = rdata::rule_var_mgr<alloc_rule>;
		using cmd_mgr = rdata::rule_cmd_mgr<alloc_rule>;
		typename outer::kit_type& rule_kit_ref = run_param_v.get_data_outer()->get_kit();
		typename outer::kit_type::owner_t& rule_ref = rule_kit_ref.get_owner();

		if constexpr (std::is_same_v<rc_tgt_t, cmd_data::var_target>)
		{
			const cmd_data::var_target& alloc_var_tgt_ref = rc_tgt_v;

			switch (alloc_var_tgt_ref.get_handle_type())
			{
			case cmd_data::op_var_type::var_type: [[fallthrough]];
			case cmd_data::op_var_type::var_self: [[fallthrough]];
			case cmd_data::op_var_type::var_tgt:
			{
				break;
			}
			case cmd_data::op_var_type::var_val:
			case cmd_data::op_var_type::var_ptr:
			{
				var_act& var_rc_ref = rule_ref.get_var_set();
				bool rc_is_created = false;
				rc_data::rc_res_inst_ref<rc_data::rc_sgl<var_mgr>> var_rc_alloc_res = var_rc_ref.add(alloc_var_tgt_ref.get_ownership(), is_incr_ref_ow, rc_is_created);

				if (var_rc_alloc_res.has_value() == false)
				{
					rdata::set_int_info
					(
						int_info_save,
						rdata::rule_int_t::alloc_var_mgr_error,
						rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, var_rc_alloc_res.error()),
						rdata::rule_int_info_cont
						(
							std::in_place_type<rdata::rule_int_info_cont_var>,
							alloc_var_tgt_ref.get_ownership(),
							alloc_var_tgt_ref.get_index(),
							alloc_var_tgt_ref.get_type(),
							alloc_var_tgt_ref.get_begin(),
							alloc_var_tgt_ref.get_width()
						)
					);
					return false;
				}

				rc_data::rc_sgl<var_mgr>& rc_sgl_ref = var_rc_alloc_res.value();
				var_mgr& var_mgr_ref = rc_sgl_ref.get_content();
				var_cont_data::var_type var_type_old = 0;
				std::size_t var_ref_old = 0;
				std::size_t var_size_old = 0;

				auto var_sgl_old_res = var_mgr_ref.get(alloc_var_tgt_ref.get_index());

				if (var_sgl_old_res.has_value() == true)
				{
					var_type_old = var_sgl_old_res.value().get().get_type();
					var_ref_old = var_sgl_old_res.value().get().get_ref();
					var_size_old = var_sgl_old_res.value().get().get_content().size();
				}

				bool var_is_inserted_new = false;
				var_data::var_mgr_res mgr_add_set_res = var_mgr_ref.add_set
				(
					alloc_var_tgt_ref.get_index(),
					alloc_var_tgt_ref.get_type(),
					alloc_var_tgt_ref.get_begin(),
					alloc_var_tgt_ref.get_handle_type() == cmd_data::op_var_type::var_val ? alloc_var_tgt_ref.get_width() : static_cast<var_cont_data::var_read_width>(cmd_data::RAW_BYTES_SIZE_VAR_TARGET),
					is_incr_ref_tgt,
					false,
					var_is_inserted_new
				);

				if (mgr_add_set_res.has_value() == false)
				{
					auto var_sgl_now_res = var_mgr_ref.get(alloc_var_tgt_ref.get_index());

					if (var_is_inserted_new == true)
					{
						if (var_sgl_now_res.has_value() == true)
						{
							var_data::var_mgr_res del_res = var_mgr_ref.del(alloc_var_tgt_ref.get_index());

							if (del_res.has_value() == false)
								throw kernel_error_throw(del_res.error(), "kernel | rule | rule_alloc | alloc_rc | 新变量回滚失败，错误不可恢复");
						}
					}
					else
					{
						if (var_sgl_now_res.has_value() == false)
							throw kernel_error_throw(mgr_add_set_res.error(), "kernel | rule | rule_alloc | alloc_rc | 变量在失败后丢失，回滚不可恢复");

						auto& var_sgl_now_ref = var_sgl_now_res.value().get();
						var_sgl_now_ref.set_type(var_type_old);

						if (var_sgl_now_ref.get_ref() != var_ref_old)
							throw kernel_error_throw(mgr_add_set_res.error(), "kernel | rule | rule_alloc | alloc_rc | 变量引用计数未恢复，回滚不可恢复");

						if (var_sgl_now_ref.get_content().size() != var_size_old)
							throw kernel_error_throw(mgr_add_set_res.error(), "kernel | rule | rule_alloc | alloc_rc | 变量内容已扩张，无法廉价回滚");
					}

					if (rc_is_created == true || is_incr_ref_ow == true)
					{
						rc_data::rc_res destroy_res = var_rc_ref.destroy(alloc_var_tgt_ref.get_ownership());

						if (destroy_res.has_value() == false)
							throw kernel_error_throw(destroy_res.error(), "kernel | rule | rule_alloc | alloc_rc | 资源回滚失败，错误不可恢复");
					}

					rdata::set_int_info
					(
						int_info_save,
						rdata::rule_int_t::alloc_var_mgr_span_error,
						rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, mgr_add_set_res.error()),
						rdata::rule_int_info_cont
						(
							std::in_place_type<rdata::rule_int_info_cont_var>,
							alloc_var_tgt_ref.get_ownership(),
							alloc_var_tgt_ref.get_index(),
							alloc_var_tgt_ref.get_type(),
							alloc_var_tgt_ref.get_begin(),
							alloc_var_tgt_ref.get_width()
						)
					);
					return false;
				}

				break;
			}
			default:
			{
				rdata::set_int_info
				(
					int_info_save,
					rdata::rule_int_t::alloc_t_no_match,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont
					(
						std::in_place_type<rdata::rule_int_info_cont_var>,
						alloc_var_tgt_ref.get_ownership(),
						alloc_var_tgt_ref.get_index(),
						alloc_var_tgt_ref.get_type(),
						alloc_var_tgt_ref.get_begin(),
						alloc_var_tgt_ref.get_width()
					)
				);
				return false;
			}
			}

			return true;
		}
		else if constexpr (std::is_same_v<rc_tgt_t, cmd_data::cmd_target>)
		{
			const cmd_data::cmd_target& alloc_cmd_tgt_ref = rc_tgt_v;

			switch (alloc_cmd_tgt_ref.get_handle_type())
			{
			case cmd_data::op_cmd_type::cmd_type: [[fallthrough]];
			case cmd_data::op_cmd_type::cmd_self: [[fallthrough]];
			case cmd_data::op_cmd_type::cmd_tgt:
			{
				break;
			}
			case cmd_data::op_cmd_type::cmd_val:
			{
				cmd_act& cmd_rc_ref = rule_ref.get_cmd_set();
				bool rc_is_created = false;
				rc_data::rc_res_inst_ref<rc_data::rc_sgl<cmd_mgr>> cmd_rc_alloc_res = cmd_rc_ref.add_ex(static_cast<cmd_data::cmd_mgr_index>(alloc_cmd_tgt_ref.get_ownership()), is_incr_ref_ow, run_param_v.get_ip().get_cmd_set_index(), run_param_v.get_cmd_it(), rc_is_created);

				if (cmd_rc_alloc_res.has_value() == false)
				{
					rdata::set_int_info
					(
						int_info_save,
						rdata::rule_int_t::alloc_cmd_mgr_error,
						rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, cmd_rc_alloc_res.error()),
						rdata::rule_int_info_cont
						(
							std::in_place_type<rdata::rule_int_info_cont_cmd>,
							static_cast<cmd_data::cmd_mgr_index>(alloc_cmd_tgt_ref.get_ownership()),
							static_cast<cmd_data::cmd_index>(alloc_cmd_tgt_ref.get_index()),
							static_cast<cmd_data::cmd_type>(alloc_cmd_tgt_ref.get_type()),
							static_cast<cmd_data::cmd_op_index>(alloc_cmd_tgt_ref.get_begin()),
							alloc_cmd_tgt_ref.get_width()
						)
					);
					return false;
				}

				rc_data::rc_sgl<cmd_mgr>& rc_sgl_ref = cmd_rc_alloc_res.value();
				cmd_mgr& cmd_mgr_ref = rc_sgl_ref.get_content();
				cmd_data::cmd_type cmd_type_old = 0;
				std::size_t cmd_ref_old = 0;
				std::size_t cmd_size_old = 0;

				auto cmd_sgl_old_res = cmd_mgr_ref.get(static_cast<cmd_data::cmd_index>(alloc_cmd_tgt_ref.get_index()));

				if (cmd_sgl_old_res.has_value() == true)
				{
					cmd_type_old = cmd_sgl_old_res.value().get_type().get();

					auto cmd_ref_old_res = cmd_mgr_ref.get_ref(static_cast<cmd_data::cmd_index>(alloc_cmd_tgt_ref.get_index()));

					if (cmd_ref_old_res.has_value() == false)
						throw kernel_error_throw(cmd_ref_old_res.error(), "kernel | rule | rule_alloc | alloc_rc | 命令引用计数读取失败，错误不可恢复");

					auto cmd_size_old_res = cmd_mgr_ref.get_op_size(static_cast<cmd_data::cmd_index>(alloc_cmd_tgt_ref.get_index()));

					if (cmd_size_old_res.has_value() == false)
						throw kernel_error_throw(cmd_size_old_res.error(), "kernel | rule | rule_alloc | alloc_rc | 命令内容大小读取失败，错误不可恢复");

					cmd_ref_old = cmd_ref_old_res.value();
					cmd_size_old = cmd_size_old_res.value();
				}

				bool cmd_is_inserted_new = false;
				cmd_data::cmd_mgr_res mgr_add_set_res = cmd_mgr_ref.add_set_ex
				(
					static_cast<cmd_data::cmd_index>(alloc_cmd_tgt_ref.get_index()),
					static_cast<cmd_data::cmd_type>(alloc_cmd_tgt_ref.get_type()),
					static_cast<cmd_data::cmd_op_index>(alloc_cmd_tgt_ref.get_begin()),
					alloc_cmd_tgt_ref.get_width(),
					is_incr_ref_tgt,
					run_param_v.get_cmd_it(),
					cmd_is_inserted_new
				);

				if (mgr_add_set_res.has_value() == false)
				{
					auto cmd_sgl_now_res = cmd_mgr_ref.get(static_cast<cmd_data::cmd_index>(alloc_cmd_tgt_ref.get_index()));

					if (cmd_is_inserted_new == true)
					{
						if (cmd_sgl_now_res.has_value() == true)
						{
							cmd_data::cmd_mgr_res del_res = cmd_mgr_ref.del_ex(static_cast<cmd_data::cmd_index>(alloc_cmd_tgt_ref.get_index()), run_param_v.get_cmd_it());

							if (del_res.has_value() == false)
								throw kernel_error_throw(del_res.error(), "kernel | rule | rule_alloc | alloc_rc | 新命令回滚失败，错误不可恢复");
						}
					}
					else
					{
						if (cmd_sgl_now_res.has_value() == false)
							throw kernel_error_throw(mgr_add_set_res.error(), "kernel | rule | rule_alloc | alloc_rc | 命令在失败后丢失，回滚不可恢复");

						cmd_sgl_now_res.value().get_type().get() = cmd_type_old;

						auto cmd_ref_now_res = cmd_mgr_ref.get_ref(static_cast<cmd_data::cmd_index>(alloc_cmd_tgt_ref.get_index()));

						if (cmd_ref_now_res.has_value() == false || cmd_ref_now_res.value() != cmd_ref_old)
							throw kernel_error_throw(mgr_add_set_res.error(), "kernel | rule | rule_alloc | alloc_rc | 命令引用计数未恢复，回滚不可恢复");

						auto cmd_size_now_res = cmd_mgr_ref.get_op_size(static_cast<cmd_data::cmd_index>(alloc_cmd_tgt_ref.get_index()));

						if (cmd_size_now_res.has_value() == false)
							throw kernel_error_throw(cmd_size_now_res.error(), "kernel | rule | rule_alloc | alloc_rc | 命令内容大小读取失败，错误不可恢复");

						if (cmd_size_now_res.value() != cmd_size_old)
							throw kernel_error_throw(mgr_add_set_res.error(), "kernel | rule | rule_alloc | alloc_rc | 命令内容已扩张，无法廉价回滚");
					}

					if (rc_is_created == true || is_incr_ref_ow == true)
					{
						rc_data::rc_res destroy_res = cmd_rc_ref.destroy(static_cast<cmd_data::cmd_mgr_index>(alloc_cmd_tgt_ref.get_ownership()));

						if (destroy_res.has_value() == false)
							throw kernel_error_throw(destroy_res.error(), "kernel | rule | rule_alloc | alloc_rc | 资源回滚失败，错误不可恢复");
					}

					rdata::set_int_info
					(
						int_info_save,
						rdata::rule_int_t::alloc_cmd_mgr_span_error,
						rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, mgr_add_set_res.error()),
						rdata::rule_int_info_cont
						(
							std::in_place_type<rdata::rule_int_info_cont_cmd>,
							static_cast<cmd_data::cmd_mgr_index>(alloc_cmd_tgt_ref.get_ownership()),
							static_cast<cmd_data::cmd_index>(alloc_cmd_tgt_ref.get_index()),
							static_cast<cmd_data::cmd_type>(alloc_cmd_tgt_ref.get_type()),
							static_cast<cmd_data::cmd_op_index>(alloc_cmd_tgt_ref.get_begin()),
							alloc_cmd_tgt_ref.get_width()
						)
					);
					return false;
				}

				break;
			}
			case cmd_data::op_cmd_type::cmd_ptr:
			{
				var_act& var_rc_ref = rule_ref.get_var_set();
				bool rc_is_created = false;
				rc_data::rc_res_inst_ref<rc_data::rc_sgl<var_mgr>> var_rc_alloc_res = var_rc_ref.add(static_cast<var_cont_data::var_mgr_index>(alloc_cmd_tgt_ref.get_ownership()), is_incr_ref_ow, rc_is_created);

				if (var_rc_alloc_res.has_value() == false)
				{
					rdata::set_int_info
					(
						int_info_save,
						rdata::rule_int_t::alloc_var_mgr_error,
						rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, var_rc_alloc_res.error()),
						rdata::rule_int_info_cont
						(
							std::in_place_type<rdata::rule_int_info_cont_var>,
							static_cast<var_cont_data::var_mgr_index>(alloc_cmd_tgt_ref.get_ownership()),
							static_cast<var_cont_data::var_index>(alloc_cmd_tgt_ref.get_index()),
							static_cast<var_cont_data::var_type>(alloc_cmd_tgt_ref.get_type()),
							static_cast<var_cont_data::var_pos>(alloc_cmd_tgt_ref.get_begin()),
							static_cast<var_cont_data::var_read_width>(cmd_data::RAW_BYTES_SIZE_CMD_TARGET)
						)
					);
					return false;
				}

				rc_data::rc_sgl<var_mgr>& rc_sgl_ref = var_rc_alloc_res.value();
				var_mgr& var_mgr_ref = rc_sgl_ref.get_content();
				var_cont_data::var_type var_type_old = 0;
				std::size_t var_ref_old = 0;
				std::size_t var_size_old = 0;

				auto var_sgl_old_res = var_mgr_ref.get(static_cast<var_cont_data::var_index>(alloc_cmd_tgt_ref.get_index()));

				if (var_sgl_old_res.has_value() == true)
				{
					var_type_old = var_sgl_old_res.value().get().get_type();
					var_ref_old = var_sgl_old_res.value().get().get_ref();
					var_size_old = var_sgl_old_res.value().get().get_content().size();
				}

				bool var_is_inserted_new = false;
				var_data::var_mgr_res mgr_add_set_res = var_mgr_ref.add_set
				(
					static_cast<var_cont_data::var_index>(alloc_cmd_tgt_ref.get_index()),
					static_cast<var_cont_data::var_type>(alloc_cmd_tgt_ref.get_type()),
					static_cast<var_cont_data::var_pos>(alloc_cmd_tgt_ref.get_begin()),
					static_cast<var_cont_data::var_read_width>(cmd_data::RAW_BYTES_SIZE_CMD_TARGET),
					is_incr_ref_tgt,
					false,
					var_is_inserted_new
				);

				if (mgr_add_set_res.has_value() == false)
				{
					auto var_sgl_now_res = var_mgr_ref.get(static_cast<var_cont_data::var_index>(alloc_cmd_tgt_ref.get_index()));

					if (var_is_inserted_new == true)
					{
						if (var_sgl_now_res.has_value() == true)
						{
							var_data::var_mgr_res del_res = var_mgr_ref.del(static_cast<var_cont_data::var_index>(alloc_cmd_tgt_ref.get_index()));

							if (del_res.has_value() == false)
								throw kernel_error_throw(del_res.error(), "kernel | rule | rule_alloc | alloc_rc | 新命令指针槽回滚失败，错误不可恢复");
						}
					}
					else
					{
						if (var_sgl_now_res.has_value() == false)
							throw kernel_error_throw(mgr_add_set_res.error(), "kernel | rule | rule_alloc | alloc_rc | 命令指针槽在失败后丢失，回滚不可恢复");

						auto& var_sgl_now_ref = var_sgl_now_res.value().get();
						var_sgl_now_ref.set_type(var_type_old);

						if (var_sgl_now_ref.get_ref() != var_ref_old)
							throw kernel_error_throw(mgr_add_set_res.error(), "kernel | rule | rule_alloc | alloc_rc | 命令指针槽引用计数未恢复，回滚不可恢复");

						if (var_sgl_now_ref.get_content().size() != var_size_old)
							throw kernel_error_throw(mgr_add_set_res.error(), "kernel | rule | rule_alloc | alloc_rc | 命令指针槽内容已扩张，无法廉价回滚");
					}

					if (rc_is_created == true || is_incr_ref_ow == true)
					{
						rc_data::rc_res destroy_res = var_rc_ref.destroy(static_cast<var_cont_data::var_mgr_index>(alloc_cmd_tgt_ref.get_ownership()));

						if (destroy_res.has_value() == false)
							throw kernel_error_throw(destroy_res.error(), "kernel | rule | rule_alloc | alloc_rc | 资源回滚失败，错误不可恢复");
					}

					rdata::set_int_info
					(
						int_info_save,
						rdata::rule_int_t::alloc_var_mgr_span_error,
						rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, mgr_add_set_res.error()),
						rdata::rule_int_info_cont
						(
							std::in_place_type<rdata::rule_int_info_cont_var>,
							static_cast<var_cont_data::var_mgr_index>(alloc_cmd_tgt_ref.get_ownership()),
							static_cast<var_cont_data::var_index>(alloc_cmd_tgt_ref.get_index()),
							static_cast<var_cont_data::var_type>(alloc_cmd_tgt_ref.get_type()),
							static_cast<var_cont_data::var_pos>(alloc_cmd_tgt_ref.get_begin()),
							static_cast<var_cont_data::var_read_width>(cmd_data::RAW_BYTES_SIZE_CMD_TARGET)
						)
					);
					return false;
				}

				break;
			}
			default:
			{
				rdata::set_int_info
				(
					int_info_save,
					rdata::rule_int_t::alloc_t_no_match,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont
					(
						std::in_place_type<rdata::rule_int_info_cont_cmd>,
						static_cast<cmd_data::cmd_mgr_index>(alloc_cmd_tgt_ref.get_ownership()),
						static_cast<cmd_data::cmd_index>(alloc_cmd_tgt_ref.get_index()),
						static_cast<cmd_data::cmd_type>(alloc_cmd_tgt_ref.get_type()),
						static_cast<cmd_data::cmd_op_index>(alloc_cmd_tgt_ref.get_begin()),
						alloc_cmd_tgt_ref.get_width()
					)
				);
				return false;
			}
			}

			return true;
		}
		else
		{
			rdata::set_int_info
			(
				int_info_save,
				rdata::rule_int_t::alloc_t_no_match,
				rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
				rdata::rule_int_info_cont(std::in_place_type<std::monostate>, std::monostate())
			);
			return false;
		}

		return false;
	}

	template<typename alloc_rule, typename outer, typename rc_tgt_t>
	inline bool dealloc_rc(rdata::rule_int_info& int_info_save, rdata::rule_cmd_act_param<alloc_rule, outer>& run_param_v, const rc_tgt_t& rc_tgt_v)
	{
		using var_act = rdata::rule_var_act<alloc_rule>;
		using cmd_act = rdata::rule_cmd_act<alloc_rule>;
		using var_mgr = rdata::rule_var_mgr<alloc_rule>;
		using cmd_mgr = rdata::rule_cmd_mgr<alloc_rule>;
		typename outer::kit_type& rule_kit_ref = run_param_v.get_data_outer()->get_kit();
		typename outer::kit_type::owner_t& rule_ref = rule_kit_ref.get_owner();

		if constexpr (std::is_same_v<rc_tgt_t, cmd_data::var_target>)
		{
			const cmd_data::var_target& alloc_var_tgt_ref = rc_tgt_v;

			switch (alloc_var_tgt_ref.get_handle_type())
			{
			case cmd_data::op_var_type::var_type: [[fallthrough]];
			case cmd_data::op_var_type::var_self: [[fallthrough]];
			case cmd_data::op_var_type::var_tgt:
			{
				break;
			}
			case cmd_data::op_var_type::var_val: [[fallthrough]];
			case cmd_data::op_var_type::var_ptr:
			{
				var_act& var_rc_ref = rule_ref.get_var_set();
				rc_data::rc_res_inst_ref<rc_data::rc_sgl<var_mgr>> rc_var_res = var_rc_ref.get(alloc_var_tgt_ref.get_ownership());

				if (rc_var_res.has_value() == false)
				{
					rdata::set_int_info
					(
						int_info_save,
						rdata::rule_int_t::dealloc_var_mgr_error,
						rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, rc_var_res.error()),
						rdata::rule_int_info_cont
						(
							std::in_place_type<rdata::rule_int_info_cont_var>,
							alloc_var_tgt_ref.get_ownership(),
							alloc_var_tgt_ref.get_index(),
							std::nullopt,
							std::nullopt,
							std::nullopt
						)
					);
					return false;
				}

				var_mgr& var_mgr_ref = rc_var_res.value().get().get_content();
				var_data::var_mgr_res var_del_res = var_mgr_ref.del(alloc_var_tgt_ref.get_index());

				if (var_del_res.has_value() == false)
				{
					rdata::set_int_info
					(
						int_info_save,
						rdata::rule_int_t::dealloc_var_mgr_span_error,
						rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, var_del_res.error()),
						rdata::rule_int_info_cont
						(
							std::in_place_type<rdata::rule_int_info_cont_var>,
							alloc_var_tgt_ref.get_ownership(),
							alloc_var_tgt_ref.get_index(),
							alloc_var_tgt_ref.get_type(),
							alloc_var_tgt_ref.get_begin(),
							alloc_var_tgt_ref.get_width()
						)
					);
					return false;
				}

				rc_data::rc_res rc_var_destroy_res = var_rc_ref.destroy(alloc_var_tgt_ref.get_ownership());

				if (rc_var_destroy_res.has_value() == false)
				{
					rdata::set_int_info
					(
						int_info_save,
						rdata::rule_int_t::dealloc_var_mgr_error,
						rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, rc_var_destroy_res.error()),
						rdata::rule_int_info_cont
						(
							std::in_place_type<rdata::rule_int_info_cont_var>,
							alloc_var_tgt_ref.get_ownership(),
							alloc_var_tgt_ref.get_index(),
							std::nullopt,
							std::nullopt,
							std::nullopt
						)
					);
					return false;
				}

				break;
			}
			default:
			{
				rdata::set_int_info
				(
					int_info_save,
					rdata::rule_int_t::dealloc_t_no_match,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont(std::in_place_type<std::monostate>, std::monostate())
				);
				return false;
			}
			}

			return true;
		}
		else if constexpr (std::is_same_v<rc_tgt_t, cmd_data::cmd_target>)
		{
			const cmd_data::cmd_target& alloc_cmd_tgt_ref = rc_tgt_v;

			switch (alloc_cmd_tgt_ref.get_handle_type())
			{
			case cmd_data::op_cmd_type::cmd_type: [[fallthrough]];
			case cmd_data::op_cmd_type::cmd_self:
			{
				break;
			}
			case cmd_data::op_cmd_type::cmd_tgt:
			{
				cmd_act& cmd_act_ref = rule_ref.get_cmd_set();
				rc_data::rc_res_inst_ref<rc_data::rc_sgl<cmd_mgr>> rc_cmd_res = cmd_act_ref.get(alloc_cmd_tgt_ref.get_ownership());

				if (rc_cmd_res.has_value() == false)
				{
					rdata::set_int_info
					(
						int_info_save,
						rdata::rule_int_t::dealloc_cmd_mgr_error,
						rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, rc_cmd_res.error()),
						rdata::rule_int_info_cont
						(
							std::in_place_type<rdata::rule_int_info_cont_cmd>,
							static_cast<cmd_data::cmd_mgr_index>(alloc_cmd_tgt_ref.get_ownership()),
							static_cast<cmd_data::cmd_index>(alloc_cmd_tgt_ref.get_index()),
							std::nullopt,
							std::nullopt,
							std::nullopt
						)
					);
					return false;
				}

				cmd_mgr& cmd_mgr_ref = rc_cmd_res.value().get().get_content();
				cmd_data::cmd_mgr_res cmd_mgr_del_res = cmd_mgr_ref.del_ex
				(
					static_cast<cmd_data::cmd_index>(alloc_cmd_tgt_ref.get_index()),
					run_param_v.get_cmd_it()
				);

				if (cmd_mgr_del_res.has_value() == false)
				{
					rdata::set_int_info
					(
						int_info_save,
						rdata::rule_int_t::dealloc_cmd_mgr_span_error,
						rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, cmd_mgr_del_res.error()),
						rdata::rule_int_info_cont
						(
							std::in_place_type<rdata::rule_int_info_cont_cmd>,
							static_cast<cmd_data::cmd_mgr_index>(alloc_cmd_tgt_ref.get_ownership()),
							static_cast<cmd_data::cmd_index>(alloc_cmd_tgt_ref.get_index()),
							std::nullopt,
							std::nullopt,
							std::nullopt
						)
					);
					return false;
				}

				rc_data::rc_res rc_cmd_destroy_res = cmd_act_ref.destroy_ex(alloc_cmd_tgt_ref.get_ownership(), run_param_v.get_ip().get_cmd_set_index(), run_param_v.get_cmd_it());

				if (rc_cmd_destroy_res.has_value() == false)
				{
					rdata::set_int_info
					(
						int_info_save,
						rdata::rule_int_t::dealloc_cmd_mgr_error,
						rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, rc_cmd_destroy_res.error()),
						rdata::rule_int_info_cont
						(
							std::in_place_type<rdata::rule_int_info_cont_cmd>,
							static_cast<cmd_data::cmd_mgr_index>(alloc_cmd_tgt_ref.get_ownership()),
							static_cast<cmd_data::cmd_index>(alloc_cmd_tgt_ref.get_index()),
							std::nullopt,
							std::nullopt,
							std::nullopt
						)
					);
					return false;
				}


				break;
			}
			case cmd_data::op_cmd_type::cmd_val:
			{
				cmd_act& cmd_act_ref = rule_ref.get_cmd_set();
				rc_data::rc_res_inst_ref<rc_data::rc_sgl<cmd_mgr>> rc_cmd_res = cmd_act_ref.get(alloc_cmd_tgt_ref.get_ownership());

				if (rc_cmd_res.has_value() == false)
				{
					rdata::set_int_info
					(
						int_info_save,
						rdata::rule_int_t::dealloc_cmd_mgr_error,
						rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, rc_cmd_res.error()),
						rdata::rule_int_info_cont
						(
							std::in_place_type<rdata::rule_int_info_cont_cmd>,
							static_cast<cmd_data::cmd_mgr_index>(alloc_cmd_tgt_ref.get_ownership()),
							static_cast<cmd_data::cmd_index>(alloc_cmd_tgt_ref.get_index()),
							std::nullopt,
							std::nullopt,
							std::nullopt
						)

					);
					return false;
				}


				cmd_mgr& cmd_mgr_ref = rc_cmd_res.value().get().get_content();
				cmd_data::cmd_mgr_res cmd_mgr_del_res = cmd_mgr_ref.del_ex
				(
					static_cast<cmd_data::cmd_index>(alloc_cmd_tgt_ref.get_index()),
					static_cast<cmd_data::cmd_op_index>(alloc_cmd_tgt_ref.get_begin()),
					static_cast<cmd_data::cmd_op_width>(alloc_cmd_tgt_ref.get_width()),
					false,
					run_param_v.get_cmd_it()
				);

				if (cmd_mgr_del_res.has_value() == false)
				{
					rdata::set_int_info
					(
						int_info_save,
						rdata::rule_int_t::dealloc_cmd_mgr_span_error,
						rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, cmd_mgr_del_res.error()),
						rdata::rule_int_info_cont
						(
							std::in_place_type<rdata::rule_int_info_cont_cmd>,
							static_cast<cmd_data::cmd_mgr_index>(alloc_cmd_tgt_ref.get_ownership()),
							static_cast<cmd_data::cmd_index>(alloc_cmd_tgt_ref.get_index()),
							static_cast<cmd_data::cmd_type>(alloc_cmd_tgt_ref.get_type()),
							static_cast<cmd_data::cmd_op_index>(alloc_cmd_tgt_ref.get_begin()),
							alloc_cmd_tgt_ref.get_width()
						)
					);
					return false;
				}

				rc_data::rc_res rc_cmd_destroy_res = cmd_act_ref.destroy_ex(alloc_cmd_tgt_ref.get_ownership(), run_param_v.get_ip().get_cmd_set_index(), run_param_v.get_cmd_it());

				if (rc_cmd_destroy_res.has_value() == false)
				{
					rdata::set_int_info
					(
						int_info_save,
						rdata::rule_int_t::dealloc_cmd_mgr_error,
						rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, rc_cmd_destroy_res.error()),
						rdata::rule_int_info_cont
						(
							std::in_place_type<rdata::rule_int_info_cont_cmd>,
							static_cast<cmd_data::cmd_mgr_index>(alloc_cmd_tgt_ref.get_ownership()),
							static_cast<cmd_data::cmd_index>(alloc_cmd_tgt_ref.get_index()),
							std::nullopt,
							std::nullopt,
							std::nullopt
						)

					);
					return false;
				}



				break;
			}
			case cmd_data::op_cmd_type::cmd_ptr:
			{
				var_act& var_rc_ref = rule_ref.get_var_set();
				rc_data::rc_res_inst_ref<rc_data::rc_sgl<var_mgr>> rc_var_res = var_rc_ref.get(static_cast<var_cont_data::var_mgr_index>(alloc_cmd_tgt_ref.get_ownership()));

				if (rc_var_res.has_value() == false)
				{
					rdata::set_int_info
					(
						int_info_save,
						rdata::rule_int_t::dealloc_var_mgr_error,
						rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, rc_var_res.error()),
						rdata::rule_int_info_cont
						(
							std::in_place_type<rdata::rule_int_info_cont_var>,
							static_cast<var_cont_data::var_mgr_index>(alloc_cmd_tgt_ref.get_ownership()),
							static_cast<var_cont_data::var_index>(alloc_cmd_tgt_ref.get_index()),
							std::nullopt,
							std::nullopt,
							std::nullopt
						)
					);
					return false;
				}


				var_mgr& var_mgr_ref = rc_var_res.value().get().get_content();
				var_data::var_mgr_res var_mgr_del_res = var_mgr_ref.del(static_cast<var_cont_data::var_index>(alloc_cmd_tgt_ref.get_index()));

				if (var_mgr_del_res.has_value() == false)
				{
					rdata::set_int_info
					(
						int_info_save,
						rdata::rule_int_t::dealloc_var_mgr_span_error,
						rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, var_mgr_del_res.error()),
						rdata::rule_int_info_cont
						(
							std::in_place_type<rdata::rule_int_info_cont_var>,
							static_cast<var_cont_data::var_mgr_index>(alloc_cmd_tgt_ref.get_ownership()),
							static_cast<var_cont_data::var_index>(alloc_cmd_tgt_ref.get_index()),
							static_cast<var_cont_data::var_type>(alloc_cmd_tgt_ref.get_type()),
							static_cast<var_cont_data::var_pos>(alloc_cmd_tgt_ref.get_begin()),
							static_cast<var_cont_data::var_read_width>(alloc_cmd_tgt_ref.get_width())
						)
					);
					return false;
				}

				rc_data::rc_res rc_var_destroy_res = var_rc_ref.destroy(static_cast<var_cont_data::var_mgr_index>(alloc_cmd_tgt_ref.get_ownership()));

				if (rc_var_destroy_res.has_value() == false)
				{
					rdata::set_int_info
					(
						int_info_save,
						rdata::rule_int_t::dealloc_var_mgr_error,
						rdata::rule_int_info_base_error_t(std::in_place_type<kernel_error>, rc_var_destroy_res.error()),
						rdata::rule_int_info_cont
						(
							std::in_place_type<rdata::rule_int_info_cont_var>,
							static_cast<var_cont_data::var_mgr_index>(alloc_cmd_tgt_ref.get_ownership()),
							static_cast<var_cont_data::var_index>(alloc_cmd_tgt_ref.get_index()),
							std::nullopt,
							std::nullopt,
							std::nullopt
						)
					);
					return false;
				}

				break;
			}
			default:
			{
				rdata::set_int_info
				(
					int_info_save,
					rdata::rule_int_t::dealloc_t_no_match,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont(std::in_place_type<std::monostate>, std::monostate())
				);
				return false;
			}
			}

			return true;
		}
		else
		{
			rdata::set_int_info
			(
				int_info_save,
				rdata::rule_int_t::dealloc_t_no_match,
				rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
				rdata::rule_int_info_cont(std::in_place_type<std::monostate>, std::monostate())
			);
			return false;
		}

		return false;
	}
}
