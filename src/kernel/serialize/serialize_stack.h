#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include "serialize_data_def.h"
#include "serialize_io.h"
#include "serialize_stack_data_def.h"

namespace rule_serialize_stack
{
	template<typename rule_t>
	rule_serialize::result write(rule_serialize::stream_writer& writer_v, rule_t& rule_v, const typename rule_t::run_ip& ip_v)
	{
		const auto& stack_v = rule_v.get_stack();
		writer_v.put_u64(stack_v.size());

		for (const auto& ip_entry_v : stack_v)
		{
			writer_v.put_u64(static_cast<std::uint64_t>(ip_entry_v.get_cmd_set_index()));
			writer_v.put_u64(static_cast<std::uint64_t>(ip_entry_v.get_index()));
		}

		writer_v.put_u64(static_cast<std::uint64_t>(ip_v.get_cmd_set_index()));
		writer_v.put_u64(static_cast<std::uint64_t>(ip_v.get_index()));

		return {};
	}

	template<typename rule_t>
	rule_serialize::result read(rule_serialize::stream_reader& reader_v, rule_t& rule_v, typename rule_t::run_ip& ip_out)
	{
		std::uint64_t stack_count_v = 0;

		if (reader_v.get_u64(stack_count_v) == false || stack_count_v > rule_serialize::SERIALIZE_MAX_ELEMENTS)
			return std::unexpected(kernel_error::serialize__truncated);

		auto& stack_v = rule_v.get_stack();
		stack_v.reserve(static_cast<std::size_t>(stack_count_v));

		for (std::uint64_t stack_i = 0; stack_i < stack_count_v; stack_i++)
		{
			std::uint64_t ownership_v = 0;
			std::uint64_t index_v = 0;

			if (!reader_v.get_u64(ownership_v) || !reader_v.get_u64(index_v))
				return std::unexpected(kernel_error::serialize__truncated);

			stack_v.emplace_back(
				static_cast<typename rule_t::run_ip::command_set_index>(ownership_v),
				static_cast<typename rule_t::run_ip::command_index>(index_v));
		}

		std::uint64_t ip_ownership_v = 0;
		std::uint64_t ip_index_v = 0;

		if (!reader_v.get_u64(ip_ownership_v) || !reader_v.get_u64(ip_index_v))
			return std::unexpected(kernel_error::serialize__truncated);

		ip_out = typename rule_t::run_ip(
			static_cast<typename rule_t::run_ip::command_set_index>(ip_ownership_v),
			static_cast<typename rule_t::run_ip::command_index>(ip_index_v));

		return {};
	}
}
