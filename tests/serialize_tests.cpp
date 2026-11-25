#include <cstdio>
#include <array>
#include <fstream>
#include <optional>
#include <sstream>
#include "rule_design.h"
#include "kernel/serialize/serialize_rule.h"

namespace desg = rule_design;
using alloc_t = std::allocator<int>;

static bool expect(const char* name, bool ok)
{
	std::printf("%s %s\n", name, ok ? "PASS" : "FAIL");
	return ok;
}

static std::array<desg::rule_desg_var_base, 8> qword_bytes(std::uint64_t value_v)
{
	std::array<desg::rule_desg_var_base, 8> bytes{};

	for (std::size_t i = 0; i < bytes.size(); i++)
		bytes[i] = static_cast<desg::rule_desg_var_base>((value_v >> (i * 8)) & 0xff);

	return bytes;
}

static std::optional<std::uint64_t> read_qword(desg::rule_desg<>& desg, desg::rule_desg_var_index var_index)
{
	auto var_mgr_res = desg.get_owner().get_owner().get_var_set().get(0);

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

	const std::array<desg::rule_desg_var_base, rule_cmd_data_def::imm_width_byte_max> imm = qword_bytes(42);
	desg.add_set_cmd
	(
		0, 0, desg::rule_desg_cmd_def_type::mov, 0,
		{
			desg::rule_desg_op(desg::rule_desg_op_var(desg::rule_desg_var_op_type::var_val, 0, 0, 0, 0, 8)),
			desg::rule_desg_op(desg::rule_desg_op_imm(desg::rule_desg_imm_op_type::qword, imm)),
		}
	);
}

static bool save_to_stream(desg::rule_desg<>& desg, const rule_data_def::rule_run_ip<alloc_t>& ip, std::ostream& out)
{
	return rule_serialize::serialize_rule(desg.get_owner().get_owner(), ip, out).has_value();
}

static bool load_from_stream(desg::rule_desg<>& desg, std::istream& in, rule_data_def::rule_run_ip<alloc_t>& ip)
{
	return rule_serialize::deserialize_rule(desg.get_owner().get_owner(), in, ip).has_value();
}

static bool test_roundtrip_and_run()
{
	desg::rule_desg<> origin;
	add_mov_program(origin);

	std::stringstream stream_v(std::ios::in | std::ios::out | std::ios::binary);
	const rule_data_def::rule_run_ip<alloc_t> ip_start(0, 0);

	if (save_to_stream(origin, ip_start, stream_v) == false)
		return false;

	desg::rule_desg<> loaded;
	rule_data_def::rule_run_ip<alloc_t> ip_load(0, 0);

	if (load_from_stream(loaded, stream_v, ip_load) == false)
		return false;

	loaded.run(0, 0);
	const auto value = read_qword(loaded, 0);

	return expect("roundtrip_and_run", value.has_value() && value.value() == 42);
}

static bool test_sparse_blocks()
{
	desg::rule_desg<> origin;
	auto& rule_v = origin.get_owner().get_owner();

	if (rule_v.get_var_set().add(0, false).has_value() == false)
		return false;

	auto mgr_res = rule_v.get_var_set().get(0);

	if (mgr_res.has_value() == false)
		return false;

	auto& var_mgr_v = mgr_res.value().get().get_content();

	if (var_mgr_v.restore_var(0, 0, 1) == false)
		return false;

	auto element_res = var_mgr_v.get(0);

	if (element_res.has_value() == false)
		return false;

	auto& content_v = element_res.value().get().get_content();
	std::array<desg::rule_desg_var_base, 4> block_a = { desg::rule_desg_var_base(1), desg::rule_desg_var_base(2), desg::rule_desg_var_base(3), desg::rule_desg_var_base(4) };
	std::array<desg::rule_desg_var_base, 4> block_b = { desg::rule_desg_var_base(5), desg::rule_desg_var_base(6), desg::rule_desg_var_base(7), desg::rule_desg_var_base(8) };

	if (content_v.restore_block(0, std::span<const desg::rule_desg_var_base>(block_a)) == false)
		return false;

	if (content_v.restore_block(100, std::span<const desg::rule_desg_var_base>(block_b)) == false)
		return false;

	std::stringstream stream_v(std::ios::in | std::ios::out | std::ios::binary);
	rule_data_def::rule_run_ip<alloc_t> ip(0, 0);

	if (save_to_stream(origin, ip, stream_v) == false)
		return false;

	desg::rule_desg<> loaded;
	rule_data_def::rule_run_ip<alloc_t> ip_load(0, 0);

	if (load_from_stream(loaded, stream_v, ip_load) == false)
		return false;

	auto& loaded_rule_v = loaded.get_owner().get_owner();
	auto loaded_mgr_res = loaded_rule_v.get_var_set().get(0);

	if (loaded_mgr_res.has_value() == false)
		return false;

	auto loaded_element_res = loaded_mgr_res.value().get().get_content().get(0);

	if (loaded_element_res.has_value() == false)
		return false;

	auto& loaded_content_v = loaded_element_res.value().get().get_content();
	const bool hole_kept = loaded_content_v.read(10, 4).has_value() == false;
	const auto span_a = loaded_content_v.read(0, 4);
	const auto span_b = loaded_content_v.read(100, 4);

	bool values_ok = span_a.has_value() && span_b.has_value();

	if (values_ok == true)
	{
		values_ok = span_a.value()[0] == desg::rule_desg_var_base(1) && span_a.value()[3] == desg::rule_desg_var_base(4) &&
			span_b.value()[0] == desg::rule_desg_var_base(5) && span_b.value()[3] == desg::rule_desg_var_base(8);
	}

	return expect("sparse_blocks", hole_kept && values_ok && loaded_content_v.size() == 104);
}

