#pragma once
#include "kernel/kernel_namespace.h"

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <utility>
#include <type_traits>
#include <functional>
#include <optional>
#include <span>
#include <vector>
#include <fstream>
#include <filesystem>
#include <string>
#include <iterator>
#include <initializer_list>
#include <cstdio>
#include "kernel/rule/rule.h"
#include "kernel/serialize/serialize_file.h"
#include "kernel/serialize/serialize_file.h"
#include "rule_design_alias.h"
#include "rule_design_data_def.h"
#include "rule_design_error.h"
#include "rule_design_text.h"

namespace rule_design
{	template<typename alloc_rule_design = std::allocator<int>>
	class rule_desg
	{
	private:
	private:
		using alloc_traits = typename std::allocator_traits<alloc_rule_design>;
		using alloc_raw_buf = typename alloc_traits::template rebind_alloc<rule_desg_var_base>;
	private:
		using invoke_ownership = rule_kit::rule_kit_context<alloc_rule_design, rule_desg>;
		using invoke_outer = rule_kit::rule_kit_outer<invoke_ownership, rule_desg>;
		using invoke_error_run = act_data::act_run_int_t;
		using invoke_error_act = act_data::act_err_t;
		using invoke_error_info = rule_kit::rule_kit_int_info;
		using invoke_int = rule_kit::rule_kit_int_t;
		using invoke_param = rule_kit::rule_kit_cmd_param<alloc_rule_design, rule_desg>;
		using invoke_act_param = typename invoke_ownership::owner_t::cmd_act_param;
		using invoke_act_res = typename invoke_ownership::owner_t::cmd_act_res;
		using invoke_op_parse_span_buf = std::span<typename invoke_ownership::op_cont>;
		using invoke_op_parse_span_buf_res = std::optional<std::reference_wrapper<const invoke_op_parse_span_buf>>;
		using invoke_res = rule_kit::rule_kit_cmd_res<alloc_rule_design>;
		using invoke_ip = rule_kit::rule_kit_cmd_ip<alloc_rule_design>;
		using invoke_cmd_sgl = rule_kit::rule_kit_cmd_sgl;
		using invoke_op_imm = rule_desg_op_imm;
		using invoke_op_var = rule_desg_op_var;
		using invoke_op_cmd = rule_desg_op_cmd;
		using invoke_op = rule_desg_op;
		using invoke_op_span = typename invoke_ownership::op_cont_span;
		using invoke_op_del = typename invoke_ownership::op_del;
		using invoke_op_span_buf = std::span<invoke_op_span>;
		using invoke_op_span_buf_res = std::optional<std::reference_wrapper<const std::span<invoke_op_span>>>;
		using invoke_raw_bytes = std::span<rule_desg_var_base>;
		using invoke_raw_bytes_buf = std::vector<rule_desg_var_base, alloc_raw_buf>;
		using error_info = rule_design_error_info<alloc_rule_design>;
		using text_alloc_info = rule_desg_text_alloc<alloc_rule_design>;
		using text_error = rule_desg_text_error<alloc_rule_design>;
		using text_parser = rule_desg_text_parser<alloc_rule_design>;
		using text_program = rule_desg_text_program<alloc_rule_design>;
		using text_var = rule_desg_text_var<alloc_rule_design>;
		using text_cmd = rule_desg_text_cmd<alloc_rule_design>;
	private:
		enum class auto_alloc_kind : std::uint8_t
		{
			var_set,
			cmd_set,
		};
		struct auto_alloc_key
		{
			auto_alloc_kind kind = auto_alloc_kind::var_set;
			std::uint64_t owner = 0;
			std::uint64_t index = 0;
		};
		using alloc_auto_alloc_key = typename alloc_traits::template rebind_alloc<auto_alloc_key>;
	private:
		alloc_rule_design allocator_;
		invoke_ownership kernel_rule_kit;
		rule_desg_cmd_mgr_index text_start_mgr = 0;
		rule_desg_cmd_index text_start_index = 0;
		invoke_raw_bytes_buf raw_buf;
		std::vector<auto_alloc_key, alloc_auto_alloc_key> auto_alloc_set;
		error_info error_state;
	private:
		static void call_back_int(invoke_outer& ownership_ref, invoke_error_run int_v, const invoke_error_info& int_info_v, invoke_ip int_ip_v, std::optional<invoke_ip>& ip_tgt_save, std::optional<invoke_cmd_sgl> int_cmd_sgl_v, bool& is_end_save)
		{
			is_end_save = false;
			return;
		}
		static void call_back_error_rc(invoke_outer& ownership_ref, kernel_error int_v, invoke_ip& int_ip_v, bool& is_end_save)
		{
			is_end_save = true;
			return;
		}
		static void call_back_error(invoke_outer& ownership_ref, invoke_error_act int_v, invoke_ip& int_ip_v, bool& is_end_save)
		{
			is_end_save = true;
			return;
		}
		static rule_kit::rule_kit_cmd_cb_hook_t call_back_hook(invoke_outer& ownership_ref, invoke_ip& tgt_ip_v, std::optional<invoke_cmd_sgl> int_cmd_sgl_v)
		{
			return rule_kit::rule_kit_cmd_cb_hook_t::pass;
		}
	private:
		template<typename elem_t>
		static consteval std::size_t raw_size_one() noexcept
		{
			if constexpr (std::is_same_v<elem_t, rule_kit::rule_kit_var_cmd_type>) return rule_kit::rule_kit_raw_bytes_size_var_cmd_type;
			else if constexpr (std::is_same_v<elem_t, rule_desg_op_var>) return rule_kit::rule_kit_raw_bytes_size_var_target;
			else if constexpr (std::is_same_v<elem_t, rule_desg_op_cmd>) return rule_kit::rule_kit_raw_bytes_size_cmd_target;
			else if constexpr (std::is_same_v<elem_t, rule_desg_op>) return rule_kit::rule_kit_raw_bytes_size_cmd_op;
			else return sizeof(elem_t);
		}
		template<typename elem_t>
		static bool op_span_to_raw(invoke_param& param, const std::span<elem_t>& op_span_v, invoke_raw_bytes raw_save)
		{
			if constexpr (std::is_same_v<elem_t, rule_desg_var_base>)
			{
				if (raw_save.size() < op_span_v.size())
					return false;

				std::copy(op_span_v.begin(), op_span_v.end(), raw_save.begin());
				return true;
			}
			else
			{
				constexpr std::size_t raw_size_one_v = raw_size_one<elem_t>();

				if (raw_save.size() < op_span_v.size() * raw_size_one_v)
					return false;

				for (std::size_t index_current = 0; index_current < op_span_v.size(); index_current++)
				{
					if (param.get_owner().trans_tgt_to_raw(param, op_span_v[index_current], raw_save.subspan(index_current * raw_size_one_v, raw_size_one_v)) == false)
						return false;
				}

				return true;
			}
		}
		static bool raw_span_need(const invoke_op_span& op_span_v, std::size_t& need_out) noexcept
		{
			need_out = 0;

			if (std::holds_alternative<invoke_raw_bytes>(op_span_v))
				return true;

			bool is_ok = true;

			std::visit
			(
				[&]<typename span_t>(const span_t& span_v)
				{
					if constexpr (std::is_same_v<span_t, std::monostate> || std::is_same_v<span_t, invoke_op_del>)
					{
						is_ok = false;
					}
					else
					{
						need_out = span_v.size() * raw_size_one<span_t>();
					}
				},
				op_span_v
			);

			return is_ok;
		}
		static bool raw_span_write(invoke_param& param, const invoke_op_span& op_span_v, invoke_raw_bytes raw_save)
		{
			bool is_ok = true;

			std::visit
			(
				[&]<typename span_t>(const span_t& span_v)
				{
					if constexpr (std::is_same_v<span_t, std::monostate> || std::is_same_v<span_t, invoke_op_del>)
					{
						is_ok = false;
					}
					else
					{
						if (op_span_to_raw(param, span_v, raw_save) == false)
							is_ok = false;
					}
				},
				op_span_v
			);

			return is_ok;
		}
		invoke_raw_bytes raw_buf_take(std::size_t size_v)
		{
			try
			{
				const std::size_t offset_v = raw_buf.size();

				raw_buf.resize(offset_v + size_v, rule_desg_var_base{ 0 });

				return invoke_raw_bytes(raw_buf.data() + offset_v, size_v);
			}
			catch (const rule_design_error_throw&)
			{
				throw;
			}
			catch (const std::bad_alloc&)
			{
				throw rule_design_error_throw(rule_design_error::internal__container, "rule_design | rule_desg | raw_buf_take | 临时缓冲区分配失败，错误不可恢复");
			}
			catch (...)
			{
				throw rule_design_error_throw(rule_design_error::internal__container, "rule_design | rule_desg | raw_buf_take | 临时缓冲区扩容异常，错误不可恢复");
			}
		}
		void raw_buf_reset(std::size_t need_v)
		{
			try
			{
				raw_buf.clear();
				raw_buf.reserve(need_v);
				return;
			}
			catch (const rule_design_error_throw&)
			{
				throw;
			}
			catch (const std::bad_alloc&)
			{
				throw rule_design_error_throw(rule_design_error::internal__container, "rule_design | rule_desg | raw_buf_reset | 临时缓冲区预留失败，错误不可恢复");
			}
			catch (...)
			{
				throw rule_design_error_throw(rule_design_error::internal__container, "rule_design | rule_desg | raw_buf_reset | 临时缓冲区重置异常，错误不可恢复");
			}
		}
		bool raw_span_view(invoke_param& param, const invoke_op_span& op_span_v, invoke_raw_bytes& raw_save)
		{
			if (const invoke_raw_bytes* raw_span = std::get_if<invoke_raw_bytes>(&op_span_v))
			{
				raw_save = *raw_span;
				return true;
			}

			std::size_t need_v = 0;

			if (raw_span_need(op_span_v, need_v) == false)
				return false;

			raw_save = raw_buf_take(need_v);
			return raw_span_write(param, op_span_v, raw_save);
		}
		static rule_desg_compare_type compare_raw(invoke_raw_bytes left_v, invoke_raw_bytes right_v) noexcept
		{
			if (left_v.size() != right_v.size())
				return rule_desg_compare_type::invalid;

			if (left_v.size() == 0)
				return rule_desg_compare_type::equal;

			const int cmp_res = std::memcmp(left_v.data(), right_v.data(), left_v.size());

			if (cmp_res < 0)
				return rule_desg_compare_type::less;
			if (cmp_res > 0)
				return rule_desg_compare_type::greater;

			return rule_desg_compare_type::equal;
		}
		template<typename value_t>
		static void set_compare_result(value_t left_v, value_t right_v, rule_desg_compare_type& res_save) noexcept
		{
			if (left_v < right_v)
			{
				res_save = rule_desg_compare_type::less;
				return;
			}

			if (left_v > right_v)
			{
				res_save = rule_desg_compare_type::greater;
				return;
			}

			if (left_v == right_v)
			{
				res_save = rule_desg_compare_type::equal;
				return;
			}

			res_save = rule_desg_compare_type::invalid;
		}
		static std::int64_t sign_extend_value(std::uint64_t value_v, std::size_t width_v) noexcept
		{
			if (width_v >= sizeof(std::uint64_t))
				return static_cast<std::int64_t>(value_v);

			const std::size_t shift_v = (sizeof(std::uint64_t) - width_v) * 8;
			const std::uint64_t value_shift_v = value_v << shift_v;

			return static_cast<std::int64_t>(value_shift_v) >> shift_v;
		}
		static bool compare_numeric(rule_desg_calc_type cmp_type_v, invoke_raw_bytes left_v, invoke_raw_bytes right_v, rule_desg_compare_type& res_save) noexcept
		{
			const std::size_t width_v = left_v.size() < right_v.size() ? left_v.size() : right_v.size();

			if (width_v == 0 || width_v > sizeof(std::uint64_t))
				return false;

			if (cmp_type_v == rule_desg_calc_type::floating_point)
			{
				if (width_v == sizeof(float))
				{
					float left_f = 0.0f;
					float right_f = 0.0f;

					std::memcpy(&left_f, left_v.data(), sizeof(float));
					std::memcpy(&right_f, right_v.data(), sizeof(float));

					set_compare_result(left_f, right_f, res_save);
					return true;
				}

				if (width_v == sizeof(double))
				{
					double left_d = 0.0;
					double right_d = 0.0;

					std::memcpy(&left_d, left_v.data(), sizeof(double));
					std::memcpy(&right_d, right_v.data(), sizeof(double));

					set_compare_result(left_d, right_d, res_save);
					return true;
				}

				return false;
			}

			std::uint64_t left_u = 0;
			std::uint64_t right_u = 0;

			std::memcpy(&left_u, left_v.data(), width_v);
			std::memcpy(&right_u, right_v.data(), width_v);

			if (cmp_type_v == rule_desg_calc_type::signed_integer)
			{
				set_compare_result(sign_extend_value(left_u, width_v), sign_extend_value(right_u, width_v), res_save);
				return true;
			}

			set_compare_result(left_u, right_u, res_save);
			return true;
		}
		bool compare_op_span(invoke_param& param, const invoke_op_span& left_v, const invoke_op_span& right_v, rule_desg_compare_type& res_save)
		{
			const bool is_left_raw = std::holds_alternative<invoke_raw_bytes>(left_v);
			const bool is_right_raw = std::holds_alternative<invoke_raw_bytes>(right_v);

			if (is_left_raw == true && is_right_raw == true)
			{
				res_save = compare_raw(std::get<invoke_raw_bytes>(left_v), std::get<invoke_raw_bytes>(right_v));
				return true;
			}

			if (is_left_raw == false && is_right_raw == false)
			{
				if (left_v.index() != right_v.index())
					return false;

				bool is_ok = true;
				res_save = rule_desg_compare_type::invalid;

				std::visit
				(
					[&]<typename span_t>(const span_t& left_span_v)
					{
						if constexpr (std::is_same_v<span_t, std::monostate> || std::is_same_v<span_t, invoke_op_del>)
						{
							is_ok = false;
						}
						else
						{
							const span_t& right_span_v = std::get<span_t>(right_v);

							if (left_span_v.size() != right_span_v.size())
							{
								res_save = rule_desg_compare_type::invalid;
								return;
							}

							const std::size_t need_v = left_span_v.size() * raw_size_one<span_t>();

							raw_buf_reset(need_v * 2);

							invoke_raw_bytes left_raw = raw_buf_take(need_v);
							invoke_raw_bytes right_raw = raw_buf_take(need_v);

							if (op_span_to_raw(param, left_span_v, left_raw) == false || op_span_to_raw(param, right_span_v, right_raw) == false)
							{
								is_ok = false;
								return;
							}

							res_save = std::memcmp(left_raw.data(), right_raw.data(), need_v) == 0 ? rule_desg_compare_type::equal : rule_desg_compare_type::invalid;
						}
					},
					left_v
				);

				return is_ok;
			}

			const invoke_op_span& typed_span_v = (is_left_raw == true) ? right_v : left_v;
			const invoke_raw_bytes raw_v = (is_left_raw == true) ? std::get<invoke_raw_bytes>(left_v) : std::get<invoke_raw_bytes>(right_v);
			std::size_t need_v = 0;

			if (raw_span_need(typed_span_v, need_v) == false)
				return false;

			raw_buf_reset(need_v);

			invoke_raw_bytes typed_raw;

			if (raw_span_view(param, typed_span_v, typed_raw) == false)
				return false;

			res_save = compare_raw(raw_v, typed_raw);
			return true;
		}
		static bool read_compare_type(const invoke_op_span& op_span_v, rule_desg_compare_type& res_save) noexcept
		{
			const invoke_raw_bytes* raw_span = std::get_if<invoke_raw_bytes>(&op_span_v);

			if (raw_span == nullptr || raw_span->size() < 1)
				return false;

			const std::uint8_t value_read = static_cast<std::uint8_t>((*raw_span)[0]);

			if (value_read > static_cast<std::uint8_t>(rule_desg_compare_type::invalid))
				return false;

			res_save = static_cast<rule_desg_compare_type>(value_read);
			return true;
		}
		static bool compare_condition_match(rule_desg_compare_type res_v, rule_desg_compare_type cond_v) noexcept
		{
			switch (cond_v)
			{
			case rule_desg_compare_type::unconditional:
			{
				return true;
			}
			case rule_desg_compare_type::invalid:
			{
				return false;
			}
			case rule_desg_compare_type::equal:
			{
				return res_v == rule_desg_compare_type::equal;
			}
			case rule_desg_compare_type::greater:
			{
				return res_v == rule_desg_compare_type::greater;
			}
			case rule_desg_compare_type::less:
			{
				return res_v == rule_desg_compare_type::less;
			}
			case rule_desg_compare_type::greater_equal:
			{
				return res_v == rule_desg_compare_type::greater || res_v == rule_desg_compare_type::equal;
			}
			case rule_desg_compare_type::less_equal:
			{
				return res_v == rule_desg_compare_type::less || res_v == rule_desg_compare_type::equal;
			}
			default: return false;
			}
		}
		static std::optional<invoke_ip> read_jump_target(invoke_param& param, const invoke_op_span& op_span_v)
		{
			if (const std::span<rule_desg_op_cmd>* cmd_tgt_span = std::get_if<std::span<rule_desg_op_cmd>>(&op_span_v))
			{
				if (cmd_tgt_span->size() < 1)
					return std::nullopt;

				const rule_desg_op_cmd& cmd_tgt = (*cmd_tgt_span)[0];

				return invoke_ip
				(
					static_cast<typename invoke_ip::command_set_index>(cmd_tgt.get_ownership()),
					static_cast<typename invoke_ip::command_index>(cmd_tgt.get_index())
				);
			}

			if (const invoke_raw_bytes* raw_span = std::get_if<invoke_raw_bytes>(&op_span_v))
			{
				rule_desg_op_cmd cmd_tgt_save;

				if (param.get_owner().trans_raw_to_tgt(param, *raw_span, cmd_tgt_save) == false)
					return std::nullopt;

				return invoke_ip
				(
					static_cast<typename invoke_ip::command_set_index>(cmd_tgt_save.get_ownership()),
					static_cast<typename invoke_ip::command_index>(cmd_tgt_save.get_index())
				);
			}

			return std::nullopt;
		}
	private:
		template<typename calc_t>
		static calc_t calc_value_op(rule_desg_cmd_def_type op_v, calc_t left_v, calc_t right_v, bool& is_error) noexcept
		{
			is_error = false;

			if constexpr (std::is_integral_v<calc_t>)
			{
				using unsigned_t = std::make_unsigned_t<calc_t>;

				const unsigned_t left_u = static_cast<unsigned_t>(left_v);
				const unsigned_t right_u = static_cast<unsigned_t>(right_v);

				switch (op_v)
				{
				case rule_desg_cmd_def_type::add:
					return static_cast<calc_t>(static_cast<unsigned_t>(left_u + right_u));
				case rule_desg_cmd_def_type::sub:
					return static_cast<calc_t>(static_cast<unsigned_t>(left_u - right_u));
				case rule_desg_cmd_def_type::mul:
					return static_cast<calc_t>(static_cast<unsigned_t>(left_u * right_u));
				case rule_desg_cmd_def_type::div:
				{
					if (right_u == unsigned_t(0))
					{
						is_error = true;
						return calc_t(0);
					}

					if constexpr (std::is_signed_v<calc_t>)
					{
						if (right_u == static_cast<unsigned_t>(-1))
							return static_cast<calc_t>(static_cast<unsigned_t>(unsigned_t(0) - left_u));
					}

					return static_cast<calc_t>(static_cast<calc_t>(left_u) / static_cast<calc_t>(right_u));
				}
				default:
				{
					is_error = true;
					return calc_t(0);
				}
				}
			}
			else
			{
				switch (op_v)
				{
				case rule_desg_cmd_def_type::add:
					return static_cast<calc_t>(left_v + right_v);
				case rule_desg_cmd_def_type::sub:
					return static_cast<calc_t>(left_v - right_v);
				case rule_desg_cmd_def_type::mul:
					return static_cast<calc_t>(left_v * right_v);
				case rule_desg_cmd_def_type::div:
				{
					if (right_v == calc_t(0))
					{
						is_error = true;
						return calc_t(0);
					}

					return static_cast<calc_t>(left_v / right_v);
				}
				default:
				{
					is_error = true;
					return calc_t(0);
				}
				}
			}
		}
		template<typename calc_t>
		static bool calc_dispatch(rule_desg_cmd_def_type op_v, invoke_raw_bytes left_v, invoke_raw_bytes right_v, invoke_raw_bytes dst_v, std::size_t width_v) noexcept
		{
			if (width_v == 0 || width_v > sizeof(calc_t))
				return false;

			if (left_v.size() < width_v || right_v.size() < width_v || dst_v.size() < width_v)
				return false;

			calc_t left_value = 0;
			calc_t right_value = 0;

			std::memcpy(&left_value, left_v.data(), width_v);
			std::memcpy(&right_value, right_v.data(), width_v);

			bool is_error = false;
			calc_t res_value = calc_value_op<calc_t>(op_v, left_value, right_value, is_error);

			if (is_error == true)
				return false;

			std::memcpy(dst_v.data(), &res_value, width_v);
			return true;
		}
		static bool calc_binary(rule_desg_calc_type calc_type_v, rule_desg_cmd_def_type op_v, invoke_raw_bytes left_v, invoke_raw_bytes right_v, invoke_raw_bytes dst_v, std::size_t width_v) noexcept
		{
			switch (calc_type_v)
			{
			case rule_desg_calc_type::unsigned_integer:
			{
				switch (width_v)
				{
				case 1:
				{
					return calc_dispatch<std::uint8_t>(op_v, left_v, right_v, dst_v, width_v);
				}
				case 2:
				{
					return calc_dispatch<std::uint16_t>(op_v, left_v, right_v, dst_v, width_v);
				}
				case 4:
				{
					return calc_dispatch<std::uint32_t>(op_v, left_v, right_v, dst_v, width_v);
				}
				case 8:
				{
					return calc_dispatch<std::uint64_t>(op_v, left_v, right_v, dst_v, width_v);
				}
				default: return false;
				}
			}
			case rule_desg_calc_type::signed_integer:
			{
				switch (width_v)
				{
				case 1:
				{
					return calc_dispatch<std::int8_t>(op_v, left_v, right_v, dst_v, width_v);
				}
				case 2:
				{
					return calc_dispatch<std::int16_t>(op_v, left_v, right_v, dst_v, width_v);
				}
				case 4:
				{
					return calc_dispatch<std::int32_t>(op_v, left_v, right_v, dst_v, width_v);
				}
				case 8:
				{
					return calc_dispatch<std::int64_t>(op_v, left_v, right_v, dst_v, width_v);
				}
				default: return false;
				}
			}
			case rule_desg_calc_type::floating_point:
			{
				switch (width_v)
				{
				case 4:
				{
					return calc_dispatch<float>(op_v, left_v, right_v, dst_v, width_v);
				}
				case 8:
				{
					return calc_dispatch<double>(op_v, left_v, right_v, dst_v, width_v);
				}
				default: return false;
				}
			}
			default:
				return false;
			}
		}
	private:
		invoke_res cmd_mov(invoke_param& param)
		{
			invoke_ownership& ownership_ref = param.get_owner();

			if (ownership_ref.check_cmd(param, static_cast<rule_kit::rule_kit_cmd_type>(rule_desg_cmd_def_type::mov), 2) == false)
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			const std::span<invoke_op>& cmd_op_span = param.get_cmd_info().get_cmd_sgl().get_content_span();

			for (const invoke_op& op_v : cmd_op_span)
			{
				if (this->ensure_alloc(param, op_v) == false)
					return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");
			}

			const invoke_op_var* dst_var = std::get_if<invoke_op_var>(&cmd_op_span[0]);
			const invoke_op_cmd* dst_cmd = std::get_if<invoke_op_cmd>(&cmd_op_span[0]);

			if (dst_var == nullptr && dst_cmd == nullptr)
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			invoke_op_parse_span_buf_res op_parse_res = ownership_ref.get_op_parse(param, { 0, 1 }, false, false);

			if (op_parse_res.has_value() == false)
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			const invoke_op_parse_span_buf& op_parse_ref = op_parse_res.value().get();

			if (ownership_ref.write_op_cont_span(param, op_parse_ref[0], op_parse_ref[1]) == false)
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			return invoke_res(invoke_int::next, std::nullopt);
		}
		invoke_res cmd_cmp(invoke_param& param)
		{
			invoke_ownership& ownership_ref = param.get_owner();

			if (ownership_ref.check_cmd(param, static_cast<rule_kit::rule_kit_cmd_type>(rule_desg_cmd_def_type::cmp), 3) == false)
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			const std::span<invoke_op>& cmd_op_span = param.get_cmd_info().get_cmd_sgl().get_content_span();

			for (const invoke_op& op_v : cmd_op_span)
			{
				if (this->ensure_alloc(param, op_v) == false)
					return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");
			}

			const invoke_op_var* res_op = std::get_if<invoke_op_var>(&cmd_op_span[0]);

			if (res_op == nullptr ||
				(res_op->get_handle_type() != rule_desg_var_op_type::var_val && res_op->get_handle_type() != rule_desg_var_op_type::var_ptr))
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			invoke_op_parse_span_buf_res op_parse_res = ownership_ref.get_op_parse(param, { 0, 1, 2 }, false, false);

			if (op_parse_res.has_value() == false)
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			invoke_op_span_buf_res op_span_res = ownership_ref.get_op_cont_span(op_parse_res.value().get());

			if (op_span_res.has_value() == false)
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			const invoke_op_span_buf& op_span_buf = op_span_res.value().get();
			const invoke_raw_bytes* res_span = std::get_if<invoke_raw_bytes>(&op_span_buf[0]);

			if (res_span == nullptr || res_span->size() < rule_kit::rule_kit_raw_bytes_size_uint_8)
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			const std::uint8_t cmp_type_read = static_cast<std::uint8_t>(res_op->get_type());

			if (cmp_type_read > static_cast<std::uint8_t>(rule_desg_calc_type::floating_point))
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			const rule_desg_calc_type cmp_type_t = static_cast<rule_desg_calc_type>(cmp_type_read);
			rule_desg_compare_type cmp_res = rule_desg_compare_type::invalid;

			std::size_t need_left = 0;
			std::size_t need_right = 0;

			if (raw_span_need(op_span_buf[1], need_left) == false || raw_span_need(op_span_buf[2], need_right) == false)
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			raw_buf_reset(need_left + need_right);

			invoke_raw_bytes left_raw;
			invoke_raw_bytes right_raw;

			if (raw_span_view(param, op_span_buf[1], left_raw) == false || raw_span_view(param, op_span_buf[2], right_raw) == false)
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			if (left_raw.size() > sizeof(std::uint64_t) || right_raw.size() > sizeof(std::uint64_t))
			{
				if (compare_op_span(param, op_span_buf[1], op_span_buf[2], cmp_res) == false)
					return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");
			}
			else if (compare_numeric(cmp_type_t, left_raw, right_raw, cmp_res) == false)
			{
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");
			}

			(*res_span)[0] = static_cast<rule_desg_var_base>(static_cast<std::uint8_t>(cmp_res));

			return invoke_res(invoke_int::next, std::nullopt);
		}
		invoke_res cmd_jmp(invoke_param& param)
		{
			invoke_ownership& ownership_ref = param.get_owner();

			if (ownership_ref.check_cmd(param, static_cast<rule_kit::rule_kit_cmd_type>(rule_desg_cmd_def_type::jmp), 3) == false)
				return this->fail_invoke(rule_design_error::exec__jump_target, "rule_desg 指令执行失败");

			const std::span<invoke_op>& cmd_op_span = param.get_cmd_info().get_cmd_sgl().get_content_span();

			if (const invoke_op_cmd* target_v = std::get_if<invoke_op_cmd>(&cmd_op_span[0]))
			{
				if (this->ensure_jump_target(param, *target_v, static_cast<rule_kit::rule_kit_cmd_type>(rule_desg_cmd_def_type::ret)) == false)
					return this->fail_invoke(rule_design_error::exec__jump_target, "rule_desg 指令执行失败");
			}

			for (std::size_t i = 1; i < 3; i++)
			{
				if (this->ensure_alloc(param, cmd_op_span[i]) == false)
					return this->fail_invoke(rule_design_error::exec__jump_target, "rule_desg 指令执行失败");
			}

			invoke_op_parse_span_buf_res op_parse_res = ownership_ref.get_op_parse(param, { 0, 1, 2 }, false, false);

			if (op_parse_res.has_value() == false)
				return this->fail_invoke(rule_design_error::exec__jump_target, "rule_desg 指令执行失败");

			invoke_op_span_buf_res op_span_res = ownership_ref.get_op_cont_span(op_parse_res.value().get());

			if (op_span_res.has_value() == false)
				return this->fail_invoke(rule_design_error::exec__jump_target, "rule_desg 指令执行失败");

			const invoke_op_span_buf& op_span_buf = op_span_res.value().get();

			std::optional<invoke_ip> ip_tgt = read_jump_target(param, op_span_buf[0]);

			if (ip_tgt.has_value() == false)
				return this->fail_invoke(rule_design_error::exec__jump_target, "rule_desg 指令执行失败");

			rule_desg_compare_type cond_t = rule_desg_compare_type::invalid;
			rule_desg_compare_type cmp_res_t = rule_desg_compare_type::invalid;

			if (read_compare_type(op_span_buf[1], cond_t) == false)
				return this->fail_invoke(rule_design_error::exec__jump_target, "rule_desg 指令执行失败");

			if (read_compare_type(op_span_buf[2], cmp_res_t) == false)
				return this->fail_invoke(rule_design_error::exec__jump_target, "rule_desg 指令执行失败");

			if (cond_t == rule_desg_compare_type::invalid)
				return this->fail_invoke(rule_design_error::exec__jump_target, "rule_desg 指令执行失败");

			if (compare_condition_match(cmp_res_t, cond_t) == true)
				return invoke_res(invoke_int::next, ip_tgt);

			return invoke_res(invoke_int::next, std::nullopt);
		}
		invoke_res cmd_call(invoke_param& param)
		{
			invoke_ownership& ownership_ref = param.get_owner();

			if (ownership_ref.check_cmd(param, static_cast<rule_kit::rule_kit_cmd_type>(rule_desg_cmd_def_type::call), 1) == false)
				return this->fail_invoke(rule_design_error::exec__jump_target, "rule_desg 指令执行失败");

			const std::span<invoke_op>& cmd_op_span = param.get_cmd_info().get_cmd_sgl().get_content_span();

			if (const invoke_op_cmd* target_v = std::get_if<invoke_op_cmd>(&cmd_op_span[0]))
			{
				if (this->ensure_jump_target(param, *target_v, static_cast<rule_kit::rule_kit_cmd_type>(rule_desg_cmd_def_type::ret)) == false)
					return this->fail_invoke(rule_design_error::exec__jump_target, "rule_desg 指令执行失败");
			}

			invoke_op_parse_span_buf_res op_parse_res = ownership_ref.get_op_parse(param, { 0 }, false, false);

			if (op_parse_res.has_value() == false)
				return this->fail_invoke(rule_design_error::exec__jump_target, "rule_desg 指令执行失败");

			invoke_op_span_buf_res op_span_res = ownership_ref.get_op_cont_span(op_parse_res.value().get());

			if (op_span_res.has_value() == false)
				return this->fail_invoke(rule_design_error::exec__jump_target, "rule_desg 指令执行失败");

			std::optional<invoke_ip> ip_tgt = read_jump_target(param, op_span_res.value().get()[0]);

			if (ip_tgt.has_value() == false)
				return this->fail_invoke(rule_design_error::exec__jump_target, "rule_desg 指令执行失败");

			return invoke_res(invoke_int::invoke, ip_tgt);
		}
		invoke_res cmd_ret(invoke_param& param)
		{
			invoke_ownership& ownership_ref = param.get_owner();

			if (ownership_ref.check_cmd(param, static_cast<rule_kit::rule_kit_cmd_type>(rule_desg_cmd_def_type::ret), 0) == false)
				return this->fail_invoke(rule_design_error::exec__check_cmd, "rule_desg 指令执行失败");

			return invoke_res(invoke_int::invoke_end, std::nullopt);
		}
		invoke_res cmd_calc(invoke_param& param, rule_desg_cmd_def_type op_v)
		{
			invoke_ownership& ownership_ref = param.get_owner();

			if (ownership_ref.check_cmd(param, static_cast<rule_kit::rule_kit_cmd_type>(op_v), 3) == false)
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			const std::span<invoke_op>& cmd_op_span = param.get_cmd_info().get_cmd_sgl().get_content_span();

			for (const invoke_op& op_v : cmd_op_span)
			{
				if (this->ensure_alloc(param, op_v) == false)
					return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");
			}

			const invoke_op_var* dst_op = std::get_if<invoke_op_var>(&cmd_op_span[0]);

			if (dst_op == nullptr ||
				(dst_op->get_handle_type() != rule_desg_var_op_type::var_val && dst_op->get_handle_type() != rule_desg_var_op_type::var_ptr))
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			const std::uint8_t calc_t_read = static_cast<std::uint8_t>(dst_op->get_type());

			if (calc_t_read > static_cast<std::uint8_t>(rule_desg_calc_type::floating_point))
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			const rule_desg_calc_type calc_type_t = static_cast<rule_desg_calc_type>(calc_t_read);

			invoke_op_parse_span_buf_res dst_parse_res = ownership_ref.get_op_parse(param, { 0 }, false, false);

			if (dst_parse_res.has_value() == false)
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			invoke_op_span_buf_res dst_span_res = ownership_ref.get_op_cont_span(dst_parse_res.value().get());

			if (dst_span_res.has_value() == false)
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			const invoke_raw_bytes* dst_span = std::get_if<invoke_raw_bytes>(&dst_span_res.value().get()[0]);

			if (dst_span == nullptr || dst_span->size() == 0)
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			const invoke_raw_bytes dst_span_save = *dst_span;

			invoke_op_parse_span_buf_res src_parse_res = ownership_ref.get_op_parse(param, { 1, 2 }, false, false);

			if (src_parse_res.has_value() == false)
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			invoke_op_span_buf_res src_span_res = ownership_ref.get_op_cont_span(src_parse_res.value().get());

			if (src_span_res.has_value() == false)
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			const invoke_op_span_buf& src_span_buf = src_span_res.value().get();
			std::size_t need_left = 0;
			std::size_t need_right = 0;

			if (raw_span_need(src_span_buf[0], need_left) == false || raw_span_need(src_span_buf[1], need_right) == false)
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			raw_buf_reset(need_left + need_right);

			invoke_raw_bytes left_raw;
			invoke_raw_bytes right_raw;

			if (raw_span_view(param, src_span_buf[0], left_raw) == false ||
				raw_span_view(param, src_span_buf[1], right_raw) == false)
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			const std::size_t width_calc = left_raw.size() < right_raw.size() ? left_raw.size() : right_raw.size();

			if (width_calc == 0 || dst_span_save.size() < width_calc)
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			if (calc_binary(calc_type_t, op_v, left_raw, right_raw, dst_span_save, width_calc) == false)
				return this->fail_invoke(rule_design_error::exec__write_op, "rule_desg 指令执行失败");

			return invoke_res(invoke_int::next, std::nullopt);
		}
		invoke_res cmd_add(invoke_param& param)
		{
			return cmd_calc(param, rule_desg_cmd_def_type::add);
		}
		invoke_res cmd_sub(invoke_param& param)
		{
			return cmd_calc(param, rule_desg_cmd_def_type::sub);
		}
		invoke_res cmd_mul(invoke_param& param)
		{
			return cmd_calc(param, rule_desg_cmd_def_type::mul);
		}
		invoke_res cmd_div(invoke_param& param)
		{
			return cmd_calc(param, rule_desg_cmd_def_type::div);
		}
		invoke_res cmd_alloc(invoke_param& param)
		{
			invoke_ownership& ownership_ref = param.get_owner();

			if (ownership_ref.check_cmd(param, static_cast<rule_kit::rule_kit_cmd_type>(rule_desg_cmd_def_type::alloc), 1) == false)
				return this->fail_invoke(rule_design_error::exec__alloc, "rule_desg 指令执行失败");

			const std::span<invoke_op>& cmd_op_span = param.get_cmd_info().get_cmd_sgl().get_content_span();
			const invoke_op_var* var_tgt = std::get_if<invoke_op_var>(&cmd_op_span[0]);
			const invoke_op_cmd* cmd_tgt = std::get_if<invoke_op_cmd>(&cmd_op_span[0]);

			if (var_tgt != nullptr)
			{
				const rule_desg_var_op_type handle_type_v = var_tgt->get_handle_type();

				if (handle_type_v != rule_desg_var_op_type::var_val && handle_type_v != rule_desg_var_op_type::var_ptr)
					return this->fail_invoke(rule_design_error::exec__alloc, "rule_desg 指令执行失败");

				if (ownership_ref.alloc(param, *var_tgt, true, true) == false)
					return this->fail_invoke(rule_design_error::exec__alloc, "rule_desg 指令执行失败");

				return invoke_res(invoke_int::next, std::nullopt);
			}

			if (cmd_tgt != nullptr)
			{
				const rule_desg_cmd_op_type handle_type_v = cmd_tgt->get_handle_type();

				if (handle_type_v != rule_desg_cmd_op_type::cmd_val && handle_type_v != rule_desg_cmd_op_type::cmd_ptr)
					return this->fail_invoke(rule_design_error::exec__alloc, "rule_desg 指令执行失败");

				if (ownership_ref.alloc(param, *cmd_tgt, true, true) == false)
					return this->fail_invoke(rule_design_error::exec__alloc, "rule_desg 指令执行失败");

				return invoke_res(invoke_int::next, std::nullopt);
			}

			return this->fail_invoke(rule_design_error::exec__alloc, "rule_desg 指令执行失败");
		}
		invoke_res cmd_dealloc(invoke_param& param)
		{
			invoke_ownership& ownership_ref = param.get_owner();

			if (ownership_ref.check_cmd(param, static_cast<rule_kit::rule_kit_cmd_type>(rule_desg_cmd_def_type::dealloc), 1) == false)
				return this->fail_invoke(rule_design_error::exec__dealloc, "rule_desg 指令执行失败");

			const std::span<invoke_op>& cmd_op_span = param.get_cmd_info().get_cmd_sgl().get_content_span();
			const invoke_op_var* var_tgt = std::get_if<invoke_op_var>(&cmd_op_span[0]);
			const invoke_op_cmd* cmd_tgt = std::get_if<invoke_op_cmd>(&cmd_op_span[0]);

			if (var_tgt != nullptr)
			{
				const rule_desg_var_op_type handle_type_v = var_tgt->get_handle_type();

				if (handle_type_v != rule_desg_var_op_type::var_val && handle_type_v != rule_desg_var_op_type::var_ptr)
					return this->fail_invoke(rule_design_error::exec__dealloc, "rule_desg 指令执行失败");

				if (ownership_ref.dealloc(param, *var_tgt) == false)
					return this->fail_invoke(rule_design_error::exec__dealloc, "rule_desg 指令执行失败");

				this->remove_auto_alloc(auto_alloc_kind::var_set, var_tgt->get_ownership(), var_tgt->get_index());
				return invoke_res(invoke_int::next, std::nullopt);
			}

			if (cmd_tgt != nullptr)
			{
				const rule_desg_cmd_op_type handle_type_v = cmd_tgt->get_handle_type();

				if (handle_type_v != rule_desg_cmd_op_type::cmd_val && handle_type_v != rule_desg_cmd_op_type::cmd_tgt && handle_type_v != rule_desg_cmd_op_type::cmd_ptr)
					return this->fail_invoke(rule_design_error::exec__dealloc, "rule_desg 指令执行失败");

				if (ownership_ref.dealloc(param, *cmd_tgt) == false)
					return this->fail_invoke(rule_design_error::exec__dealloc, "rule_desg 指令执行失败");

				if (cmd_tgt->get_handle_type() == rule_desg_cmd_op_type::cmd_ptr)
					this->remove_auto_alloc(auto_alloc_kind::var_set, cmd_tgt->get_ownership(), cmd_tgt->get_index());
				else
					this->remove_auto_alloc(auto_alloc_kind::cmd_set, cmd_tgt->get_ownership(), cmd_tgt->get_index());

				return invoke_res(invoke_int::next, std::nullopt);
			}

			return this->fail_invoke(rule_design_error::exec__dealloc, "rule_desg 指令执行失败");
		}
		template<auto member_fn>
		static invoke_res cmd_entry(invoke_param& param)
		{
			auto& outer = *param.get_owner().get_owner().get_actuator_().get_data_outer();
			auto& self = outer.get_user();
			return (self.*member_fn)(param);
		}
		template<auto member_fn>
		static invoke_act_res cmd_act_entry(invoke_act_param& param)
		{
			rule_kit::rule_kit_cmd_proxy_call<alloc_rule_design, rule_desg> proxy(&rule_desg::cmd_entry<member_fn>);
			return proxy(param);
		}
		void register_cmd_def()
		{
			this->kernel_rule_kit.add_set_cmd_def(static_cast<rule_kit::rule_kit_cmd_type>(rule_desg_cmd_def_type::mov), &rule_desg::cmd_act_entry<&rule_desg::cmd_mov>);
			this->kernel_rule_kit.add_set_cmd_def(static_cast<rule_kit::rule_kit_cmd_type>(rule_desg_cmd_def_type::cmp), &rule_desg::cmd_act_entry<&rule_desg::cmd_cmp>);
			this->kernel_rule_kit.add_set_cmd_def(static_cast<rule_kit::rule_kit_cmd_type>(rule_desg_cmd_def_type::jmp), &rule_desg::cmd_act_entry<&rule_desg::cmd_jmp>);
			this->kernel_rule_kit.add_set_cmd_def(static_cast<rule_kit::rule_kit_cmd_type>(rule_desg_cmd_def_type::call), &rule_desg::cmd_act_entry<&rule_desg::cmd_call>);
			this->kernel_rule_kit.add_set_cmd_def(static_cast<rule_kit::rule_kit_cmd_type>(rule_desg_cmd_def_type::ret), &rule_desg::cmd_act_entry<&rule_desg::cmd_ret>);
			this->kernel_rule_kit.add_set_cmd_def(static_cast<rule_kit::rule_kit_cmd_type>(rule_desg_cmd_def_type::add), &rule_desg::cmd_act_entry<&rule_desg::cmd_add>);
			this->kernel_rule_kit.add_set_cmd_def(static_cast<rule_kit::rule_kit_cmd_type>(rule_desg_cmd_def_type::sub), &rule_desg::cmd_act_entry<&rule_desg::cmd_sub>);
			this->kernel_rule_kit.add_set_cmd_def(static_cast<rule_kit::rule_kit_cmd_type>(rule_desg_cmd_def_type::mul), &rule_desg::cmd_act_entry<&rule_desg::cmd_mul>);
			this->kernel_rule_kit.add_set_cmd_def(static_cast<rule_kit::rule_kit_cmd_type>(rule_desg_cmd_def_type::div), &rule_desg::cmd_act_entry<&rule_desg::cmd_div>);
			this->kernel_rule_kit.add_set_cmd_def(static_cast<rule_kit::rule_kit_cmd_type>(rule_desg_cmd_def_type::alloc), &rule_desg::cmd_act_entry<&rule_desg::cmd_alloc>);
			this->kernel_rule_kit.add_set_cmd_def(static_cast<rule_kit::rule_kit_cmd_type>(rule_desg_cmd_def_type::dealloc), &rule_desg::cmd_act_entry<&rule_desg::cmd_dealloc>);
			return;
		}
	private:
		void track_auto_alloc(auto_alloc_kind kind_v, std::uint64_t owner_v, std::uint64_t index_v)
		{
			try
			{
				this->auto_alloc_set.push_back(auto_alloc_key{ kind_v, owner_v, index_v });
				return;
			}
			catch (const rule_design_error_throw&)
			{
				throw;
			}
			catch (const std::bad_alloc&)
			{
				throw rule_design_error_throw(rule_design_error::internal__container, "rule_design | rule_desg | track_auto_alloc | 自动分配账本扩容失败，错误不可恢复");
			}
			catch (...)
			{
				throw rule_design_error_throw(rule_design_error::internal__container, "rule_design | rule_desg | track_auto_alloc | 自动分配账本写入异常，错误不可恢复");
			}
		}
		void remove_auto_alloc(auto_alloc_kind kind_v, std::uint64_t owner_v, std::uint64_t index_v)
		{
			for (auto it = this->auto_alloc_set.begin(); it != this->auto_alloc_set.end();)
			{
				if (it->kind == kind_v && it->owner == owner_v && it->index == index_v)
					it = this->auto_alloc_set.erase(it);
				else
					++it;
			}

			return;
		}
		bool test_var_exist(std::uint64_t owner_v, std::uint64_t index_v) const
		{
			auto var_mgr_res = this->kernel_rule_kit.get_owner().get_var_set().get(static_cast<rule_kit::rule_kit_var_mgr_index>(owner_v));

			if (var_mgr_res.has_value() == false)
				return false;

			return var_mgr_res.value().get().get_content().get(static_cast<rule_kit::rule_kit_var_index>(index_v)).has_value();
		}
		bool test_cmd_exist(std::uint64_t owner_v, std::uint64_t index_v) const
		{
			auto cmd_mgr_res = this->kernel_rule_kit.get_owner().get_cmd_set().get(static_cast<rule_kit::rule_kit_cmd_mgr_index>(owner_v));

			if (cmd_mgr_res.has_value() == false)
				return false;

			return cmd_mgr_res.value().get().get_content().get(static_cast<rule_kit::rule_kit_cmd_index>(index_v)).has_value();
		}
		bool ensure_alloc(invoke_param& param, const rule_kit::rule_kit_var_tgt& target_v)
		{
			if (target_v.get_handle_type() != rule_kit::rule_kit_var_handle_type::var_val && target_v.get_handle_type() != rule_kit::rule_kit_var_handle_type::var_ptr)
				return true;

			if (this->test_var_exist(target_v.get_ownership(), target_v.get_index()) == true)
				return true;

			if (this->kernel_rule_kit.alloc(param, target_v, true, true) == false)
				return false;

			this->track_auto_alloc(auto_alloc_kind::var_set, target_v.get_ownership(), target_v.get_index());
			return true;
		}
		bool ensure_alloc(invoke_param& param, const rule_kit::rule_kit_cmd_tgt& target_v)
		{
			const rule_kit::rule_kit_cmd_handle_type handle_v = target_v.get_handle_type();

			if (handle_v == rule_kit::rule_kit_cmd_handle_type::cmd_val)
			{
				if (this->test_cmd_exist(target_v.get_ownership(), target_v.get_index()) == true)
					return true;

				if (this->kernel_rule_kit.alloc(param, target_v, true, true) == false)
					return false;

				this->track_auto_alloc(auto_alloc_kind::cmd_set, target_v.get_ownership(), target_v.get_index());
				return true;
			}

			if (handle_v == rule_kit::rule_kit_cmd_handle_type::cmd_ptr)
			{
				if (this->test_var_exist(target_v.get_ownership(), target_v.get_index()) == true)
					return true;

				if (this->kernel_rule_kit.alloc(param, target_v, true, true) == false)
					return false;

				this->track_auto_alloc(auto_alloc_kind::var_set, target_v.get_ownership(), target_v.get_index());
				return true;
			}

			return true;
		}
		bool ensure_alloc(invoke_param& param, const invoke_op& op_v)
		{
			if (const auto* var_v = std::get_if<rule_kit::rule_kit_var_tgt>(&op_v))
				return this->ensure_alloc(param, *var_v);

			if (const auto* cmd_v = std::get_if<rule_kit::rule_kit_cmd_tgt>(&op_v))
				return this->ensure_alloc(param, *cmd_v);

			return true;
		}
		bool ensure_jump_target(invoke_param& param, const rule_kit::rule_kit_cmd_tgt& target_v, rule_kit::rule_kit_cmd_type create_type_v)
		{
			if (this->test_cmd_exist(target_v.get_ownership(), target_v.get_index()) == true)
				return true;

			const rule_kit::rule_kit_cmd_tgt create_target_v
			(
				rule_kit::rule_kit_cmd_handle_type::cmd_val,
				static_cast<rule_kit::rule_kit_var_cmd_mgr_index>(target_v.get_ownership()),
				static_cast<rule_kit::rule_kit_var_cmd_index>(target_v.get_index()),
				static_cast<rule_kit::rule_kit_var_cmd_type>(create_type_v),
				0,
				0
			);

			if (this->kernel_rule_kit.alloc(param, create_target_v, true, true) == false)
				return false;

			this->track_auto_alloc(auto_alloc_kind::cmd_set, target_v.get_ownership(), target_v.get_index());
			return true;
		}
		void release_auto_alloc_impl()
		{
			for (const auto_alloc_key& key_v : this->auto_alloc_set)
			{
				if (key_v.kind == auto_alloc_kind::var_set)
				{
					auto var_mgr_res = this->kernel_rule_kit.get_owner().get_var_set().get(static_cast<rule_kit::rule_kit_var_mgr_index>(key_v.owner));

					if (var_mgr_res.has_value() == true)
					{
						if (var_mgr_res.value().get().get_content().del(static_cast<rule_kit::rule_kit_var_index>(key_v.index)).has_value() == false)
							throw rule_design_error_throw(rule_design_error::exec__internal, "rule_desg | release_auto_alloc | 自动变量删除失败，无法回滚");
					}

					if (this->kernel_rule_kit.get_owner().get_var_set().destroy(static_cast<rule_kit::rule_kit_var_mgr_index>(key_v.owner)).has_value() == false)
						throw rule_design_error_throw(rule_design_error::exec__internal, "rule_desg | release_auto_alloc | 自动变量 manager 销毁失败，无法回滚");
				}
				else
				{
					auto cmd_mgr_res = this->kernel_rule_kit.get_owner().get_cmd_set().get(static_cast<rule_kit::rule_kit_cmd_mgr_index>(key_v.owner));

					if (cmd_mgr_res.has_value() == true)
					{
						if (cmd_mgr_res.value().get().get_content().del(static_cast<rule_kit::rule_kit_cmd_index>(key_v.index)).has_value() == false)
							throw rule_design_error_throw(rule_design_error::exec__internal, "rule_desg | release_auto_alloc | 自动命令删除失败，无法回滚");
					}

					if (this->kernel_rule_kit.get_owner().get_cmd_set().destroy(static_cast<rule_kit::rule_kit_cmd_mgr_index>(key_v.owner)).has_value() == false)
						throw rule_design_error_throw(rule_design_error::exec__internal, "rule_desg | release_auto_alloc | 自动命令 manager 销毁失败，无法回滚");
				}
			}

			this->auto_alloc_set.clear();
			return;
		}
		invoke_res fail_invoke(rule_design_error code_v, std::string_view message_v)
		{
			this->set_error_safe(code_v, message_v);
			return invoke_res(invoke_int::error, std::nullopt);
		}
		bool fail_bool(rule_design_error code_v, std::string_view message_v)
		{
			this->set_error_safe(code_v, message_v);
			return false;
		}
		void set_error_safe(rule_design_error code_v, std::string_view message_v) noexcept
		{
			this->error_state.code = code_v;

			try
			{
				this->error_state.message.assign(message_v.begin(), message_v.end());
			}
			catch (...)
			{
				this->error_state.message.clear();
			}

			return;
		}
		void clear_error()
		{
			this->error_state.clear();
			return;
		}
		void sync_text_error(const text_error& error_v)
		{
			if (error_v.code != rule_design_error::none)
				this->set_error_safe(error_v.code, std::string_view(error_v.message.data(), error_v.message.size()));
			return;
		}
	public:
		const error_info& get_error() const noexcept
		{
			return this->error_state;
		}
		bool has_error() const noexcept
		{
			return this->error_state.has_error();
		}
		void throw_error(rule_design_error code_v, const std::string& message_v) const
		{
			throw rule_design_error_throw(code_v, message_v);
		}
	private:
	public:
		explicit rule_desg(const alloc_rule_design& alloc_val) :
			allocator_(alloc_val),
			kernel_rule_kit(call_back_int, call_back_error_rc, call_back_error, call_back_hook, *this, alloc_val),
			raw_buf(alloc_raw_buf(alloc_val)),
			auto_alloc_set(alloc_auto_alloc_key(alloc_val)),
			error_state(alloc_val)
		{
			this->register_cmd_def();
		}
		rule_desg() :
			rule_desg(alloc_rule_design())
		{

		}
		rule_desg(const rule_desg&) = delete;
		rule_desg& operator=(const rule_desg&) = delete;
		rule_desg(rule_desg&&) = delete;
		rule_desg& operator=(rule_desg&&) = delete;
	public:
		bool add_set_cmd(rule_desg_cmd_mgr_index mgr_index_v, rule_desg_cmd_index cmd_index_v, rule_desg_cmd_def_type cmd_type_v, rule_desg_cmd_op_index cmd_op_begin_v, const std::initializer_list<rule_desg_op>& cmd_op_set_v)
		{
			try
			{
				if (this->kernel_rule_kit.add_set_cmd_set(mgr_index_v, cmd_index_v, static_cast<rule_kit::rule_kit_cmd_type>(cmd_type_v), cmd_op_begin_v, cmd_op_set_v) == false)
					return this->fail_bool(rule_design_error::exec__internal, "rule_desg 指令写入失败");

				return true;
			}
			catch (const rule_design_error_throw&)
			{
				throw;
			}
			catch (const kernel_error_throw& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__kernel, std::string("rule_design | add_set_cmd | 内核异常: ") + error_v.error_msg);
			}
			catch (const std::bad_alloc&)
			{
				throw rule_design_error_throw(rule_design_error::internal__container, "rule_design | add_set_cmd | 指令容器分配失败，错误不可恢复");
			}
			catch (const std::exception& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__exception, std::string("rule_design | add_set_cmd | 异常: ") + error_v.what());
			}
			catch (...)
			{
				throw rule_design_error_throw(rule_design_error::internal__exception, "rule_design | add_set_cmd | 未知异常，错误不可恢复");
			}
		}
		bool add_set_var(rule_desg_var_mgr_index mgr_index_v, rule_desg_var_index var_index_v, rule_desg_var_type var_type_v, rule_desg_var_pos_index var_pos_begin_v, const std::span<rule_desg_var_base>& var_content_v)
		{
			try
			{
				if (this->kernel_rule_kit.add_set_var_set(mgr_index_v, var_index_v, var_type_v, var_pos_begin_v, var_content_v) == false)
					return this->fail_bool(rule_design_error::exec__internal, "rule_desg 变量写入失败");

				return true;
			}
			catch (const rule_design_error_throw&)
			{
				throw;
			}
			catch (const kernel_error_throw& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__kernel, std::string("rule_design | add_set_var | 内核异常: ") + error_v.error_msg);
			}
			catch (const std::bad_alloc&)
			{
				throw rule_design_error_throw(rule_design_error::internal__container, "rule_design | add_set_var | 变量容器分配失败，错误不可恢复");
			}
			catch (const std::exception& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__exception, std::string("rule_design | add_set_var | 异常: ") + error_v.what());
			}
			catch (...)
			{
				throw rule_design_error_throw(rule_design_error::internal__exception, "rule_design | add_set_var | 未知异常，错误不可恢复");
			}
		}
		void run(rule_desg_cmd_mgr_index mgr_index_v, rule_desg_cmd_index cmd_index_v)
		{
			this->clear_error();

			try
			{
				this->kernel_rule_kit.run(mgr_index_v, cmd_index_v);
				return;
			}
			catch (const rule_design_error_throw&)
			{
				throw;
			}
			catch (const kernel_error_throw& error_v)
			{
				this->set_error_safe(rule_design_error::internal__kernel, error_v.error_msg);
				throw rule_design_error_throw(rule_design_error::internal__kernel, std::string("rule_design | rule_desg | run | 内核异常: ") + error_v.error_msg);
			}
			catch (const std::bad_alloc&)
			{
				throw rule_design_error_throw(rule_design_error::internal__container, "rule_design | rule_desg | run | 执行期内存分配失败，错误不可恢复");
			}
			catch (const std::exception& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__exception, std::string("rule_design | rule_desg | run | 执行异常: ") + error_v.what());
			}
			catch (...)
			{
				throw rule_design_error_throw(rule_design_error::internal__exception, "rule_design | rule_desg | run | 未知执行异常，错误不可恢复");
			}
		}
	public:
		bool save(rule_desg_cmd_mgr_index mgr_index_v, rule_desg_cmd_index cmd_index_v, std::ostream& out_v)
		{
			this->clear_error();

			try
			{
				const invoke_ip ip_v
				(
					static_cast<typename invoke_ip::command_set_index>(mgr_index_v),
					static_cast<typename invoke_ip::command_index>(cmd_index_v)
				);

				if (rule_serialize::serialize_rule(this->kernel_rule_kit.get_owner(), ip_v, out_v).has_value() == false)
					return this->fail_bool(rule_design_error::exec__internal, "rule_desg 序列化保存失败");

				return true;
			}
			catch (const rule_design_error_throw&)
			{
				throw;
			}
			catch (const kernel_error_throw& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__serialize, std::string("rule_design | save | 内核序列化异常: ") + error_v.error_msg);
			}
			catch (const std::bad_alloc&)
			{
				throw rule_design_error_throw(rule_design_error::internal__container, "rule_design | save | 序列化缓冲区分配失败，错误不可恢复");
			}
			catch (const std::exception& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__serialize, std::string("rule_design | save | 序列化异常: ") + error_v.what());
			}
			catch (...)
			{
				throw rule_design_error_throw(rule_design_error::internal__serialize, "rule_design | save | 未知序列化异常，错误不可恢复");
			}
		}
		bool load(std::istream& in_v, rule_desg_cmd_mgr_index& mgr_index_v, rule_desg_cmd_index& cmd_index_v)
		{
			this->clear_error();

			try
			{
				invoke_ip ip_v;

				if (rule_serialize::deserialize_rule(this->kernel_rule_kit.get_owner(), in_v, ip_v).has_value() == false)
					return this->fail_bool(rule_design_error::exec__internal, "rule_desg 序列化加载失败");

				mgr_index_v = static_cast<rule_desg_cmd_mgr_index>(ip_v.get_cmd_set_index());
				cmd_index_v = static_cast<rule_desg_cmd_index>(ip_v.get_index());
				return true;
			}
			catch (const rule_design_error_throw&)
			{
				throw;
			}
			catch (const kernel_error_throw& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__serialize, std::string("rule_design | load | 内核反序列化异常: ") + error_v.error_msg);
			}
			catch (const std::bad_alloc&)
			{
				throw rule_design_error_throw(rule_design_error::internal__container, "rule_design | load | 反序列化缓冲区分配失败，错误不可恢复");
			}
			catch (const std::exception& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__serialize, std::string("rule_design | load | 反序列化异常: ") + error_v.what());
			}
			catch (...)
			{
				throw rule_design_error_throw(rule_design_error::internal__serialize, "rule_design | load | 未知反序列化异常，错误不可恢复");
			}
		}
		void resume(rule_desg_cmd_mgr_index mgr_index_v, rule_desg_cmd_index cmd_index_v)
		{
			try
			{
				this->kernel_rule_kit.resume
				(
					invoke_ip
					(
						static_cast<typename invoke_ip::command_set_index>(mgr_index_v),
						static_cast<typename invoke_ip::command_index>(cmd_index_v)
					)
				);
				return;
			}
			catch (const rule_design_error_throw&)
			{
				throw;
			}
			catch (const kernel_error_throw& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__kernel, std::string("rule_design | resume | 内核异常: ") + error_v.error_msg);
			}
			catch (const std::bad_alloc&)
			{
				throw rule_design_error_throw(rule_design_error::internal__container, "rule_design | resume | 恢复执行内存分配失败，错误不可恢复");
			}
			catch (const std::exception& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__exception, std::string("rule_design | resume | 异常: ") + error_v.what());
			}
			catch (...)
			{
				throw rule_design_error_throw(rule_design_error::internal__exception, "rule_design | resume | 未知异常，错误不可恢复");
			}
		}
		bool save_file(const std::filesystem::path& path_v, rule_desg_cmd_mgr_index mgr_index_v, rule_desg_cmd_index cmd_index_v)
		{
			try
			{
				const invoke_ip ip_v
				(
					static_cast<typename invoke_ip::command_set_index>(mgr_index_v),
					static_cast<typename invoke_ip::command_index>(cmd_index_v)
				);

				if (this->kernel_rule_kit.save(ip_v, path_v) == false)
					return this->fail_bool(rule_design_error::exec__internal, "rule_desg 序列化输出文件保存失败");

				return true;
			}
			catch (const rule_design_error_throw&)
			{
				throw;
			}
			catch (const std::exception& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__serialize, std::string("rule_design | save_file | 文件异常: ") + error_v.what());
			}
			catch (...)
			{
				throw rule_design_error_throw(rule_design_error::internal__serialize, "rule_design | save_file | 未知文件异常，错误不可恢复");
			}
		}
		bool load_file(const std::filesystem::path& path_v, rule_desg_cmd_mgr_index& mgr_index_v, rule_desg_cmd_index& cmd_index_v)
		{
			try
			{
				invoke_ip ip_v;

				if (this->kernel_rule_kit.load(path_v, ip_v) == false)
					return this->fail_bool(rule_design_error::exec__internal, "rule_desg 序列化输入文件加载失败");

				mgr_index_v = static_cast<rule_desg_cmd_mgr_index>(ip_v.get_cmd_set_index());
				cmd_index_v = static_cast<rule_desg_cmd_index>(ip_v.get_index());
				return true;
			}
			catch (const rule_design_error_throw&)
			{
				throw;
			}
			catch (const std::exception& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__serialize, std::string("rule_design | load_file | 文件异常: ") + error_v.what());
			}
			catch (...)
			{
				throw rule_design_error_throw(rule_design_error::internal__serialize, "rule_design | load_file | 未知文件异常，错误不可恢复");
			}
		}
	public:
		bool load_text(std::string_view text_v, text_error& error_v)
		{
			this->clear_error();

			try
			{
			text_parser parser_v(this->allocator_);
			text_program program_v(this->allocator_);

			if (parser_v.parse(text_v, program_v, error_v) == false)
			{
				this->sync_text_error(error_v);
				return false;
			}

			this->clear();

			for (const rule_kit::rule_kit_var_mgr_index block_v : program_v.var_blocks)
			{
				if (this->kernel_rule_kit.get_owner().get_var_set().add(block_v, false).has_value() == false)
				{
					error_v.line = 0;
					error_v.column = 0;
					error_v.code = rule_design_error::text__var_load;
					error_v.message.assign("defvar 资源创建失败");
					this->clear();
					this->sync_text_error(error_v);
					return false;
				}
			}

			for (const rule_kit::rule_kit_cmd_mgr_index block_v : program_v.cmd_blocks)
			{
				if (this->kernel_rule_kit.get_owner().get_cmd_set().add(block_v, false).has_value() == false)
				{
					error_v.line = 0;
					error_v.column = 0;
					error_v.code = rule_design_error::text__cmd_load;
					error_v.message.assign("defcmd 资源创建失败");
					this->clear();
					this->sync_text_error(error_v);
					return false;
				}
			}

			for (const text_var& var_v : program_v.vars)
			{
				if (this->add_set_var
				(
					var_v.mgr,
					var_v.index,
					var_v.type,
					var_v.begin,
					std::span<rule_desg_var_base>(const_cast<rule_desg_var_base*>(var_v.bytes.data()), var_v.bytes.size())
				) == false)
				{
					error_v.line = 0;
					error_v.column = 0;
					error_v.code = rule_design_error::text__var_load;
					error_v.message.assign("变量加载失败: ");
					error_v.message.append(var_v.name);
					this->clear();
					this->sync_text_error(error_v);
					return false;
				}
			}

			for (const text_cmd& cmd_v : program_v.cmds)
			{
				rule_desg_cmd_def_type type_v;

				if (rule_desg_text_type(cmd_v.mnemonic, type_v) == false)
				{
					error_v.line = 0;
					error_v.column = 0;
					error_v.code = rule_design_error::text__cmd;
					error_v.message.assign("未知指令: ");
					error_v.message.append(cmd_v.mnemonic);
					this->clear();
					this->sync_text_error(error_v);
					return false;
				}

				if (this->kernel_rule_kit.add_set_cmd_set
				(
					cmd_v.mgr,
					cmd_v.index,
					static_cast<rule_kit::rule_kit_cmd_type>(type_v),
					0,
					std::span<const rule_kit::rule_kit_cmd_op_sgl>(cmd_v.ops.data(), cmd_v.ops.size())
				) == false)
				{
					error_v.line = 0;
					error_v.column = 0;
					error_v.code = rule_design_error::text__cmd_load;
					error_v.message.assign("指令加载失败: ");
					error_v.message.append(cmd_v.mnemonic);
					this->clear();
					this->sync_text_error(error_v);
					return false;
				}
			}

			this->text_start_mgr = program_v.start_mgr;
			this->text_start_index = program_v.start_index;
			return true;
			}
			catch (const rule_design_error_throw& error_v)
			{
				this->set_error_safe(error_v.error_code, error_v.error_msg);
				throw;
			}
			catch (const kernel_error_throw& error_v)
			{
				this->set_error_safe(rule_design_error::internal__kernel, error_v.error_msg);
				throw rule_design_error_throw(rule_design_error::internal__kernel, std::string("rule_design | load_text | 内核异常: ") + error_v.error_msg);
			}
			catch (const std::bad_alloc&)
			{
				throw rule_design_error_throw(rule_design_error::internal__container, "rule_design | load_text | 文本程序内存分配失败，错误不可恢复");
			}
			catch (const std::exception& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__exception, std::string("rule_design | load_text | 加载异常: ") + error_v.what());
			}
			catch (...)
			{
				throw rule_design_error_throw(rule_design_error::internal__exception, "rule_design | load_text | 未知加载异常，错误不可恢复");
			}
		}
		bool load_text_file(const std::filesystem::path& path_v, text_error& error_v)
		{
			try
			{
			std::error_code file_error_v;

			if (path_v.empty() || std::filesystem::is_regular_file(path_v, file_error_v) == false)
			{
				error_v.line = 0;
				error_v.column = 0;
				error_v.code = rule_design_error::text__file_open;
				error_v.message.assign("无法打开代码文件: ");
				error_v.message.append(path_v.string());
				this->sync_text_error(error_v);
				return false;
			}

			const std::uintmax_t file_size_v = std::filesystem::file_size(path_v, file_error_v);

			if (file_error_v || file_size_v > static_cast<std::uintmax_t>(rule_serialize::SERIALIZE_MAX_FILE_SIZE))
			{
				error_v.line = 0;
				error_v.column = 0;
				error_v.code = rule_design_error::text__file_read;
				error_v.message.assign("读取代码文件失败: ");
				error_v.message.append(path_v.string());
				this->sync_text_error(error_v);
				return false;
			}

			std::ifstream in_v(path_v, std::ios::binary);

			if (in_v.good() == false)
			{
				error_v.line = 0;
				error_v.column = 0;
				error_v.code = rule_design_error::text__file_open;
				error_v.message.assign("无法打开代码文件: ");
				error_v.message.append(path_v.string());
				this->sync_text_error(error_v);
				return false;
			}

			std::string text_v
			(
				(std::istreambuf_iterator<char>(in_v)),
				std::istreambuf_iterator<char>()
			);

			if (in_v.bad() == true)
			{
				error_v.line = 0;
				error_v.column = 0;
				error_v.code = rule_design_error::text__file_read;
				error_v.message.assign("读取代码文件失败: ");
				error_v.message.append(path_v.string());
				this->sync_text_error(error_v);
				return false;
			}

			if (text_v.size() >= 3 &&
				static_cast<unsigned char>(text_v[0]) == 0xEF &&
				static_cast<unsigned char>(text_v[1]) == 0xBB &&
				static_cast<unsigned char>(text_v[2]) == 0xBF)
			{
				text_v.erase(0, 3);
			}

			std::size_t bad_offset_v = 0;

			if (rule_desg_text_utf8_valid(std::string_view(text_v.data(), text_v.size()), bad_offset_v) == false)
			{
				std::size_t line_v = 1;
				std::size_t column_v = 1;

				for (std::size_t i = 0; i < bad_offset_v; i++)
				{
					if (text_v[i] == '\n')
					{
						line_v++;
						column_v = 1;
					}
					else
					{
						column_v++;
					}
				}

				error_v.line = line_v;
				error_v.column = column_v;
				error_v.code = rule_design_error::text__utf8;
				error_v.message.assign("代码文件不是合法的 UTF-8 编码: ");
				error_v.message.append(path_v.string());
				this->sync_text_error(error_v);
				return false;
			}

			return this->load_text(std::string_view(text_v.data(), text_v.size()), error_v);
			}
			catch (const rule_design_error_throw& error_v)
			{
				this->set_error_safe(error_v.error_code, error_v.error_msg);
				throw;
			}
			catch (const std::bad_alloc&)
			{
				throw rule_design_error_throw(rule_design_error::internal__container, "rule_design | load_text_file | 文件内容内存分配失败，错误不可恢复");
			}
			catch (const std::ios_base::failure& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__exception, std::string("rule_design | load_text_file | 文件流异常: ") + error_v.what());
			}
			catch (const std::exception& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__exception, std::string("rule_design | load_text_file | 异常: ") + error_v.what());
			}
			catch (...)
			{
				throw rule_design_error_throw(rule_design_error::internal__exception, "rule_design | load_text_file | 未知文件异常，错误不可恢复");
			}
		}
		void run_start()
		{
			this->run(this->text_start_mgr, this->text_start_index);
			return;
		}
		void release_auto_alloc()
		{
			this->release_auto_alloc_impl();
			return;
		}
	private:
		static bool rule_desg_text_type(std::string_view name_v, rule_desg_cmd_def_type& type_save)
		{
			if (name_v == "mov") { type_save = rule_desg_cmd_def_type::mov; return true; }
			if (name_v == "cmp") { type_save = rule_desg_cmd_def_type::cmp; return true; }
			if (name_v == "jmp") { type_save = rule_desg_cmd_def_type::jmp; return true; }
			if (name_v == "call") { type_save = rule_desg_cmd_def_type::call; return true; }
			if (name_v == "ret") { type_save = rule_desg_cmd_def_type::ret; return true; }
			if (name_v == "add") { type_save = rule_desg_cmd_def_type::add; return true; }
			if (name_v == "sub") { type_save = rule_desg_cmd_def_type::sub; return true; }
			if (name_v == "mul") { type_save = rule_desg_cmd_def_type::mul; return true; }
			if (name_v == "div") { type_save = rule_desg_cmd_def_type::div; return true; }
			if (name_v == "alloc") { type_save = rule_desg_cmd_def_type::alloc; return true; }
			if (name_v == "dealloc") { type_save = rule_desg_cmd_def_type::dealloc; return true; }
			return false;
		}
	public:
		invoke_ownership& get_owner() noexcept
		{
			return this->kernel_rule_kit;
		}
		const invoke_ownership& get_owner() const noexcept
		{
			return this->kernel_rule_kit;
		}
		void clear()
		{
			this->kernel_rule_kit.clear();
			raw_buf.clear();
			this->auto_alloc_set.clear();
			return;
		}
		void shrink_to_fit()
		{
			try
			{
				this->kernel_rule_kit.shrink_to_fit();
				raw_buf.clear();
				raw_buf.shrink_to_fit();
				return;
			}
			catch (const rule_design_error_throw&)
			{
				throw;
			}
			catch (const kernel_error_throw& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__kernel, std::string("rule_design | shrink_to_fit | 内核异常: ") + error_v.error_msg);
			}
			catch (const std::bad_alloc&)
			{
				throw rule_design_error_throw(rule_design_error::internal__container, "rule_design | shrink_to_fit | 收缩内存失败，错误不可恢复");
			}
			catch (const std::exception& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__exception, std::string("rule_design | shrink_to_fit | 异常: ") + error_v.what());
			}
			catch (...)
			{
				throw rule_design_error_throw(rule_design_error::internal__exception, "rule_design | shrink_to_fit | 未知异常，错误不可恢复");
			}
		}
		void copy_from(const rule_desg& other_v)
		{
			try
			{
				this->kernel_rule_kit.copy_from(other_v.kernel_rule_kit);
				this->register_cmd_def();
				return;
			}
			catch (const rule_design_error_throw&)
			{
				throw;
			}
			catch (const kernel_error_throw& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__kernel, std::string("rule_design | copy_from | 内核异常: ") + error_v.error_msg);
			}
			catch (const std::bad_alloc&)
			{
				throw rule_design_error_throw(rule_design_error::internal__container, "rule_design | copy_from | 拷贝分配失败，错误不可恢复");
			}
			catch (const std::exception& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__exception, std::string("rule_design | copy_from | 异常: ") + error_v.what());
			}
			catch (...)
			{
				throw rule_design_error_throw(rule_design_error::internal__exception, "rule_design | copy_from | 未知异常，错误不可恢复");
			}
		}
		void copy_to(rule_desg& other_v)
		{
			try
			{
				this->kernel_rule_kit.copy_to(other_v.kernel_rule_kit);
				other_v.register_cmd_def();
				return;
			}
			catch (const rule_design_error_throw&)
			{
				throw;
			}
			catch (const kernel_error_throw& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__kernel, std::string("rule_design | copy_to | 内核异常: ") + error_v.error_msg);
			}
			catch (const std::bad_alloc&)
			{
				throw rule_design_error_throw(rule_design_error::internal__container, "rule_design | copy_to | 拷贝分配失败，错误不可恢复");
			}
			catch (const std::exception& error_v)
			{
				throw rule_design_error_throw(rule_design_error::internal__exception, std::string("rule_design | copy_to | 异常: ") + error_v.what());
			}
			catch (...)
			{
				throw rule_design_error_throw(rule_design_error::internal__exception, "rule_design | copy_to | 未知异常，错误不可恢复");
			}
		}
	};
}
