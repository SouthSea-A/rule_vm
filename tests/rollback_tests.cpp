#include <cstdio>
#include <cstdint>
#include <array>
#include <optional>
#include "rule.h"
#include "test_callback_adapter.h"


using alloc_t = std::allocator<int>;
using kit_t = rule_tool::rule_kit<alloc_t>;
using param_t = rule_tool::rule_kit_cmd_param<alloc_t>;
using res_t = rule_tool::rule_kit_cmd_res<alloc_t>;
using ip_t = rule_tool::rule_kit_cmd_ip<alloc_t>;
using int_t = rule_tool::rule_kit_int_t;
using var_target_t = rule_cmd_data_def::var_target;
using cmd_target_t = rule_cmd_data_def::cmd_target;
using test_t = test_callback::adapter<kit_t>;

struct obs
{
	int cb = 0;
};

static kit_t make_kit(obs& o)
{
	static std::monostate user_outer_v;
	test_t::commands.clear();
	test_t::cb_int = [&o](test_t::outer_t&, rule_act_data_def::act_run_int_t, const rule_data_def::rule_int_info&, ip_t,
		std::optional<ip_t>&, std::optional<rule_data_def::rule_cmd_sgl>, bool&) { o.cb++; };
	test_t::cb_err_rc = [](test_t::outer_t&, kernel_error, ip_t&, bool& is_end) { is_end = true; };
	test_t::cb_err = [](test_t::outer_t&, rule_act_data_def::act_err_t, ip_t&, bool& is_end) { is_end = true; };
	test_t::cb_hook = [](test_t::outer_t&, ip_t&, std::optional<rule_data_def::rule_cmd_sgl>) { return rule_act_data_def::act_hook_t::pass; };
	return kit_t(test_t::cb_int_thunk, test_t::cb_err_rc_thunk, test_t::cb_err_thunk, test_t::cb_hook_thunk, user_outer_v);
}

static bool expect(const char* name, bool ok)
{
	std::printf("%s %s\n", ok ? "PASS" : "FAIL", name);
	return ok;
}

static bool run_kit(kit_t& kit)
{
	try { kit.run(0, 0); }
	catch (const kernel_error_throw& e)
	{
		std::printf("  THROW code=%d msg=%s\n", static_cast<int>(e.error_code), e.error_msg.c_str());
		return false;
	}
	return true;
}

// 1. 新变量 + is_incr_ref_tgt=true + 失败：空壳必须删除，manager 也应销毁
static bool test_new_var_shell_deleted_inc_true()
{
	obs o;
	kit_t kit = make_kit(o);
	bool alloc_ok = true;

	test_t::add_command(kit, 1, [&alloc_ok](param_t& p) -> res_t
	{
		const var_target_t tgt(rule_cmd_data_def::op_var_type::var_val, 0, 0, 7, 0, UINT64_MAX);
		alloc_ok = p.get_owner().alloc(p, tgt, true, true);
		return res_t(int_t::next, std::nullopt);
	});
	kit.add_set_cmd_set(0, 0, 1, 0, {});

	const bool run_ok = run_kit(kit);
	const bool empty = kit.get_owner().get_var_set().size() == 0;

	return expect("new_var_shell_deleted_inc_true", run_ok && alloc_ok == false && empty);
}

// 2. 新变量 + is_incr_ref_tgt=false + 失败：空壳必须删除
static bool test_new_var_shell_deleted_inc_false()
{
	obs o;
	kit_t kit = make_kit(o);
	bool alloc_ok = true;

	test_t::add_command(kit, 1, [&alloc_ok](param_t& p) -> res_t
	{
		const var_target_t tgt(rule_cmd_data_def::op_var_type::var_val, 0, 0, 7, 0, UINT64_MAX);
		alloc_ok = p.get_owner().alloc(p, tgt, true, false);
		return res_t(int_t::next, std::nullopt);
	});
	kit.add_set_cmd_set(0, 0, 1, 0, {});

	const bool run_ok = run_kit(kit);
	const bool empty = kit.get_owner().get_var_set().size() == 0;

	return expect("new_var_shell_deleted_inc_false", run_ok && alloc_ok == false && empty);
}