static bool test_stack_and_ip()
{
	desg::rule_desg<> origin;
	add_mov_program(origin);
	auto& rule_v = origin.get_owner().get_owner();
	rule_v.get_stack().emplace_back(0, 7);

	std::stringstream stream_v(std::ios::in | std::ios::out | std::ios::binary);
	const rule_data_def::rule_run_ip<alloc_t> ip_start(0, 3);

	if (save_to_stream(origin, ip_start, stream_v) == false)
		return false;

	desg::rule_desg<> loaded;
	rule_data_def::rule_run_ip<alloc_t> ip_load(0, 0);

	if (load_from_stream(loaded, stream_v, ip_load) == false)
		return false;

	auto& loaded_rule_v = loaded.get_owner().get_owner();
	const bool stack_ok = loaded_rule_v.get_stack().size() == 1 &&
		loaded_rule_v.get_stack()[0].get_cmd_set_index() == 0 &&
		loaded_rule_v.get_stack()[0].get_index() == 7;
	const bool ip_ok = ip_load.get_cmd_set_index() == 0 && ip_load.get_index() == 3;

	return expect("stack_and_ip", stack_ok && ip_ok);
}

static bool test_resume_with_stack()
{
	desg::rule_desg<> origin;
	std::array<desg::rule_desg_var_base, 8> data{};
	origin.add_set_var(0, 0, 0, 0, std::span<desg::rule_desg_var_base>(data));
	origin.add_set_cmd(0, 0, desg::rule_desg_cmd_def_type::ret, 0, {});

	const std::array<desg::rule_desg_var_base, rule_cmd_data_def::imm_width_byte_max> imm = qword_bytes(42);
	origin.add_set_cmd(0, 1, desg::rule_desg_cmd_def_type::mov, 0,
		{
			desg::rule_desg_op(desg::rule_desg_op_var(desg::rule_desg_var_op_type::var_val, 0, 0, 0, 0, 8)),
			desg::rule_desg_op(desg::rule_desg_op_imm(desg::rule_desg_imm_op_type::qword, imm)),
		});

	auto& rule_v = origin.get_owner().get_owner();
	rule_v.get_stack().emplace_back(0, 0);

	std::stringstream stream_v(std::ios::in | std::ios::out | std::ios::binary);
	const rule_data_def::rule_run_ip<alloc_t> ip_start(0, 0);

	if (save_to_stream(origin, ip_start, stream_v) == false)
		return false;

	desg::rule_desg<> loaded;
	rule_data_def::rule_run_ip<alloc_t> ip_load(0, 1);

	if (load_from_stream(loaded, stream_v, ip_load) == false)
		return false;

	loaded.get_owner().get_owner().resume(ip_load);
	const auto value = read_qword(loaded, 0);

	return expect("resume_with_stack", value.has_value() && value.value() == 42 && loaded.get_owner().get_owner().get_stack().size() == 0);
}

static bool test_ref_counts()
{
	desg::rule_desg<> origin;
	std::array<desg::rule_desg_var_base, 8> data{};
	origin.add_set_var(0, 0, 0, 0, std::span<desg::rule_desg_var_base>(data));
	origin.add_set_var(0, 0, 0, 0, std::span<desg::rule_desg_var_base>(data));

	std::stringstream stream_v(std::ios::in | std::ios::out | std::ios::binary);
	rule_data_def::rule_run_ip<alloc_t> ip(0, 0);

	if (save_to_stream(origin, ip, stream_v) == false)
		return false;

	desg::rule_desg<> loaded;
	rule_data_def::rule_run_ip<alloc_t> ip_load(0, 0);

	if (load_from_stream(loaded, stream_v, ip_load) == false)
		return false;

	auto mgr_res = loaded.get_owner().get_owner().get_var_set().get(0);

	if (mgr_res.has_value() == false)
		return false;

	auto ref_res = mgr_res.value().get().get_content().get_ref(0);

	return expect("ref_counts", ref_res.has_value() && ref_res.value() == 2);
}

