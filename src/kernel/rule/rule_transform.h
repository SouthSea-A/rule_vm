#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <bit>
#include <type_traits>
#include <span>
#include "rule_data_def.h"

namespace rule_transform
{
	template<typename val_t>
	inline val_t transform_val_bytes_order(val_t val_v) noexcept
	{
		if constexpr (std::endian::native == cmd_data::RAW_BYTES_ORDER)
		{
			return val_v;
		}
		else if constexpr (std::is_integral_v<val_t>)
		{
			return std::byteswap(val_v);
		}
		else
		{
			using uint_val_t = std::conditional_t<sizeof(val_t) == sizeof(std::uint32_t), std::uint32_t, std::uint64_t>;

			return std::bit_cast<val_t>(std::byteswap(std::bit_cast<uint_val_t>(val_v)));
		}
	}

	template<typename val_t>
	inline constexpr std::size_t get_val_raw_size() noexcept
	{
		if constexpr (std::is_same_v<val_t, std::int8_t>) return cmd_data::RAW_BYTES_SIZE_INT_8;
		else if constexpr (std::is_same_v<val_t, std::int16_t>) return cmd_data::RAW_BYTES_SIZE_INT_16;
		else if constexpr (std::is_same_v<val_t, std::int32_t>) return cmd_data::RAW_BYTES_SIZE_INT_32;
		else if constexpr (std::is_same_v<val_t, std::int64_t>) return cmd_data::RAW_BYTES_SIZE_INT_64;
		else if constexpr (std::is_same_v<val_t, std::uint8_t>) return cmd_data::RAW_BYTES_SIZE_UINT_8;
		else if constexpr (std::is_same_v<val_t, std::uint16_t>) return cmd_data::RAW_BYTES_SIZE_UINT_16;
		else if constexpr (std::is_same_v<val_t, std::uint32_t>) return cmd_data::RAW_BYTES_SIZE_UINT_32;
		else if constexpr (std::is_same_v<val_t, std::uint64_t>) return cmd_data::RAW_BYTES_SIZE_UINT_64;
		else if constexpr (std::is_same_v<val_t, float>) return cmd_data::RAW_BYTES_SIZE_FLOAT_32;
		else if constexpr (std::is_same_v<val_t, double>) return cmd_data::RAW_BYTES_SIZE_FLOAT_64;
		else return 0;
	}

	template<typename tgt_t>
	inline bool transform_raw_bytes_to(rdata::rule_int_info& int_info_save, const std::span<rdata::rule_var_base_t>& span_raw_bytes_v, tgt_t& res_save)
	{
		auto bytes_order_transform = [](cmd_data::raw_bytes_base_field_type& data) -> void
		{
			if constexpr (std::endian::native != cmd_data::RAW_BYTES_ORDER) data = std::byteswap(data);
			return;
		};

		auto handle_imm_tgt = [&](const std::span<rdata::rule_var_base_t>& raw_bytes_v, cmd_data::imm_target& res_imm_save) -> bool
		{
			if (raw_bytes_v.size_bytes() < cmd_data::RAW_BYTES_SIZE_IMM_TARGET)
			{
				rdata::set_int_info
				(
					int_info_save,
					rdata::rule_int_t::trans_raw_to_imm_bytes_size_no_match,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, std::nullopt, std::nullopt, std::nullopt)
				);
				return false;
			}

			rdata::rule_var_base_t* raw_bytes_addr_begin = raw_bytes_v.data();
			cmd_data::raw_bytes_base_field_type type_set_read = 0;

			std::memcpy(&type_set_read, raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 0, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);

			bytes_order_transform(type_set_read);

			if (type_set_read >= static_cast<cmd_data::raw_bytes_base_field_type>(cmd_data::imm_type::invalid))
			{
				rdata::set_int_info
				(
					int_info_save,
					rdata::rule_int_t::trans_raw_to_imm_t_no_match,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, std::nullopt, std::nullopt, std::nullopt)
				);
				return false;
			}

