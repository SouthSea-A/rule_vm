#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <cstddef>
#include <memory>
#include <utility>
#include <type_traits>
#include <functional>
#include <limits>
#include <initializer_list>
#include <array>
#include <optional>
#include <span>
#include <vector>
#include <unordered_map>
#include <variant>
#include "rule_data_def.h"
#include "rule_check_data_def.h"
#include "rule_check.h"
#include "rule_transform.h"
#include "rule_alloc.h"
#include "rule_parse_data_def.h"
#include "rule_parse.h"
#include "rule_write_data_def.h"
#include "rule_write.h"
#include "rule_kit_data_def.h"

namespace rule_tool
{
	template<typename alloc_var>
	using rule_var = var_mgr_ns::variable_manager<rdata::rule_var_base_t, alloc_var>;

	template<typename alloc_var_rc>
	using rule_var_rc = rc_ns::rc_variable<rdata::rule_var_base_t, alloc_var_rc>;

	template<typename alloc_cmd>
	using rule_cmd = cmd_mgr_ns::command_manager<alloc_cmd>;

	template<typename alloc_cmd_rc>
	using rule_cmd_rc = rc_ns::rc_command<alloc_cmd_rc>;

	template<typename alloc_cmd_rc, typename alloc_stk>
	using rule_stk = std::vector<act_data::act_cmd_ip<rule_cmd_rc<alloc_cmd_rc>>, alloc_stk>;

	template<typename alloc_rule, typename user_outer>
	class rule
	{
	public:
		using kit_t = rule_kit<alloc_rule, user_outer>;
		using outer_t = rule_kit_outer<kit_t, user_outer>;
		using alloc_traits = typename std::allocator_traits<alloc_rule>;
		using alloc_var = rdata::rule_alloc_var<alloc_rule>;
		using alloc_cmd = rdata::rule_alloc_cmd<alloc_rule>;
		using alloc_stk = rdata::rule_alloc_stk<alloc_rule>;
		using var_mgr = rdata::rule_var_mgr<alloc_rule>;
		using cmd_mgr = rdata::rule_cmd_mgr<alloc_rule>;
		using var_act = rdata::rule_var_act<alloc_rule>;
		using cmd_act = rdata::rule_cmd_act<alloc_rule>;
		using stk_act = rdata::rule_stk_act<alloc_rule>;
		using cmd_def_act = rdata::rule_cmd_def<cmd_act, outer_t>;
		using alloc_cmd_def_set = alloc_traits::template rebind_alloc<std::pair<const cmd_data::cmd_type, cmd_def_act>>;
		using cmd_def_set_act = rdata::rule_cmd_def_set<cmd_act, outer_t, alloc_cmd_def_set>;
		using cmd_act_param = rdata::rule_cmd_act_param<alloc_rule, outer_t>;
		using cmd_act_res = rdata::rule_cmd_act_res<alloc_rule>;
		using run_ip = rdata::rule_run_ip<alloc_rule>;
		using parse_op = parse_data::op<alloc_rule>;
		using op_del = parse_data::op_del;
		using op_parse_span = parse_data::op_span;
		using parse_run_param = rparse::run_param<alloc_rule, outer_t>;

		using serialize_var_base_t = rdata::rule_var_base_t;
		using serialize_cmd_op_single_t = cmd_data::cmd_op_single;
		using serialize_imm_target_t = cmd_data::imm_target;
		using serialize_var_target_t = cmd_data::var_target;
		using serialize_cmd_target_t = cmd_data::cmd_target;
		using serialize_info_t = rdata::rule_int_info;
		using serialize_var_mgr_index_t = var_cont_data::var_mgr_index;
		using serialize_var_index_t = var_cont_data::var_index;
		using serialize_var_type_t = var_cont_data::var_type;
		using serialize_var_ref_t = var_cont_data::var_ref;
		using serialize_var_pos_t = var_cont_data::var_pos;
		using serialize_cmd_mgr_index_t = cmd_data::cmd_mgr_index;
		using serialize_cmd_index_t = cmd_data::cmd_index;
		using serialize_cmd_type_t = cmd_data::cmd_type;
		using serialize_cmd_op_index_t = cmd_data::cmd_op_index;
		using serialize_cmd_op_width_t = cmd_data::cmd_op_width;