static bool test_error_paths()
{
	desg::rule_desg<> origin;
	add_mov_program(origin);

	std::stringstream stream_v(std::ios::in | std::ios::out | std::ios::binary);
	const rule_data_def::rule_run_ip<alloc_t> ip_start(0, 0);

	if (save_to_stream(origin, ip_start, stream_v) == false)
		return false;

	const std::string bytes = stream_v.str();

	const bool bad_magic = [&]()
	{
		desg::rule_desg<> loaded;
		std::string bad = bytes;
		bad[0] = 'X';
		std::stringstream bad_stream(bad, std::ios::in | std::ios::out | std::ios::binary);
		rule_data_def::rule_run_ip<alloc_t> ip(0, 0);
		const auto res = rule_serialize::deserialize_rule(loaded.get_owner().get_owner(), bad_stream, ip);
		return res.has_value() == false && res.error() == kernel_error::serialize__magic_mismatch &&
			loaded.get_owner().get_owner().get_var_set().size() == 0;
	}();

	const bool bad_crc = [&]()
	{
		desg::rule_desg<> loaded;
		std::string bad = bytes;
		bad.back() = static_cast<char>(static_cast<unsigned char>(bad.back()) ^ 0xFF);
		std::stringstream bad_stream(bad, std::ios::in | std::ios::out | std::ios::binary);
		rule_data_def::rule_run_ip<alloc_t> ip(0, 0);
		const auto res = rule_serialize::deserialize_rule(loaded.get_owner().get_owner(), bad_stream, ip);
		return res.has_value() == false && res.error() == kernel_error::serialize__crc_mismatch;
	}();

	const bool truncated = [&]()
	{
		desg::rule_desg<> loaded;
		std::string bad = bytes.substr(0, bytes.size() - 1);
		std::stringstream bad_stream(bad, std::ios::in | std::ios::out | std::ios::binary);
		rule_data_def::rule_run_ip<alloc_t> ip(0, 0);
		const auto res = rule_serialize::deserialize_rule(loaded.get_owner().get_owner(), bad_stream, ip);

		return res.has_value() == false && (res.error() == kernel_error::serialize__truncated || res.error() == kernel_error::serialize__crc_mismatch);
	}();

	const bool missing_type = [&]()
	{
		desg::rule_desg<> loaded;
		loaded.get_owner().get_owner().get_cmd_def_set().erase(static_cast<rule_cmd_data_def::cmd_type>(0));
		std::stringstream bad_stream(bytes, std::ios::in | std::ios::out | std::ios::binary);
		rule_data_def::rule_run_ip<alloc_t> ip(0, 0);
		const auto res = rule_serialize::deserialize_rule(loaded.get_owner().get_owner(), bad_stream, ip);
		return res.has_value() == false && res.error() == kernel_error::serialize__cmd_type_missing;
	}();


	return expect("error_paths", bad_magic && bad_crc && truncated && missing_type);
}

static bool test_disk_roundtrip()
{
	const char* path_v = "serialize_disk_test.rvm";
	desg::rule_desg<> origin;
	add_mov_program(origin);

	desg::rule_desg_cmd_mgr_index mgr_index_v = 0;
	desg::rule_desg_cmd_index cmd_index_v = 0;

	if (origin.save_file(path_v, mgr_index_v, cmd_index_v) == false)
		return false;

	desg::rule_desg<> loaded;
	mgr_index_v = 0;
	cmd_index_v = 0;

	if (loaded.load_file(path_v, mgr_index_v, cmd_index_v) == false)
		return false;

	std::remove(path_v);
	loaded.resume(mgr_index_v, cmd_index_v);
	const auto value = read_qword(loaded, 0);

	return expect("disk_roundtrip", value.has_value() && value.value() == 42);
}

int main()
{
	bool ok = true;

	ok &= test_roundtrip_and_run();
	ok &= test_sparse_blocks();
	ok &= test_stack_and_ip();
	ok &= test_resume_with_stack();
	ok &= test_ref_counts();
	ok &= test_error_paths();
	ok &= test_disk_roundtrip();

	std::printf("%s\n", ok ? "ALL PASS" : "SOME FAILED");
	return ok ? 0 : 1;
}
