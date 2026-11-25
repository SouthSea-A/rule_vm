#include <cstdio>
#include <cstdint>
#include <optional>
#include <functional>
#include "rule.h"
#include "test_callback_adapter.h"


using alloc_t = std::allocator<int>;
using kit_t = rule_tool::rule_kit<alloc_t>;
using param_t = rule_tool::rule_kit_cmd_param<alloc_t>;
using res_t = rule_tool::rule_kit_cmd_res<alloc_t>;
using ip_t = rule_tool::rule_kit_cmd_ip<alloc_t>;
using int_t = rule_tool::rule_kit_int_t;
using cmd_tgt_t = rule_cmd_data_def::cmd_target;
using cmd_type_t = rule_cmd_data_def::cmd_type;
using cmd_index_t = rule_cmd_data_def::cmd_index;
using cmd_mgr_t = rule_cmd_data_def::cmd_mgr_index;
using var_tgt_t = rule_cmd_data_def::var_target;
using test_t = test_callback::adapter<kit_t>;

struct obs
{
	int cb = 0;
	int err = 0;
};

static kit_t make_kit(obs& o)
{
	static std::monostate user_outer_v;
	test_t::commands.clear();
	test_t::cb_int = [&o](test_t::outer_t&, rule_act_data_def::act_run_int_t, const rule_data_def::rule_int_info&, ip_t,
		std::optional<ip_t>&, std::optional<rule_data_def::rule_cmd_sgl>, bool&) { o.cb++; };
	test_t::cb_err_rc = [&o](test_t::outer_t&, kernel_error e, ip_t&, bool& is_end) { o.err++; std::printf("  err_rc=%d\n", static_cast<int>(e)); is_end = true; };
	test_t::cb_err = [&o](test_t::outer_t&, rule_act_data_def::act_err_t e, ip_t&, bool& is_end) { o.err++; std::printf("  err_act=%d\n", static_cast<int>(e)); is_end = true; };
	test_t::cb_hook = [](test_t::outer_t&, ip_t&, std::optional<rule_data_def::rule_cmd_sgl>) { return rule_act_data_def::act_hook_t::pass; };
	return kit_t(test_t::cb_int_thunk, test_t::cb_err_rc_thunk, test_t::cb_err_thunk, test_t::cb_hook_thunk, user_outer_v);
}

static bool run_kit(kit_t& kit, cmd_mgr_t mgr, cmd_index_t index)
{
	try { kit.run(mgr, index); }
	catch (const kernel_error_throw& e)
	{
		std::printf("  THROW code=%d msg=%s\n", static_cast<int>(e.error_code), e.error_msg.c_str());
		return false;
	}
	catch (const std::exception& e)
	{
		std::printf("  THROW std:%s\n", e.what());
		return false;
	}
	return true;
}

static void add_cmd(kit_t& kit, cmd_mgr_t mgr, cmd_index_t index, cmd_type_t type)
{
	kit.add_set_cmd_set(mgr, index, type, 0, {});
}

static bool expect(const char* name, bool ok)
{
	std::printf("%s %s\n", ok ? "PASS" : "FAIL", name);
	return ok;
}

// 0：顺序执行
static bool test_sequential()
{
	obs o;
	kit_t kit = make_kit(o);
	int count = 0;

	for (int i = 0; i < 3; i++)
		test_t::add_command(kit, static_cast<cmd_type_t>(i + 1), [&count](param_t&) -> res_t { count++; return res_t(int_t::next, std::nullopt); });

	add_cmd(kit, 0, 0, 1);
	add_cmd(kit, 0, 1, 2);
	add_cmd(kit, 0, 2, 3);

	return expect("sequential", run_kit(kit, 0, 0) && count == 3 && o.cb == 3);
}

// 1：条件/无条件跳转（向前）
static bool test_jump_forward()
{
	obs o;
	kit_t kit = make_kit(o);
	int count = 0;

	test_t::add_command(kit, 1, [](param_t&) -> res_t { return res_t(int_t::next, ip_t(0, 3)); });
	test_t::add_command(kit, 2, [&count](param_t&) -> res_t { count++; return res_t(int_t::next, std::nullopt); });

	add_cmd(kit, 0, 0, 1);
	add_cmd(kit, 0, 1, 2);
	add_cmd(kit, 0, 2, 2);
	add_cmd(kit, 0, 3, 2);

	return expect("jump_forward", run_kit(kit, 0, 0) && count == 1);
}