			cmd_data::imm_type type_set = static_cast<cmd_data::imm_type>(type_set_read);
			std::array<cmd_data::cmd_var_base_type, cmd_data::imm_width_byte_max> data_set{};

			std::memcpy(data_set.data(), raw_bytes_v.data() + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 1, cmd_data::imm_width_byte_max * sizeof(cmd_data::cmd_var_base_type));

			res_imm_save.set_type(type_set);
			res_imm_save.set_data(data_set);
			return true;
		};

		auto handle_var_tgt = [&](const std::span<rdata::rule_var_base_t>& raw_bytes_v, cmd_data::var_target& res_var_save) -> bool
		{
			if (raw_bytes_v.size_bytes() < cmd_data::RAW_BYTES_SIZE_VAR_TARGET)
			{
				rdata::set_int_info
				(
					int_info_save,
					rdata::rule_int_t::trans_raw_to_var_bytes_size_no_match,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, std::nullopt, std::nullopt, std::nullopt)
				);
				return false;
			}

			rdata::rule_var_base_t* raw_bytes_addr_begin = raw_bytes_v.data();
			cmd_data::raw_bytes_base_field_type handle_type_set_read = 0;

			std::memcpy(&handle_type_set_read, raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 0, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);

			bytes_order_transform(handle_type_set_read);

			if (handle_type_set_read >= static_cast<cmd_data::raw_bytes_base_field_type>(cmd_data::op_var_type::invalid))
			{
				rdata::set_int_info
				(
					int_info_save,
					rdata::rule_int_t::trans_raw_to_var_t_no_match,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, std::nullopt, std::nullopt, std::nullopt)
				);
				return false;
			}

			cmd_data::op_var_type handle_type_set = static_cast<cmd_data::op_var_type>(handle_type_set_read);
			cmd_data::raw_bytes_base_field_type ownership_set_read = 0;

			std::memcpy(&ownership_set_read, raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 1, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);

			bytes_order_transform(ownership_set_read);

			var_cont_data::var_mgr_index ownership_set = static_cast<var_cont_data::var_mgr_index>(ownership_set_read);
			cmd_data::raw_bytes_base_field_type index_set_read = 0;

			std::memcpy(&index_set_read, raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 2, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);

			bytes_order_transform(index_set_read);

			var_cont_data::var_index index_set = static_cast<var_cont_data::var_index>(index_set_read);
			cmd_data::raw_bytes_base_field_type type_set_read = 0;

			std::memcpy(&type_set_read, raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 3, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);

			bytes_order_transform(type_set_read);

			var_cont_data::var_type type_set = static_cast<var_cont_data::var_type>(type_set_read);
			cmd_data::raw_bytes_base_field_type begin_set_read = 0;

			std::memcpy(&begin_set_read, raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 4, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);

			bytes_order_transform(begin_set_read);

			var_cont_data::var_pos begin_set = static_cast<var_cont_data::var_pos>(begin_set_read);
			cmd_data::raw_bytes_base_field_type width_set_read = 0;

			std::memcpy(&width_set_read, raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 5, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);

			bytes_order_transform(width_set_read);

			var_cont_data::var_read_width width_set = static_cast<var_cont_data::var_read_width>(width_set_read);

			res_var_save.set_handle_type(handle_type_set);
			res_var_save.set_ownership(ownership_set);
			res_var_save.set_index(index_set);
			res_var_save.set_type(type_set);
			res_var_save.set_begin(begin_set);
			res_var_save.set_width(width_set);

			return true;
		};

		auto handle_cmd_tgt = [&](const std::span<rdata::rule_var_base_t>& raw_bytes_v, cmd_data::cmd_target& res_cmd_save) -> bool
		{
			if (raw_bytes_v.size_bytes() < cmd_data::RAW_BYTES_SIZE_CMD_TARGET)
			{
				rdata::set_int_info
				(
					int_info_save,
					rdata::rule_int_t::trans_raw_to_cmd_bytes_size_no_match,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, std::nullopt, std::nullopt, std::nullopt)
				);
				return false;
			}

			rdata::rule_var_base_t* raw_bytes_addr_begin = raw_bytes_v.data();
			cmd_data::raw_bytes_base_field_type handle_type_set_read = 0;

			std::memcpy(&handle_type_set_read, raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 0, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);

			bytes_order_transform(handle_type_set_read);

			if (handle_type_set_read >= static_cast<cmd_data::raw_bytes_base_field_type>(cmd_data::op_cmd_type::invalid))
			{
				rdata::set_int_info
				(
					int_info_save,
					rdata::rule_int_t::trans_raw_to_cmd_t_no_match,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, std::nullopt, std::nullopt, std::nullopt)
				);
				return false;
			}

			cmd_data::op_cmd_type handle_type_set = static_cast<cmd_data::op_cmd_type>(handle_type_set_read);
			cmd_data::raw_bytes_base_field_type ownership_set_read = 0;

			std::memcpy(&ownership_set_read, raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 1, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);

			bytes_order_transform(ownership_set_read);

			cmd_data::var_cmd_mgr_index ownership_set = static_cast<cmd_data::var_cmd_mgr_index>(ownership_set_read);
			cmd_data::raw_bytes_base_field_type index_set_read = 0;

			std::memcpy(&index_set_read, raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 2, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);

			bytes_order_transform(index_set_read);

			cmd_data::var_cmd_index index_set = static_cast<cmd_data::var_cmd_index>(index_set_read);
			cmd_data::raw_bytes_base_field_type type_set_read = 0;

			std::memcpy(&type_set_read, raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 3, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);

			bytes_order_transform(type_set_read);

			cmd_data::var_cmd_type type_set = static_cast<cmd_data::var_cmd_type>(type_set_read);
			cmd_data::raw_bytes_base_field_type begin_set_read = 0;

			std::memcpy(&begin_set_read, raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 4, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);

			bytes_order_transform(begin_set_read);

			cmd_data::var_cmd_begin_pos begin_set = static_cast<cmd_data::var_cmd_begin_pos>(begin_set_read);
			cmd_data::raw_bytes_base_field_type width_set_read = 0;

			std::memcpy(&width_set_read, raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 5, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);

			bytes_order_transform(width_set_read);

			cmd_data::cmd_op_width width_set = static_cast<cmd_data::cmd_op_width>(width_set_read);

			res_cmd_save.set_handle_type(handle_type_set);
			res_cmd_save.set_ownership(ownership_set);
			res_cmd_save.set_index(index_set);
			res_cmd_save.set_type(type_set);
			res_cmd_save.set_begin(begin_set);
			res_cmd_save.set_width(width_set);

			return true;
		};

		auto handle_val = [&]<typename val_t>(const std::span<rdata::rule_var_base_t>&raw_bytes_v, val_t & res_save) -> bool
		{
			constexpr std::size_t bytes_size_tgt = get_val_raw_size<val_t>();

			if (raw_bytes_v.size_bytes() < bytes_size_tgt)
			{
				rdata::set_int_info
				(
					int_info_save,
					rdata::rule_int_t::trans_raw_to_cmd_bytes_size_no_match,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, std::nullopt, std::nullopt, std::nullopt)
				);
				return false;
			}

			std::memcpy(&res_save, raw_bytes_v.data(), bytes_size_tgt);

			if constexpr (std::endian::native != cmd_data::RAW_BYTES_ORDER) res_save = transform_val_bytes_order(res_save);

			return true;
		};

		if constexpr (std::is_same_v<tgt_t, cmd_data::imm_target>)
		{
			return handle_imm_tgt(span_raw_bytes_v, res_save);
		}
		else if constexpr (std::is_same_v<tgt_t, cmd_data::var_target>)
		{
			return handle_var_tgt(span_raw_bytes_v, res_save);
		}
		else if constexpr (std::is_same_v<tgt_t, cmd_data::cmd_target>)
		{
			return handle_cmd_tgt(span_raw_bytes_v, res_save);
		}
		else if constexpr (std::is_same_v<tgt_t, cmd_data::cmd_op_single>)
		{
			if (span_raw_bytes_v.size_bytes() < cmd_data::RAW_BYTES_SIZE_CMD_OP)
			{
				rdata::set_int_info
				(
					int_info_save,
					rdata::rule_int_t::trans_raw_to_cmd_op_bytes_size_no_match,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, std::nullopt, std::nullopt, std::nullopt)
				);
				return false;
			}

			rdata::rule_var_base_t* raw_bytes_addr_begin = span_raw_bytes_v.data();
			cmd_data::raw_bytes_base_field_type type_set_read = 0;

			std::memcpy(&type_set_read, raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 0, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);

			bytes_order_transform(type_set_read);

			cmd_data::cmd_op_type type_set = static_cast<cmd_data::cmd_op_type>(type_set_read);
			cmd_data::cmd_op_single& cmd_op_sgl_save = res_save;
			std::span<rdata::rule_var_base_t> span_handle_new = span_raw_bytes_v.subspan(cmd_data::RAW_BYTES_SIZE_FIELD_BASE);

			switch (type_set)
			{
			case cmd_data::cmd_op_type::imm:
			{
				cmd_data::imm_target imm_emplace;

				if (handle_imm_tgt(span_handle_new, imm_emplace) == false)
					return false;

				cmd_op_sgl_save.emplace<cmd_data::imm_target>(imm_emplace);
				break;
			}
			case cmd_data::cmd_op_type::var:
			{
				cmd_data::var_target var_emplace;

				if (handle_var_tgt(span_handle_new, var_emplace) == false)
					return false;

				cmd_op_sgl_save.emplace<cmd_data::var_target>(var_emplace);
				break;
			}
			case cmd_data::cmd_op_type::cmd:
			{
				cmd_data::cmd_target cmd_emplace;

				if (handle_cmd_tgt(span_handle_new, cmd_emplace) == false)
					return false;

				cmd_op_sgl_save.emplace<cmd_data::cmd_target>(cmd_emplace);
				break;
			}
			default:
			{
				rdata::set_int_info
				(
					int_info_save,
					rdata::rule_int_t::trans_raw_to_cmd_op_t_no_match,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, std::nullopt, std::nullopt, std::nullopt)
				);
				return false;
			}
			}

			return true;
		}
		else if constexpr (std::is_same_v<tgt_t, std::int8_t>)
		{
			return handle_val(span_raw_bytes_v, res_save);
		}
		else if constexpr (std::is_same_v<tgt_t, std::int16_t>)
		{
			return handle_val(span_raw_bytes_v, res_save);
		}
		else if constexpr (std::is_same_v<tgt_t, std::int32_t>)
		{
			return handle_val(span_raw_bytes_v, res_save);
		}
		else if constexpr (std::is_same_v<tgt_t, std::int64_t>)
		{
			return handle_val(span_raw_bytes_v, res_save);
		}
		else if constexpr (std::is_same_v<tgt_t, std::uint8_t>)
		{
			return handle_val(span_raw_bytes_v, res_save);
		}
		else if constexpr (std::is_same_v<tgt_t, std::uint16_t>)
		{
			return handle_val(span_raw_bytes_v, res_save);
		}
		else if constexpr (std::is_same_v<tgt_t, std::uint32_t>)
		{
			return handle_val(span_raw_bytes_v, res_save);
		}
		else if constexpr (std::is_same_v<tgt_t, std::uint64_t>)
		{
			return handle_val(span_raw_bytes_v, res_save);
		}
		else if constexpr (std::is_same_v<tgt_t, float>)
		{
			return handle_val(span_raw_bytes_v, res_save);
		}
		else if constexpr (std::is_same_v<tgt_t, double>)
		{
			return handle_val(span_raw_bytes_v, res_save);
		}
		else
		{
			rdata::set_int_info
			(
				int_info_save,
				rdata::rule_int_t::trans_t_invalid,
				rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
				rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, std::nullopt, std::nullopt, std::nullopt)
			);
			return false;
		}

		return false;
	}

	template<typename tgt_t>
	inline bool transform_tgt_to_raw_bytes(rdata::rule_int_info& int_info_save, const tgt_t& tgt_v, const std::span<rdata::rule_var_base_t>& span_raw_bytes_save)
	{
		auto bytes_order_transform = [](cmd_data::raw_bytes_base_field_type data) -> cmd_data::raw_bytes_base_field_type
		{
			if constexpr (std::endian::native != cmd_data::RAW_BYTES_ORDER) return std::byteswap(data);
			else return data;
		};

		auto handle_imm_tgt = [&](const cmd_data::imm_target& imm_transform_v, const std::span<rdata::rule_var_base_t>& raw_bytes_save) -> bool
		{
			if (raw_bytes_save.size_bytes() < cmd_data::RAW_BYTES_SIZE_IMM_TARGET)
			{
				rdata::set_int_info
				(
					int_info_save,
					rdata::rule_int_t::trans_imm_to_raw_bytes_size_no_match,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, std::nullopt, std::nullopt, std::nullopt)
				);
				return false;
			}

			cmd_data::raw_bytes_base_field_type type_transform = bytes_order_transform(static_cast<cmd_data::raw_bytes_base_field_type>(imm_transform_v.get_type()));
			const std::array<cmd_data::cmd_var_base_type, cmd_data::imm_width_byte_max>& data_transform = imm_transform_v.get_data();
			rdata::rule_var_base_t* raw_bytes_addr_begin = raw_bytes_save.data();

			std::memcpy(raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 0, &type_transform, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);
			std::memcpy(raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 1, data_transform.data(), cmd_data::imm_width_byte_max * sizeof(cmd_data::cmd_var_base_type));

			return true;
		};

		auto handle_var_tgt = [&](const cmd_data::var_target& var_transform_v, const std::span<rdata::rule_var_base_t>& raw_bytes_save) -> bool
		{
			if (raw_bytes_save.size_bytes() < cmd_data::RAW_BYTES_SIZE_VAR_TARGET)
			{
				rdata::set_int_info
				(
					int_info_save,
					rdata::rule_int_t::trans_var_to_raw_bytes_size_no_match,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, std::nullopt, std::nullopt, std::nullopt)
				);
				return false;
			}

			cmd_data::raw_bytes_base_field_type handle_type_transform = bytes_order_transform(static_cast<cmd_data::raw_bytes_base_field_type>(var_transform_v.get_handle_type()));
			cmd_data::raw_bytes_base_field_type ownership_transform = bytes_order_transform(static_cast<cmd_data::raw_bytes_base_field_type>(var_transform_v.get_ownership()));
			cmd_data::raw_bytes_base_field_type index_transform = bytes_order_transform(static_cast<cmd_data::raw_bytes_base_field_type>(var_transform_v.get_index()));
			cmd_data::raw_bytes_base_field_type type_transform = bytes_order_transform(static_cast<cmd_data::raw_bytes_base_field_type>(var_transform_v.get_type()));
			cmd_data::raw_bytes_base_field_type begin_transform = bytes_order_transform(static_cast<cmd_data::raw_bytes_base_field_type>(var_transform_v.get_begin()));
			cmd_data::raw_bytes_base_field_type width_transform = bytes_order_transform(static_cast<cmd_data::raw_bytes_base_field_type>(var_transform_v.get_width()));
			rdata::rule_var_base_t* raw_bytes_addr_begin = raw_bytes_save.data();

			std::memcpy(raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 0, &handle_type_transform, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);
			std::memcpy(raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 1, &ownership_transform, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);
			std::memcpy(raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 2, &index_transform, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);
			std::memcpy(raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 3, &type_transform, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);
			std::memcpy(raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 4, &begin_transform, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);
			std::memcpy(raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 5, &width_transform, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);

			return true;
		};

		auto handle_cmd_tgt = [&](const cmd_data::cmd_target& cmd_transform_v, const std::span<rdata::rule_var_base_t>& raw_bytes_save) -> bool
		{
			if (raw_bytes_save.size_bytes() < cmd_data::RAW_BYTES_SIZE_CMD_TARGET)
			{
				rdata::set_int_info
				(
					int_info_save,
					rdata::rule_int_t::trans_cmd_to_raw_bytes_size_no_match,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, std::nullopt, std::nullopt, std::nullopt)
				);
				return false;
			}

			cmd_data::raw_bytes_base_field_type handle_type_transform = bytes_order_transform(static_cast<cmd_data::raw_bytes_base_field_type>(cmd_transform_v.get_handle_type()));
			cmd_data::raw_bytes_base_field_type ownership_transform = bytes_order_transform(static_cast<cmd_data::raw_bytes_base_field_type>(cmd_transform_v.get_ownership()));
			cmd_data::raw_bytes_base_field_type index_transform = bytes_order_transform(static_cast<cmd_data::raw_bytes_base_field_type>(cmd_transform_v.get_index()));
			cmd_data::raw_bytes_base_field_type type_transform = bytes_order_transform(static_cast<cmd_data::raw_bytes_base_field_type>(cmd_transform_v.get_type()));
			cmd_data::raw_bytes_base_field_type begin_transform = bytes_order_transform(static_cast<cmd_data::raw_bytes_base_field_type>(cmd_transform_v.get_begin()));
			cmd_data::raw_bytes_base_field_type width_transform = bytes_order_transform(static_cast<cmd_data::raw_bytes_base_field_type>(cmd_transform_v.get_width()));
			rdata::rule_var_base_t* raw_bytes_addr_begin = raw_bytes_save.data();

			std::memcpy(raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 0, &handle_type_transform, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);
			std::memcpy(raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 1, &ownership_transform, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);
			std::memcpy(raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 2, &index_transform, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);
			std::memcpy(raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 3, &type_transform, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);
			std::memcpy(raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 4, &begin_transform, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);
			std::memcpy(raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 5, &width_transform, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);

			return true;
		};

		auto handle_val = [&]<typename val_t>(val_t val_transform_v, const std::span<rdata::rule_var_base_t>&raw_bytes_save) -> bool
		{
			constexpr std::size_t bytes_size_tgt = get_val_raw_size<val_t>();

			if (raw_bytes_save.size_bytes() < bytes_size_tgt)
			{
				rdata::set_int_info
				(
					int_info_save,
					rdata::rule_int_t::trans_cmd_to_raw_bytes_size_no_match,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, std::nullopt, std::nullopt, std::nullopt)
				);
				return false;
			}

			if constexpr (std::endian::native != cmd_data::RAW_BYTES_ORDER) val_transform_v = transform_val_bytes_order(val_transform_v);

			std::memcpy(raw_bytes_save.data(), &val_transform_v, bytes_size_tgt);
			return true;
		};

		if constexpr (std::is_same_v<tgt_t, cmd_data::imm_target>)
		{
			return handle_imm_tgt(tgt_v, span_raw_bytes_save);
		}
		else if constexpr (std::is_same_v<tgt_t, cmd_data::var_target>)
		{
			return handle_var_tgt(tgt_v, span_raw_bytes_save);
		}
		else if constexpr (std::is_same_v<tgt_t, cmd_data::cmd_target>)
		{
			return handle_cmd_tgt(tgt_v, span_raw_bytes_save);
		}
		else if constexpr (std::is_same_v<tgt_t, cmd_data::cmd_op_single>)
		{
			if (span_raw_bytes_save.size_bytes() < cmd_data::RAW_BYTES_SIZE_CMD_OP)
			{
				rdata::set_int_info
				(
					int_info_save,
					rdata::rule_int_t::trans_cmd_op_to_raw_bytes_size_no_match,
					rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
					rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, std::nullopt, std::nullopt, std::nullopt)
				);
				return false;
			}

			const cmd_data::cmd_op_single& cmd_op_sgl_transform = tgt_v;
			cmd_data::raw_bytes_base_field_type op_type_transform = bytes_order_transform(static_cast<cmd_data::raw_bytes_base_field_type>(cmd_op_sgl_transform.index()));
			rdata::rule_var_base_t* raw_bytes_addr_begin = span_raw_bytes_save.data();

			auto handle_transform_imm_tgt = [&](const cmd_data::imm_target& imm_tgt) -> bool
				{
					std::memcpy(raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 0, &op_type_transform, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);

					return handle_imm_tgt
					(
						imm_tgt,
						span_raw_bytes_save.subspan(cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 1)
					);
				};

			auto handle_transform_var_tgt = [&](const cmd_data::var_target& var_tgt) -> bool
				{
					std::memcpy(raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 0, &op_type_transform, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);

					return handle_var_tgt
					(
						var_tgt,
						span_raw_bytes_save.subspan(cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 1)
					);
				};

			auto handle_transform_cmd_tgt = [&](const cmd_data::cmd_target& cmd_tgt) -> bool
				{
					std::memcpy(raw_bytes_addr_begin + cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 0, &op_type_transform, cmd_data::RAW_BYTES_SIZE_FIELD_BASE);

					return handle_cmd_tgt
					(
						cmd_tgt,
						span_raw_bytes_save.subspan(cmd_data::RAW_BYTES_SIZE_FIELD_BASE * 1)
					);
				};

			return std::visit(rdata::t_case_distribute{ handle_transform_imm_tgt ,handle_transform_var_tgt ,handle_transform_cmd_tgt }, cmd_op_sgl_transform);
		}
		else if constexpr (std::is_same_v<tgt_t, std::int8_t>)
		{
			return handle_val(tgt_v, span_raw_bytes_save);
		}
		else if constexpr (std::is_same_v<tgt_t, std::int16_t>)
		{
			return handle_val(tgt_v, span_raw_bytes_save);
		}
		else if constexpr (std::is_same_v<tgt_t, std::int32_t>)
		{
			return handle_val(tgt_v, span_raw_bytes_save);
		}
		else if constexpr (std::is_same_v<tgt_t, std::int64_t>)
		{
			return handle_val(tgt_v, span_raw_bytes_save);
		}
		else if constexpr (std::is_same_v<tgt_t, std::uint8_t>)
		{
			return handle_val(tgt_v, span_raw_bytes_save);
		}
		else if constexpr (std::is_same_v<tgt_t, std::uint16_t>)
		{
			return handle_val(tgt_v, span_raw_bytes_save);
		}
		else if constexpr (std::is_same_v<tgt_t, std::uint32_t>)
		{
			return handle_val(tgt_v, span_raw_bytes_save);
		}
		else if constexpr (std::is_same_v<tgt_t, std::uint64_t>)
		{
			return handle_val(tgt_v, span_raw_bytes_save);
		}
		else if constexpr (std::is_same_v<tgt_t, float>)
		{
			return handle_val(tgt_v, span_raw_bytes_save);
		}
		else if constexpr (std::is_same_v<tgt_t, double>)
		{
			return handle_val(tgt_v, span_raw_bytes_save);
		}
		else
		{
			rdata::set_int_info
			(
				int_info_save,
				rdata::rule_int_t::trans_t_invalid,
				rdata::rule_int_info_base_error_t(std::in_place_type<std::monostate>, std::monostate()),
				rdata::rule_int_info_cont(std::in_place_type<rdata::rule_int_info_cont_act>, std::nullopt, std::nullopt, std::nullopt)
			);
			return false;
		}

		return false;
	}
}
