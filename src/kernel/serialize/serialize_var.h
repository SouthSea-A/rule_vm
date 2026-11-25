#pragma once
#include "../kernel_namespace.h"
#include <cstdint>
#include <cstring>
#include <span>
#include <limits>
#include <array>
#include "serialize_data_def.h"
#include "serialize_io.h"
#include "serialize_var_data_def.h"

namespace rule_serialize_var
{
	template<typename rule_t>
	rule_serialize::result write(rule_serialize::stream_writer& writer_v, rule_t& rule_v)
	{
		using raw_t = typename rule_t::serialize_var_base_t;
		auto& var_set_v = rule_v.get_var_set();
		writer_v.put_u64(var_set_v.size());

		for (auto it = var_set_v.begin(); it != var_set_v.end(); ++it)
		{
			writer_v.put_u64(static_cast<std::uint64_t>((*it).get_index()));
			writer_v.put_u64(static_cast<std::uint64_t>((*it).get_ref()));

			auto& var_mgr_v = (*it).get_content();
			writer_v.put_u64(var_mgr_v.size());

			for (auto element_it = var_mgr_v.begin(); element_it != var_mgr_v.end(); ++element_it)
			{
				const auto& element_v = *element_it;
				const auto& content_v = element_v.get_content();

				writer_v.put_u64(static_cast<std::uint64_t>(element_v.get_index()));
				writer_v.put_u64(static_cast<std::uint64_t>(element_v.get_type()));
				writer_v.put_u64(static_cast<std::uint64_t>(element_v.get_ref()));

				const auto blocks_v = content_v.get_blocks();
				writer_v.put_u64(blocks_v.size());

				for (const auto& block_v : blocks_v)
				{
					writer_v.put_u64(static_cast<std::uint64_t>(block_v.get_start()));
					writer_v.put_u64(static_cast<std::uint64_t>(block_v.get_length()));

					const auto& data_v = block_v.get_data();
					writer_v.put_bytes(std::span<const rule_serialize::byte_t>(
						reinterpret_cast<const rule_serialize::byte_t*>(data_v.data()),
						data_v.size() * sizeof(raw_t)));
				}
			}
		}

		return {};
	}

	template<typename rule_t>
	rule_serialize::result read(rule_serialize::stream_reader& reader_v, rule_t& rule_v)
	{
		using var_mgr_index_t = typename rule_t::serialize_var_mgr_index_t;
		using var_index_t = typename rule_t::serialize_var_index_t;
		using var_type_t = typename rule_t::serialize_var_type_t;
		using var_ref_t = typename rule_t::serialize_var_ref_t;
		using var_pos_t = typename rule_t::serialize_var_pos_t;
		using raw_t = typename rule_t::serialize_var_base_t;
		std::uint64_t var_manager_count_v = 0;

		if (reader_v.get_u64(var_manager_count_v) == false || var_manager_count_v > rule_serialize::SERIALIZE_MAX_MANAGERS)
			return std::unexpected(kernel_error::serialize__limit_exceeded);

		auto& var_set_v = rule_v.get_var_set();

		for (std::uint64_t i = 0; i < var_manager_count_v; i++)
		{
			std::uint64_t ownership_v = 0;
			std::uint64_t manager_ref_v = 0;
			std::uint64_t element_count_v = 0;

			if (!reader_v.get_u64(ownership_v) || !reader_v.get_u64(manager_ref_v) || !reader_v.get_u64(element_count_v))
				return std::unexpected(kernel_error::serialize__truncated);

			if (element_count_v > rule_serialize::SERIALIZE_MAX_ELEMENTS || manager_ref_v == 0)
				return std::unexpected(kernel_error::serialize__limit_exceeded);

			if (!var_set_v.add(static_cast<var_mgr_index_t>(ownership_v), false).has_value())
				return std::unexpected(kernel_error::serialize__manager_restore_failed);

			for (std::uint64_t ref_i = 1; ref_i < manager_ref_v; ref_i++)
				if (!var_set_v.add(static_cast<var_mgr_index_t>(ownership_v), true).has_value())
					return std::unexpected(kernel_error::serialize__manager_restore_failed);

			auto manager_res_v = var_set_v.get(static_cast<var_mgr_index_t>(ownership_v));

			if (manager_res_v.has_value() == false)
				return std::unexpected(kernel_error::serialize__manager_restore_failed);

			auto& var_mgr_v = manager_res_v.value().get().get_content();

			for (std::uint64_t element_i = 0; element_i < element_count_v; element_i++)
			{
				rule_serialize_var_data_def::var_record record_v;
				std::uint64_t index_v = 0;
				std::uint64_t type_v = 0;
				std::uint64_t ref_v = 0;
				std::uint64_t block_count_v = 0;

				if (!reader_v.get_u64(index_v) || !reader_v.get_u64(type_v) || !reader_v.get_u64(ref_v) || !reader_v.get_u64(block_count_v))
					return std::unexpected(kernel_error::serialize__truncated);

				record_v.set_index(index_v);
				record_v.set_type(type_v);
				record_v.set_ref(ref_v);
				record_v.set_block_count(block_count_v);

				if (record_v.get_block_count() > rule_serialize::SERIALIZE_MAX_BLOCKS)
					return std::unexpected(kernel_error::serialize__limit_exceeded);

				if (var_mgr_v.restore_var(static_cast<var_index_t>(record_v.get_index()), static_cast<var_type_t>(record_v.get_type()), static_cast<var_ref_t>(record_v.get_ref())) == false)
					return std::unexpected(kernel_error::serialize__var_restore_failed);

				auto element_res_v = var_mgr_v.get(static_cast<var_index_t>(record_v.get_index()));

				if (element_res_v.has_value() == false)
					return std::unexpected(kernel_error::serialize__var_restore_failed);

				auto& content_v = element_res_v.value().get().get_content();

				for (std::uint64_t block_i = 0; block_i < record_v.get_block_count(); block_i++)
				{
					std::uint64_t pos_v = 0;
					std::uint64_t len_v = 0;

					if (!reader_v.get_u64(pos_v) || !reader_v.get_u64(len_v))
						return std::unexpected(kernel_error::serialize__truncated);

					rule_serialize_var_data_def::var_block_record block_record_v(pos_v, len_v);

					if (block_record_v.get_length() > std::numeric_limits<std::size_t>::max())
						return std::unexpected(kernel_error::serialize__limit_exceeded);

					if (block_record_v.get_length() == 0)
						continue;

					auto block_span_v = content_v.restore_block_create(
						static_cast<var_pos_t>(block_record_v.get_pos()),
						static_cast<std::size_t>(block_record_v.get_length()));

					if (block_span_v.empty() == true)
						return std::unexpected(kernel_error::serialize__var_restore_failed);

					std::span<rule_serialize::byte_t> block_bytes_v(
						reinterpret_cast<rule_serialize::byte_t*>(block_span_v.data()),
						block_span_v.size() * sizeof(raw_t));

					if (reader_v.get_bytes(block_bytes_v) == false)
						return std::unexpected(kernel_error::serialize__truncated);
				}
			}
		}

		return {};
	}
}
