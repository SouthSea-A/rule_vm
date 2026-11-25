#include <cstdio>
#include <array>
#include <span>
#include <optional>
#include "rule_design.h"

namespace desg = rule_design;

static bool expect(const char* name, bool ok)
{
	std::printf("%s %s\n", name, ok ? "PASS" : "FAIL");
	return ok;
}

static std::array<desg::rule_desg_var_base, 8> make_qword(std::uint64_t value_v)
{
	std::array<desg::rule_desg_var_base, 8> bytes{};

	for (std::size_t i = 0; i < bytes.size(); i++)
		bytes[i] = static_cast<desg::rule_desg_var_base>((value_v >> (i * 8)) & 0xff);

	return bytes;
}

static std::optional<std::uint64_t> read_qword(desg::rule_desg<>& desg, desg::rule_desg_var_mgr_index mgr_index, desg::rule_desg_var_index var_index)
{
	auto var_mgr_res = desg.get_owner().get_owner().get_var_set().get(mgr_index);

	if (var_mgr_res.has_value() == false)
		return std::nullopt;

	auto var_res = var_mgr_res.value().get().get_content().get(var_index);

	if (var_res.has_value() == false)
		return std::nullopt;

	auto span_res = var_res.value().get().get_content().read(0, 8);

	if (span_res.has_value() == false)
		return std::nullopt;

	std::uint64_t value_v = 0;

	for (std::size_t i = 0; i < 8; i++)
		value_v |= static_cast<std::uint64_t>(span_res.value()[i]) << (i * 8);

	return value_v;
}

static void add_mov_program(desg::rule_desg<>& desg)
{
	std::array<desg::rule_desg_var_base, 8> data{};
	desg.add_set_var(0, 0, 0, 0, std::span<desg::rule_desg_var_base>(data));

	const std::array<desg::rule_desg_var_base, rule_cmd_data_def::imm_width_byte_max> imm_data = make_qword(42);
	desg.add_set_cmd
	(
		0, 0, desg::rule_desg_cmd_def_type::mov, 0,
		{
			desg::rule_desg_op(desg::rule_desg_op_var(desg::rule_desg_var_op_type::var_val, 0, 0, 0, 0, 8)),
			desg::rule_desg_op(desg::rule_desg_op_imm(desg::rule_desg_imm_op_type::qword, imm_data)),
		}
	);
}

// 1. 指令定义改为静态函数后，copy_from 的副本必须使用副本自己的 kit 执行
static bool test_static_def_copy_runs_on_copy()
{
	desg::rule_desg<> origin;
	add_mov_program(origin);

	desg::rule_desg<> copy;
	copy.copy_from(origin);
	copy.run(0, 0);

	const std::optional<std::uint64_t> value_copy = read_qword(copy, 0, 0);
	const std::optional<std::uint64_t> value_origin = read_qword(origin, 0, 0);

	return expect("static_def_copy_runs_on_copy", value_copy.has_value() && value_copy.value() == 42 && value_origin.has_value() && value_origin.value() == 0);
}

// 2. alloc/dealloc 指令（var_target）
static bool test_alloc_dealloc_var()
{
	desg::rule_desg<> desg;
	const desg::rule_desg_op_var var_tgt(desg::rule_desg_var_op_type::var_val, 0, 5, 0, 0, 8);

	desg.add_set_cmd(0, 0, desg::rule_desg_cmd_def_type::alloc, 0, { desg::rule_desg_op(var_tgt) });
	desg.add_set_cmd(0, 1, desg::rule_desg_cmd_def_type::dealloc, 0, { desg::rule_desg_op(var_tgt) });
	desg.run(0, 0);

	return expect("alloc_dealloc_var", desg.get_owner().get_owner().get_var_set().size() == 0);
}

