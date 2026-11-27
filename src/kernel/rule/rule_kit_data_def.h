#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <cstddef>
#include <utility>
#include <type_traits>
#include <functional>
#include <optional>
#include <span>
#include "rule_data_def.h"

namespace rule_tool
{
	using rule_kit_kernel_error = kernel_error_throw;

	using rule_kit_cmd_handle_type = cmd_data::op_cmd_type;
	using rule_kit_cmd_mgr_index = cmd_data::cmd_mgr_index;
	using rule_kit_cmd_type = cmd_data::cmd_type;
	using rule_kit_cmd_index = cmd_data::cmd_index;
	using rule_kit_cmd_op_index = cmd_data::cmd_op_index;

	using rule_kit_var_handle_type = cmd_data::op_var_type;
	using rule_kit_var_mgr_index = var_cont_data::var_mgr_index;
	using rule_kit_var_type = var_cont_data::var_type;
	using rule_kit_var_index = var_cont_data::var_index;
	using rule_kit_var_pos_index = var_cont_data::var_pos;

	using rule_kit_var_base = rdata::rule_var_base_t;

	using rule_kit_imm_handle_type = cmd_data::imm_type;

	using rule_kit_imm_tgt = cmd_data::imm_target;
	using rule_kit_var_tgt = cmd_data::var_target;
	using rule_kit_cmd_tgt = cmd_data::cmd_target;
	using rule_kit_cmd_op_sgl = cmd_data::cmd_op_single;

	using rule_kit_int_info = rdata::rule_int_info;

	using rule_kit_cmd_sgl = rdata::rule_cmd_sgl;

	template<typename alloc_rule_kit>
	using rule_kit_cmd_ip = rdata::rule_run_ip<alloc_rule_kit>;

