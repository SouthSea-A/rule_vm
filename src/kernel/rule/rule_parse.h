#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <cstddef>
#include <utility>
#include <type_traits>
#include <functional>
#include <optional>
#include <span>
#include <variant>
#include <initializer_list>
#include "rule_parse_data_def.h"
#include "rule_alloc.h"
#include "rule_transform.h"

namespace rule_parse
{
	template<typename alloc_rule, typename outer>
	struct run_param
	{
	public:
		using cmd_mgr = rdata::rule_cmd_mgr<alloc_rule>;
		using cmd_act_param = rdata::rule_cmd_act_param<alloc_rule, outer>;
	public:
		std::optional<std::reference_wrapper<cmd_mgr>> cmd_mgr_tgt;
		std::optional<rdata::rule_cmd_sgl> cmd;
	public:
		template<typename self_this>
		decltype(auto) get_cmd_mgr_tgt(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.cmd_mgr_tgt);
		}
		template<typename self_this>
		decltype(auto) get_cmd(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.cmd);
		}
	public:
		void set_cmd_mgr_tgt(const std::optional<std::reference_wrapper<cmd_mgr>>& cmd_mgr_tgt_v)
		{
			this->cmd_mgr_tgt = cmd_mgr_tgt_v;
		}
		void set_cmd(const std::optional<rdata::rule_cmd_sgl>& cmd_v)
		{
			this->cmd = cmd_v;
		}
	public:
		run_param() :
			cmd_mgr_tgt(std::nullopt), cmd(std::nullopt)
		{

		}
		run_param(cmd_act_param& run_param_v) :
			cmd_mgr_tgt(std::nullopt), cmd(std::nullopt)
		{
			typename outer::kit_type& rule_kit_ref = run_param_v.get_data_outer()->get_kit();
			typename outer::kit_type::owner_t& rule_data = rule_kit_ref.get_owner();

			rdata::rule_run_ip<alloc_rule>& ip_tgt = run_param_v.get_ip();

			rc_data::rc_res_inst_ref<rc_data::rc_sgl<cmd_mgr>> rc_cmd_res = rule_data.get_cmd_set().get(ip_tgt.get_cmd_set_index());

			if (rc_cmd_res.has_value() == true)
			{
				rc_data::rc_sgl<cmd_mgr>& rc_cmd_tgt = rc_cmd_res.value().get();
				cmd_mgr& cmd_mgr_tgt_ref = rc_cmd_tgt.get_content();
				cmd_data::cmd_mgr_res_cmd_single_it<typename cmd_mgr::iterator> cmd_sgl_it_tgt = cmd_mgr_tgt_ref.get_it(ip_tgt.get_index());

				if (cmd_sgl_it_tgt.has_value() == true)
				{
					typename cmd_mgr::iterator& cmd_sgl_it_tgt_ref = cmd_sgl_it_tgt.value();

					if (cmd_sgl_it_tgt_ref == run_param_v.get_cmd_it())
					{
						set_cmd_mgr_tgt(std::make_optional<std::reference_wrapper<cmd_mgr>>(std::ref(cmd_mgr_tgt_ref)));
						set_cmd(std::make_optional<rdata::rule_cmd_sgl>(*cmd_sgl_it_tgt_ref));
					}
				}
			}
		}
		run_param(const std::optional<std::reference_wrapper<cmd_mgr>>& cmd_mgr_tgt_v, const std::optional<rdata::rule_cmd_sgl>& cmd_v) :
			cmd_mgr_tgt(cmd_mgr_tgt_v), cmd(cmd_v)
		{

		}
	};

	template<typename alloc_rule, typename outer>
	inline bool get_parse_op(rdata::rule_int_info& int_info_save, rdata::rule_cmd_act_param<alloc_rule, outer>& run_param_v, const rdata::rule_cmd_sgl& rule_cmd_sgl_v, cmd_data::cmd_op_index op_parse_index_v, parse_data::op<alloc_rule>& op_parse_res_save, bool is_alloc_rc, bool is_incr_ref)
	{
		using var_mgr = rdata::rule_var_mgr<alloc_rule>;
		using cmd_mgr = rdata::rule_cmd_mgr<alloc_rule>;
		using alloc_var = rdata::rule_alloc_var<alloc_rule>;

		typename outer::kit_type& rule_kit_ref = run_param_v.get_data_outer()->get_kit();
		typename outer::kit_type::owner_t& rule_ref = rule_kit_ref.get_owner();

		const std::span<cmd_data::cmd_op_single>& cmd_op_span_tgt_ref = rule_cmd_sgl_v.get_content_span();

		if (op_parse_index_v >= cmd_op_span_tgt_ref.size())
		{
			rdata::set_int_info
			(
				int_info_save,
				rdata::rule_int_t::check_cmd_op_size_no_match,
				rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
				rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, rule_cmd_sgl_v, op_parse_index_v, std::nullopt)
			);
			return false;
		}

		cmd_data::cmd_op_single& op_parse_v = cmd_op_span_tgt_ref[op_parse_index_v];

		auto handle_imm = [&](cmd_data::imm_target& imm_tgt_ref) -> bool
		{
			std::array<rdata::rule_var_base_t, cmd_data::imm_width_byte_max>& imm_tgt_data_ref = imm_tgt_ref.get_data();

			switch (imm_tgt_ref.get_type())
			{
			case cmd_data::imm_type::self:
			{
				op_parse_res_save.template emplace<parse_data::op_imm>(op_parse_v, std::nullopt);
				break;
			}
			case cmd_data::imm_type::byte:
			{
				op_parse_res_save.template emplace<parse_data::op_imm>(op_parse_v, std::span<rdata::rule_var_base_t>(imm_tgt_data_ref.begin(), 1));
				break;
			}
			case cmd_data::imm_type::word:
			{
				op_parse_res_save.template emplace<parse_data::op_imm>(op_parse_v, std::span<rdata::rule_var_base_t>(imm_tgt_data_ref.begin(), 2));
				break;
			}
			case cmd_data::imm_type::dword:
			{
				op_parse_res_save.template emplace<parse_data::op_imm>(op_parse_v, std::span<rdata::rule_var_base_t>(imm_tgt_data_ref.begin(), 4));
				break;
			}
			case cmd_data::imm_type::qword:
			{
				op_parse_res_save.template emplace<parse_data::op_imm>(op_parse_v, std::span<rdata::rule_var_base_t>(imm_tgt_data_ref.begin(), 8));
				break;
			}
			default:
			{
				op_parse_res_save.template emplace<std::monostate>();
				rdata::set_int_info
				(
					int_info_save,
					rdata::rule_int_t::parse_op_imm_t_no_match,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, rule_cmd_sgl_v, op_parse_index_v, op_parse_v)
				);
				return false;
			}
			}

			return true;
		};

		auto handle_var = [&](cmd_data::var_target& var_tgt_ref) -> bool
		{
			auto set_int_info_var = [&]<typename error_code_t>(const error_code_t & error_v, rdata::rule_int_t int_t_v) -> void
			{
				rdata::set_int_info
				(
					int_info_save,
					int_t_v,
					rdata::rule_int_info_base_error_t(std::in_place_type<error_code_t>, error_v),
					rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, rule_cmd_sgl_v, op_parse_index_v, op_parse_v)
				);
				return;
			};

			if (var_tgt_ref.get_handle_type() == cmd_data::op_var_type::var_self)
			{
				op_parse_res_save.template emplace<parse_data::op_var<alloc_rule>>(op_parse_v, var_tgt_ref, std::nullopt, std::nullopt);
				return true;
			}

			if (var_tgt_ref.get_handle_type() == cmd_data::op_var_type::var_tgt || var_tgt_ref.get_handle_type() == cmd_data::op_var_type::var_del)
			{
				op_parse_res_save.template emplace<parse_data::op_var<alloc_rule>>(op_parse_v, var_tgt_ref, std::nullopt, std::nullopt);
				return true;
			}

			if (is_alloc_rc == true)
			{
				if (ralloc::alloc_rc<alloc_rule>(int_info_save, run_param_v, var_tgt_ref, is_incr_ref, is_incr_ref) == false)
				{
					op_parse_res_save.template emplace<parse_data::op_var<alloc_rule>>(op_parse_v, var_tgt_ref, std::nullopt, std::nullopt);
					return false;
				}
			}

			rc_data::rc_res_inst_ref<rc_data::rc_sgl<var_mgr>> var_mgr_tgt_res = rule_ref.get_var_set().get(var_tgt_ref.get_ownership());

			if (var_mgr_tgt_res.has_value() == false)
			{
				op_parse_res_save.template emplace<parse_data::op_var<alloc_rule>>(op_parse_v, var_tgt_ref, std::nullopt, std::nullopt);
				set_int_info_var(var_mgr_tgt_res.error(), rdata::rule_int_t::parse_op_var_get_error);
				return false;
			}

			var_mgr& var_mgr_tgt_ref = var_mgr_tgt_res.value().get().get_content();
			var_data::var_mgr_res_var_sgl<rdata::rule_var_base_t, alloc_var> var_sgl_tgt_res = var_mgr_tgt_ref.get(var_tgt_ref.get_index());

			if (var_sgl_tgt_res.has_value() == false)
			{
				op_parse_res_save.template emplace<parse_data::op_var<alloc_rule>>(op_parse_v, var_tgt_ref, std::nullopt, std::nullopt);
				set_int_info_var(var_sgl_tgt_res.error(), rdata::rule_int_t::parse_op_var_mgr_sgl_get_error);
				return false;
			}

			var_data::var_sgl<rdata::rule_var_base_t, alloc_var>& var_sgl_ref = var_sgl_tgt_res.value().get();

			switch (var_tgt_ref.get_handle_type())
			{
			case cmd_data::op_var_type::var_type:
			{
				op_parse_res_save.template emplace<parse_data::op_var<alloc_rule>>(op_parse_v, var_tgt_ref, parse_data::op_var_info<alloc_rule>(var_mgr_tgt_ref, var_sgl_ref.get_type(), {}), std::nullopt);
				break;
			}
			case cmd_data::op_var_type::var_val:
			{
				var_cont_data::var_cont_res_span<rdata::rule_var_base_t> var_span_tgt_res = var_sgl_ref.get_content().read(var_tgt_ref.get_begin(), var_tgt_ref.get_width());

				if (var_span_tgt_res.has_value() == false)
				{
					op_parse_res_save.template emplace<parse_data::op_var<alloc_rule>>(op_parse_v, var_tgt_ref, std::nullopt, std::nullopt);
					set_int_info_var(var_span_tgt_res.error(), rdata::rule_int_t::parse_op_var_mgr_sgl_span_get_error);
					return false;
				}

				op_parse_res_save.template emplace<parse_data::op_var<alloc_rule>>(op_parse_v, var_tgt_ref, parse_data::op_var_info<alloc_rule>(var_mgr_tgt_ref, var_sgl_ref.get_type(), var_span_tgt_res.value()), std::nullopt);

				break;
			}
			case cmd_data::op_var_type::var_ptr:
			{
				var_cont_data::var_cont_res_span<rdata::rule_var_base_t> var_span_tgt_res = var_sgl_ref.get_content().read(var_tgt_ref.get_begin(), static_cast<var_cont_data::var_read_width>(cmd_data::RAW_BYTES_SIZE_VAR_TARGET));

				if (var_span_tgt_res.has_value() == false)
				{
					op_parse_res_save.template emplace<parse_data::op_var<alloc_rule>>(op_parse_v, var_tgt_ref, std::nullopt, std::nullopt);
					set_int_info_var(var_span_tgt_res.error(), rdata::rule_int_t::parse_op_var_ptr_get_error);
					return false;
				}

				cmd_data::var_target var_tgt_ptr;

				if (trans::transform_raw_bytes_to(int_info_save, var_span_tgt_res.value(), var_tgt_ptr) == false)
				{
					op_parse_res_save.template emplace<parse_data::op_var<alloc_rule>>(op_parse_v, var_tgt_ref, std::nullopt, parse_data::op_var_info<alloc_rule>(var_mgr_tgt_ref, var_sgl_ref.get_type(), var_span_tgt_res.value()));
					return false;
				}

				rc_data::rc_res_inst_ref<rc_data::rc_sgl<var_mgr>> var_mgr_tgt_ptr_res = rule_ref.get_var_set().get(var_tgt_ptr.get_ownership());

				if (var_mgr_tgt_ptr_res.has_value() == false)
				{
					op_parse_res_save.template emplace<parse_data::op_var<alloc_rule>>(op_parse_v, var_tgt_ref, std::nullopt, parse_data::op_var_info<alloc_rule>(var_mgr_tgt_ref, var_sgl_ref.get_type(), var_span_tgt_res.value()));
					set_int_info_var(var_mgr_tgt_ptr_res.error(), rdata::rule_int_t::parse_op_var_mgr_ptr_get_error);
					return false;
				}

				var_mgr& var_mgr_tgt_ptr_ref = var_mgr_tgt_ptr_res.value().get().get_content();
				var_data::var_mgr_res_var_sgl<rdata::rule_var_base_t, alloc_var> var_sgl_tgt_ptr_res = var_mgr_tgt_ptr_ref.get(var_tgt_ptr.get_index());

				if (var_sgl_tgt_ptr_res.has_value() == false)
				{
					op_parse_res_save.template emplace<parse_data::op_var<alloc_rule>>(op_parse_v, var_tgt_ref, std::nullopt, parse_data::op_var_info<alloc_rule>(var_mgr_tgt_ref, var_sgl_ref.get_type(), var_span_tgt_res.value()));
					set_int_info_var(var_sgl_tgt_ptr_res.error(), rdata::rule_int_t::parse_op_var_mgr_sgl_ptr_get_error);
					return false;
				}

				var_data::var_sgl<rdata::rule_var_base_t, alloc_var>& var_sgl_tgt_ptr_ref = var_sgl_tgt_ptr_res.value().get();
				var_cont_data::var_cont_res_span<rdata::rule_var_base_t> var_span_ptr_res = var_sgl_tgt_ptr_ref.get_content().read(var_tgt_ptr.get_begin(), var_tgt_ptr.get_width());

				if (var_span_ptr_res.has_value() == false)
				{
					op_parse_res_save.template emplace<parse_data::op_var<alloc_rule>>(op_parse_v, var_tgt_ref, std::nullopt, parse_data::op_var_info<alloc_rule>(var_mgr_tgt_ref, var_sgl_ref.get_type(), var_span_tgt_res.value()));
					set_int_info_var(var_span_ptr_res.error(), rdata::rule_int_t::parse_op_var_mgr_sgl_span_ptr_get_error);
					return false;
				}

				op_parse_res_save.template emplace<parse_data::op_var<alloc_rule>>
				(
					op_parse_v,
					var_tgt_ref,
					parse_data::op_var_info<alloc_rule>
					(
						var_mgr_tgt_ptr_ref,
						var_sgl_tgt_ptr_ref.get_type(),
						var_span_ptr_res.value()
					),
					parse_data::op_var_info<alloc_rule>
					(
						var_mgr_tgt_ref,
						var_sgl_ref.get_type(),
						var_span_tgt_res.value()
					)
				);
				break;
			}
			default:
			{
				op_parse_res_save.template emplace<std::monostate>();
				set_int_info_var(std::monostate(), rdata::rule_int_t::parse_op_var_t_no_match);
				return false;
			}
			}

			return true;
		};

		auto handle_cmd = [&](cmd_data::cmd_target& cmd_tgt_ref) -> bool
		{
			auto set_int_info_cmd = [&]<typename error_code_t>(const error_code_t & error_v, rdata::rule_int_t int_t_v) -> void
				{
					rdata::set_int_info
					(
						int_info_save,
						int_t_v,
						rdata::rule_int_info_base_error_t(std::in_place_type<error_code_t>, error_v),
						rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, rule_cmd_sgl_v, op_parse_index_v, op_parse_v)
					);
					return;
				};

			switch (cmd_tgt_ref.get_handle_type())
			{
			case cmd_data::op_cmd_type::cmd_self: [[fallthrough]];
			case cmd_data::op_cmd_type::cmd_tgt: [[fallthrough]];
			case cmd_data::op_cmd_type::cmd_del:
			{
				op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>(op_parse_v, cmd_tgt_ref, std::nullopt, std::nullopt);
				break;
			}
			case cmd_data::op_cmd_type::cmd_type: [[fallthrough]];
			case cmd_data::op_cmd_type::cmd_val:
			{
				if (is_alloc_rc == true)
				{
					if (ralloc::alloc_rc<alloc_rule>(int_info_save, run_param_v, cmd_tgt_ref, is_incr_ref, is_incr_ref) == false)
					{
						op_parse_res_save.template emplace<std::monostate>();
						return false;
					}
				}

				rc_data::rc_res_inst_ref<rc_data::rc_sgl<cmd_mgr>> cmd_mgr_tgt_res = rule_ref.get_cmd_set().get(static_cast<cmd_data::cmd_mgr_index>(cmd_tgt_ref.get_ownership()));

				if (cmd_mgr_tgt_res.has_value() == false)
				{
					op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>(op_parse_v, cmd_tgt_ref, std::nullopt, std::nullopt);
					set_int_info_cmd(cmd_mgr_tgt_res.error(), rdata::rule_int_t::parse_op_cmd_mgr_get_error);
					return false;
				}

				cmd_mgr& cmd_mgr_tgt_ref = cmd_mgr_tgt_res.value().get().get_content();
				cmd_data::cmd_mgr_res_cmd<rdata::rule_cmd_sgl> cmd_sgl_tgt_res = cmd_mgr_tgt_ref.get(cmd_tgt_ref.get_index());

				if (cmd_sgl_tgt_res.has_value() == false)
				{
					op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>(op_parse_v, cmd_tgt_ref, std::nullopt, std::nullopt);
					set_int_info_cmd(cmd_sgl_tgt_res.error(), rdata::rule_int_t::parse_op_cmd_mgr_sgl_get_error);
					return false;
				}

				rdata::rule_cmd_sgl& cmd_sgl_tgt_ref = cmd_sgl_tgt_res.value();

				switch (cmd_tgt_ref.get_handle_type())
				{
				case cmd_data::op_cmd_type::cmd_type:
				{
					op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>(op_parse_v, cmd_tgt_ref, parse_data::op_cmd_info<alloc_rule>(cmd_mgr_tgt_ref, cmd_sgl_tgt_ref.get_type(), {}), std::nullopt);
					break;
				}
				case cmd_data::op_cmd_type::cmd_val:
				{
					std::span<cmd_data::cmd_op_single>& cmd_sgl_tgt_op_span_ref = cmd_sgl_tgt_ref.get_content_span();

					std::size_t begin_num = static_cast<std::size_t>(cmd_tgt_ref.get_begin());
					std::size_t width_num = static_cast<std::size_t>(cmd_tgt_ref.get_width());

					if (begin_num > (std::numeric_limits<std::size_t>::max() - width_num))
					{
						op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>(op_parse_v, cmd_tgt_ref, std::nullopt, std::nullopt);
						set_int_info_cmd(std::monostate(), rdata::rule_int_t::parse_op_cmd_mgr_sgl_span_get_range_overflow);
						return false;
					}

					if (begin_num + width_num > cmd_sgl_tgt_op_span_ref.size())
					{
						op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>(op_parse_v, cmd_tgt_ref, std::nullopt, std::nullopt);
						set_int_info_cmd(std::monostate(), rdata::rule_int_t::parse_op_cmd_mgr_sgl_span_get_range_overflow);
						return false;
					}

					op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>
					(
						op_parse_v,
						cmd_tgt_ref,
						parse_data::op_cmd_info<alloc_rule>
						(
							cmd_mgr_tgt_ref,
							cmd_sgl_tgt_ref.get_type().get(),
							cmd_sgl_tgt_op_span_ref.subspan(static_cast<std::size_t>(cmd_tgt_ref.get_begin()), static_cast<std::size_t>(cmd_tgt_ref.get_width()))
						),
						std::nullopt
					);

					break;
				}
				}

				break;
			}
			case cmd_data::op_cmd_type::cmd_ptr:
			{
				if (is_alloc_rc == true)
				{
					if (ralloc::alloc_rc<alloc_rule>(int_info_save, run_param_v, cmd_tgt_ref, is_incr_ref, is_incr_ref) == false)
					{
						op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>(op_parse_v, cmd_tgt_ref, std::nullopt, std::nullopt);
						return false;
					}
				}

				rc_data::rc_res_inst_ref<rc_data::rc_sgl<var_mgr>> var_mgr_tgt_res = rule_ref.get_var_set().get(static_cast<var_cont_data::var_mgr_index>(cmd_tgt_ref.get_ownership()));

				if (var_mgr_tgt_res.has_value() == false)
				{
					op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>(op_parse_v, cmd_tgt_ref, std::nullopt, std::nullopt);
					set_int_info_cmd(var_mgr_tgt_res.error(), rdata::rule_int_t::parse_op_cmd_var_mgr_ptr_get_error);
					return false;
				}

				var_mgr& var_mgr_tgt_ref = var_mgr_tgt_res.value().get().get_content();
				var_data::var_mgr_res_var_sgl<rdata::rule_var_base_t, alloc_var> var_sgl_tgt_res = var_mgr_tgt_ref.get(static_cast<var_cont_data::var_index>(cmd_tgt_ref.get_index()));

				if (var_sgl_tgt_res.has_value() == false)
				{
					op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>(op_parse_v, cmd_tgt_ref, std::nullopt, std::nullopt);
					set_int_info_cmd(var_sgl_tgt_res.error(), rdata::rule_int_t::parse_op_cmd_var_mgr_sgl_ptr_get_error);
					return false;
				}

				var_data::var_sgl<rdata::rule_var_base_t, alloc_var>& var_sgl_tgt_ref = var_sgl_tgt_res.value().get();
				var_cont_data::var_cont_res_span<rdata::rule_var_base_t> var_span_tgt_res = var_sgl_tgt_ref.get_content().read(static_cast<var_cont_data::var_pos>(cmd_tgt_ref.get_begin()), static_cast<var_cont_data::var_read_width>(cmd_data::RAW_BYTES_SIZE_CMD_TARGET));

				if (var_span_tgt_res.has_value() == false)
				{
					op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>(op_parse_v, cmd_tgt_ref, std::nullopt, std::nullopt);
					set_int_info_cmd(var_span_tgt_res.error(), rdata::rule_int_t::parse_op_cmd_var_mgr_sgl_span_ptr_get_error);
					return false;
				}

				cmd_data::cmd_target cmd_tgt_ptr;

				if (trans::transform_raw_bytes_to(int_info_save, var_span_tgt_res.value(), cmd_tgt_ptr) == false)
				{
					op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>(op_parse_v, cmd_tgt_ref, std::nullopt, std::nullopt);
					return false;
				}

				switch (cmd_tgt_ptr.get_handle_type())
				{
				case cmd_data::op_cmd_type::cmd_tgt:
				{
					op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>
					(
						op_parse_v,
						cmd_tgt_ref,
						std::nullopt,
						parse_data::op_var_info<alloc_rule>
						(
							var_mgr_tgt_ref,
							var_sgl_tgt_ref.get_type(),
							var_span_tgt_res.value()
						)
					);

					break;
				}
				case cmd_data::op_cmd_type::cmd_type: [[fallthrough]];
				case cmd_data::op_cmd_type::cmd_self: [[fallthrough]];
				case cmd_data::op_cmd_type::cmd_val:
				{
					rc_data::rc_res_inst_ref<rc_data::rc_sgl<cmd_mgr>> cmd_mgr_tgt_res = rule_ref.get_cmd_set().get(static_cast<cmd_data::cmd_mgr_index>(cmd_tgt_ptr.get_ownership()));

					if (cmd_mgr_tgt_res.has_value() == false)
					{
						op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>(op_parse_v, cmd_tgt_ref, std::nullopt, parse_data::op_var_info<alloc_rule>(var_mgr_tgt_ref, var_sgl_tgt_ref.get_type(), var_span_tgt_res.value()));
						set_int_info_cmd(cmd_mgr_tgt_res.error(), rdata::rule_int_t::parse_op_cmd_mgr_ptr_get_error);
						return false;
					}

					cmd_mgr& cmd_mgr_tgt_ref = cmd_mgr_tgt_res.value().get().get_content();
					cmd_data::cmd_mgr_res_cmd<rdata::rule_cmd_sgl> cmd_sgl_tgt_res = cmd_mgr_tgt_ref.get(cmd_tgt_ptr.get_index());

					if (cmd_sgl_tgt_res.has_value() == false)
					{
						op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>(op_parse_v, cmd_tgt_ref, std::nullopt, parse_data::op_var_info<alloc_rule>(var_mgr_tgt_ref, var_sgl_tgt_ref.get_type(), var_span_tgt_res.value()));
						set_int_info_cmd(cmd_sgl_tgt_res.error(), rdata::rule_int_t::parse_op_cmd_mgr_sgl_ptr_get_error);
						return false;
					}

					rdata::rule_cmd_sgl& cmd_sgl_tgt_ref = cmd_sgl_tgt_res.value();
					std::span<cmd_data::cmd_op_single>& cmd_sgl_tgt_op_span_ref = cmd_sgl_tgt_ref.get_content_span();

					switch (cmd_tgt_ptr.get_handle_type())
					{
					case cmd_data::op_cmd_type::cmd_type:
					{
						op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>(op_parse_v, cmd_tgt_ref, parse_data::op_cmd_info<alloc_rule>(cmd_mgr_tgt_ref, cmd_sgl_tgt_ref.get_type().get(), {}), std::nullopt);
						break;
					}
					case cmd_data::op_cmd_type::cmd_self:
					{
						if (static_cast<std::size_t>(cmd_tgt_ptr.get_begin()) >= cmd_sgl_tgt_op_span_ref.size())
						{
							op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>(op_parse_v, cmd_tgt_ref, std::nullopt, parse_data::op_var_info<alloc_rule>(var_mgr_tgt_ref, var_sgl_tgt_ref.get_type(), var_span_tgt_res.value()));
							set_int_info_cmd(std::monostate(), rdata::rule_int_t::parse_op_cmd_mgr_sgl_span_ptr_get_range_overflow);
							return false;
						}

						op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>
						(
							op_parse_v,
							cmd_tgt_ref,
							parse_data::op_cmd_info<alloc_rule>
							(
								cmd_mgr_tgt_ref,
								cmd_sgl_tgt_ref.get_type().get(),
								cmd_sgl_tgt_op_span_ref.subspan(static_cast<std::size_t>(cmd_tgt_ptr.get_begin()), 1)
							),
							parse_data::op_var_info<alloc_rule>
							(
								var_mgr_tgt_ref,
								var_sgl_tgt_ref.get_type(),
								var_span_tgt_res.value()
							)
						);

						break;
					}
					case cmd_data::op_cmd_type::cmd_val:
					{
						std::size_t begin_num = static_cast<std::size_t>(cmd_tgt_ptr.get_begin());
						std::size_t width_num = static_cast<std::size_t>(cmd_tgt_ptr.get_width());

						if (begin_num > std::numeric_limits<std::size_t>::max() - width_num)
						{
							op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>(op_parse_v, cmd_tgt_ref, std::nullopt, parse_data::op_var_info<alloc_rule>(var_mgr_tgt_ref, var_sgl_tgt_ref.get_type(), var_span_tgt_res.value()));
							set_int_info_cmd(std::monostate(), rdata::rule_int_t::parse_op_cmd_mgr_sgl_span_ptr_get_range_overflow);
							return false;
						}

						if (begin_num + width_num > cmd_sgl_tgt_op_span_ref.size())
						{
							op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>(op_parse_v, cmd_tgt_ref, std::nullopt, parse_data::op_var_info<alloc_rule>(var_mgr_tgt_ref, var_sgl_tgt_ref.get_type(), var_span_tgt_res.value()));
							set_int_info_cmd(std::monostate(), rdata::rule_int_t::parse_op_cmd_mgr_sgl_span_ptr_get_range_overflow);
							return false;
						}

						op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>
						(
							op_parse_v,
							cmd_tgt_ref,
							parse_data::op_cmd_info<alloc_rule>
							(
								cmd_mgr_tgt_ref,
								cmd_sgl_tgt_ref.get_type().get(),
								cmd_sgl_tgt_op_span_ref.subspan(static_cast<std::size_t>(cmd_tgt_ptr.get_begin()), static_cast<std::size_t>(cmd_tgt_ptr.get_width()))
							),
							parse_data::op_var_info<alloc_rule>
							(
								var_mgr_tgt_ref,
								var_sgl_tgt_ref.get_type(),
								var_span_tgt_res.value()
							)
						);

						break;
					}
					}

					break;
				}
				case cmd_data::op_cmd_type::cmd_ptr:
				{
					op_parse_res_save.template emplace<parse_data::op_cmd<alloc_rule>>
					(
						op_parse_v,
						cmd_tgt_ref,
						std::nullopt,
						parse_data::op_var_info<alloc_rule>
						(
							var_mgr_tgt_ref,
							var_sgl_tgt_ref.get_type(),
							var_span_tgt_res.value()
						)
					);

					break;
				}
				default:
				{
					op_parse_res_save.template emplace<std::monostate>();
					set_int_info_cmd(std::monostate(), rdata::rule_int_t::parse_op_cmd_t_no_match);
					return false;
				}
				}
				break;
			}
			default:
			{
				op_parse_res_save.template emplace<std::monostate>();
				set_int_info_cmd(std::monostate(), rdata::rule_int_t::parse_op_cmd_t_no_match);
				return false;
			}
			}

			return true;
		};

		return std::visit(rdata::t_case_distribute{ handle_imm ,handle_var ,handle_cmd }, op_parse_v);
	}

	template<typename alloc_rule, typename outer>
	inline bool get_parse_op_mul(rdata::rule_int_info& int_info_save, rdata::rule_cmd_act_param<alloc_rule, outer>& run_param_v, const std::initializer_list<parse_data::op_handle_set<alloc_rule>>& op_parse_set)
	{
		typename std::initializer_list<parse_data::op_handle_set<alloc_rule>>::const_iterator op_parse_it = op_parse_set.begin();

		if (op_parse_it == op_parse_set.end())
			return true;

		parse_data::op<alloc_rule> op_alloc;

		do
		{
			if (op_parse_it->get_is_alloc_rc() == true)
			{
				bool parse_res = get_parse_op<alloc_rule, outer>
				(
					int_info_save,
					run_param_v,
					op_parse_it->get_cmd_sgl_tgt().get(),
					op_parse_it->get_op_tgt_index(),
					op_alloc,
					true,
					op_parse_it->get_is_incr_ref()
				);

				if (parse_res == false)
					return false;

			}
		} while (++op_parse_it != op_parse_set.end());

		op_parse_it = op_parse_set.begin();

		do
		{
			bool parse_res = get_parse_op<alloc_rule, outer>
			(
				int_info_save,
				run_param_v,
				op_parse_it->get_cmd_sgl_tgt().get(),
				op_parse_it->get_op_tgt_index(),
				op_parse_it->get_parse_save().get(),
				false,
				op_parse_it->get_is_incr_ref()
			);

			if (parse_res == false)
				return false;

		} while (++op_parse_it != op_parse_set.end());

		return true;
	}

	template<typename alloc_rule>
	inline void get_op_parse_span(parse_data::op<alloc_rule>& op_parse_v, parse_data::op_span& op_parse_span_save)
	{
		auto handle_none = [&](std::monostate&) -> void
		{
			op_parse_span_save.template emplace<std::monostate>();
			return;
		};

		auto handle_imm = [&](parse_data::op_imm& parse_imm_ref) -> void
		{
			if (parse_imm_ref.get_imm_span().has_value() == true) op_parse_span_save.template emplace<std::span<rdata::rule_var_base_t>>(parse_imm_ref.get_imm_span().value());
			else op_parse_span_save.template emplace<std::span<cmd_data::cmd_op_single>>(&parse_imm_ref.get_op_ow().get(), 1);

			return;
		};

		auto handle_var = [&](parse_data::op_var<alloc_rule>& parse_var_ref) -> void
		{
			cmd_data::var_target& var_tgt_ref = parse_var_ref.get_var_tgt().get();

			switch (var_tgt_ref.get_handle_type())
			{
			case cmd_data::op_var_type::var_self:
			{
				op_parse_span_save.template emplace<std::span<cmd_data::cmd_op_single>>(&parse_var_ref.get_op_ow().get(), 1);
				break;
			}
			case cmd_data::op_var_type::var_tgt:
			{
				op_parse_span_save.template emplace<std::span<cmd_data::var_target>>(&var_tgt_ref, 1);
				break;
			}
			case cmd_data::op_var_type::var_del:
			{
				op_parse_span_save.template emplace<parse_data::op_del>(var_tgt_ref);
				break;
			}
			case cmd_data::op_var_type::var_type: [[fallthrough]];
			case cmd_data::op_var_type::var_val:
			{
				if (parse_var_ref.get_var_info_val().has_value() == false)
				{
					op_parse_span_save.template emplace<std::monostate>();
					break;
				}

				parse_data::op_var_info<alloc_rule>& info_ref = parse_var_ref.get_var_info_val().value();

				switch (var_tgt_ref.get_handle_type())
				{
				case cmd_data::op_var_type::var_type:
				{
					op_parse_span_save.template emplace<std::span<cmd_data::var_cmd_type>>(&info_ref.get_var_type_tgt().get(), 1);
					break;
				}
				case cmd_data::op_var_type::var_val:
				{
					op_parse_span_save.template emplace<std::span<rdata::rule_var_base_t>>(info_ref.get_var_span());
					break;
				}
				}


				break;
			}
			case cmd_data::op_var_type::var_ptr:
			{
				if (parse_var_ref.get_var_info_ptr().has_value() == false)
				{
					op_parse_span_save.template emplace<std::monostate>();
					break;
				}

				parse_data::op_var_info<alloc_rule>& info_ptr_ref = parse_var_ref.get_var_info_ptr().value();

				if (parse_var_ref.get_var_info_val().has_value() == false)
				{
					op_parse_span_save.template emplace<std::span<rdata::rule_var_base_t>>(info_ptr_ref.get_var_span());
					break;
				}

				parse_data::op_var_info<alloc_rule>& info_val_ref = parse_var_ref.get_var_info_val().value();
				op_parse_span_save.template emplace<std::span<rdata::rule_var_base_t>>(info_val_ref.get_var_span());
				break;
			}
			}

			return;
		};

		auto handle_cmd = [&](parse_data::op_cmd<alloc_rule>& parse_cmd_ref) -> void
		{
			cmd_data::cmd_target& cmd_tgt_ref = parse_cmd_ref.get_cmd_tgt();

			switch (cmd_tgt_ref.get_handle_type())
			{
			case cmd_data::op_cmd_type::cmd_self:
			{
				op_parse_span_save.template emplace<std::span<cmd_data::cmd_op_single>>(&parse_cmd_ref.get_op_ow().get(), 1);
				break;
			}
			case cmd_data::op_cmd_type::cmd_tgt:
			{
				op_parse_span_save.template emplace<std::span<cmd_data::cmd_target>>(&cmd_tgt_ref, 1);
				break;
			}
			case cmd_data::op_cmd_type::cmd_del:
			{
				op_parse_span_save.template emplace<parse_data::op_del>(cmd_tgt_ref);
				break;
			}
			case cmd_data::op_cmd_type::cmd_type: [[fallthrough]];
			case cmd_data::op_cmd_type::cmd_val:
			{
				if (parse_cmd_ref.get_cmd_info_val().has_value() == false)
				{
					op_parse_span_save.template emplace<std::monostate>();
					break;
				}

				parse_data::op_cmd_info<alloc_rule>& parse_cmd_info_ref = parse_cmd_ref.get_cmd_info_val().value();

				switch (cmd_tgt_ref.get_handle_type())
				{
				case cmd_data::op_cmd_type::cmd_type:
				{
					op_parse_span_save.template emplace<std::span<cmd_data::var_cmd_type>>(&parse_cmd_info_ref.get_cmd_type_tgt().get(), 1);
					break;
				}
				case cmd_data::op_cmd_type::cmd_val:
				{
					op_parse_span_save.template emplace<std::span<cmd_data::cmd_op_single>>(parse_cmd_info_ref.get_cmd_op_span());
					break;
				}
				}

				break;
			}
			case cmd_data::op_cmd_type::cmd_ptr:
			{
				if (parse_cmd_ref.get_cmd_info_ptr().has_value() == false)
				{
					op_parse_span_save.template emplace<std::monostate>();
					break;
				}

				parse_data::op_var_info<alloc_rule>& cmd_info_ptr_ref = parse_cmd_ref.get_cmd_info_ptr().value();

				if (parse_cmd_ref.get_cmd_info_val().has_value() == false)
				{
					op_parse_span_save.template emplace<std::span<rdata::rule_var_base_t>>(cmd_info_ptr_ref.get_var_span());
					break;
				}

				parse_data::op_cmd_info<alloc_rule>& parse_cmd_info_ref = parse_cmd_ref.get_cmd_info_val().value();
				op_parse_span_save.template emplace<std::span<cmd_data::cmd_op_single>>(parse_cmd_info_ref.get_cmd_op_span());
				break;
			}
			}

			return;
		};

		std::visit(rdata::t_case_distribute{ handle_none, handle_imm ,handle_var ,handle_cmd }, op_parse_v);
		return;
	}

	template<typename alloc_rule>
	inline bool get_op_parse_span_mul(const std::initializer_list<parse_data::op_span_set<alloc_rule>>& op_parse_span_get_set)
	{
		typename std::initializer_list<parse_data::op_span_set<alloc_rule>>::const_iterator op_parse_span_get_it = op_parse_span_get_set.begin();

		if (op_parse_span_get_it == op_parse_span_get_set.end())
			return true;

		do
		{
			get_op_parse_span<alloc_rule>(op_parse_span_get_it->get_parse_get_tgt(), op_parse_span_get_it->get_span_tgt_save().get());
		} while (++op_parse_span_get_it != op_parse_span_get_set.end());

		return true;
	}
}
