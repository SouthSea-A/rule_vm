#pragma once
#include "../kernel_namespace.h"
#include <array>
#include <cstring>
#include <span>
#include <variant>
#include <limits>
#include "serialize_data_def.h"
#include "serialize_io.h"
#include "serialize_cmd_data_def.h"

namespace rule_serialize_cmd
{
	namespace detail
	{
		template<typename rule_t>
		inline bool write_target(rule_serialize::stream_writer& writer_v, rule_t& rule_v, const auto& tgt_v, std::uint8_t tag_v, std::size_t raw_size_v)
		{
			using raw_t = typename rule_t::serialize_var_base_t;
			using info_t = typename rule_t::serialize_info_t;
			std::array<raw_t, rule_t::serialize_raw_bytes_size_op> raw_v{};
			std::span<raw_t> raw_span_v(raw_v.data(), raw_size_v);
			info_t int_info_v{};

			if (rule_v.transform_tgt_to_raw_bytes_run(int_info_v, tgt_v, raw_span_v) == false)
				return false;

			writer_v.put_u8(tag_v);
			writer_v.put_bytes(std::span<const rule_serialize::byte_t>(
				reinterpret_cast<const rule_serialize::byte_t*>(raw_span_v.data()),
				raw_span_v.size()));
			return true;
		}

		template<typename rule_t>
		inline bool write_op(rule_serialize::stream_writer& writer_v, rule_t& rule_v, const typename rule_t::serialize_cmd_op_single_t& op_v)
		{
			using imm_t = typename rule_t::serialize_imm_target_t;
			using var_target_t = typename rule_t::serialize_var_target_t;
			using cmd_target_t = typename rule_t::serialize_cmd_target_t;
			if (const auto* imm_v = std::get_if<imm_t>(&op_v))
				return write_target(writer_v, rule_v, *imm_v, 0, rule_t::serialize_raw_bytes_size_imm);

			if (const auto* var_v = std::get_if<var_target_t>(&op_v))
				return write_target(writer_v, rule_v, *var_v, 1, rule_t::serialize_raw_bytes_size_var);

			if (const auto* cmd_v = std::get_if<cmd_target_t>(&op_v))
				return write_target(writer_v, rule_v, *cmd_v, 2, rule_t::serialize_raw_bytes_size_cmd);

			return false;
		}

		template<typename rule_t>
		bool read_target(rule_serialize::stream_reader& reader_v, rule_t& rule_v, typename rule_t::serialize_cmd_op_single_t& op_save)
		{
			using raw_t = typename rule_t::serialize_var_base_t;
			using info_t = typename rule_t::serialize_info_t;
			using imm_t = typename rule_t::serialize_imm_target_t;
			using var_target_t = typename rule_t::serialize_var_target_t;
			using cmd_target_t = typename rule_t::serialize_cmd_target_t;
			std::uint8_t tag_v = 0;

			if (reader_v.get_u8(tag_v) == false)
				return false;

			std::size_t raw_size_v = 0;

			switch (tag_v)
			{
			case 0:
			{
				raw_size_v = rule_t::serialize_raw_bytes_size_imm;
				break;
			}
			case 1:
			{
				raw_size_v = rule_t::serialize_raw_bytes_size_var;
				break;
			}
			case 2:
			{
				raw_size_v = rule_t::serialize_raw_bytes_size_cmd;
				break;
			}
			default:
			{
				return false;
			}
			}

			std::array<rule_serialize::byte_t, rule_t::serialize_raw_bytes_size_op> raw_bytes_v{};

			if (reader_v.get_bytes(std::span<rule_serialize::byte_t>(raw_bytes_v.data(), raw_size_v)) == false)
				return false;

			std::array<raw_t, rule_t::serialize_raw_bytes_size_op> raw_buf_v{};
			std::memcpy(raw_buf_v.data(), raw_bytes_v.data(), raw_size_v);
			std::span<raw_t> raw_span_v(raw_buf_v.data(), raw_size_v);
			info_t int_info_v{};

			if (tag_v == 0)
			{
				imm_t tgt_v;

				if (rule_v.transform_raw_bytes_to_run(int_info_v, raw_span_v, tgt_v) == false)
					return false;

				op_save.template emplace<imm_t>(tgt_v);
				return true;
			}

			if (tag_v == 1)
			{
				var_target_t tgt_v;

				if (rule_v.transform_raw_bytes_to_run(int_info_v, raw_span_v, tgt_v) == false)
					return false;

				op_save.template emplace<var_target_t>(tgt_v);
				return true;
			}

			cmd_target_t tgt_v;

			if (rule_v.transform_raw_bytes_to_run(int_info_v, raw_span_v, tgt_v) == false)
				return false;

			op_save.template emplace<cmd_target_t>(tgt_v);
			return true;
		}
	}

	template<typename rule_t>
	rule_serialize::result write(rule_serialize::stream_writer& writer_v, rule_t& rule_v)
	{
		auto& cmd_set_v = rule_v.get_cmd_set();
		writer_v.put_u64(cmd_set_v.size());

		for (auto it = cmd_set_v.begin(); it != cmd_set_v.end(); ++it)
		{
			writer_v.put_u64(static_cast<std::uint64_t>((*it).get_index()));
			writer_v.put_u64(static_cast<std::uint64_t>((*it).get_ref()));

			auto& cmd_mgr_v = (*it).get_content();
			writer_v.put_u64(cmd_mgr_v.size());

			for (auto element_it = cmd_mgr_v.begin(); element_it != cmd_mgr_v.end(); ++element_it)
			{
				const auto element_v = *element_it;
				const auto content_v = element_v.get_content_span();
				const auto ref_v = cmd_mgr_v.get_ref(element_v.get_index());

				if (ref_v.has_value() == false)
					return std::unexpected(kernel_error::serialize__cmd_restore_failed);

				writer_v.put_u64(static_cast<std::uint64_t>(element_v.get_index()));
				writer_v.put_u64(static_cast<std::uint64_t>(element_v.get_type().get()));
				writer_v.put_u64(static_cast<std::uint64_t>(ref_v.value()));
				writer_v.put_u64(static_cast<std::uint64_t>(content_v.size()));

				for (const auto& op_v : content_v)
				{
					if (detail::write_op<rule_t>(writer_v, rule_v, op_v) == false)
						return std::unexpected(kernel_error::serialize__invalid_section);
				}
			}
		}

		return {};
	}

