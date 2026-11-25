#pragma once
#include "../kernel_namespace.h"
#include <istream>
#include <ostream>
#include "serialize_data_def.h"
#include "serialize_header_data_def.h"
#include "serialize_header.h"
#include "serialize_var.h"
#include "serialize_cmd.h"
#include "serialize_stack.h"

namespace rule_serialize
{
	template<typename rule_t>
	rule_serialize::result serialize_rule(rule_t& rule_v, const typename rule_t::run_ip& ip_v, std::ostream& out_v)
	{
		const std::streampos header_pos_v = out_v.tellp();

		if (header_pos_v == std::streampos(-1))
			return std::unexpected(kernel_error::serialize__io_error);

		const serialize_header_info header_placeholder_v(0, 0);

		if (rule_serialize_header::write<rule_t>(out_v, header_placeholder_v).has_value() == false)
			return std::unexpected(kernel_error::serialize__io_error);

		stream_writer writer_v(out_v);

		rule_serialize::result var_result_v = rule_serialize_var::write(writer_v, rule_v);

		if (var_result_v.has_value() == false)
			return var_result_v;

		rule_serialize::result cmd_result_v = rule_serialize_cmd::write(writer_v, rule_v);

		if (cmd_result_v.has_value() == false)
			return cmd_result_v;

		rule_serialize::result stack_result_v = rule_serialize_stack::write(writer_v, rule_v, ip_v);

		if (stack_result_v.has_value() == false)
			return stack_result_v;

		if (writer_v.good() == false)
			return std::unexpected(kernel_error::serialize__io_error);

		const std::streampos end_pos_v = out_v.tellp();

		if (end_pos_v == std::streampos(-1))
			return std::unexpected(kernel_error::serialize__io_error);

		out_v.seekp(header_pos_v);

		if (out_v.good() == false)
			return std::unexpected(kernel_error::serialize__io_error);

		const serialize_header_info header_final_v(writer_v.get_count(), writer_v.get_crc());

		if (rule_serialize_header::write<rule_t>(out_v, header_final_v).has_value() == false)
			return std::unexpected(kernel_error::serialize__io_error);

		out_v.seekp(end_pos_v);
		out_v.flush();

		if (out_v.good() == false)
			return std::unexpected(kernel_error::serialize__io_error);

		return {};
	}

	template<typename rule_t>
	rule_serialize::result deserialize_rule(rule_t& rule_v, std::istream& in_v, typename rule_t::run_ip& ip_out)
	{
		serialize_header_info header_v;

		rule_serialize::result header_result_v = rule_serialize_header::read<rule_t>(in_v, header_v);

		if (header_result_v.has_value() == false)
		{
			rule_v.clear();
			return header_result_v;
		}

		rule_v.clear();
		stream_reader reader_v(in_v);

		rule_serialize::result var_result_v = rule_serialize_var::read(reader_v, rule_v);

		if (var_result_v.has_value() == false)
		{
			rule_v.clear();
			return var_result_v;
		}

		rule_serialize::result cmd_result_v = rule_serialize_cmd::read(reader_v, rule_v);

		if (cmd_result_v.has_value() == false)
		{
			rule_v.clear();
			return cmd_result_v;
		}

		rule_serialize::result stack_result_v = rule_serialize_stack::read(reader_v, rule_v, ip_out);

		if (stack_result_v.has_value() == false)
		{
			rule_v.clear();
			return stack_result_v;
		}

		if (reader_v.get_count() != header_v.get_payload_size() || reader_v.get_crc() != header_v.get_crc())
		{
			rule_v.clear();
			return std::unexpected(kernel_error::serialize__crc_mismatch);
		}

		return {};
	}
}