// 3. alloc/dealloc 指令（cmd_target）
static bool test_alloc_dealloc_cmd()
{
	desg::rule_desg<> desg;
	const desg::rule_desg_op_cmd alloc_cmd_tgt(desg::rule_desg_cmd_op_type::cmd_val, 0, 2, 9, 0, 0);
	const desg::rule_desg_op_cmd dealloc_cmd_tgt(desg::rule_desg_cmd_op_type::cmd_tgt, 0, 2, 0, 0, 0);

	desg.add_set_cmd(0, 0, desg::rule_desg_cmd_def_type::alloc, 0, { desg::rule_desg_op(alloc_cmd_tgt) });
	desg.add_set_cmd(0, 1, desg::rule_desg_cmd_def_type::dealloc, 0, { desg::rule_desg_op(dealloc_cmd_tgt) });
	desg.run(0, 0);

	auto cmd_mgr_res = desg.get_owner().get_owner().get_cmd_set().get(0);
	bool cmd2_absent = false;

	if (cmd_mgr_res.has_value() == true)
		cmd2_absent = cmd_mgr_res.value().get().get_content().get(2).has_value() == false;

	return expect("alloc_dealloc_cmd", desg.get_owner().get_owner().get_cmd_set().size() == 1 && cmd2_absent);
}

// 4. add 指令：var0 = var1 + imm
static bool test_add()
{
	desg::rule_desg<> desg;
	const auto data0 = make_qword(10);
	const auto data1 = make_qword(10);
	desg.add_set_var(0, 0, 0, 0, std::span<desg::rule_desg_var_base>(const_cast<desg::rule_desg_var_base*>(data0.data()), data0.size()));
	desg.add_set_var(0, 1, 0, 0, std::span<desg::rule_desg_var_base>(const_cast<desg::rule_desg_var_base*>(data1.data()), data1.size()));

	const std::array<desg::rule_desg_var_base, rule_cmd_data_def::imm_width_byte_max> imm_data = make_qword(5);
	desg.add_set_cmd
	(
		0, 0, desg::rule_desg_cmd_def_type::add, 0,
		{
			desg::rule_desg_op(desg::rule_desg_op_var(desg::rule_desg_var_op_type::var_val, 0, 0, static_cast<desg::rule_desg_var_type>(desg::rule_desg_calc_type::unsigned_integer), 0, 8)),
			desg::rule_desg_op(desg::rule_desg_op_var(desg::rule_desg_var_op_type::var_val, 0, 1, 0, 0, 8)),
			desg::rule_desg_op(desg::rule_desg_op_imm(desg::rule_desg_imm_op_type::qword, imm_data)),
		}
	);
	desg.run(0, 0);

	const std::optional<std::uint64_t> value = read_qword(desg, 0, 0);



	return expect("add", value.has_value() && value.value() == 15);
}

// 5. sub / mul / div 指令：dst = var1 op imm
static bool run_arith_imm(desg::rule_desg_cmd_def_type op_v, std::uint64_t left_v, std::uint64_t right_v, std::uint64_t expected_v)
{
	desg::rule_desg<> desg;
	std::array<desg::rule_desg_var_base, 8> data_dst{};
	const auto data_src = make_qword(left_v);
	desg.add_set_var(0, 0, 0, 0, std::span<desg::rule_desg_var_base>(data_dst));
	desg.add_set_var(0, 1, 0, 0, std::span<desg::rule_desg_var_base>(const_cast<desg::rule_desg_var_base*>(data_src.data()), data_src.size()));

	const std::array<desg::rule_desg_var_base, rule_cmd_data_def::imm_width_byte_max> imm_data = make_qword(right_v);
	desg.add_set_cmd
	(
		0, 0, op_v, 0,
		{
			desg::rule_desg_op(desg::rule_desg_op_var(desg::rule_desg_var_op_type::var_val, 0, 0, static_cast<desg::rule_desg_var_type>(desg::rule_desg_calc_type::unsigned_integer), 0, 8)),
			desg::rule_desg_op(desg::rule_desg_op_var(desg::rule_desg_var_op_type::var_val, 0, 1, 0, 0, 8)),
			desg::rule_desg_op(desg::rule_desg_op_imm(desg::rule_desg_imm_op_type::qword, imm_data)),
		}
	);
	desg.run(0, 0);

	const std::optional<std::uint64_t> value = read_qword(desg, 0, 0);

	return value.has_value() && value.value() == expected_v;
}

static bool test_sub_mul_div()
{
	const bool sub_ok = run_arith_imm(desg::rule_desg_cmd_def_type::sub, 20, 3, 17);
	const bool mul_ok = run_arith_imm(desg::rule_desg_cmd_def_type::mul, 20, 3, 60);
	const bool div_ok = run_arith_imm(desg::rule_desg_cmd_def_type::div, 20, 3, 6);

	return expect("sub_mul_div", sub_ok && mul_ok && div_ok);
}

