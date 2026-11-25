#pragma once
#include "../kernel_namespace.h"
#include <istream>
#include <ostream>
#include "serialize_header_data_def.h"
#include "serialize_io.h"

namespace rule_serialize_header
{
	template<typename rule_t>
	inline rule_serialize::result write(std::ostream& out_v, const rule_serialize::serialize_header_info& header_v)
	{
		const char magic_v[4] = { 'R', 'V', 'M', '1' };

		out_v.write(magic_v, 4);
		rule_serialize::write_u32_raw(out_v, rule_serialize::SERIALIZE_VERSION);
		rule_serialize::write_u32_raw(out_v, static_cast<std::uint32_t>(sizeof(typename rule_t::serialize_cmd_op_single_t)));
		rule_serialize::write_u32_raw(out_v, static_cast<std::uint32_t>(sizeof(typename rule_t::serialize_imm_target_t)));
		rule_serialize::write_u32_raw(out_v, static_cast<std::uint32_t>(sizeof(typename rule_t::serialize_var_target_t)));
		rule_serialize::write_u32_raw(out_v, static_cast<std::uint32_t>(sizeof(typename rule_t::serialize_cmd_target_t)));
		rule_serialize::write_u32_raw(out_v, static_cast<std::uint32_t>(sizeof(typename rule_t::serialize_var_base_t)));
		rule_serialize::write_u64_raw(out_v, header_v.get_payload_size());
		rule_serialize::write_u32_raw(out_v, header_v.get_crc());

		if (out_v.good() == false)
			return std::unexpected(kernel_error::serialize__io_error);

		return {};
	}

	template<typename rule_t>
	inline rule_serialize::result read(std::istream& in_v, rule_serialize::serialize_header_info& header_save)
	{
		char magic_v[4] = {};

		if (!in_v.read(magic_v, 4) || magic_v[0] != 'R' || magic_v[1] != 'V' || magic_v[2] != 'M' || magic_v[3] != '1')
			return std::unexpected(kernel_error::serialize__magic_mismatch);

		std::uint32_t version_v = 0;
		std::uint32_t size_cmd_op = 0;
		std::uint32_t size_imm = 0;
		std::uint32_t size_var = 0;
		std::uint32_t size_cmd = 0;
		std::uint32_t size_base = 0;
		std::uint64_t payload_size_v = 0;
		std::uint32_t crc_v = 0;

		if (!rule_serialize::read_u32_raw(in_v, version_v) ||
			!rule_serialize::read_u32_raw(in_v, size_cmd_op) ||
			!rule_serialize::read_u32_raw(in_v, size_imm) ||
			!rule_serialize::read_u32_raw(in_v, size_var) ||
			!rule_serialize::read_u32_raw(in_v, size_cmd) ||
			!rule_serialize::read_u32_raw(in_v, size_base) ||
			!rule_serialize::read_u64_raw(in_v, payload_size_v) ||
			!rule_serialize::read_u32_raw(in_v, crc_v))
			return std::unexpected(kernel_error::serialize__truncated);

		if (version_v != rule_serialize::SERIALIZE_VERSION)
			return std::unexpected(kernel_error::serialize__version_mismatch);

		if (size_cmd_op != sizeof(typename rule_t::serialize_cmd_op_single_t) ||
			size_imm != sizeof(typename rule_t::serialize_imm_target_t) ||
			size_var != sizeof(typename rule_t::serialize_var_target_t) ||
			size_cmd != sizeof(typename rule_t::serialize_cmd_target_t) ||
			size_base != sizeof(typename rule_t::serialize_var_base_t))
			return std::unexpected(kernel_error::serialize__layout_mismatch);

		header_save.set_payload_size(payload_size_v);
		header_save.set_crc(crc_v);
		return {};
	}
}
