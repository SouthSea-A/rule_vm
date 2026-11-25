#include <chrono>
#include <cstdio>
#include <cstdint>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include "rule_design.h"
#include "kernel/manager/manager.h"

namespace desg = rule_design;
using steady_t = std::chrono::steady_clock;

inline std::size_t perf_allocs = 0;
inline std::size_t perf_bytes = 0;
inline std::size_t perf_live_bytes = 0;
inline std::size_t perf_peak_bytes = 0;

template<typename t>
struct perf_allocator
{
public:
	using value_type = t;
public:
	perf_allocator() = default;
	template<typename u>
	perf_allocator(const perf_allocator<u>&) noexcept
	{
	}
	t* allocate(std::size_t count)
	{
		perf_allocs++;
		const std::size_t bytes_v = count * sizeof(t);
		perf_bytes += bytes_v;
		perf_live_bytes += bytes_v;
		perf_peak_bytes = perf_peak_bytes < perf_live_bytes ? perf_live_bytes : perf_peak_bytes;
		return std::allocator<t>().allocate(count);
	}
	void deallocate(t* pointer_v, std::size_t count) noexcept
	{
		std::allocator<t>().deallocate(pointer_v, count);
		perf_live_bytes -= count * sizeof(t);
	}
	template<typename u>
	struct rebind
	{
		using other = perf_allocator<u>;
	};
};

template<typename t, typename u>
bool operator==(const perf_allocator<t>&, const perf_allocator<u>&) noexcept
{
	return true;
}

template<typename t, typename u>
bool operator!=(const perf_allocator<t>&, const perf_allocator<u>&) noexcept
{
	return false;
}

using alloc_t = perf_allocator<int>;
using desg_t = desg::rule_desg<alloc_t>;
using error_t = desg::rule_desg_text_error<alloc_t>;

static double elapsed_ms(steady_t::time_point begin_v)
{
	return std::chrono::duration<double, std::milli>(steady_t::now() - begin_v).count();
}

static void reset_perf()
{
	perf_allocs = 0;
	perf_bytes = 0;
	perf_live_bytes = 0;
	perf_peak_bytes = 0;
	return;
}

static void print_result(const char* name_v, std::size_t size_v, double load_ms_v, double run_ms_v, std::size_t units_v)
{
	const double unit_ms_v = load_ms_v + run_ms_v;
	const double throughput_v = unit_ms_v > 0.0 ? static_cast<double>(units_v) / unit_ms_v : 0.0;

	std::printf("%-22s size=%-9zu load=%9.3f ms run=%9.3f ms total=%9.3f ms throughput=%10.1f k/s allocs=%-8zu bytes=%-10zu peak=%-10zu\n",
		name_v,
		size_v,
		load_ms_v,
		run_ms_v,
		unit_ms_v,
		throughput_v,
		perf_allocs,
		perf_bytes,
		perf_peak_bytes);
	return;
}

static std::uint64_t read_qword(desg_t& desg, desg::rule_desg_var_index index_v)
{
	auto mgr_res = desg.get_owner().get_owner().get_var_set().get(0);
	auto var_res = mgr_res.value().get().get_content().get(index_v);
	auto span_res = var_res.value().get().get_content().read(0, 8);
	std::uint64_t value_v = 0;

	for (std::size_t i = 0; i < 8; i++)
		value_v |= static_cast<std::uint64_t>(span_res.value()[i]) << (i * 8);

	return value_v;
}

static bool load_text_checked(desg_t& desg, const std::string& text_v)
{
	error_t error_v;
	const bool loaded = desg.load_text(text_v, error_v);

	if (loaded == false)
		std::printf("  LOAD ERROR: %s\n", error_v.message.c_str());

	return loaded;
}

static void bench_chain(std::size_t count_v)
{
	std::string text_v;
	text_v.reserve(count_v * 16 + 128);
	text_v += "defvar 0 {\nvar a 0:0:0:0:8\n}\ndefcmd 0 {\nmov a, 0\n";

	for (std::size_t i = 0; i < count_v; i++)
		text_v += "add a, a, 1\n";

	text_v += "ret\n}\nstart defcmd 0\n";

	desg_t desg;
	reset_perf();
	const auto load_begin_v = steady_t::now();
	const bool loaded = load_text_checked(desg, text_v);
	const double load_ms_v = elapsed_ms(load_begin_v);

	const auto run_begin_v = steady_t::now();
	desg.run_start();
	const double run_ms_v = elapsed_ms(run_begin_v);
	const auto value_v = read_qword(desg, 0);

	print_result("chain_add", count_v, load_ms_v, run_ms_v, count_v);
	std::printf("  check: loaded=%d value=%llu expected=%zu\n", (int)loaded, static_cast<unsigned long long>(value_v), count_v);
	return;
}

