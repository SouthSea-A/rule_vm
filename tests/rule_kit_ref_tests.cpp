#include <array>
#include <cstdio>
#include <memory>
#include <optional>
#include <span>
#include "rule_kit.h"

using alloc_t = std::allocator<int>;

struct app_state
{
	bool var_info_ok = false;
	bool cmd_info_ok = false;
	std::size_t var_ref = 0;
	std::size_t cmd_ref = 0;
};

using kit_t = rule_kit::rule_kit_context<alloc_t, app_state>;
using outer_t = rule_kit::rule_kit_outer<kit_t, app_state>;
using proxy_t = typename kit_t::cmd_invoke_proxy_param;
using bridge_t = typename kit_t::cmd_invoke_param;
using act_res_t = typename kit_t::cmd_invoke_res;
using res_t = rule_kit::rule_kit_cmd_res<alloc_t>;

static res_t probe_handler(proxy_t& param)
{
	app_state& state = param.get_owner().get_owner().get_actuator_().get_data_outer()->get_user();

	std::optional<rule_kit::rule_kit_var_read_info> var_info =
		param.get_owner().get_var_cont_info(param, 0, 0, 0, 8);

	if (var_info.has_value() == true)
	{
		state.var_info_ok = true;
		state.var_ref = static_cast<std::size_t>(var_info->get_ref());
	}

	std::optional<rule_kit::rule_kit_cmd_read_info> cmd_info =
		param.get_owner().get_cmd_cont_info(param, 0, 0, 0, 0);

	if (cmd_info.has_value() == true)
	{
		state.cmd_info_ok = true;
		state.cmd_ref = static_cast<std::size_t>(cmd_info->get_ref());
	}

	return res_t(rule_kit::rule_kit_int_t::next, std::nullopt);
}

static act_res_t probe_bridge(bridge_t& param)
{
	rule_kit::rule_kit_cmd_proxy_call<alloc_t, app_state> proxy(&probe_handler);
	return proxy(param);
}

int main()
{
	app_state state;
	kit_t kit(nullptr, nullptr, nullptr, nullptr, state);

	std::array<rule_kit::rule_kit_var_base, 8> bytes{};
	const bool var_ok = kit.add_set_var_set(0, 0, 0, 0, std::span<rule_kit::rule_kit_var_base>(bytes));
	kit.add_set_cmd_def(1, &probe_bridge);
	const bool cmd_ok = kit.add_set_cmd_set(0, 0, 1, 0, {});
	kit.run(0, 0);

	const bool ok = var_ok && cmd_ok && state.var_info_ok && state.cmd_info_ok &&
		state.var_ref == 1 && state.cmd_ref == 1;

	std::printf("var_ref=%zu cmd_ref=%zu var_ok=%d cmd_ok=%d\n",
		state.var_ref,
		state.cmd_ref,
		(int)state.var_info_ok,
		(int)state.cmd_info_ok);
	std::printf("%s\n", ok ? "ALL PASS" : "FAIL");
	return ok ? 0 : 1;
}