	template<typename alloc_rule_kit, typename user_outer = std::monostate>
	struct rule_kit_cmd_info
	{
	public:
		using cmd_mgr_kit = typename rule<alloc_rule_kit, user_outer>::cmd_mgr;
	public:
		rule_kit_cmd_ip<alloc_rule_kit> cmd_ip;
		std::reference_wrapper<cmd_mgr_kit> cmd_mgr;
		rule_kit_cmd_sgl cmd_sgl;
	public:
		template<typename self_this>
		decltype(auto) get_cmd_ip(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.cmd_ip);
		}
		template<typename self_this>
		decltype(auto) get_cmd_mgr(this self_this&& self) noexcept
		{
			return self.cmd_mgr.get();
		}
		template<typename self_this>
		decltype(auto) get_cmd_sgl(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.cmd_sgl);
		}
	public:
		rule_kit_cmd_info() = delete;
		rule_kit_cmd_info(const rule_kit_cmd_ip<alloc_rule_kit>& cmd_ip_v, cmd_mgr_kit& cmd_mgr_v, rule_kit_cmd_sgl& cmd_sgl_v) :
			cmd_ip(cmd_ip_v), cmd_mgr(cmd_mgr_v), cmd_sgl(cmd_sgl_v)
		{

		}
	};

	template<typename alloc_rule_kit, typename user_outer = std::monostate>
	struct rule_kit_cmd_param
	{
		friend class rule_kit<alloc_rule_kit, user_outer>;
	public:
		using cmd_invoke_param = typename rule<alloc_rule_kit, user_outer>::cmd_act_param;
	private:
		cmd_invoke_param& param_orign;
		template<typename self_this>
		decltype(auto) get_param_orign(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.param_orign);
		}
		bool sync_cmd_info()
		{
			auto cmd_mgr_res = this->owner.get_owner().get_cmd_set().get(this->cmd_info.cmd_ip.get_cmd_set_index());

			if (cmd_mgr_res.has_value() == false)
				return false;

			auto& cmd_mgr_v = cmd_mgr_res.value().get().get_content();
			auto it_res = cmd_mgr_v.get_it(this->cmd_info.cmd_ip.get_index());

			if (it_res.has_value() == false)
				return false;

			this->cmd_info.cmd_mgr = std::ref(cmd_mgr_v);
			this->cmd_info.cmd_sgl = *it_res.value();
			return true;
		}
	public:
		rule_kit<alloc_rule_kit, user_outer>& owner;
		rule_kit_int_info& int_info_save;
		rule_kit_cmd_info<alloc_rule_kit, user_outer> cmd_info;
	public:
		template<typename self_this>
		decltype(auto) get_owner(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.owner);
		}
		template<typename self_this>
		decltype(auto) get_int_info_save(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.int_info_save);
		}
		template<typename self_this>
		decltype(auto) get_cmd_info(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.cmd_info);
		}
	public:
		void set_int_info_save(const rule_kit_int_info& int_info_save_v)
		{
			this->int_info_save = int_info_save_v;
			return;
		}
	public:
		rule_kit_cmd_param() = delete;
		rule_kit_cmd_param(cmd_invoke_param& param_orign_v, rule_kit<alloc_rule_kit, user_outer>& owner_v, rule_kit_int_info& int_info_save_v, const rule_kit_cmd_info<alloc_rule_kit, user_outer>& cmd_info_v) :
			param_orign(param_orign_v), owner(owner_v), int_info_save(int_info_save_v), cmd_info(cmd_info_v)
		{

		}
	};

	using rule_kit_int_t = act_data::act_run_int_t;

	template<typename alloc_rule_kit>
	struct rule_kit_cmd_res
	{
	public:
		rule_kit_int_t res_t;
		std::optional<rule_kit_cmd_ip<alloc_rule_kit>> cmd_tgt_ip;
	public:
		template<typename self_this>
		decltype(auto) get_res_t(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.res_t);
		}
		template<typename self_this>
		decltype(auto) get_cmd_tgt_ip(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.cmd_tgt_ip);
		}
	public:
		void set_res_t(rule_kit_int_t res_t_v)
		{
			this->res_t = res_t_v;
			return;
		}
		void set_cmd_tgt_ip(const std::optional<rule_kit_cmd_ip<alloc_rule_kit>>& cmd_tgt_ip_v)
		{
			this->cmd_tgt_ip = cmd_tgt_ip_v;
			return;
		}
	public:
		rule_kit_cmd_res() :
			res_t(rule_kit_int_t::next), cmd_tgt_ip(std::nullopt)
		{

		}
		rule_kit_cmd_res(rule_kit_int_t res_t_v, const std::optional<rule_kit_cmd_ip<alloc_rule_kit>>& cmd_tgt_ip_v) :
			res_t(res_t_v), cmd_tgt_ip(cmd_tgt_ip_v)
		{

		}
	};

	template<typename alloc_rule_kit, typename user_outer = std::monostate>
	using rule_kit_cmd_call = rule_kit_cmd_res<alloc_rule_kit> (*)(rule_kit_cmd_param<alloc_rule_kit, user_outer>&);

	template<typename alloc_rule_kit, typename user_outer = std::monostate>
	struct rule_kit_cmd_proxy_call
	{
	public:
		using owner_t = rule<alloc_rule_kit, user_outer>;
		using owner_kit_t = rule_kit<alloc_rule_kit, user_outer>;
		using cmd_invoke_param = typename owner_t::cmd_act_param;
		using cmd_invoke_res = typename owner_t::cmd_act_res;
		using cmd_invoke_act_ip = typename owner_t::run_ip;
		using cmd_invoke_proxy_param = rule_kit_cmd_param<alloc_rule_kit, user_outer>;
		using cmd_invoke_info = rule_kit_cmd_info<alloc_rule_kit, user_outer>;
	public:
		rule_kit_cmd_call<alloc_rule_kit, user_outer> cmd_outer_call;
	public:
		cmd_invoke_res operator()(cmd_invoke_param& param_ref) const
		{
			owner_kit_t& rule_kit_ref = param_ref.get_data_outer()->get_kit();
			owner_t& owner_ref = rule_kit_ref.get_owner();
			rule_kit_int_info rule_kit_int;
			cmd_invoke_res res;

			owner_ref.set_int_info_run
			(
				rule_kit_int,
				rdata::rule_int_t::none,
				rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
				rdata::rule_int_info_cont(std::in_place_type<std::monostate>, std::monostate())
			);

			if (this->cmd_outer_call == nullptr)
			{
				owner_ref.set_act_run_int(res, act_data::act_run_int_t::error, param_ref, std::nullopt, rule_kit_int);
				return res;
			}

			typename owner_t::parse_run_param param_parse(param_ref);

			if (param_parse.get_cmd_mgr_tgt().has_value() == false || param_parse.get_cmd().has_value() == false)
			{
				owner_ref.set_int_info_run
				(
					rule_kit_int,
					rdata::rule_int_t::parse_run_param_error,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont(std::in_place_type<std::monostate>, std::monostate())
				);
				owner_ref.set_act_run_int(res, act_data::act_run_int_t::error, param_ref, std::nullopt, rule_kit_int);
				return res;
			}

			cmd_invoke_proxy_param proxy_param
			(
				param_ref,
				rule_kit_ref,
				rule_kit_int,
				cmd_invoke_info
				(
					param_ref.get_ip(),
					param_parse.get_cmd_mgr_tgt().value().get(),
					param_parse.get_cmd().value()
				)
			);

			rule_kit_cmd_res invoke_res = this->cmd_outer_call(proxy_param);

			owner_ref.set_act_run_int
			(
				res,
				invoke_res.get_res_t(),
				param_ref,
				invoke_res.get_cmd_tgt_ip().has_value() == false ? std::nullopt : std::make_optional(invoke_res.get_cmd_tgt_ip().value()),
				rule_kit_int
			);

			return res;
		}
	public:
		template<typename self_this>
		decltype(auto) get_cmd_outer_call(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.cmd_outer_call);
		}
	public:
		void set_cmd_outer_call(const rule_kit_cmd_call<alloc_rule_kit, user_outer>& cmd_outer_call_v)
		{
			this->cmd_outer_call = cmd_outer_call_v;
			return;
		}
	public:
		rule_kit_cmd_proxy_call() = delete;
		rule_kit_cmd_proxy_call(const rule_kit_cmd_call<alloc_rule_kit, user_outer>& cmd_def_v) :
			cmd_outer_call(cmd_def_v)
		{

		}
	};

	template<typename alloc_rule_kit, typename user_outer = std::monostate>
	using rule_kit_cmd_cb_int = void (*)(rule_kit_outer<rule_kit<alloc_rule_kit, user_outer>, user_outer>&, act_data::act_run_int_t, const rule_kit_int_info&, rule_kit_cmd_ip<alloc_rule_kit>, std::optional<rule_kit_cmd_ip<alloc_rule_kit>>&, std::optional<rule_kit_cmd_sgl>, bool&);

	template<typename alloc_rule_kit, typename user_outer = std::monostate>
	using rule_kit_cmd_cb_rc_err = void (*)(rule_kit_outer<rule_kit<alloc_rule_kit, user_outer>, user_outer>&, kernel_error, rule_kit_cmd_ip<alloc_rule_kit>&, bool&);

	template<typename alloc_rule_kit, typename user_outer = std::monostate>
	using rule_kit_cmd_cb_err = void (*)(rule_kit_outer<rule_kit<alloc_rule_kit, user_outer>, user_outer>&, act_data::act_err_t, rule_kit_cmd_ip<alloc_rule_kit>&, bool&);

	using rule_kit_cmd_cb_hook_t = act_data::act_hook_t;

	template<typename alloc_rule_kit, typename user_outer = std::monostate>
	using rule_kit_cmd_cb_hook = rule_kit_cmd_cb_hook_t (*)(rule_kit_outer<rule_kit<alloc_rule_kit, user_outer>, user_outer>&, rule_kit_cmd_ip<alloc_rule_kit>&, std::optional<rule_kit_cmd_sgl>);

	struct rule_kit_var_read_info
	{
	public:
		var_cont_data::var_mgr_index mgr_index;
		var_cont_data::var_index index;
		var_cont_data::var_type& type;
		var_cont_data::var_pos begin;
		var_cont_data::var_read_width width;
		var_cont_data::var_ref ref;
		std::span<rdata::rule_var_base_t> span_cont;
	public:
		template<typename self_this>
		decltype(auto) get_mgr_index(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.mgr_index);
		}
		template<typename self_this>
		decltype(auto) get_index(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.index);
		}
		template<typename self_this>
		decltype(auto) get_type(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.type);
		}
		template<typename self_this>
		decltype(auto) get_begin(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.begin);
		}
		template<typename self_this>
		decltype(auto) get_width(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.width);
		}
		template<typename self_this>
		decltype(auto) get_ref(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.ref);
		}
		template<typename self_this>
		decltype(auto) get_span_cont(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.span_cont);
		}
	public:
		void set_mgr_index(var_cont_data::var_mgr_index mgr_index_v)
		{
			this->mgr_index = mgr_index_v;
			return;
		}
		void set_index(var_cont_data::var_index index_v)
		{
			this->index = index_v;
			return;
		}
		void set_type(var_cont_data::var_type type_v)
		{
			this->type = type_v;
			return;
		}
		void set_begin(var_cont_data::var_pos begin_v)
		{
			this->begin = begin_v;
			return;
		}
		void set_width(var_cont_data::var_read_width width_v)
		{
			this->width = width_v;
			return;
		}
		void set_ref(var_cont_data::var_ref ref_v)
		{
			this->ref = ref_v;
			return;
		}
		void set_span_cont(const std::span<rdata::rule_var_base_t>& span_cont_v)
		{
			this->span_cont = span_cont_v;
			return;
		}
	public:
		rule_kit_var_read_info() = delete;
		rule_kit_var_read_info(var_cont_data::var_mgr_index mgr_index_v, var_cont_data::var_index index_v, var_cont_data::var_type& type_v, var_cont_data::var_pos begin_v, var_cont_data::var_read_width width_v, var_cont_data::var_ref ref_v, const std::span<rdata::rule_var_base_t>& span_cont_v) :
			mgr_index(mgr_index_v), index(index_v), type(type_v), begin(begin_v), width(width_v), ref(ref_v), span_cont(span_cont_v)
		{

		}
	};

	struct rule_kit_cmd_read_info
	{
	public:
		cmd_data::cmd_mgr_index mgr_index;
		cmd_data::cmd_index index;
		cmd_data::cmd_type& type;
		cmd_data::cmd_op_index begin;
		cmd_data::cmd_op_width width;
		std::size_t ref;
		std::span<cmd_data::cmd_op_single> span_cont;
	public:
		template<typename self_this>
		decltype(auto) get_mgr_index(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.mgr_index);
		}
		template<typename self_this>
		decltype(auto) get_index(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.index);
		}
		template<typename self_this>
		decltype(auto) get_type(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.type);
		}
		template<typename self_this>
		decltype(auto) get_begin(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.begin);
		}
		template<typename self_this>
		decltype(auto) get_width(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.width);
		}
		template<typename self_this>
		decltype(auto) get_ref(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.ref);
		}
		template<typename self_this>
		decltype(auto) get_span_cont(this self_this&& self) noexcept
		{
			return std::forward_like<self_this>(self.span_cont);
		}
	public:
		void set_mgr_index(cmd_data::cmd_mgr_index mgr_index_v)
		{
			this->mgr_index = mgr_index_v;
			return;
		}
		void set_index(cmd_data::cmd_index index_v)
		{
			this->index = index_v;
			return;
		}
		void set_type(cmd_data::cmd_type type_v)
		{
			this->type = type_v;
			return;
		}
		void set_begin(cmd_data::cmd_op_index begin_v)
		{
			this->begin = begin_v;
			return;
		}
		void set_width(cmd_data::cmd_op_width width_v)
		{
			this->width = width_v;
			return;
		}
		void set_ref(std::size_t ref_v)
		{
			this->ref = ref_v;
			return;
		}
		void set_span_cont(const std::span<cmd_data::cmd_op_single>& span_cont_v)
		{
			this->span_cont = span_cont_v;
			return;
		}
	public:
		rule_kit_cmd_read_info() = delete;
		rule_kit_cmd_read_info(cmd_data::cmd_mgr_index mgr_index_v, cmd_data::cmd_index index_v, cmd_data::cmd_type& type_v, cmd_data::cmd_op_index begin_v, cmd_data::cmd_op_width width_v, std::size_t ref_v, const std::span<cmd_data::cmd_op_single>& span_cont_v) :
			mgr_index(mgr_index_v), index(index_v), type(type_v), begin(begin_v), width(width_v), ref(ref_v), span_cont(span_cont_v)
		{

		}
	};
}