	template<typename rule_t>
	rule_serialize::result read(rule_serialize::stream_reader& reader_v, rule_t& rule_v)
	{
		using cmd_def_key_t = typename rule_t::cmd_def_set_act::key_type;
		using cmd_mgr_index_t = typename rule_t::serialize_cmd_mgr_index_t;
		using cmd_index_t = typename rule_t::serialize_cmd_index_t;
		using cmd_type_t = typename rule_t::serialize_cmd_type_t;
		using cmd_op_width_t = typename rule_t::serialize_cmd_op_width_t;

		std::uint64_t cmd_manager_count_v = 0;

		if (reader_v.get_u64(cmd_manager_count_v) == false || cmd_manager_count_v > rule_serialize::SERIALIZE_MAX_MANAGERS)
			return std::unexpected(kernel_error::serialize__limit_exceeded);

		auto& cmd_set_v = rule_v.get_cmd_set();

		for (std::uint64_t i = 0; i < cmd_manager_count_v; i++)
		{
			std::uint64_t ownership_v = 0;
			std::uint64_t manager_ref_v = 0;
			std::uint64_t element_count_v = 0;

			if (!reader_v.get_u64(ownership_v) || !reader_v.get_u64(manager_ref_v) || !reader_v.get_u64(element_count_v))
				return std::unexpected(kernel_error::serialize__truncated);

			if (element_count_v > rule_serialize::SERIALIZE_MAX_ELEMENTS || manager_ref_v == 0)
				return std::unexpected(kernel_error::serialize__limit_exceeded);

			if (!cmd_set_v.add(static_cast<cmd_mgr_index_t>(ownership_v), false).has_value())
				return std::unexpected(kernel_error::serialize__manager_restore_failed);

			for (std::uint64_t ref_i = 1; ref_i < manager_ref_v; ref_i++)
				if (!cmd_set_v.add(static_cast<cmd_mgr_index_t>(ownership_v), true).has_value())
					return std::unexpected(kernel_error::serialize__manager_restore_failed);

			auto manager_res_v = cmd_set_v.get(static_cast<cmd_mgr_index_t>(ownership_v));

			if (manager_res_v.has_value() == false)
				return std::unexpected(kernel_error::serialize__manager_restore_failed);

			auto& cmd_mgr_v = manager_res_v.value().get().get_content();

			for (std::uint64_t element_i = 0; element_i < element_count_v; element_i++)
			{
				std::uint64_t index_v = 0;
				std::uint64_t type_v = 0;
				std::uint64_t ref_v = 0;
				std::uint64_t op_count_v = 0;

				if (!reader_v.get_u64(index_v) || !reader_v.get_u64(type_v) || !reader_v.get_u64(ref_v) || !reader_v.get_u64(op_count_v))
					return std::unexpected(kernel_error::serialize__truncated);

				rule_serialize_cmd_data_def::cmd_record record_v;
				record_v.set_index(index_v);
				record_v.set_type(type_v);
				record_v.set_ref(ref_v);
				record_v.set_op_count(op_count_v);

				if (record_v.get_op_count() > rule_serialize::SERIALIZE_MAX_OPS || record_v.get_ref() == 0)
					return std::unexpected(kernel_error::serialize__limit_exceeded);

				if (rule_v.get_cmd_def_set().find(static_cast<cmd_def_key_t>(record_v.get_type())) == rule_v.get_cmd_def_set().end())
					return std::unexpected(kernel_error::serialize__cmd_type_missing);

				if (!cmd_mgr_v.add_set(
					static_cast<cmd_index_t>(record_v.get_index()),
					static_cast<cmd_type_t>(record_v.get_type()),
					0,
					static_cast<cmd_op_width_t>(record_v.get_op_count()),
					false).has_value())
					return std::unexpected(kernel_error::serialize__cmd_restore_failed);

				for (std::uint64_t ref_i = 1; ref_i < record_v.get_ref(); ref_i++)
					if (!cmd_mgr_v.add_set(
						static_cast<cmd_index_t>(record_v.get_index()),
						static_cast<cmd_type_t>(record_v.get_type()),
						0,
						static_cast<cmd_op_width_t>(record_v.get_op_count()),
						true).has_value())
						return std::unexpected(kernel_error::serialize__cmd_restore_failed);

				if (record_v.get_op_count() > 0)
				{
					auto content_res_v = cmd_mgr_v.get(static_cast<cmd_index_t>(record_v.get_index()));

					if (content_res_v.has_value() == false)
						return std::unexpected(kernel_error::serialize__cmd_restore_failed);

					auto content_span_v = content_res_v.value().get_content_span();

					for (std::uint64_t op_i = 0; op_i < record_v.get_op_count(); op_i++)
						if (detail::read_target(reader_v, rule_v, content_span_v[static_cast<std::size_t>(op_i)]) == false)
							return std::unexpected(kernel_error::serialize__invalid_section);
				}
			}
		}

		return {};
	}
}