// 3. 已存在变量失败：type/ref/content 尺寸全部恢复
static bool test_existing_var_restored()
{
	obs o;
	kit_t kit = make_kit(o);
	bool alloc_ok = true;
	std::array<rule_data_def::rule_var_base_t, 8> data{};
	kit.add_set_var_set(0, 0, 1, 0, std::span(data));

	test_t::add_command(kit, 1, [&alloc_ok](param_t& p) -> res_t
	{
		const var_target_t tgt(rule_cmd_data_def::op_var_type::var_val, 0, 0, 9, UINT64_MAX, 8);
		alloc_ok = p.get_owner().alloc(p, tgt, true, true);
		return res_t(int_t::next, std::nullopt);
	});
	kit.add_set_cmd_set(0, 0, 1, 0, {});

	const bool run_ok = run_kit(kit);
	auto var_mgr_res = kit.get_owner().get_var_set().get(0);
	bool state_ok = false;

	if (var_mgr_res.has_value() == true)
	{
		auto var_res = var_mgr_res.value().get().get_content().get(0);

		if (var_res.has_value() == true)
		{
			auto& var_ref = var_res.value().get();
			state_ok = var_ref.get_type() == 1 && var_ref.get_ref() == 1 && var_ref.get_content().size() == 8;
		}
	}

	return expect("existing_var_restored", run_ok && alloc_ok == false && state_ok);
}

// 4. 已存在命令失败：type/ref/content 尺寸全部恢复
static bool test_existing_cmd_restored()
{
	obs o;
	kit_t kit = make_kit(o);
	bool alloc_ok = true;
	std::array<rule_cmd_data_def::cmd_var_base_type, rule_cmd_data_def::imm_width_byte_max> bytes{};
	kit.add_set_cmd_set(0, 1, 5, 0, { rule_cmd_data_def::imm_target(rule_cmd_data_def::imm_type::invalid, bytes) });

	test_t::add_command(kit, 1, [&alloc_ok](param_t& p) -> res_t
	{
		const cmd_target_t tgt(rule_cmd_data_def::op_cmd_type::cmd_val, 0, 0, 9, UINT64_MAX, 8);
		alloc_ok = p.get_owner().alloc(p, tgt, true, true);
		return res_t(int_t::next, std::nullopt);
	});
	kit.add_set_cmd_set(0, 0, 1, 0, {});

	const bool run_ok = run_kit(kit);
	auto cmd_mgr_res = kit.get_owner().get_cmd_set().get(0);
	bool state_ok = false;
	int shown_type = -1;
	int shown_ref = -1;
	int shown_size = -1;

	if (cmd_mgr_res.has_value() == true)
	{
		auto& cmd_mgr_ref = cmd_mgr_res.value().get().get_content();
		auto cmd_res = cmd_mgr_ref.get(1);
		auto ref_res = cmd_mgr_ref.get_ref(1);
		auto size_res = cmd_mgr_ref.get_op_size(1);

		if (cmd_res.has_value() == true && ref_res.has_value() == true && size_res.has_value() == true)
		{
			shown_type = static_cast<int>(cmd_res.value().get_type().get());
			shown_ref = static_cast<int>(ref_res.value());
			shown_size = static_cast<int>(size_res.value());
			state_ok = shown_type == 5 && shown_ref == 1 && shown_size == 1;
		}
	}

	std::printf("  existing_cmd run=%d alloc=%d type=%d ref=%d size=%d\n", static_cast<int>(run_ok), static_cast<int>(alloc_ok), shown_type, shown_ref, shown_size);

	return expect("existing_cmd_restored", run_ok && alloc_ok == false && state_ok);
}