// 2：call/ret 栈
static bool test_call_ret()
{
	obs o;
	kit_t kit = make_kit(o);
	int count = 0;

	test_t::add_command(kit, 1, [&count](param_t&) -> res_t { count++; return res_t(int_t::next, std::nullopt); });
	test_t::add_command(kit, 2, [](param_t&) -> res_t { return res_t(int_t::invoke_end, std::nullopt); });
	test_t::add_command(kit, 3, [](param_t&) -> res_t { return res_t(int_t::invoke, ip_t(0, 3)); });

	add_cmd(kit, 0, 0, 3);
	add_cmd(kit, 0, 1, 1);
	add_cmd(kit, 0, 2, 2);
	add_cmd(kit, 0, 3, 1);
	add_cmd(kit, 0, 4, 2);

	return expect("call_ret", run_kit(kit, 0, 0) && count == 2);
}

// 3：运行中在游标之前插入命令（自修改），游标应随插入后移
static bool test_insert_before_cursor()
{
	obs o;
	kit_t kit = make_kit(o);
	int marker = 0;
	int inserted = 0;

	test_t::add_command(kit, 1, [](param_t&) -> res_t { return res_t(int_t::next, std::nullopt); });
	test_t::add_command(kit, 2, [](param_t&) -> res_t { return res_t(int_t::next, std::nullopt); });
	test_t::add_command(kit, 8, [&marker](param_t&) -> res_t { marker++; return res_t(int_t::next, std::nullopt); });
	test_t::add_command(kit, 9, [&inserted](param_t&) -> res_t { inserted++; return res_t(int_t::next, std::nullopt); });

	test_t::add_command(kit, 3, [](param_t& p) -> res_t
	{
		kit_t& k = p.get_owner();
		const cmd_tgt_t ins_tgt(rule_cmd_data_def::op_cmd_type::cmd_val, 0, 2, 9, 0, 0);

		if (k.alloc(p, ins_tgt, true, false) == false)
			return res_t(int_t::error, std::nullopt);

		return res_t(int_t::next, std::nullopt);
	});

	add_cmd(kit, 0, 0, 1);
	add_cmd(kit, 0, 1, 2);
	add_cmd(kit, 0, 5, 3);
	add_cmd(kit, 0, 6, 8);

	return expect("insert_before_cursor", run_kit(kit, 0, 0) && marker == 1 && inserted == 0);
}

// 4：指令删除自己（manager 仍被别的引用持有，应存活）
static bool test_delete_self_manager_survives()
{
	obs o;
	kit_t kit = make_kit(o);
	int marker = 0;

	test_t::add_command(kit, 1, [](param_t& p) -> res_t
	{
		kit_t& k = p.get_owner();
		const cmd_tgt_t alloc_tgt(rule_cmd_data_def::op_cmd_type::cmd_val, 0, 0, 0, 0, 0);
		const cmd_tgt_t del_tgt(rule_cmd_data_def::op_cmd_type::cmd_tgt, 0, 0, 0, 0, 0);

		if (k.alloc(p, alloc_tgt, true, false) == false)
			return res_t(int_t::error, std::nullopt);

		if (k.dealloc(p, del_tgt) == false)
			return res_t(int_t::error, std::nullopt);

		return res_t(int_t::next, std::nullopt);
	});
	test_t::add_command(kit, 2, [&marker](param_t&) -> res_t { marker++; return res_t(int_t::next, std::nullopt); });

	add_cmd(kit, 0, 0, 1);
	add_cmd(kit, 0, 1, 2);

	return expect("delete_self_manager_survives", run_kit(kit, 0, 0) && marker == 1);
}

