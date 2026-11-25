#pragma once

#include <functional>
#include <unordered_map>
#include "rule_kit.h"

// 测试适配层：内核回调已经是函数指针；测试为了保留 lambda 捕获写法，
// 在测试侧集中持有 std::function 并用固定 thunk 转发。不属于内核。
namespace test_callback
{
	template<typename kit_t>
	struct adapter
	{
	public:
		using alloc_t = typename kit_t::allocator_type;
		using outer_t = typename kit_t::outer_t;
		using act_param_t = typename kit_t::owner_t::cmd_act_param;
		using act_res_t = typename kit_t::owner_t::cmd_act_res;
		using param_t = rule_kit::rule_kit_cmd_param<alloc_t, typename kit_t::user_outer_type>;
		using res_t = rule_kit::rule_kit_cmd_res<alloc_t>;
		using cmd_type_t = rule_kit::rule_kit_cmd_type;

	public:
		inline static std::unordered_map<cmd_type_t, std::function<res_t(param_t&)>> commands;
		inline static std::function<void(outer_t&, rule_kit::rule_kit_int_t, const rule_kit::rule_kit_int_info&, rule_kit::rule_kit_cmd_ip<alloc_t>, std::optional<rule_kit::rule_kit_cmd_ip<alloc_t>>&, std::optional<rule_kit::rule_kit_cmd_sgl>, bool&)> cb_int;
		inline static std::function<void(outer_t&, rule_kit::rule_kit_error, rule_kit::rule_kit_cmd_ip<alloc_t>&, bool&)> cb_err_rc;
		inline static std::function<void(outer_t&, rule_kit::rule_kit_act_err_t, rule_kit::rule_kit_cmd_ip<alloc_t>&, bool&)> cb_err;
		inline static std::function<rule_kit::rule_kit_cmd_cb_hook_t(outer_t&, rule_kit::rule_kit_cmd_ip<alloc_t>&, std::optional<rule_kit::rule_kit_cmd_sgl>)> cb_hook;

	public:
		static void cb_int_thunk(outer_t& outer_v, rule_kit::rule_kit_int_t type_v, const rule_kit::rule_kit_int_info& info_v, rule_kit::rule_kit_cmd_ip<alloc_t> ip_v, std::optional<rule_kit::rule_kit_cmd_ip<alloc_t>>& ip_tgt_v, std::optional<rule_kit::rule_kit_cmd_sgl> sgl_v, bool& end_v)
		{
			if (cb_int)
				cb_int(outer_v, type_v, info_v, ip_v, ip_tgt_v, sgl_v, end_v);
			else
				end_v = true;
			return;
		}
		static void cb_err_rc_thunk(outer_t& outer_v, rule_kit::rule_kit_error error_v, rule_kit::rule_kit_cmd_ip<alloc_t>& ip_v, bool& end_v)
		{
			if (cb_err_rc)
				cb_err_rc(outer_v, error_v, ip_v, end_v);
			else
				end_v = true;
			return;
		}
		static void cb_err_thunk(outer_t& outer_v, rule_kit::rule_kit_act_err_t error_v, rule_kit::rule_kit_cmd_ip<alloc_t>& ip_v, bool& end_v)
		{
			if (cb_err)
				cb_err(outer_v, error_v, ip_v, end_v);
			else
				end_v = true;
			return;
		}
		static rule_kit::rule_kit_cmd_cb_hook_t cb_hook_thunk(outer_t& outer_v, rule_kit::rule_kit_cmd_ip<alloc_t>& ip_v, std::optional<rule_kit::rule_kit_cmd_sgl> sgl_v)
		{
			if (cb_hook)
				return cb_hook(outer_v, ip_v, sgl_v);
			return rule_kit::rule_kit_cmd_cb_hook_t::pass;
		}

	public:
		static res_t command_dispatch(param_t& param_v)
		{
			auto& sgl_v = param_v.get_cmd_info().get_cmd_sgl();
			auto it_v = commands.find(sgl_v.get_type());

			if (it_v == commands.end())
				return res_t(rule_kit::rule_kit_int_t::error, std::nullopt);

			return it_v->second(param_v);
		}
		static act_res_t command_act_dispatch(act_param_t& param_v)
		{
			rule_kit::rule_kit_cmd_proxy_call<alloc_t, typename kit_t::user_outer_type> proxy(&command_dispatch);
			return proxy(param_v);
		}
		static void add_command(kit_t& kit_v, cmd_type_t cmd_type_v, std::function<res_t(param_t&)> command_v)
		{
			commands[cmd_type_v] = std::move(command_v);
			kit_v.add_set_cmd_def(cmd_type_v, &command_act_dispatch);
			return;
		}
	};
}