// 5. 新命令 + 失败：空壳必须删除
static bool test_new_cmd_shell_deleted()
{
	obs o;
	kit_t kit = make_kit(o);
	bool alloc_ok = true;

	test_t::add_command(kit, 1, [&alloc_ok](param_t& p) -> res_t
	{
		const cmd_target_t tgt(rule_cmd_data_def::op_cmd_type::cmd_val, 0, 0, 9, UINT64_MAX, 8);
		alloc_ok = p.get_owner().alloc(p, tgt, true, true);
		return res_t(int_t::next, std::nullopt);
	});
	kit.add_set_cmd_set(0, 0, 1, 0, {});

	const bool run_ok = run_kit(kit);
	bool shell_deleted = false;
	int shown_ref1 = -1;
	int shown_cmd0 = -1;

	auto cmd_mgr_res = kit.get_owner().get_cmd_set().get(0);

	if (cmd_mgr_res.has_value() == true)
	{
		auto& cmd_mgr_ref = cmd_mgr_res.value().get().get_content();
		auto cmd_res = cmd_mgr_ref.get(1);
		auto ref_res = cmd_mgr_ref.get_ref(0);
		shown_cmd0 = static_cast<int>(cmd_res.has_value());
		shown_ref1 = ref_res.has_value() ? static_cast<int>(ref_res.value()) : -1;
		shell_deleted = cmd_res.has_value() == false && ref_res.has_value() == true;
	}

	std::printf("  new_cmd run=%d alloc=%d cmd0=%d ref1=%d\n", static_cast<int>(run_ok), static_cast<int>(alloc_ok), shown_cmd0, shown_ref1);

	return expect("new_cmd_shell_deleted", run_ok && alloc_ok == false && shell_deleted);
}

// 6. inc=true 在游标之前插入新命令后失败：回滚必须同时修正执行游标
static bool test_new_cmd_before_cursor_rollback()
{
	obs o;
	kit_t kit = make_kit(o);
	bool alloc_ok = true;
	int marker = 0;

	test_t::add_command(kit, 1, [](param_t&) -> res_t { return res_t(int_t::next, std::nullopt); });
	test_t::add_command(kit, 2, [](param_t&) -> res_t { return res_t(int_t::next, std::nullopt); });
	test_t::add_command(kit, 3, [&alloc_ok](param_t& p) -> res_t
	{
		const cmd_target_t tgt(rule_cmd_data_def::op_cmd_type::cmd_val, 0, 2, 9, UINT64_MAX, 8);
		alloc_ok = p.get_owner().alloc(p, tgt, true, true);
		return res_t(int_t::next, std::nullopt);
	});
	test_t::add_command(kit, 8, [&marker](param_t&) -> res_t { marker++; return res_t(int_t::next, std::nullopt); });

	kit.add_set_cmd_set(0, 0, 1, 0, {});
	kit.add_set_cmd_set(0, 1, 2, 0, {});
	kit.add_set_cmd_set(0, 5, 3, 0, {});
	kit.add_set_cmd_set(0, 6, 8, 0, {});

	const bool run_ok = run_kit(kit);
	bool cmd2_absent = false;

	auto cmd_mgr_res = kit.get_owner().get_cmd_set().get(0);

	if (cmd_mgr_res.has_value() == true)
	{
		auto& cmd_mgr_ref = cmd_mgr_res.value().get().get_content();
		cmd2_absent = cmd_mgr_ref.get(2).has_value() == false;
	}

	return expect("new_cmd_before_cursor_rollback", run_ok && alloc_ok == false && marker == 1 && cmd2_absent);
}

int main()
{
	bool ok = true;

	ok &= test_new_var_shell_deleted_inc_true();
	ok &= test_new_var_shell_deleted_inc_false();
	ok &= test_existing_var_restored();
	ok &= test_existing_cmd_restored();
	ok &= test_new_cmd_shell_deleted();
	ok &= test_new_cmd_before_cursor_rollback();

	std::printf("%s\n", ok ? "ALL PASS" : "SOME FAILED");
	return ok ? 0 : 1;
}
