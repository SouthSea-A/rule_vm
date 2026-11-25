#include <cstdio>
#include <memory>
#include <optional>
#include <type_traits>
#include "rule_design.h"

using alloc_t = std::allocator<int>;
using kit_t = rule_kit::rule_kit_context<alloc_t, std::monostate>;
using outer_t = rule_kit::rule_kit_outer<kit_t, std::monostate>;
using ip_t = rule_kit::rule_kit_cmd_ip<alloc_t>;

static void cb_int(outer_t&, rule_kit::rule_kit_int_t, const rule_kit::rule_kit_int_info&, ip_t, std::optional<ip_t>&, std::optional<rule_kit::rule_kit_cmd_sgl>, bool&)
{
	return;
}

static void cb_rc_err(outer_t&, rule_kit::rule_kit_error, ip_t&, bool& is_end)
{
	is_end = true;
	return;
}

static void cb_err(outer_t&, rule_kit::rule_kit_act_err_t, ip_t&, bool& is_end)
{
	is_end = true;
	return;
}

static rule_kit::rule_kit_cmd_cb_hook_t cb_hook(outer_t&, ip_t&, std::optional<rule_kit::rule_kit_cmd_sgl>)
{
	return rule_kit::rule_kit_cmd_cb_hook_t::pass;
}

struct app_state
{
	int calls = 0;
};

using app_kit_t = rule_kit::rule_kit_context<alloc_t, app_state>;
using app_outer_t = rule_kit::rule_kit_outer<app_kit_t, app_state>;
using app_param_t = typename app_kit_t::cmd_invoke_proxy_param;
using app_res_t = rule_kit::rule_kit_cmd_res<alloc_t>;

static app_res_t app_cmd(app_param_t& param_v)
{
	auto& outer_v = *param_v.get_owner().get_owner().get_actuator_().get_data_outer();
	outer_v.get_user().calls++;
	return app_res_t(rule_kit::rule_kit_int_t::next, std::nullopt);
}

static app_kit_t::cmd_invoke_res app_act_cmd(typename app_kit_t::cmd_invoke_param& param_v)
{
	rule_kit::rule_kit_cmd_proxy_call<alloc_t, app_state> proxy(&app_cmd);
	return proxy(param_v);
}
int main()
{
	static_assert(std::is_same_v<rule_kit::rule_kit_cmd_tgt, rule_tool::rule_kit_cmd_tgt>);
	static_assert(std::is_same_v<rule_kit::rule_kit_var_tgt, rule_tool::rule_kit_var_tgt>);
	static_assert(std::is_same_v<rule_kit::rule_kit_cmd_op_sgl, rule_tool::rule_kit_cmd_op_sgl>);
	static_assert(std::is_same_v<rule_kit::rule_kit_context<alloc_t, std::monostate>, rule_tool::rule_kit<alloc_t, std::monostate>>);

	std::monostate user_outer_v;
	kit_t kit(cb_int, cb_rc_err, cb_err, cb_hook, user_outer_v);

	rule_kit::rule_kit_cmd_tgt cmd_tgt(rule_kit::rule_kit_cmd_handle_type::cmd_tgt, 0, 0, 0, 0, 0);
	rule_kit::rule_kit_var_tgt var_tgt(rule_kit::rule_kit_var_handle_type::var_val, 0, 0, 0, 0, 8);
	rule_kit::rule_kit_cmd_op_sgl op_v = var_tgt;

	app_state state_v;
	app_kit_t app_kit(nullptr, nullptr, nullptr, nullptr, state_v);
	app_kit.add_set_cmd_set(0, 0, 1, 0, {});
	app_kit.add_set_cmd_def(1, &app_act_cmd);
	app_kit.run(0, 0);
	const bool outer_ok = state_v.calls == 1;

	std::printf("public_types cmd=%llu var=%llu op=%zu outer_ok=%d\n",
		static_cast<unsigned long long>(cmd_tgt.get_index()),
		static_cast<unsigned long long>(var_tgt.get_index()),
		op_v.index(),
		(int)outer_ok);
	std::printf("ALL PASS\n");
	return 0;
}