static void bench_auto_alloc(std::size_t count_v)
{
	std::string text_v;
	text_v.reserve(count_v * 34 + 128);
	text_v += "defvar 0 {}\ndefcmd 0 {\n";

	for (std::size_t i = 0; i < count_v; i++)
	{
		text_v += "mov v(0:";
		text_v += std::to_string(i);
		text_v += ":0:0:8), ";
		text_v += std::to_string(i * 3 + 1);
		text_v += "\n";
	}

	text_v += "ret\n}\nstart defcmd 0\n";

	desg_t desg;
	reset_perf();
	const auto load_begin_v = steady_t::now();
	const bool loaded = load_text_checked(desg, text_v);
	const double load_ms_v = elapsed_ms(load_begin_v);

	const auto run_begin_v = steady_t::now();
	desg.run_start();
	const double run_ms_v = elapsed_ms(run_begin_v);

	auto var_mgr_res = desg.get_owner().get_owner().get_var_set().get(0);
	const bool content_ok = var_mgr_res.has_value() &&
		var_mgr_res.value().get().get_content().get(static_cast<desg::rule_desg_var_index>(count_v - 1)).has_value();

	print_result("auto_alloc_var", count_v, load_ms_v, run_ms_v, count_v);
	std::printf("  check: loaded=%d vars=%d\n", (int)loaded, (int)content_ok);
	return;
}

static void bench_loop(std::size_t count_v)
{
	std::string text_v;
	text_v.reserve(512);
	text_v += "defvar 0 {\n";
	text_v += "var i 0:0:0:0:8\n";
	text_v += "var one 0:1:0:0:8 = 1\n";
	text_v += "var zero 0:2:0:0:8 = 0\n";
	text_v += "var flag 0:3:0:0:1\n";
	text_v += "}\ndefcmd 0 {\nmov i, " + std::to_string(count_v) + "\n";
	text_v += "loop:\ncmp flag, i, zero\njmp done, eq, flag\n";
	text_v += "sub i, i, one\njmp loop\ndone:\nret\n}\nstart defcmd 0\n";

	desg_t desg;
	reset_perf();
	const auto load_begin_v = steady_t::now();
	const bool loaded = load_text_checked(desg, text_v);
	const double load_ms_v = elapsed_ms(load_begin_v);

	const auto run_begin_v = steady_t::now();
	desg.run_start();
	const double run_ms_v = elapsed_ms(run_begin_v);
	const auto value_v = read_qword(desg, 0);

	print_result("loop_countdown", count_v, load_ms_v, run_ms_v, count_v);
	std::printf("  check: loaded=%d i=%llu\n", (int)loaded, static_cast<unsigned long long>(value_v));
	return;
}

static void bench_call(std::size_t depth_v)
{
	std::string text_v;
	text_v.reserve(depth_v * 48 + 128);
	text_v += "defvar 0 {\nvar a 0:0:0:0:8\n}\ndefcmd 0 {\n";

	for (std::size_t i = 0; i < depth_v; i++)
	{
		text_v += "call c(cmd_tgt:0:";
		text_v += std::to_string((i + 1) * 2);
		text_v += ":0:0:0)\nret\n";
	}

	text_v += "ret\n}\nstart defcmd 0\n";

	desg_t desg;
	reset_perf();
	const auto load_begin_v = steady_t::now();
	const bool loaded = load_text_checked(desg, text_v);
	const double load_ms_v = elapsed_ms(load_begin_v);

	const auto run_begin_v = steady_t::now();
	desg.run_start();
	const double run_ms_v = elapsed_ms(run_begin_v);

	print_result("call_depth", depth_v, load_ms_v, run_ms_v, depth_v);
	std::printf("  check: loaded=%d\n", (int)loaded);
	return;
}

static void bench_string(std::size_t size_v)
{
	std::string payload_v(size_v, 'A');

	std::string text_v;
	text_v.reserve(size_v + 256);
	text_v += "defvar 0 {\nvar s1 0:0:0:0 = \"";
	text_v += payload_v;
	text_v += "\"\nvar s2 0:1:0:0:";
	text_v += std::to_string(size_v);
	text_v += "\n}\ndefcmd 0 {\nmov s2, s1\nret\n}\nstart defcmd 0\n";

	desg_t desg;
	reset_perf();
	const auto load_begin_v = steady_t::now();
	const bool loaded = load_text_checked(desg, text_v);
	const double load_ms_v = elapsed_ms(load_begin_v);

	const auto run_begin_v = steady_t::now();
	desg.run_start();
	const double run_ms_v = elapsed_ms(run_begin_v);

	auto var_mgr_res = desg.get_owner().get_owner().get_var_set().get(0);
	bool copy_ok = false;

	if (var_mgr_res.has_value())
	{
		auto var_res = var_mgr_res.value().get().get_content().get(1);

		if (var_res.has_value())
		{
			auto span_res = var_res.value().get().get_content().read(0, static_cast<rule_var_cont_data_def::var_read_width>(size_v));
			copy_ok = span_res.has_value() && std::to_integer<unsigned char>(span_res.value().front()) == static_cast<unsigned char>('A') && std::to_integer<unsigned char>(span_res.value().back()) == static_cast<unsigned char>('A');
		}
	}

	print_result("string_copy", size_v, load_ms_v, run_ms_v, size_v);
	std::printf("  check: loaded=%d copy=%d\n", (int)loaded, (int)copy_ok);
	return;
}