// 5：指令删除自己且 manager 引用归零 -> 应失效化并干净结束
static bool test_delete_self_manager_gone()
{
	obs o;
	kit_t kit = make_kit(o);

	test_t::add_command(kit, 1, [](param_t& p) -> res_t
	{
		kit_t& k = p.get_owner();
		const cmd_tgt_t del_tgt(rule_cmd_data_def::op_cmd_type::cmd_tgt, 0, 0, 0, 0, 0);

		if (k.dealloc(p, del_tgt) == false)
			return res_t(int_t::error, std::nullopt);

		return res_t(int_t::next, std::nullopt);
	});

	add_cmd(kit, 0, 0, 1);

	return expect("delete_self_manager_gone", run_kit(kit, 0, 0) && kit.get_owner().get_cmd_set().size() == 0 && o.cb == 1);
}

// 6：删除游标之前的命令（manager 存活）
static bool test_delete_before_cursor()
{
	obs o;
	kit_t kit = make_kit(o);
	int marker = 0;
	int head = 0;

	test_t::add_command(kit, 1, [&head](param_t&) -> res_t { head++; return res_t(int_t::next, std::nullopt); });
	test_t::add_command(kit, 2, [](param_t& p) -> res_t
	{
		kit_t& k = p.get_owner();
		const cmd_tgt_t alloc_tgt(rule_cmd_data_def::op_cmd_type::cmd_val, 0, 1, 0, 0, 0);
		const cmd_tgt_t del_tgt(rule_cmd_data_def::op_cmd_type::cmd_tgt, 0, 0, 0, 0, 0);

		if (k.alloc(p, alloc_tgt, true, false) == false)
			return res_t(int_t::error, std::nullopt);

		if (k.dealloc(p, del_tgt) == false)
			return res_t(int_t::error, std::nullopt);

		return res_t(int_t::next, std::nullopt);
	});
	test_t::add_command(kit, 3, [&marker](param_t&) -> res_t { marker++; return res_t(int_t::next, std::nullopt); });

	add_cmd(kit, 0, 0, 1);
	add_cmd(kit, 0, 1, 2);
	add_cmd(kit, 0, 2, 3);

	return expect("delete_before_cursor", run_kit(kit, 0, 0) && head == 1 && marker == 1);
}

// 7：运行中新建 manager（rc 插入），后续指令应正常
static bool test_create_manager_during_run()
{
	obs o;
	kit_t kit = make_kit(o);
	int marker = 0;

	test_t::add_command(kit, 1, [](param_t& p) -> res_t
	{
		kit_t& k = p.get_owner();
		const cmd_tgt_t tgt(rule_cmd_data_def::op_cmd_type::cmd_val, 1, 0, 0, 0, 0);

		if (k.alloc(p, tgt, true, false) == false)
			return res_t(int_t::error, std::nullopt);

		return res_t(int_t::next, std::nullopt);
	});
	test_t::add_command(kit, 2, [&marker](param_t&) -> res_t { marker++; return res_t(int_t::next, std::nullopt); });

	add_cmd(kit, 0, 0, 1);
	add_cmd(kit, 0, 1, 2);

	return expect("create_manager_during_run", run_kit(kit, 0, 0) && marker == 1 && kit.get_owner().get_cmd_set().size() == 2);
}

// 8：释放排在执行 manager 之前的 manager（erase 导致搬迁）
static bool test_destroy_lower_manager()
{
	obs o;
	kit_t kit = make_kit(o);
	int marker = 0;

	test_t::add_command(kit, 1, [](param_t& p) -> res_t
	{
		kit_t& k = p.get_owner();
		const cmd_tgt_t del_tgt(rule_cmd_data_def::op_cmd_type::cmd_tgt, 0, 0, 0, 0, 0);

		if (k.dealloc(p, del_tgt) == false)
			return res_t(int_t::error, std::nullopt);

		return res_t(int_t::next, std::nullopt);
	});
	test_t::add_command(kit, 2, [&marker](param_t&) -> res_t { marker++; return res_t(int_t::next, std::nullopt); });

	add_cmd(kit, 0, 0, 2);
	add_cmd(kit, 1, 0, 1);
	add_cmd(kit, 1, 1, 2);

	return expect("destroy_lower_manager", run_kit(kit, 1, 0) && marker == 1);
}