// 6. jmp 指令：无条件跳到 cmd3，跳过 cmd1/cmd2
static bool test_jmp()
{
	desg::rule_desg<> desg;
	std::array<desg::rule_desg_var_base, 8> data{};
	desg.add_set_var(0, 0, 0, 0, std::span<desg::rule_desg_var_base>(data));

	std::array<desg::rule_desg_var_base, rule_cmd_data_def::imm_width_byte_max> imm_cond = make_qword(static_cast<std::uint64_t>(desg::rule_desg_compare_type::unconditional));

	desg.add_set_cmd
	(
		0, 0, desg::rule_desg_cmd_def_type::jmp, 0,
		{
			desg::rule_desg_op(desg::rule_desg_op_cmd(desg::rule_desg_cmd_op_type::cmd_tgt, 0, 3, 0, 0, 0)),
			desg::rule_desg_op(desg::rule_desg_op_imm(desg::rule_desg_imm_op_type::qword, imm_cond)),
			desg::rule_desg_op(desg::rule_desg_op_imm(desg::rule_desg_imm_op_type::qword, imm_cond)),
		}
	);
	desg.add_set_cmd(0, 1, desg::rule_desg_cmd_def_type::mov, 0,
		{
			desg::rule_desg_op(desg::rule_desg_op_var(desg::rule_desg_var_op_type::var_val, 0, 0, 0, 0, 8)),
			desg::rule_desg_op(desg::rule_desg_op_imm(desg::rule_desg_imm_op_type::qword, make_qword(1))),
		});
	const std::array<desg::rule_desg_var_base, rule_cmd_data_def::imm_width_byte_max> imm7 = make_qword(7);
	desg.add_set_cmd(0, 3, desg::rule_desg_cmd_def_type::mov, 0,
		{
			desg::rule_desg_op(desg::rule_desg_op_var(desg::rule_desg_var_op_type::var_val, 0, 0, 0, 0, 8)),
			desg::rule_desg_op(desg::rule_desg_op_imm(desg::rule_desg_imm_op_type::qword, imm7)),
		});
	desg.run(0, 0);

	const std::optional<std::uint64_t> value = read_qword(desg, 0, 0);

	return expect("jmp", value.has_value() && value.value() == 7);
}

// 6. call/ret：call 跳到 cmd2 的 ret，返回到 cmd1 继续执行
static bool test_call_ret()
{
	desg::rule_desg<> desg;
	std::array<desg::rule_desg_var_base, 8> data{};
	desg.add_set_var(0, 0, 0, 0, std::span<desg::rule_desg_var_base>(data));

	desg.add_set_cmd(0, 0, desg::rule_desg_cmd_def_type::call, 0,
		{
			desg::rule_desg_op(desg::rule_desg_op_cmd(desg::rule_desg_cmd_op_type::cmd_tgt, 0, 2, 0, 0, 0)),
		});
	desg.add_set_cmd(0, 1, desg::rule_desg_cmd_def_type::mov, 0,
		{
			desg::rule_desg_op(desg::rule_desg_op_var(desg::rule_desg_var_op_type::var_val, 0, 0, 0, 0, 8)),
			desg::rule_desg_op(desg::rule_desg_op_imm(desg::rule_desg_imm_op_type::qword, make_qword(9))),
		});
	desg.add_set_cmd(0, 2, desg::rule_desg_cmd_def_type::ret, 0, {});
	desg.run(0, 0);

	const std::optional<std::uint64_t> value = read_qword(desg, 0, 0);

	return expect("call_ret", value.has_value() && value.value() == 9);
}

int main()
{
	bool ok = true;

	ok &= test_static_def_copy_runs_on_copy();
	ok &= test_alloc_dealloc_var();
	ok &= test_alloc_dealloc_cmd();
	ok &= test_add();
	ok &= test_sub_mul_div();
	ok &= test_jmp();
	ok &= test_call_ret();

	std::printf("%s\n", ok ? "ALL PASS" : "SOME FAILED");
	return ok ? 0 : 1;
}