static void bench_jit()
{
	desg_t desg;
	error_t error_v;
	reset_perf();
	const auto load_begin_v = steady_t::now();
	const bool loaded = desg.load_text_file("example/programs/jit_expr3.rule", error_v);
	const double load_ms_v = elapsed_ms(load_begin_v);

	const auto run_begin_v = steady_t::now();
	desg.run_start();
	const double run_ms_v = elapsed_ms(run_begin_v);
	const auto value_v = read_qword(desg, 6);

	print_result("jit_3stage", 3, load_ms_v, run_ms_v, 3);
	std::printf("  check: loaded=%d r=%llu\n", (int)loaded, static_cast<unsigned long long>(value_v));
	return;
}

static void bench_serialize(std::size_t count_v)
{
	std::string text_v;
	text_v.reserve(count_v * 16 + 128);
	text_v += "defvar 0 {\nvar a 0:0:0:0:8\n}\ndefcmd 0 {\nmov a, 0\n";

	for (std::size_t i = 0; i < count_v; i++)
		text_v += "add a, a, 1\n";

	text_v += "ret\n}\nstart defcmd 0\n";

	desg_t desg;
	reset_perf();
	const auto load_begin_v = steady_t::now();
	const bool loaded = load_text_checked(desg, text_v);
	const double load_ms_v = elapsed_ms(load_begin_v);

	std::ostringstream out_v(std::ios::out | std::ios::binary);
	const auto save_begin_v = steady_t::now();
	const bool saved = desg.save(0, 0, out_v);
	const double save_ms_v = elapsed_ms(save_begin_v);

	std::istringstream in_v(out_v.str(), std::ios::binary);
	desg::rule_desg_cmd_mgr_index mgr_v = 0;
	desg::rule_desg_cmd_index index_v = 0;
	const auto restore_begin_v = steady_t::now();
	const bool restored = desg.load(in_v, mgr_v, index_v);
	const double restore_ms_v = elapsed_ms(restore_begin_v);

	print_result("serialize", count_v, load_ms_v, save_ms_v + restore_ms_v, count_v);
	std::printf("  check: loaded=%d saved=%d restored=%d save_ms=%.3f restore_ms=%.3f stream=%zu mgr=%llu index=%llu\n",
		(int)loaded,
		(int)saved,
		(int)restored,
		save_ms_v,
		restore_ms_v,
		out_v.str().size(),
		static_cast<unsigned long long>(mgr_v),
		static_cast<unsigned long long>(index_v));

	return;
}

static void bench_manager(std::size_t count_v)
{
	using mgr_t = rule_manager::manager<int, 4>;
	mgr_t mgr;

	const auto build_begin_v = steady_t::now();
	bool build_ok = true;

	for (std::size_t i = 0; i < count_v; i++)
	{
		if (mgr.push_back(static_cast<int>(i)).has_value() == false)
		{
			build_ok = false;
			break;
		}
	}

	const double build_ms_v = elapsed_ms(build_begin_v);

	const auto query_begin_v = steady_t::now();
	std::uint64_t sum_v = 0;

	for (std::size_t i = 0; i < count_v; i++)
	{
		auto value_v = mgr.at(i);

		if (value_v.has_value())
			sum_v += static_cast<std::uint64_t>(value_v.value().get());
	}

	const double query_ms_v = elapsed_ms(query_begin_v);

	reset_perf();
	print_result("manager_push_at", count_v, build_ms_v, query_ms_v, count_v);
	std::printf("  check: build=%d size=%zu sum=%llu\n", (int)build_ok, mgr.size(), static_cast<unsigned long long>(sum_v));
	return;
}

int main()
{
	std::printf("case                  size      load_ms    run_ms    total_ms   throughput(k/s) allocs   bytes   peak\n");
	std::printf("----------------------------------------------------------------------------------------------------\n");

	bench_chain(10000);
	bench_chain(100000);
	bench_auto_alloc(10000);
	bench_auto_alloc(100000);
	bench_loop(1000000);
	bench_call(2000);
	bench_string(1024 * 1024);
	bench_string(8 * 1024 * 1024);
	bench_jit();
	bench_serialize(10000);
	bench_serialize(100000);
	bench_manager(1000000);

	return 0;
}