// 9：运行中分配变量（var 资源表）
static bool test_alloc_var_during_run()
{
	obs o;
	kit_t kit = make_kit(o);
	int marker = 0;

	test_t::add_command(kit, 1, [](param_t& p) -> res_t
	{
		kit_t& k = p.get_owner();
		const var_tgt_t tgt(rule_cmd_data_def::op_var_type::var_val, 0, 0, 0, 0, 8);

		if (k.alloc(p, tgt, true, false) == false)
			return res_t(int_t::error, std::nullopt);

		return res_t(int_t::next, std::nullopt);
	});
	test_t::add_command(kit, 2, [&marker](param_t&) -> res_t { marker++; return res_t(int_t::next, std::nullopt); });

	add_cmd(kit, 0, 0, 1);
	add_cmd(kit, 0, 1, 2);

	return expect("alloc_var_during_run", run_kit(kit, 0, 0) && marker == 1 && kit.get_owner().get_var_set().size() == 1);
}

// 10：运行中在游标之后插入命令并继续执行到它
static bool test_insert_after_cursor()
{
	obs o;
	kit_t kit = make_kit(o);
	int inserted = 0;

	test_t::add_command(kit, 1, [](param_t& p) -> res_t
	{
		kit_t& k = p.get_owner();
		const cmd_tgt_t ins_tgt(rule_cmd_data_def::op_cmd_type::cmd_val, 0, 1, 9, 0, 0);

		if (k.alloc(p, ins_tgt, true, false) == false)
			return res_t(int_t::error, std::nullopt);

		return res_t(int_t::next, std::nullopt);
	});
	test_t::add_command(kit, 9, [&inserted](param_t&) -> res_t { inserted++; return res_t(int_t::next, std::nullopt); });

	add_cmd(kit, 0, 0, 1);

	return expect("insert_after_cursor", run_kit(kit, 0, 0) && inserted == 1);
}

// 11：插入到游标之前后 alloc 失败回滚（del_ex 恢复游标）
static bool test_insert_fail_rollback()
{
	obs o;
	kit_t kit = make_kit(o);
	int marker = 0;

	test_t::add_command(kit, 1, [](param_t&) -> res_t { return res_t(int_t::next, std::nullopt); });
	test_t::add_command(kit, 2, [](param_t& p) -> res_t
	{
		kit_t& k = p.get_owner();
		const cmd_tgt_t bad_tgt(rule_cmd_data_def::op_cmd_type::cmd_val, 0, 2, 0, static_cast<rule_cmd_data_def::var_cmd_begin_pos>(-1), 8);

		if (k.alloc(p, bad_tgt, true, false) == true)
			return res_t(int_t::error, std::nullopt);

		return res_t(int_t::next, std::nullopt);
	});
	test_t::add_command(kit, 3, [&marker](param_t&) -> res_t { marker++; return res_t(int_t::next, std::nullopt); });

	add_cmd(kit, 0, 0, 1);
	add_cmd(kit, 0, 1, 1);
	add_cmd(kit, 0, 5, 2);
	add_cmd(kit, 0, 6, 3);

	return expect("insert_fail_rollback", run_kit(kit, 0, 0) && marker == 1);
}

// 12：循环中反复创建/销毁 manager（压力）
static bool test_loop_create_destroy()
{
	obs o;
	kit_t kit = make_kit(o);
	int counter = 0;

	test_t::add_command(kit, 1, [&counter](param_t& p) -> res_t
	{
		kit_t& k = p.get_owner();
		counter++;

		const cmd_tgt_t alloc_tgt(rule_cmd_data_def::op_cmd_type::cmd_val, 1, 0, 0, 0, 0);
		const cmd_tgt_t del_tgt(rule_cmd_data_def::op_cmd_type::cmd_tgt, 1, 0, 0, 0, 0);

		if (k.alloc(p, alloc_tgt, true, false) == false)
			return res_t(int_t::error, std::nullopt);

		if (k.dealloc(p, del_tgt) == false)
			return res_t(int_t::error, std::nullopt);

		if (counter < 8)
			return res_t(int_t::next, ip_t(0, 0));

		return res_t(int_t::next, std::nullopt);
	});

	add_cmd(kit, 0, 0, 1);

	return expect("loop_create_destroy", run_kit(kit, 0, 0) && counter == 8 && kit.get_owner().get_cmd_set().size() == 1);
}