		static constexpr std::size_t serialize_raw_bytes_size_imm = cmd_data::RAW_BYTES_SIZE_IMM_TARGET;
		static constexpr std::size_t serialize_raw_bytes_size_var = cmd_data::RAW_BYTES_SIZE_VAR_TARGET;
		static constexpr std::size_t serialize_raw_bytes_size_cmd = cmd_data::RAW_BYTES_SIZE_CMD_TARGET;
		static constexpr std::size_t serialize_raw_bytes_size_op = cmd_data::RAW_BYTES_SIZE_CMD_OP;

		using cb_int_outer = rule_kit_cmd_cb_int<alloc_rule, user_outer>;
		using cb_rc_err_outer = rule_kit_cmd_cb_rc_err<alloc_rule, user_outer>;
		using cb_err_outer = rule_kit_cmd_cb_err<alloc_rule, user_outer>;
		using cb_hook_outer = rule_kit_cmd_cb_hook<alloc_rule, user_outer>;
	public:
		struct cb_def
		{
		public:
			struct int_cb
			{
			public:
				cb_int_outer cb_outer;
			public:
				int_cb() = delete;
				int_cb(const cb_int_outer& cb_outer_v) :
					cb_outer(cb_outer_v)
				{

				}
			public:
				template<typename self_this>
				decltype(auto) get_cb_outer(this self_this&& self) noexcept
				{
					return std::forward_like<self_this>(self.cb_outer);
				}
			public:
				void set_cb_outer(const cb_int_outer& cb_outer_v)
				{
					this->cb_outer = cb_outer_v;
				}
			public:
				void operator()(cmd_act_param& run_param, cmd_act_res& int_param, bool& is_end)
				{
					if (this->cb_outer == nullptr)
					{
						is_end = true;
						return;
					}

					this->cb_outer
					(
						*run_param.get_data_outer(),
						int_param.get_type(),
						int_param.get_info(),
						int_param.get_ip_current(),
						int_param.get_ip_target(),
						run_param.get_cmd_it().is_valid() == false ? std::nullopt : std::make_optional(*run_param.get_cmd_it()),
						is_end
					);

					return;
				}
			};
			struct err_cb_rc
			{
			public:
				cb_rc_err_outer cb_outer;
			public:
				template<typename self_this>
				decltype(auto) get_cb_outer(this self_this&& self) noexcept
				{
					return std::forward_like<self_this>(self.cb_outer);
				}
			public:
				void set_cb_outer(const cb_rc_err_outer& cb_outer_v)
				{
					this->cb_outer = cb_outer_v;
				}
			public:
				err_cb_rc() = delete;
				err_cb_rc(const cb_rc_err_outer& cb_outer_v) :
					cb_outer(cb_outer_v)
				{

				}
			public:
				void operator()(act_data::act_err_param<cmd_act, typename cmd_act::unexpected_error_type, outer_t> err_param, bool& is_end)
				{
					if (this->cb_outer == nullptr)
					{
						is_end = true;
						return;
					}

					act_data::act_err_param_base<cmd_act, outer_t>& base_ref = err_param.get_base();

					this->cb_outer
					(
						*base_ref.get_data_outer(),
						err_param.get_error_outer(),
						base_ref.get_ip(),
						is_end
					);

					return;
				}
			};
			struct err_cb
			{
			public:
				cb_err_outer cb_outer;
			public:
				template<typename self_this>
				decltype(auto) get_cb_outer(this self_this&& self) noexcept
				{
					return std::forward_like<self_this>(self.cb_outer);
				}
			public:
				void set_cb_outer(const cb_err_outer& cb_outer_v)
				{
					this->cb_outer = cb_outer_v;
				}
			public:
				err_cb() = delete;
				err_cb(const cb_err_outer& cb_outer_v) :
					cb_outer(cb_outer_v)
				{

				}
			public:
				void operator()(act_data::act_err_param_base<cmd_act, outer_t> err_param, bool& is_end)
				{
					if (this->cb_outer == nullptr)
					{
						is_end = true;
						return;
					}

					this->cb_outer
					(
						*err_param.get_data_outer(),
						err_param.get_error(),
						err_param.get_ip(),
						is_end
					);
					return;
				}
			};
			struct hook_cb
			{
			public:
				cb_hook_outer cb_outer;
			public:
				template<typename self_this>
				decltype(auto) get_cb_outer(this self_this&& self) noexcept
				{
					return std::forward_like<self_this>(self.cb_outer);
				}
			public:
				void set_cb_outer(const cb_hook_outer& cb_outer_v)
				{
					this->cb_outer = cb_outer_v;
				}
			public:
				hook_cb() = delete;
				hook_cb(const cb_hook_outer& cb_outer_v) :
					cb_outer(cb_outer_v)
				{

				}
			public:
				act_data::act_hook_t operator()(act_data::act_hook_param<cmd_act, outer_t> hook_param)
				{
					if (this->cb_outer == nullptr)
						return act_data::act_hook_t::pass;

					return this->cb_outer
					(
						*hook_param.get_data_outer(),
						hook_param.get_ip(),
						hook_param.get_cmd_it().is_valid() == false ? std::nullopt : std::make_optional(*hook_param.get_cmd_it())
					);
				}
			};
		public:
			using call_back_interrupt_info_type = rdata::rule_int_info;
			using call_back_interrupt_type = int_cb;
			using call_back_error_resource_type = err_cb_rc;
			using call_back_error_type = err_cb;
			using call_back_hook_type = hook_cb;
		public:
			int_cb cb_int;
			err_cb_rc cb_err_rc;
			err_cb cb_err;
			hook_cb cb_hook;
		public:
			template<typename self_this>
			decltype(auto) get_call_back_interrupt(this self_this&& self) noexcept
			{
				return std::forward_like<self_this>(self.cb_int);
			}
			template<typename self_this>
			decltype(auto) get_call_back_error_rc(this self_this&& self) noexcept
			{
				return std::forward_like<self_this>(self.cb_err_rc);
			}
			template<typename self_this>
			decltype(auto) get_call_back_error(this self_this&& self) noexcept
			{
				return std::forward_like<self_this>(self.cb_err);
			}
			template<typename self_this>
			decltype(auto) get_call_back_hook(this self_this&& self) noexcept
			{
				return std::forward_like<self_this>(self.cb_hook);
			}
		public:
			void set_call_back_interrupt(const cb_int_outer& cb_int_v)
			{
				this->cb_int.set_cb_outer(cb_int_v);
				return;
			}
			void set_call_back_error_rc(const cb_rc_err_outer& cb_err_rc_v)
			{
				this->cb_err_rc.set_cb_outer(cb_err_rc_v);
				return;
			}
			void set_call_back_error(const cb_err_outer& cb_err_v)
			{
				this->cb_err.set_cb_outer(cb_err_v);
				return;
			}
			void set_call_back_hook(const cb_hook_outer& cb_hook_v)
			{
				this->cb_hook.set_cb_outer(cb_hook_v);
				return;
			}
		public:
			cb_def(const cb_int_outer& cb_int_v, const cb_rc_err_outer& cb_err_rc_v, const cb_err_outer& cb_err_v, const cb_hook_outer& cb_hook_v) :
				cb_int(cb_int_v), cb_err_rc(cb_err_rc_v), cb_err(cb_err_v), cb_hook(cb_hook_v)
			{

			}
		};
		using act_def = act_ns::actuator<cmd_act, stk_act, cmd_def_set_act, cb_def, outer_t>;
	private:
		var_act var_set;
		cmd_act cmd_set;
		stk_act stack;
		cmd_def_set_act cmd_def_set;
		cb_def call_bak_set;
		outer_t outer_data;
		act_def actuator_;
	public:
		template<typename self_this>
		decltype(auto) get_var_set(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.var_set);
		}
		template<typename self_this>
		decltype(auto) get_cmd_set(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.cmd_set);
		}
		template<typename self_this>
		decltype(auto) get_stack(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.stack);
		}
		template<typename self_this>
		decltype(auto) get_cmd_def_set(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.cmd_def_set);
		}
		template<typename self_this>
		decltype(auto) get_call_bak_set(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.call_bak_set);
		}
		template<typename self_this>
		decltype(auto) get_actuator_(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.actuator_);
		}
	public:
		void set_int_info_run(rdata::rule_int_info& int_info_save, rdata::rule_int_t int_t_v, const rdata::rule_int_info_base_error_t& type_base_int_v, const rdata::rule_int_info_cont& cont_int_v)
		{
			rdata::set_int_info(int_info_save, int_t_v, type_base_int_v, cont_int_v);
			return;
		}
		void set_act_run_int(cmd_act_res& act_run_int_save, act_data::act_run_int_t act_int_t_v, cmd_act_param& run_param_v, const std::optional<run_ip>& ip_tgt_v, const rdata::rule_int_info& rule_int_info_v)
		{
			act_run_int_save.set_type(act_int_t_v);
			act_run_int_save.set_ip_current(run_param_v.get_ip());
			act_run_int_save.set_ip_target(ip_tgt_v);
			act_run_int_save.set_info(rule_int_info_v);
			return;
		}
		bool check_cmd_info_run(rdata::rule_int_info& int_info_save, const rdata::rule_cmd_sgl& cmd_tgt_v, cmd_data::cmd_type cmd_tgt_t_v, std::size_t op_size_v)
		{
			return check_ns::check_cmd_info(int_info_save, cmd_tgt_v, cmd_tgt_t_v, op_size_v);
		}
		bool check_cmd_op_info_run(rdata::rule_int_info& int_info_save, const rdata::rule_cmd_sgl& rule_cmd_sgl_v, cmd_data::cmd_op_index op_index_check_v, cmd_data::cmd_op_type op_t_check_v, bool is_exc)
		{
			return check_ns::check_cmd_op_info(int_info_save, rule_cmd_sgl_v, op_index_check_v, op_t_check_v, is_exc);
		}
		bool check_cmd_op_info_mul_run(rdata::rule_int_info& int_info_save, const std::initializer_list<check_data::ck_cmd_op_set>& op_check_set)
		{
			return check_ns::check_cmd_op_info_mul(int_info_save, op_check_set);
		}
		template<typename tgt_t>
		bool transform_raw_bytes_to_run(rdata::rule_int_info& int_info_save, const std::span<rdata::rule_var_base_t>& span_raw_bytes_v, tgt_t& res_save)
		{
			return trans::transform_raw_bytes_to(int_info_save, span_raw_bytes_v, res_save);
		}
		template<typename tgt_t>
		bool transform_tgt_to_raw_bytes_run(rdata::rule_int_info& int_info_save, const tgt_t& tgt_v, const std::span<rdata::rule_var_base_t>& span_raw_bytes_save)
		{
			return trans::transform_tgt_to_raw_bytes(int_info_save, tgt_v, span_raw_bytes_save);
		}
		template<typename rc_tgt_t>
		bool alloc_rc_run(rdata::rule_int_info& int_info_save, cmd_act_param& run_param_v, const rc_tgt_t& rc_tgt_v, bool is_incr_ref_ow, bool is_incr_ref_tgt)
		{
			return ralloc::alloc_rc<alloc_rule, outer_t>(int_info_save, run_param_v, rc_tgt_v, is_incr_ref_ow, is_incr_ref_tgt);
		}
		template<typename rc_tgt_t>
		bool dealloc_rc_run(rdata::rule_int_info& int_info_save, cmd_act_param& run_param_v, const rc_tgt_t& rc_tgt_v)
		{
			return ralloc::dealloc_rc<alloc_rule, outer_t>(int_info_save, run_param_v, rc_tgt_v);
		}
		bool get_parse_op_run(rdata::rule_int_info& int_info_save, cmd_act_param& run_param_v, const rdata::rule_cmd_sgl& rule_cmd_sgl_v, cmd_data::cmd_op_index op_parse_index_v, parse_op& op_parse_res_save, bool is_alloc_rc, bool is_incr_ref)
		{
			return rparse::get_parse_op<alloc_rule, outer_t>(int_info_save, run_param_v, rule_cmd_sgl_v, op_parse_index_v, op_parse_res_save, is_alloc_rc, is_incr_ref);
		}
		bool get_parse_op_mul_run(rdata::rule_int_info& int_info_save, cmd_act_param& run_param_v, const std::initializer_list<parse_data::op_handle_set<alloc_rule>>& op_parse_set)
		{
			return rparse::get_parse_op_mul<alloc_rule, outer_t>(int_info_save, run_param_v, op_parse_set);
		}
		void get_op_parse_span_run(parse_op& op_parse_v, op_parse_span& op_parse_span_save)
		{
			rparse::get_op_parse_span<alloc_rule>(op_parse_v, op_parse_span_save);
			return;
		}
		bool get_op_parse_span_mul_run(const std::initializer_list<parse_data::op_span_set<alloc_rule>>& op_parse_span_get_set)
		{
			return rparse::get_op_parse_span_mul<alloc_rule>(op_parse_span_get_set);
		}
		bool write_op_span_run(rdata::rule_int_info& int_info_save, cmd_act_param& run_param_v, parse_op& write_op_parse_tgt, parse_op& write_op_parse_src)
		{
			return rwrite::write_op_span<alloc_rule, outer_t>(int_info_save, run_param_v, write_op_parse_tgt, write_op_parse_src);
		}
		bool write_op_span_mul_run(rdata::rule_int_info& int_info_save, cmd_act_param& run_param_v, const std::initializer_list<write_data::write_op_set<alloc_rule>>& write_set)
		{
			return rwrite::write_op_span_mul<alloc_rule, outer_t>(int_info_save, run_param_v, write_set);
		}
		void clear()
		{
			this->var_set.clear();
			this->cmd_set.clear();
			this->stack.clear();
			return;
		}
	public:
		rule() = delete;
		explicit rule(kit_t& rule_kit_v, user_outer& user_outer_v, const cb_int_outer& cb_int_v, const cb_rc_err_outer& cb_err_rc_v, const cb_err_outer& cb_err_v, const cb_hook_outer& cb_hook_v, const alloc_rule& alloc_val = alloc_rule()) :
			var_set(alloc_var(alloc_val)),
			cmd_set(alloc_cmd(alloc_val)),
			stack(alloc_stk(alloc_val)),
			cmd_def_set(alloc_cmd_def_set(alloc_val)),
			call_bak_set(cb_int_v, cb_err_rc_v, cb_err_v, cb_hook_v),
			outer_data(rule_kit_v, user_outer_v),
			actuator_(this->cmd_set, this->stack, this->cmd_def_set, this->call_bak_set, &this->outer_data)
		{

		}
		rule(const rule&) = delete;
		rule& operator=(const rule&) = delete;
		rule(rule&&) = delete;
		rule& operator=(rule&&) = delete;
	public:
		void set_cb_int(const cb_int_outer& cb_int_v)
		{
			this->call_bak_set.set_call_back_interrupt(cb_int_v);
			return;
		}
		void set_cb_err_rc(const cb_rc_err_outer& cb_err_rc_v)
		{
			this->call_bak_set.set_call_back_error_rc(cb_err_rc_v);
			return;
		}
		void set_cb_err(const cb_err_outer& cb_err_v)
		{
			this->call_bak_set.set_call_back_error(cb_err_v);
		}
		void set_cb_hook(const cb_hook_outer& cb_hook_v)
		{
			this->call_bak_set.set_call_back_hook(cb_hook_v);
			return;
		}
		void add_set_cmd_def(cmd_data::cmd_type cmd_id_v, cmd_def_act cmd_act_v)
		{
			this->cmd_def_set[cmd_id_v] = cmd_act_v;
			return;
		}
		bool add_set_cmd_set(cmd_data::cmd_mgr_index rc_index, cmd_data::cmd_index cmd_index_v, cmd_data::cmd_type cmd_type_v, cmd_data::cmd_op_index cmd_op_index_v, const std::initializer_list<cmd_data::cmd_op_single>& cmd_op_v)
		{
			rc_data::rc_res_inst_ref<rc_data::rc_sgl<cmd_mgr>> cmd_mgr_add_set_res = this->cmd_set.add(rc_index, false);

			if (cmd_mgr_add_set_res.has_value() == false)
				return false;

			cmd_mgr& cmd_mgr_ref = cmd_mgr_add_set_res.value().get().get_content();

			if (cmd_mgr_ref.add_set(cmd_index_v, cmd_type_v, cmd_op_index_v, std::span(cmd_op_v.begin(), cmd_op_v.size()), true).has_value() == false)
				return false;

			return true;
		}
		bool add_set_cmd_set(cmd_data::cmd_mgr_index rc_index, cmd_data::cmd_index cmd_index_v, cmd_data::cmd_type cmd_type_v, cmd_data::cmd_op_index cmd_op_index_v, std::span<const cmd_data::cmd_op_single> cmd_op_v)
		{
			rc_data::rc_res_inst_ref<rc_data::rc_sgl<cmd_mgr>> cmd_mgr_add_set_res = this->cmd_set.add(rc_index, false);

			if (cmd_mgr_add_set_res.has_value() == false)
				return false;

			cmd_mgr& cmd_mgr_ref = cmd_mgr_add_set_res.value().get().get_content();

			if (cmd_mgr_ref.add_set(cmd_index_v, cmd_type_v, cmd_op_index_v, cmd_op_v, true).has_value() == false)
				return false;

			return true;
		}
		bool add_set_var_set(var_cont_data::var_mgr_index rc_index, var_cont_data::var_index var_index_v, var_cont_data::var_type var_type_v, var_cont_data::var_pos pos_begin_v, const std::span<rdata::rule_var_base_t>& span_write_v)
		{
			rc_data::rc_res_inst_ref<rc_data::rc_sgl<var_mgr>> var_mgr_add_set_res = this->var_set.add(rc_index, false);

			if (var_mgr_add_set_res.has_value() == false)
				return false;

			var_mgr& var_mgr_ref = var_mgr_add_set_res.value().get().get_content();

			if (var_mgr_ref.add_set(var_index_v, var_type_v, pos_begin_v, span_write_v, true).has_value() == false)
				return false;

			return true;
		}
		void run(cmd_data::cmd_mgr_index rc_index, cmd_data::cmd_index cmd_index_v)
		{
			this->actuator_.run(run_ip(rc_index, cmd_index_v), true);
			return;
		}
	public:
		void resume(const run_ip& ip_v)
		{
			this->actuator_.run(ip_v, false);
			return;
		}
	public:
		void copy_from(const rule& rule_v)
		{
			this->var_set = rule_v.var_set;
			this->cmd_set = rule_v.cmd_set;
			this->stack = rule_v.stack;
			this->cmd_def_set = rule_v.cmd_def_set;
			this->call_bak_set = rule_v.call_bak_set;
			return;
		}
		void copy_to(kit_t& rule_kit_v, rule& rule_v)
		{
			rule_v.var_set = this->var_set;
			rule_v.cmd_set = this->cmd_set;
			rule_v.stack = this->stack;
			rule_v.cmd_def_set = this->cmd_def_set;
			rule_v.call_bak_set = this->call_bak_set;
			rule_v.outer_data = outer_t(rule_kit_v, rule_v.outer_data.get_user());
			rule_v.actuator_.set(rule_v.cmd_set, rule_v.stack, rule_v.cmd_def_set, rule_v.call_bak_set, &rule_v.outer_data);
			return;
		}
		void shrink_to_fit()
		{
			(void)this->var_set.shrink_to_fit();
			(void)this->cmd_set.shrink_to_fit();
			this->stack.shrink_to_fit();
			return;
		}
	};
}