// 13：删除游标之后的命令（游标不应移动）
static bool test_delete_after_cursor()
{
	obs o;
	kit_t kit = make_kit(o);
	int marker = 0;

	test_t::add_command(kit, 1, [](param_t& p) -> res_t
	{
		kit_t& k = p.get_owner();
		const cmd_tgt_t alloc_tgt(rule_cmd_data_def::op_cmd_type::cmd_val, 0, 2, 0, 0, 0);
		const cmd_tgt_t del_tgt(rule_cmd_data_def::op_cmd_type::cmd_tgt, 0, 2, 0, 0, 0);

		if (k.alloc(p, alloc_tgt, true, false) == false)
			return res_t(int_t::error, std::nullopt);

		if (k.dealloc(p, del_tgt) == false)
			return res_t(int_t::error, std::nullopt);

		return res_t(int_t::next, std::nullopt);
	});
	test_t::add_command(kit, 2, [&marker](param_t&) -> res_t { marker++; return res_t(int_t::next, std::nullopt); });

	add_cmd(kit, 0, 0, 1);
	add_cmd(kit, 0, 1, 2);
	add_cmd(kit, 0, 2, 1);

	return expect("delete_after_cursor", run_kit(kit, 0, 0) && marker == 1 && kit.get_owner().get_cmd_set().get(0).has_value() == true);
}

// 14：删除最后一条命令（manager 仍存活，游标到 end）
static bool test_delete_last_command_manager_survives()
{
	obs o;
	kit_t kit = make_kit(o);

	test_t::add_command(kit, 1, [](param_t& p) -> res_t
	{
		kit_t& k = p.get_owner();
		const cmd_tgt_t alloc_tgt(rule_cmd_data_def::op_cmd_type::cmd_val, 0, 0, 0, 0, 0);
		const cmd_tgt_t del_tgt(rule_cmd_data_def::op_cmd_type::cmd_tgt, 0, 0, 0, 0, 0);

		if (k.alloc(p, alloc_tgt, true, false) == false)
			return res_t(int_t::error, std::nullopt);

		if (k.dealloc(p, del_tgt) == false)
			return res_t(int_t::error, std::nullopt);

		return res_t(int_t::next, std::nullopt);
	});

	add_cmd(kit, 0, 0, 1);

	return expect("delete_last_command_manager_survives", run_kit(kit, 0, 0) && kit.get_owner().get_cmd_set().size() == 1 && o.cb == 1);
}

// 15：运行中在更低的 ownership 上新建 manager（rc 插入到执行 manager 之前）
static bool test_create_lower_manager_during_run()
{
	obs o;
	kit_t kit = make_kit(o);
	int marker = 0;

	test_t::add_command(kit, 1, [](param_t& p) -> res_t
	{
		kit_t& k = p.get_owner();
		const cmd_tgt_t tgt(rule_cmd_data_def::op_cmd_type::cmd_val, 0, 0, 0, 0, 0);

		if (k.alloc(p, tgt, true, false) == false)
			return res_t(int_t::error, std::nullopt);

		return res_t(int_t::next, std::nullopt);
	});
	test_t::add_command(kit, 2, [&marker](param_t&) -> res_t { marker++; return res_t(int_t::next, std::nullopt); });

	add_cmd(kit, 1, 0, 1);
	add_cmd(kit, 1, 1, 2);

	return expect("create_lower_manager", run_kit(kit, 1, 0) && marker == 1 && kit.get_owner().get_cmd_set().size() == 2);
}

int main()
{
	bool ok = true;

	ok &= test_sequential();
	ok &= test_jump_forward();
	ok &= test_call_ret();
	ok &= test_insert_before_cursor();
	ok &= test_delete_self_manager_survives();
	ok &= test_delete_self_manager_gone();
	ok &= test_delete_before_cursor();
	ok &= test_create_manager_during_run();
	ok &= test_destroy_lower_manager();
	ok &= test_alloc_var_during_run();
	ok &= test_insert_after_cursor();
	ok &= test_insert_fail_rollback();
	ok &= test_loop_create_destroy();
	ok &= test_delete_after_cursor();
	ok &= test_delete_last_command_manager_survives();
	ok &= test_create_lower_manager_during_run();

	std::printf("%s\n", ok ? "ALL PASS" : "SOME FAILED");
	return ok ? 0 : 1;
}
